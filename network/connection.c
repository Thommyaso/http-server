#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include "./listener.h"
#include "./connection.h"
#include "../http/parser.h"
#include "../http/response.h"
#include "../utils/buff.h"

int server_run(sock_fd_t lis_sock_fd)
{
    int fail;
    int nfdsp1 = lis_sock_fd + 1;

    struct pollfd poll_fds[POLL_FD_LIMIT];

    poll_fds[LIS_FD_IDX].fd = lis_sock_fd;
    poll_fds[LIS_FD_IDX].events = POLLRDNORM;

    buff_t poll_requests[POLL_FD_LIMIT];
    buff_t poll_responses[POLL_FD_LIMIT];

    for(int idx = 1; idx < POLL_FD_LIMIT; idx++){
         // -1 indicates available space for accepted fd to be put
        poll_fds[idx].fd = -1;
    }

    for(;;){
        int nready = poll(poll_fds, nfdsp1, -1);

        if(nready <= 0){
            // gonna have to read those errors and see what to do about them
            continue;
        }

        // HANDLING NEW CONNECTIONS
        if (poll_fds[LIS_FD_IDX].revents & POLLRDNORM) {
            sock_fd_t conn_sock_fd = accept(lis_sock_fd, NULL, NULL);

            if(conn_sock_fd < 0){
                // TODO: handle errors in here?
                continue; // connection socket failed;
            }

            int flags = fcntl(conn_sock_fd, F_GETFL, 0);
            fcntl(conn_sock_fd, F_SETFL, flags | O_NONBLOCK);

            int should_close = 1;
            for (int idx = 1; idx < POLL_FD_LIMIT; idx++) {
                if(poll_fds[idx].fd == -1){
                    if(poll_requests[idx].data != NULL || poll_responses[idx].data != NULL){
                        // something went wrong, probably forgot to clean up req/res heap after finishing with the area
                        // we shouldn't arrive here unless idx indicates that this is a free position but the value is still a pointer to something
                        // just overwriting it could result in something else trying  to read it in the future (possibly not terminated correctly?)
                        // got to make sure that  after finishing connection both req and res pointers are reset to null in arrays (poll_requests and poll_responses)
                        continue;
                    }

                    fail = init_buff(&poll_requests[idx], 0);
                    if(fail){
                        // error allocating request buff memory
                        // it aint happening for this client, sorry
                        break;
                    }

                    fail = init_buff(&poll_responses[idx], 0);
                    if(fail){
                        // error allocating response buff memory
                        // it aint happening for this client, sorry
                        kill_buff(&poll_requests[idx]);
                        break;
                    }

                    poll_fds[idx].fd = conn_sock_fd;
                    poll_fds[idx].events = POLLRDNORM;

                    if(nfdsp1 <= idx){
                        nfdsp1 = idx + 1;
                    }

                    should_close = 0;
                    break;
                }
            }

            // max number of file descriptors has been reached or error initiating buffer for req/response,
            // get rid of this connection
            if(should_close){
                close(conn_sock_fd);
            }

            if(--nready == 0){
                continue; 
            }
        }

        // HANDLING CONNECTED CLIENTS
        for(int idx = 1; idx < POLL_FD_LIMIT && nready > 0; idx++){
            int fd = poll_fds[idx].fd;
            if(fd < 0){
                continue;
            }

            buff_t *preq_buff = &poll_requests[idx];
            buff_t *pres_buff = &poll_responses[idx];

            // CLIENT WITH AN ERROR
            if (poll_fds[idx].revents & POLLERR) {
                // TODO: figure out what to do here, for now just close connection
                kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                printf("error nr:\n", errno);
                nready--;
                continue;
            }

            // CLIENT WITH REQUEST
            if (poll_fds[idx].revents & POLLRDNORM) {
                nready--;

                if(preq_buff->data == NULL || pres_buff->data == NULL){
                    // for some reason the req/res buff is not initiated for this client
                    // this shouldn't happen but if it does, close connection and bye
                    close(fd);
                    poll_fds[idx].fd = -1;
                    break;
                }

                int available_req_space = preq_buff->size - preq_buff->used;
                if(available_req_space <= 0) {
                    fail = buff_increase(preq_buff, BUFF_SIZE);
                    if(fail){
                        kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                        continue;
                    }

                    available_req_space = preq_buff->size - preq_buff->used;
                }

                char *pwrite_start = preq_buff->data + preq_buff->used;
                int recv_size = recv(fd, pwrite_start, available_req_space, 0);

                if(recv_size < 0){
                    if(errno == ECONNRESET){
                        // client sent RST, close connection
                        kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                    }else{
                        // TODO: this needs handling,
                        // for now yolo
                        return EXIT_FAILURE;
                    }
                } else if(recv_size == 0){
                    printf("killing here 2\n");
                    // client closed connection
                    kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                } else {
                    preq_buff->used += recv_size;
                    preq_buff->data[preq_buff->used] = '\0';

                    headers_map_t headers_map = {0};
                    int parse_result = parse_req(preq_buff, &headers_map);

                    if(parse_result == REQ_NOT_COMPLETE){
                        continue;
                    } else if(parse_result == REQ_FAIL){
                        // request malformed ignore and close the connection
                        kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                        continue;
                    }

                    // got full request, time to create response,
                    // I'm not listening for any more requests on that socket until i respond to the current one
                    poll_fds[idx].events = POLLOUT;
                    fail = build_response(&headers_map, pres_buff);

                    if(fail){
                        // response build failed, close connection
                        kill_client_connection(fd, &poll_fds[idx], preq_buff, pres_buff);
                        continue;
                    }
                }
            }

            // CLIENT AWAITING RESPONSE
            if (poll_fds[idx].revents & POLLOUT) {
                char *psend_start= pres_buff->data + pres_buff->processed;
                int size_to_send = pres_buff->used - pres_buff->processed;   
                int sent_size = send(fd, psend_start, size_to_send, 0);

                if(sent_size < 0){
                    if(errno != EWOULDBLOCK){
                        // some error, create handling
                    }
                    continue;
                }

                pres_buff->processed += sent_size;
                if(pres_buff->processed == pres_buff->used){
                    // complete response has benn passed,
                    // can reset it, and listen for next request on this connection
                    preq_buff->used = 0;
                    pres_buff->used = 0;
                    poll_fds[idx].events = POLLRDNORM;
                } else{
                    // only part of response has been sent, wait till rest can be uploaded
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

void kill_client_connection(sock_fd_t fd, struct pollfd *poll_fd, buff_t *preq_buff, buff_t *pres_buff){
    close(fd);
    poll_fd->fd = -1;
    kill_buff(preq_buff);
    kill_buff(pres_buff);
}
