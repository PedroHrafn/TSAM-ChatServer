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

#define PORT    5555
#define MAXMSG  512

int rightInARow;

int FIRSTKNOCK = 5553;
int SECONDKNOCK = 5554;
int LASTPORT = 5555;

std::vector<std::string> IPs;

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

int read_from_client (int filedes)
{
  char buffer[MAXMSG];
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
      fprintf (stderr, "Server: got message: `%s'\n %i\n", buffer, rightInARow);
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
        if (i == firstKnockSock)
        {
          if(rightInARow == 0) //change this to see if rightInARow is 0 from a specific IP
          {
            rightInARow++;
            fprintf (stderr,
                    "Server: KNOCK1 from host %s, port %hd.\n",
                    inet_ntoa (clientname.sin_addr),
                    ntohs (clientname.sin_port));
          }
          else
          {
            //do ban the specific IP for two minutes??
            fprintf (stderr,
                    "Server: KNOCK1 REPEAT from host %s, port %hd. BAN NOE PLES\n",
                    inet_ntoa (clientname.sin_addr),
                    ntohs (clientname.sin_port));
          }
          int newSock;
          size = sizeof (clientname);
          accept (firstKnockSock,
              (struct sockaddr *) &clientname,
              (socklen_t*)&size);
        }
        else if (i == secondKnockSock)
        {
          if(rightInARow == 1) //change this to see if rightInARow is 1 from a specific IP
          {
            rightInARow++;
            fprintf (stderr,
                    "Server: KNOCK2 from host %s, port %hd.\n",
                    inet_ntoa (clientname.sin_addr),
                    ntohs (clientname.sin_port));
          }
          else
          {
            //do ban the specific IP for two minutes??
            fprintf (stderr,
                    "Server: KNOCK2 REPEAT from host %s, port %hd. BAN NOE PLES\n",
                    inet_ntoa (clientname.sin_addr),
                    ntohs (clientname.sin_port));
          }
          int newSock;
          size = sizeof (clientname);
          accept (secondKnockSock,
              (struct sockaddr *) &clientname,
              (socklen_t*)&size);
        }
        else if (i == sock)
        {
          if(rightInARow != 2)
          {
            fprintf (stderr,
                    "Server: HASNT FINISHED KNOCKS from host %s, port %hd. BAN NOE PLES\n",
                    inet_ntoa (clientname.sin_addr),
                    ntohs (clientname.sin_port));
            accept (sock,
                          (struct sockaddr *) &clientname,
                          (socklen_t*)&size);
          }
          else
          {
            /* Connection request on original socket. */
            int newSock;
            size = sizeof (clientname);
            newSock = accept (sock,
                          (struct sockaddr *) &clientname,
                          (socklen_t*)&size);
            fprintf (stderr,
                      "Server: connect from host %s, port %hd.\n",
                      inet_ntoa (clientname.sin_addr),
                      ntohs (clientname.sin_port));
            FD_SET (newSock, &active_fd_set);
          }
        }
        else
        {
          /* Data arriving on an already-connected socket. */
          if (read_from_client (i) < 0)
          {
            close (i);
            FD_CLR (i, &active_fd_set);
          }
        }
      }
  }
}