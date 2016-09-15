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
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
  //if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
  //    error("ERROR connecting");
  char one = 1;
  ioctl(sockfd, FIONBIO, (char *)&one); 
}


int NetworkingSystem::Send(const char * buffer, int len)
{
  return sendto(sockfd, buffer, len, 0, &serv_addr, sizeof(serv_addr));
  //return write(sockfd,buffer,len);
}
int NetworkingSystem::Receive(char * buffer, int len)
{
  sockaddr addr;
  unsigned int fromlen;
  while(true)
  {
    int b = recvfrom(sockfd, buffer, len, 0, (&addr), &fromlen);
    if(b <= 0)return b;
    if (serv_addr.addr.sa_family == addr.sa_family)
    {
      if (((sockaddr_in*)&serv_addr.addr)->sin_addr.s_addr == ((sockaddr_in*)&addr)->sin_addr.s_addr)
      {
        return b;
      }
    }
  }
  return -1;
}