# 🌐 Webserv — A Lightweight Non-Blocking HTTP Server in C++

## 📖 Overview

**Webserv** is a high-performance, fully non-blocking HTTP server implemented in modern C++ (≥ C++11).  
It follows the [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) HTTP/1.1 specification and uses **epoll** for scalable I/O multiplexing.

The goal of this project is to understand how a real web server like **NGINX** or **Apache** handles connections, parses requests, serves files, and interacts with **CGI** processes — all without blocking the main event loop.

---

## ✨ Features

- ✅ HTTP/1.1 compliant (GET, POST, DELETE)
- ✅ Non-blocking I/O using **epoll**
- ✅ Multiple ports and server blocks via configuration file
- ✅ Static file serving
- ✅ File upload (POST)
- ✅ Custom error pages
- ✅ Directory listing (optional)
- ✅ CGI support (e.g., PHP, Python)
- ✅ Logging system (info, error, access)
- ✅ Graceful client disconnection handling
- ✅ Configurable max body size
- ✅ Fast and resilient under stress

---

## 🧠 Architecture Overview

### 🏗 High-Level Components

| Webserv                                                 |
| ------------------------------------------------------- |
| epoll (event loop)                                      |
| - monitors listening sockets (accept)                   |
| - monitors client sockets (read/write)                  |
| - monitors CGI pipes (read/write)                       |
| ------------------------------------------------------- |
| Core Modules:                                           |
| - ConfigParser: parse config file                       |
| - Server: manage listeners and clients                  |
| - Connection: track state of each client                |
| - RequestParser: parse HTTP requests                    |
| - ResponseBuilder: build HTTP responses                 |
| - **CgiHandler: execute and monitor CGI**               |
| - Logger: structured logging                            |
| ------------------------------------------------------- |



---

### 🧩 4. Transition Diagram

        ┌──────────────────┐
        │  WAIT_REQUEST    │
        └──────┬───────────┘
               │ EPOLLIN (data)
               ▼
        ┌──────────────────┐
        │ READING_REQUEST  │
        └──────┬───────────┘
               │ request complete
               ▼
        ┌──────────────────┐
        │ PROCESSING       │
        └──────┬───────────┘
               │ static file  │ CGI request
               │              ▼
               │         ┌──────────────┐
               │         │ CGI_RUNNING  │
               │         └──────┬───────┘
               │ CGI output     │
               ▼                ▼
        ┌──────────────────┐  ┌─────────────────┐
        │ WRITING_RESPONSE │  │ CGI_READING     │
        └──────┬───────────┘  └──────┬──────────┘
               │ EPOLLOUT            │ EOF
               ▼                     ▼
        ┌──────────────────┐
        │ COMPLETE         │
        └──────┬───────────┘
               │
         keep-alive?
           │    │
           │    ▼
           │  WAIT_REQUEST
           │
           ▼
         CLOSING



---
## 🧩 Internal Data Flow

WAIT_REQUEST → READING_REQUEST → READY → WRITING_RESPONSE → COMPLETE

# 🌐 Webserv Project Overview

A lightweight HTTP server in C++ supporting **static files**, **CGI scripts**, **keep-alive**, and **chunked transfer encoding**.  

---

## ⚡ Typical Request Lifecycle

<details>
<summary>Click to expand</summary>

```text
Client
  │
  ▼
[epoll_wait()]
  │
  ├──> Accept new client (EPOLLIN on listening socket)
  │
  ├──> Read request (EPOLLIN)
  │       ↓
  │   RequestParser
  │       ↓
  │   Route lookup → static file | CGI
  │
  ├──> If static:
  │       read() file → ResponseBuilder → send()
  │
  ├──> If CGI:
  │       CgiHandler forks → pipe()
  │       collect stdout → ResponseBuilder
  │
  └──> Send response (EPOLLOUT)
          ↓
        close() when done

</details>
---

### Request Lifecycle (keep connection alive(HTTP1.1, no close) & chunked transfer)
<details> <summary>Click to expand</summary>
[epoll_wait()]
  │
  ├──> Read request
  │       - If new request
  │       - Parse headers
  │       - Detect keep-alive / chunked
  │
  ├──> Write response
  │       - If Content-Length known → normal write
  │       - Else → chunked encoding
  │
  ├──> If keep-alive → reset state
  │       else → close socket

</details>

---

## ⚙️ Directory Structure
<details> <summary>Click to expand</summary>

webserv/
├── include/
│   ├── core/
│   │   ├── Config.hpp
│   │   ├── Logger.hpp
│   │   └── Utils.hpp
│   ├── server/
│   │   ├── Server.hpp
│   │   ├── Connection.hpp
│   │   ├── RequestParser.hpp
│   │   ├── ResponseBuilder.hpp
│   │   └── CgiHandler.hpp
│   └── http/
│       ├── Request.hpp
│       └── Response.hpp
│
├── src/
│   ├── core/
│   │   ├── Config.cpp
│   │   ├── Logger.cpp
│   │   └── Utils.cpp
│   ├── server/
│   │   ├── Server.cpp
│   │   ├── Connection.cpp
│   │   ├── RequestParser.cpp
│   │   ├── ResponseBuilder.cpp
│   │   └── CgiHandler.cpp
│   └── main.cpp
│
├── config/
│   └── default.conf
│
├── www/
│   ├── index.html
│   ├── upload/
│   └── cgi-bin/
│       └── test.py
│
├── logs/
│   ├── access.log
│   └── error.log
│
├── Makefile
└── README.md
</details>
---

## 🧱 CGI Lifecycle (Non-Blocking)
<details> <summary>Click to expand</summary>
[Client]
  │
  ▼
Webserv (EPOLLIN)
  │
  ├── Parse request and detect CGI extension
  │
  ├── Setup environment variables:
  │     - CONTENT_LENGTH, CONTENT_TYPE
  │     - REQUEST_METHOD, QUERY_STRING
  │     - SCRIPT_FILENAME, SERVER_PROTOCOL, etc.
  │
  ├── Create two pipes:
  │     - pipe_in:  send request body → CGI stdin
  │     - pipe_out: receive CGI stdout → response
  │
  ├── fork() + execve(script_path, envp)
  │
  ├── parent registers pipe_out[0] to epoll
  │     (non-blocking read from CGI output)
  │
  ├── child runs interpreter (e.g. /usr/bin/python3)
  │     and writes output to pipe_out[1]
  │
  ├── parent reads CGI output as stream:
  │     "Content-Type: text/html\r\n\r\n<html>..."
  │
  └── ResponseBuilder wraps it into valid HTTP response

Everything stays non-blocking — no waitpid() or read() blocking calls.
The CGI’s pipe file descriptors are monitored by epoll just like sockets.
</details> ```
---
