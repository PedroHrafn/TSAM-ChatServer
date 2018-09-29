#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <thread> 

int sockfd1, sockfd2, sockfd3, n;

char buffer[256];

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void readFromServer()
{
    while(true)
    {
        bzero(buffer,256);
        n = read(sockfd3,buffer,255);
        if (n < 0) 
        {
            error("ERROR reading from socket");
        }
        printf("%s\n",buffer);
    }
}

void WriteToServer()
{
    while(true)
    {
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(sockfd3,buffer,strlen(buffer));

        if (n < 0) 
        {
            error("ERROR writing to socket");
        }
    }
}

int main(int argc, char *argv[])
{
    int port1, port2, port3;
    struct sockaddr_in serv_addr;           // Socket address structure
    struct hostent *server;
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    port1 = atoi(argv[2]);     // Read Port No from command line
    port2 = atoi(argv[3]); 
    port3 = atoi(argv[4]); 

    sockfd1 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    sockfd2 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket
    sockfd3 = socket(AF_INET, SOCK_STREAM, 0); // Open Socket

    if (sockfd1 < 0 || sockfd2 < 0 || sockfd3 < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);        // Get host from IP

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET; // This is always set to AF_INET

    // Host address is stored in network byte order
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);

    serv_addr.sin_port = htons(port1);

    if (connect(sockfd1,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    { 
        error("ERROR connecting");
    }

    close(sockfd1);
    serv_addr.sin_port = htons(port2);

    if (connect(sockfd2,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }
    
    close(sockfd2);
    serv_addr.sin_port = htons(port3);

    if (connect(sockfd3,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
    }
    printf("Welcome to the chat server");
        
    // Read and write to socket
    std::thread read (readFromServer);
    std::thread write (WriteToServer);

    read.join();
    write.join();
    
    close(sockfd3);
    return 0;
}
