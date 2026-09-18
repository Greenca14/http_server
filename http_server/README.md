# C++ HTTP Server

A multithreaded HTTP/1.1 server written in modern C++17. Serves static files from a `public/` directory.

## Features

- Multithreaded request handling via a custom thread pool
- HTTP/1.1 request parsing (method, path, version, headers)
- Static file serving with MIME type detection
- Path traversal protection via `std::filesystem`
- Graceful shutdown on Ctrl+C
- Timestamped request logging

## Requirements

- C++17 compiler (MSVC 2019+, GCC 9+, Clang 10+)
- CMake 3.16+
- Windows (uses Winsock2)

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Run

```bash
./http_server.exe [port] [root_directory]
```

Defaults: port `8080`, root `public`.

Example:

```bash
./http_server.exe 8080 public
```

Then open `http://localhost:8080/` in a browser.

## Usage examples

```bash
# Get index page
curl http://localhost:8080/

# Get CSS file
curl http://localhost:8080/style.css

# 404
curl http://localhost:8080/nope

# Method not allowed
curl -X POST http://localhost:8080/

# Path traversal protection
curl --path-as-is http://localhost:8080/../etc/passwd
```

## Project structure

```
http_server/
??? CMakeLists.txt
??? http_server.sln
??? http_server/
    ??? main.cpp              # entry point, request handling
    ??? socket.hpp/.cpp       # RAII socket wrapper
    ??? thread_pool.hpp/.cpp  # custom thread pool
    ??? wsa_init.hpp          # Winsock RAII
    ??? public/               # static files
```
