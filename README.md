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

## Notes

At the moment, the path to the served domain is hardcoded.  
The root directory is defined via `ROOT_DIR` in `src/router.h`.

This will be replaced with configuration-based setup in a future update.


## Planned Work

- Configuration file parsing  
- Support for multiple domains / virtual hosts  
- Switch to epoll() to follow nginx setup
- Improved request parsing and error handling  
- Additional HTTP features  

Work in progress..
