## Simple client-server UDP
UDP client-server which can transfer file and calculate check value

### Using:
In 'MakeFile.sh' replace %path_to_file% with the real file path and run.
CMake will be built, server and client will do a one-time file transfer.

### Build:
```
cmake -S . -B ./build
```
#### Run server:
```
./build/server/server
```
#### Run client:
Run with "path to file"
```
./build/client/client %path_to_file%
```

### Task:
The file transfer service consists of two components:
- UDP server that can receive data, write it to RAM and calculate a checksum from it;
- A client that can send data to the server and verify the checksum.

Packages types:
PUT: when sending data from the client to the server;
ACK: upon confirmation of receiving data.

Interaction protocol:
1. The client starts sending data to the server using PUT packages.
2. The server receives the data and writes it to the RAM, after each received package it sends ACK package to the client for the corresponding file.
3. If the client does not receive an ACK for a sent package within the timeout, then the sent package is re-sent.
