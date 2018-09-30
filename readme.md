The server and client were compiled on ubuntu16

- Fortune must be installed ('sudo apt-get install fortune' in linux)

- To compile the server use 'g++ server.cpp -o server' and then ./server to run.
- To compile the server use 'g++ client.cpp -o client -pthread' and then ./client to run.
- The client takes the server's address as an argument, to run it locally type: './client 192.0.0.1'
- The ports that have to be knocked are hardcoded in the server.cpp and client.cpp as 5003, 5004 and 5005 in respectively.


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
