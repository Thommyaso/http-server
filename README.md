# HTTP Server (C)

This is a work-in-progress HTTP server written in C using POSIX sockets on Linux.  
It communicates directly over TCP and does not use any external HTTP or networking libraries.

The project is based on *UNIX Network Programming* (2nd Edition) by W. Richard Stevens and was written to better understand TCP, HTTP, and how servers like Nginx work internally.

## Current State

- POSIX sockets over TCP  
- Non-blocking I/O  
- `poll()` for handling multiple connections  
- Single-threaded, event-driven design  
- **GET requests only**  
- No external HTTP libraries  

## Configuration

Before building, edit `server_config.h` to set:

- **SERVER_PORT** – Port to listen on (default: `"8080"`)
- **ROOT_DIR** – Absolute path to your website files
- **DEFAULT_FILE** – File to serve for directory requests (default: `"/index.html"`)

Example:

```c
#define SERVER_PORT "8080"
#define ROOT_DIR "/var/www/mysite"
#define DEFAULT_FILE "/index.html"
```

## Building

The project uses a simple Makefile.

From the project root, run:

```bash
make
```

This will build the `server` binary.

To remove build artifacts:

```bash
make clean
```


## Planned Work

- Multi domain configuration file parsing, similar to nginx config files in /etc/nginx/sites_available
- Switch to epoll() to follow nginx setup
- Improved request parsing and error handling  
- Additional HTTP features  

Work in progress..
