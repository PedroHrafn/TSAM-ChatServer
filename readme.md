The server and client were compiled on ubuntu16

- Fortune must be installed ('sudo apt-get install fortune' in linux)

- To compile the server use 'g++ server.cpp -o server' and then ./server to run.
- To compile the server use 'g++ client.cpp -o client -pthread' and then ./client to run.
- The client needs 4 arguments to run it, the first being the host and next three the ports to knock in the correct order, e.g. './client localhost 5003 5004 5005'
- The ports that have to be knocked are hardcoded in the server.cpp as 5003, 5004 and 5005 in respectively.


Commands for the client:
- 'ID' -returns the ID of the server
- 'CONNECT <NAME>' -log in the server with provided name
- 'LEAVE' -exit the client
- 'WHO' -lists all logged users on the server
- 'MSG <USERNAME> <MESSAGE>' -sends provided message to provided username
- 'MSG <ALL> <MESSAGE>' -sends provided message to all other logged users
- 'CHANGE ID' -changes the ID of the server


Done By:
- Pedro Hrafn Martinez
- Kristófer Óttar Úlfarsson
- Jóhann Egilsson
- (TSAM group 18, 30th sept 2018)