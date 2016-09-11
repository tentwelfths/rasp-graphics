#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

class NetworkingSystem
{
public:
  NetworkingSystem(int port, const char * ip);
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256];
  int Send(const char * buffer, int len);
  void Receive(char * buffer, int len);
};