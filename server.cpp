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
#include <sstream>
#include <ctime>

#define FIRSTKNOCK    5553
#define SECONDKNOCK   5554
#define LASTPORT      5555
#define MAXMSG        512

// The ID of the Server
std::string SERVER_ID = "";
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

int send_message(int userSock, std::string message)
{
  message += "\n";
  return (send(userSock, message.c_str(), message.size(), 0));
}

std::string get_user_by_fd(int userSock)
{
  for (auto it=logged_users.begin(); it!=logged_users.end(); ++it)
  {
    if(it->second == userSock)
    {
      return it->first;
    }
  }
  return "";
}

//returns the server id
std::string get_id()
{
  return SERVER_ID;
}
//generates a new server id
void generate_id()
{
  SERVER_ID = "Group 18";
  std::time_t time = std::time(nullptr);
  std::stringstream ss;
  ss << time;
  std::string time_stamp = ss.str().append(" ");
  SERVER_ID.insert(0, time_stamp);

  std::string command("fortune -s");

  std::array<char, 256> buffer;
  std::string result;

  FILE *pipe = popen(command.c_str(), "r");

  while (fgets(buffer.data(), 256, pipe) != NULL)
  {
    result += buffer.data();
  }

  pclose(pipe);
  // correctly formats the id string
  SERVER_ID.insert(0, result);
}

/*
 * In this function we read the socket message from a client
 * and handle all possible commands asked from the client.
 */
int read_from_client (int userSock)
{
  char buffer[MAXMSG] = {};
  int nbytes;

  nbytes = read (userSock, buffer, MAXMSG);
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
    /*
     * Message recieved from buffer are handled below.
     * We erase its \n and split it to command and message
     * at the index of the first space char.
     */
    std::string read_string = buffer;
    read_string.erase(read_string.size() - 1);
    size_t splitter = read_string.find(' ');
    std::string command = read_string.substr(0, splitter);
    // If read_string has no space then message == command.
    // If read_string ends after first space then message == ""
    std::string message = read_string.substr(splitter + 1);
    std::string client_name = get_user_by_fd(userSock);

    // Check if client is connected, else he has to connect for further use.
    if(client_name == "")
    {
      if(command != "CONNECT")
        send_message(userSock, "Please log in by typing: CONNECT <name>");
      else if(message != "" && message != "ALL" && message != command)
      {
        std::string username = message;
        auto checkUsername = logged_users.find(username);
        if(checkUsername == logged_users.end())
        {
          logged_users.insert(std::pair<std::string,int>(username,userSock));
          send_message(userSock, "Joined server successfully");
        }
        else 
        {
          send_message(userSock, "Username taken");
        }
      }
      else
        send_message(userSock, "Invalid username");
    }
    else if (command == "CONNECT")
      send_message(userSock, "You are already connected as: " + client_name);
    else if (command == "ID")
      send_message(userSock, get_id());
    else if (command == "CHANGE" && message == "ID")
    {
      generate_id();
      SERVER_ID = get_id();
    }
    else if(command == "WHO")
    {
      send_message(userSock, "\nUsers logged in the chatroom:");
      for (auto it=logged_users.begin(); it!=logged_users.end(); ++it)
        send_message(userSock, it->first);
    }
    else if(command == "MSG")
    {
      int splitter = message.find(' ');
      std::string reciever = message.substr(0, splitter);
      std::string message_to_rec = message.substr(splitter + 1);
      if(reciever == "ALL")
      {
        for (auto it=logged_users.begin(); it!=logged_users.end(); ++it)
        {
          if(it->second != userSock)
          {
            send_message(it->second, client_name + " yelled: " + message_to_rec);
          }
        }
        send_message(userSock, "Message has been sent to everybody");
        return 0;
      }
      auto recSock = logged_users.find(reciever);
      if(recSock != logged_users.end())
      {
        send_message(recSock->second, 
                     client_name + " whispered: " +  message_to_rec);
        send_message(userSock, "To " + reciever + ": " + message_to_rec);
      }
      else
        send_message(userSock, "User with name '" + reciever + "' not found");
    }
    else
      send_message(userSock, "ERROR: Command not found!");
    return 0;
  }
}

int main (void)
{
  int firstKnockSock, secondKnockSock, thirdKnockSock;
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;
  
  //initializes the server id
  generate_id();

  /* Create the socket and set it up to accept connections. */
  firstKnockSock = make_socket (FIRSTKNOCK);
  secondKnockSock = make_socket (SECONDKNOCK);
  thirdKnockSock = make_socket (LASTPORT);

  if (listen (thirdKnockSock, 1) < 0 
      || listen (secondKnockSock, 1) < 0 
      || listen (firstKnockSock, 1) < 0)
  {
    perror ("listen");
    exit (EXIT_FAILURE);
  }

  /* Initialize the set of active sockets. */
  FD_ZERO (&active_fd_set);
  FD_SET (firstKnockSock, &active_fd_set);
  FD_SET (thirdKnockSock, &active_fd_set);
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
        /* Handle port knocking */
        /* For the port knocking we store every IP in a map that has done a correct 
         * sequence of port knocking on the server, with an integer of right knocks
         * in a row as soon as an incorrect port is knocked, the IP in the map is 
         * erased, or reinitialized when knocked on the port that is first in the 
         * sequence.
         */
        if (i == firstKnockSock || i == secondKnockSock || i == thirdKnockSock)
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
              /* Knocking succeeded and FD is set for the new user */
              FD_SET (newSock, &active_fd_set);
              correct_knocks.erase(knock);
              send_message(newSock, "Welcome to the chat server!");
            }
          }
        }
        
        else
        {
          /* Data arriving on an already-connected socket. */
          if (read_from_client (i) < 0)
          {
            logged_users.erase( get_user_by_fd(i) );
            close (i);
            FD_CLR (i, &active_fd_set);
          }
        }
      }
  }
}