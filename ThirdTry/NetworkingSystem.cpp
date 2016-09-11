#include "NetworkingSystem.h"
#include "Object.h"


void error(const char *msg)
{
    perror(msg);
    exit(0);
}

NetworkingSystem::NetworkingSystem(int port, const char * ip)
{
  portno = port;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
      error("ERROR opening socket");
  server = gethostbyname(ip);
  if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
       (char *)&serv_addr.sin_addr.s_addr,
       server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
      error("ERROR connecting");
  char one = 1;
  ioctl(sockfd, FIONBIO, (char *)&one); 
}


int NetworkingSystem::Send(const char * buffer, int len)
{
  n = write(sockfd,buffer,len);
}
void NetworkingSystem::Receive(const char * buffer, int len)
{
  read(sockfd,buffer,len);
}