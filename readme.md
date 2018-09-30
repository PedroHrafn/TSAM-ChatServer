The server and client where tested on ubuntu16

- Fortune must be installed ("sudo apt-get install fortune")

- To compile the server use "g++ server.cpp -o server" and then ./server to run.
- To compile the server use "g++ client.cpp -o client -pthread" and then ./client to run.
- To connect to the server a user has to knock on ports 5553, 5554, 5555 in that order.
- Commands should be entered with "_" instead of spaces example "CHANGE_ID"
