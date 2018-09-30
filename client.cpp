#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <thread> 
#include <iostream>

#define FIRSTKNOCK    5553
#define SECONDKNOCK   5554
#define LASTPORT      5555
#define MAXMSG        512

int SOCKFD1, SOCKFD2, SOCKFD3;

char buffer[256];

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void readFromServer()
{
    int n;
    while(true)
    {
        bzero(buffer,256);
        n = read(SOCKFD3,buffer,255);
        if (n < 0) 
        {
            error("ERROR reading from socket");
        }
        printf("%s\n",buffer);
    }
}

void WriteToServer()
{
    int n;
    while(true)
    {
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(SOCKFD3,buffer,strlen(buffer));

        if (n < 0) 
        {
            error("ERROR writing to socket");
        }  
        std::string command = buffer;
        command.erase(command.size()-1);
        if(command == "LEAVE")
        {
            close(SOCKFD3);
            exit(0);
        }  
    }
}

int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    
    if (argc != 2) {
       fprintf(stderr,"usage: %s hostname\n", argv[0]);
       exit(0);
    }

    SOCKFD1 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    SOCKFD2 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    SOCKFD3 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket

    if (SOCKFD1 < 0 || SOCKFD2 < 0 || SOCKFD3 < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);        // Get host from IP

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    // Host address is stored in network byte order
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(FIRSTKNOCK);

    if (connect(SOCKFD1,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    { 
        error("ERROR connecting");
    }

    close(SOCKFD1);
    serv_addr.sin_port = htons(SECONDKNOCK);

    if (connect(SOCKFD2,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }
    
    close(SOCKFD2);
    serv_addr.sin_port = htons(LASTPORT);

    if (connect(SOCKFD3,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }
        
    // Read and write to socket
    std::thread read (readFromServer);
    std::thread write (WriteToServer);

    read.join();
    write.join();
    
    close(SOCKFD3);
    return 0;
}