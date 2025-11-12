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

<details><summary>Click to expand</summary>

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

</details>```
---

## 🔄 Request Lifecycle (Keep-Alive & Chunked Transfer)
<details>
<summary>Click to expand</summary>
```text
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
