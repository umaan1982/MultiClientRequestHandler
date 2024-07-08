# Multi-Client Request Handler

This repository contains a multi-client request handler designed for efficient concurrent processing using OS-level threading APIs. It supports multiple user requests on a single thread, ensuring high performance and scalability.

## Manual

### Client-Side
To run the client-side code, provide two command line arguments: the server's IP address and the port number (fixed at 50000). The client supports the following commands:

- `add`, `sub`, `mult`, `div`
- `kill pid`, `kill name`
- `list`, `list all`
- `run name`
- `exit`

### Server-Side
To run the server-side code, execute the compiled file (e.g., `./testing`). The server supports the following commands:

- `list`
- `list serialno`
- `print message`
- `print serialno message`

Both client and server are implemented in C++ using Visual Studio. Ensure you adjust the permissions of the executable to run it properly.

## Architecture

### Client
The client operates with two threads:

- **Main Thread:** Reads user input and sends commands to the server through a designated socket.
- **Output Receiver Thread:** Receives and handles output from the server.

### Server
The server architecture includes three main lists:

- **Active Process List:** Tracks active processes for each client.
- **Process List:** Includes all processes, both active and terminated, for each client.
- **Global Client List:** Maintains information on all connected clients.

To manage multiple client connections, the server uses `fork`, creating a child process for each client. This child process handles incoming client connections and processes commands. The parent process remains available to accept new connections.

The server also uses two threads:

- **Server Input Thread:** Handles server terminal inputs (e.g., list, print).
- **Client Handler Thread:** Created for each client, it manages active processes for its respective client.

These threads are linked through a pipe, allowing the server input thread to trigger the client handler thread, which prints the active process list for the client. The server input thread can also send messages to specific clients or all clients.

## Limitation
If a client program is terminated externally or crashes, the server's process list isn't updated until the client sends a new command. This is because the server is waiting for the client's next command and cannot handle this condition through a signal function due to the non-global nature of the process list.

## Bonus Details
When a specific process is terminated externally, the server notifies the invoking client of the termination. However, due to server serialization, the client isn't informed about the terminated process until it sends a new command to the server.

This README provides an overview of the multi-client request handler, including its functionality, architecture, limitations, and bonus features. For more detailed usage and implementation information, refer to the source code and comments within.
