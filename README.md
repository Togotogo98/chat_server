# Multi-Client TCP Chat Server

## Overview

This project is a TCP-based multi-client chat server built as a hands-on exercise to understand socket programming, concurrency, non-blocking I/O, and scalable network server design.

The project was completed in multiple phases, from a simple blocking TCP server to an event-driven server using **epoll** and **RAII**.

---

## Features

* TCP client-server architecture
* Multiple client support
* Message broadcasting to connected clients
* Non-blocking sockets
* Event-driven I/O using Linux `epoll`
* Object-oriented wrapper classes for Socket and Epoll
* Clean shutdown of owned resources through destructors

---

## Phase plan

### Phase 1

* Basic TCP server and client
* Single client communication

### Phase 2

* Thread-per-client implementation

### Phase 3

* Shared client list
* Message broadcasting

### Phase 4

* Non-blocking sockets

### Phase 5

* Linux `epoll`
* Event-driven architecture
* Removed thread-per-client model

### Phase 6

* Introduced RAII
* Created `Socket` wrapper class
* Created `Epoll` wrapper class
* Automatic resource cleanup
* Improved encapsulation

---

## Build

The compilation command depends on the project phase:

### Phases 1–4 (Thread-based implementation)

The server and client use C++11 threads, so compile with the pthread library:

```bash
g++ -std=c++11 -pthread server.cpp -o server
g++ -std=c++11 -pthread client.cpp -o client
```

### Phases 5–6 (epoll-based implementation)

The server no longer uses `std::thread`, so `-pthread` is not required:

```bash
g++ -std=c++11 server.cpp -o server
```

The client still uses a background receiver thread, so it must still be compiled with(will be improved):

```bash
g++ -std=c++11 -pthread client.cpp -o client
```

---

## Running

Start the server:

```bash
./server
```

Open multiple terminals and start clients:

```bash
./client
```

Each client enters a username and can exchange messages with all connected clients.

---

## Example

Server

```text
--Server Initialized--
Connected clients: 1
Connected clients: 2
Connected clients: 3
Rim : hii...
Pai : sup bro
Rocky : hiyaa
Rocky : byee
Client [7] Disconnected.
Connected clients: 2
Pai : bye rocks
Client [6] Disconnected.
Connected clients: 1
Rim : okies
Client [5] Disconnected.
Connected clients: 0
```

Client 1

```text
--Connected to Server--
Enter your name:
Rim
hii...
Pai : sup bro
Rocky : hiyaa
Rocky : byee
Pai : bye rocks
okies
```

Client 2

```text
--Connected to Server--
Enter your name:
Pai
Rim : hii...
sup bro
Rocky : hiyaa
Rocky : byee
bye rocks
```

Client 3

```text
--Connected to Server--
Enter your name:
Rocky
Rim : hii...
Pai : sup bro
hiyaa
byee
```

---

## Future Improvements

* Chat logging
* Improve server shutdown
* Client wrapper class
* Command system (`/who`, `/quit`, etc.)
* Proper username implementation
* Better error logging

---

## Author

**Rimjhim Chakraborty**

This project was developed as part of a personal C++ systems programming learning roadmap focused on building production-style Linux applications using modern C++.
