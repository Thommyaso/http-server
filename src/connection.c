#include <asm-generic/errno.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>
#include <sys/sendfile.h>

#include "./listener.h"
#include "./connection.h"
#include "parser.h"
#include "response.h"
#include "utils/buff.h"

// TODO: currently, the listening socket is in a blocking state. 
// This creates a possible scenario where in busy server, client-server handshake completes (3 way handshake in listen()),
// but by the time my server gets to accept(), client could break connection, leaving my server stuck infinitely,
// waiting for a connection that doesn't exist anymore. (small likelyhood, living this for later)

void server_run(fd_t lis_fd)
{
    struct pollfd poll_fds[POLL_FD_LIMIT];
    buff_t poll_requests[POLL_FD_LIMIT] ={0};
    res_buff_t poll_responses[POLL_FD_LIMIT] ={0};

    int nfdsp1 = LIS_FD_IDX + 1;
    poll_fds[LIS_FD_IDX].fd = lis_fd;
    poll_fds[LIS_FD_IDX].events = POLLRDNORM;

    for(int idx = 1; idx < POLL_FD_LIMIT; idx++){
         // -1 indicates available space for accepted fd to be placed
        poll_fds[idx].fd = -1;
        poll_requests[idx].data = NULL;
        poll_responses[idx].base.data = NULL;
        poll_responses[idx].file_fd = -1;
    }

    for(;;){
        int nready = poll(poll_fds, nfdsp1, INF_TIM);

        //TODO: gonna have to read those errors and see what to do about them
        if(nready <= 0) continue;

        // HANDLING NEW CONNECTIONS
        if (poll_fds[LIS_FD_IDX].revents & POLLRDNORM) {
            handle_new_connection(lis_fd, poll_fds, poll_requests, poll_responses, &nfdsp1);

            if(--nready == 0) continue; 
        }

        // HANDLING CONNECTED CLIENTS
        for(int idx = 1; idx < POLL_FD_LIMIT && nready > 0; idx++){
            if(poll_fds[idx].fd < 0) continue; // fd not in use (-1)

            buff_t *preq_buff = &poll_requests[idx];
            res_buff_t *pres_buff = &poll_responses[idx];
            struct pollfd *ppoll_fd = &poll_fds[idx];

            // CLIENT WITH AN ERROR
            if (ppoll_fd->revents & POLLERR) {
                // TODO: some error, figure out what to do here, for now just close connection
                nready--;
                kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
                continue;
            }

            // CLIENT WITH REQUEST
            if (ppoll_fd->revents & POLLRDNORM) {
                nready--;
                handle_client_request(ppoll_fd, preq_buff, pres_buff);
            }

            // CLIENT AWAITING RESPONSE
            if (ppoll_fd->revents & POLLOUT) {
                nready--;
                handle_client_response(ppoll_fd, preq_buff, pres_buff);
            }
        }
    }
}

void handle_new_connection(
    fd_t lis_fd,
    struct pollfd poll_fds[],
    buff_t poll_requests[],
    res_buff_t poll_responses[],
    int *nfdsp1
){
    int fail;
    fd_t conn_fd = accept(lis_fd, NULL, NULL);

    if(conn_fd < 0){
        // TODO: handle errors in here?
        return; // connection socket failed;
    }

    int flags = fcntl(conn_fd, F_GETFL, 0);
    fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK);

    int should_close = 1;
    for (int idx = 1; idx < POLL_FD_LIMIT; idx++) {
        if(poll_fds[idx].fd != -1) continue; // position taken;

        if(poll_requests[idx].data != NULL || poll_responses[idx].base.data != NULL){
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

        fail = init_res_buff(&poll_responses[idx], 0);
        if(fail){
            // error allocating response buff memory
            // it aint happening for this client, sorry
            kill_buff(&poll_requests[idx]);
            break;
        }

        poll_fds[idx].fd = conn_fd;
        poll_fds[idx].events = POLLRDNORM;

        if(*nfdsp1 <= idx){
            *nfdsp1 = idx + 1;
        }

        should_close = 0;
        break;
    }

    // max number of file descriptors has been reached or error initiating buffer for req/res,
    // get rid of this connection
    if(should_close){
        close(conn_fd);
    }
}

void handle_client_request(
    struct pollfd *ppoll_fd,
    buff_t *preq_buff,
    res_buff_t *pres_buff
){
    int fail;

    if(preq_buff->data == NULL || pres_buff->base.data == NULL){
        // for some reason the req/res buff is not initiated for this client
        // this shouldn't happen but if it does, close connection and bye
        kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
        return;
    }

    int available_req_space = preq_buff->total_size - preq_buff->size_used;
    if(available_req_space <= 0) {
        fail = buff_increase(preq_buff, BUFF_SIZE);
        if(fail){
            kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            return;
        }

        available_req_space = preq_buff->total_size - preq_buff->size_used;
    }

    char *pwrite_start = preq_buff->data + preq_buff->size_used;
    int recv_size = recv(ppoll_fd->fd, pwrite_start, available_req_space, 0);

    if(recv_size < 0){
        if(errno == ECONNRESET){
            // client sent RST, close connection
            kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            return;
        }else{
            // TODO: this needs handling, for now yolo
            kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            return;
        }
    } else if(recv_size == 0){
        // client closed connection (timeout etc.)
        kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
    } else {
        preq_buff->size_used += recv_size;
        preq_buff->data[preq_buff->size_used] = '\0';

        headers_map_t headers_map = {0};
        int parse_result = parse_req(preq_buff, &headers_map);

        if(parse_result == REQ_NOT_COMPLETE){
            return;
        } else if(parse_result == REQ_FAIL){
            // request malformed ignore and close the connection
            kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            return;
        }

        // got full request, time to create response,
        // I'm not listening for any more requests on that socket until i respond to the current one
        ppoll_fd->events = POLLOUT;
        fail = build_response(&headers_map, pres_buff);

        if(fail){
            // response build failed, close connection
            kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            return;
        }
    }
}

void handle_client_response(
    struct pollfd *ppoll_fd,
    buff_t *preq_buff,
    res_buff_t *pres_buff
){
    int sent_size = 0;
    int size_to_send = 0;
    char *psend_start;

    //sending headers
    if(pres_buff->base.size_used != pres_buff->base.size_processed){
        psend_start = pres_buff->base.data + pres_buff->base.size_processed;
        size_to_send = pres_buff->base.size_used - pres_buff->base.size_processed;   
        sent_size = send(ppoll_fd->fd, psend_start, size_to_send, 0);
    }

    if(sent_size < 0){
        if(errno != EWOULDBLOCK){
            //TODO: some error, create handling
        }
        return;
    }  

    pres_buff->base.size_processed += sent_size;

    // headers didnt fit in one send buffer, gotta wait till there is room and send rest
    if(sent_size < size_to_send){
        return;
    }

    // complete headers have been passed,
    //there is a file that needs sending together with headers
    if(pres_buff->filesize > 0){
        int sent_filesize =  sendfile(ppoll_fd->fd, pres_buff->file_fd, &pres_buff->size_uploaded, pres_buff->filesize - pres_buff->size_uploaded);
        printf("file size: %d\n", sent_filesize);
        if(sent_filesize < 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN){
                // TODO: handle  this better later
                // some error close connection
                kill_client_connection(ppoll_fd->fd, ppoll_fd, preq_buff, pres_buff);
            }
            return;
        }

        if(pres_buff->size_uploaded < pres_buff->filesize){
            //whole file didn't fit in sendfile buff got to wait till space frees to send the rest
            return;
        }
    }

    // whole headers and (potential)file have been sent, req and res buffers can be reset, and poll fd can be set to listening
    preq_buff->size_used = 0;
    preq_buff->data[0] = '\0';
    pres_buff->base.size_used = 0;
    pres_buff->base.size_processed = 0;
    pres_buff->base.data[0] = '\0';
    pres_buff->filesize = 0;
    if(pres_buff->file_fd > 0) close(pres_buff->file_fd);
    pres_buff->file_fd = -1;
    pres_buff->size_uploaded = 0;

    ppoll_fd->events = POLLRDNORM;
}

void kill_client_connection(
    fd_t fd,
    struct pollfd *poll_fd,
    buff_t *preq_buff,
    res_buff_t *pres_buff
){
    if(fd >= 0) close(fd); 
    poll_fd->fd = -1;
    kill_buff(preq_buff);

    kill_res_buff(pres_buff);
    if(pres_buff->file_fd > 0) close(pres_buff->file_fd);
    pres_buff->file_fd = -1;
}
