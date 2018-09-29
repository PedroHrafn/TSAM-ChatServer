#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <string.h>
#include <iostream>
#include <map>

#define PORT    5555
#define MAXMSG  512

int rightInARow;

int FIRSTKNOCK = 5553;
int SECONDKNOCK = 5554;
int LASTPORT = 5555;

/*  This will probably not be used
struct chat_user
{
  std::string IP;
  std::string name;
  int socket;
};
std::vector<chat_user> chat_users; */


// A map of username and it's corresponding FD
std::map<std::string, int> logged_users;
// A map of an IP address and it's corresponding correct knocks in a row
std::map<std::string, int> correct_knocks;

int make_socket (uint16_t port)
{
  int sock;
  struct sockaddr_in name;

  /* Create the socket. */
  sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Give the socket a name. */
  name.sin_family = AF_INET;
  name.sin_port = htons (port);
  name.sin_addr.s_addr = htonl (INADDR_ANY);
  if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
  {
    perror ("bind");
    exit (EXIT_FAILURE);
  }

  return sock;
}

int send_message(int filedes, std::string message)
{
  return (send(filedes, message.c_str(), sizeof(message.c_str()), 0));
}

int read_from_client (int filedes)
{
  char buffer[MAXMSG] = {};
  int nbytes;

  nbytes = read (filedes, buffer, MAXMSG);
  if (nbytes < 0)
    {
      /* Read error. */
      perror ("read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    /* End-of-file. */
    return -1;
  else
    {
      /* Data read. */
      fprintf (stderr, "Server: got message: %s\n", buffer);
      //std::string msg = "kalli\n";
      //send_message(filedes, msg);
      std::string s = buffer;
      size_t splitter = s.find(' ');
      if(splitter == std::string::npos || splitter == s.size() - 2)
      {
        std::cout << "TYPE COMMAND, SPACE, SOMETHING" << std::endl;
        return 0;
      }
      std::string command = s.substr(0, splitter);
      std::string message = s.substr(splitter + 1);
      message = message.substr(0, message.size() - 1);
      return 0;
    }
}

void changePorts()
{
  FIRSTKNOCK = 5556;
  SECONDKNOCK = 5557;
  LASTPORT = 5558;
}

int knocksOfIP(char* IP)
{
  return rightInARow;
}



int main (void)
{
  //changePorts();
  rightInARow = 0;
  int sock, firstKnockSock, secondKnockSock;
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;

  /* Create the socket and set it up to accept connections. */
  firstKnockSock = make_socket (FIRSTKNOCK);
  secondKnockSock = make_socket (SECONDKNOCK);
  sock = make_socket (LASTPORT);

  if (listen (sock, 1) < 0 || listen (secondKnockSock, 1) < 0 || listen (firstKnockSock, 1) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (firstKnockSock, &active_fd_set);
  FD_SET (sock, &active_fd_set);
  FD_SET (secondKnockSock, &active_fd_set);

  while (1)
  {
    /* Block until input arrives on one or more active sockets. */
    read_fd_set = active_fd_set;
    if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
    {
      perror ("select");
      exit (EXIT_FAILURE);
    }

    /* Service all the sockets with input pending. */
    for (i = 0; i < FD_SETSIZE; ++i)
      if (FD_ISSET (i, &read_fd_set))
      {
        /* Checked what socket has input pending */
        if (i == firstKnockSock || i == secondKnockSock || i == sock)
        {
          int newSock;
          size = sizeof (clientname);
          newSock = accept (i, (struct sockaddr *) &clientname, (socklen_t*)&size);
          std::string IP_addr = inet_ntoa (clientname.sin_addr);
          auto knock = correct_knocks.find(IP_addr);
          if (i == firstKnockSock)
          {
            close(newSock);
            if(knock == correct_knocks.end())
              correct_knocks.insert( std::pair<std::string,int>(IP_addr,1) );
            else
              knock->second = 1;
          }
          else if (i == secondKnockSock)
          {
            close(newSock);
            if(knock != correct_knocks.end() && knock->second == 1)
              knock->second++; // knock sucess
            else if(knock != correct_knocks.end())
              correct_knocks.erase(knock); // wrong port knock
          }
          else
          {
            if(knock == correct_knocks.end() || knock->second != 2)
              close(newSock);
            else
            {
              /* Knocking succeeded and FD is set for the new socket */
              FD_SET (newSock, &active_fd_set);
              // TODO: insert to logged_users
              //logged_users.insert( std::pair<std::string,int>(USERNAME, newSock?));
            }
            if(knock != correct_knocks.end())
              correct_knocks.erase(knock);
          }
        }
        
        else
        {
          /* Data arriving on an already-connected socket. */
          if (read_from_client (i) < 0)
          {
            // TODO: delete user from logged_users
            close (i);
            FD_CLR (i, &active_fd_set);
          }
        }
      }
  }
}