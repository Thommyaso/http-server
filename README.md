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

The focus so far has been on learning and correctness, not completeness.

## Planned Work

- Configuration file parsing  
- Support for multiple domains / virtual hosts  
- Improved request parsing and error handling  
- Additional HTTP features  

## Status

Work in progress.
