#include "GraphicsSystem.h"
#include "NetworkingSystem.h"
#include "Object.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdbool.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <memory>
#include <queue>
#include "MCP3008SPI.h"

std::string inputstream = "";
//mcp3008Spi a2d("/dev/spidev0.0", SPI_MODE_0, 1000000, 8);

std::unordered_map<unsigned int, Object*> gObjects[50];
std::unordered_map<unsigned int, Object*> gObjectMap;
int count[50];

bool Input ( void )
{
  static int first = 1;
  static int mouseFd = -1;
  static int keyboardFd = -1;
  static int timeout = 0;
  struct input_event ev[64];
  int rd;

  bool ret = true;
  //if(timeout++ > 20)return false;

  // Set up the devices on the first call
  if(first)
  {
    DIR *dirp;
    struct dirent *dp;
    regex_t kbd,mouse;

    char fullPath[1024];
    char *dirName = "/dev/input/by-id";
    int result;

    if(regcomp(&kbd,"event-kbd",0)!=0)
    {
        printf("regcomp for kbd failed\n");
        return false;

    }
    if(regcomp(&mouse,"event-mouse",0)!=0)
    {
        printf("regcomp for mouse failed\n");
        return false;

    }

    if ((dirp = opendir(dirName)) == NULL) {
        perror("couldn't open '/dev/input/by-id'");
        return false;
    }

    // Find any files that match the regex for keyboard or mouse

    do {
      errno = 0;
      if ((dp = readdir(dirp)) != NULL) 
      {
        printf("readdir (%s)\n",dp->d_name);
        if(regexec (&kbd, dp->d_name, 0, NULL, 0) == 0)
        {
            printf("match for the kbd = %s\n",dp->d_name);
            sprintf(fullPath,"%s/%s",dirName,dp->d_name);
            keyboardFd = open(fullPath,O_RDONLY | O_NONBLOCK);
            printf("%s Fd = %d\n",fullPath,keyboardFd);
            //printf("Getting exclusive access: ");
            //result = ioctl(keyboardFd, EVIOCGRAB, 1);
            //printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");

        }
      }
    } while (dp != NULL);
    //std::cout<<"...."<<std::endl;
    closedir(dirp);


    regfree(&kbd);
    regfree(&mouse);

    
    first = 0;
    if((keyboardFd == -1)) return false;

  }

    // Read events from keyboard

    rd = read(keyboardFd,ev,sizeof(ev));
    //std::cout<<"read "<<rd<<std::endl;
    if(rd > 0)
    {
      int count,n;
      struct input_event *evp;

      count = rd / sizeof(struct input_event);
      n = 0;
      while(count--)
      {
        evp = &ev[n++];
        if(evp->type == 1)
        {
          if(evp->value == 0)
          {
            for(unsigned i = 0; i < sizeof(decltype(evp->code)); ++i)
              inputstream += ((unsigned char *)(&evp->code))[i];
            inputstream += '0';
          }
          else if(evp->value == 1)
          {
            for(unsigned i = 0; i < sizeof(decltype(evp->code)); ++i)
              inputstream += ((unsigned char *)(&evp->code))[i];
            inputstream += '1';
          }
          //std::cout<<evp->code<<" --- "<<evp->value<<std::endl;
          
          if((evp->code == KEY_Q) && (evp->value == 1))
              ret = false;
        }
      }

    }
    
    //std::cout<<a2d.GetChannelData(1)<<std::endl;
    //std::cout<<a2d.GetChannelData(0)<<std::endl;

    return(ret);
}


void GetClientNumber(int & pos, int & clientNumber, const char * buf)
{
  if(buf[pos] == '~')++pos;
  clientNumber = 0;
  while(buf[pos] != '~')
  {
    clientNumber *= 10;
    clientNumber += buf[pos++] - '0';
  }
}

std::queue<std::string> commands;
std::string unfinished = "";
unsigned short lastFrameSeen = 0;

void ProcessResponse(int& pos, int & clientNumber, const char * command, int len, GraphicsSystem * g)
{
  //for (int i = 0; i < len; ++i)
  //{
  //  if (buf[i] == '!'){
  //    commands.push(unfinished);
  //    unfinished = "";
  //  }
  //  else
  //  {
  //    unfinished += buf[i];
  //  }
  //}
  if (command[0] == '@')//clientNumber
  {
    clientNumber = (int)((unsigned char)command[1]);
  } 
  else
  {
    int pos = 1;
    unsigned short frame = *static_cast<const unsigned short *>(static_cast<const void *>(&(command[pos])));
    pos += sizeof(unsigned short);
    //std::cout<<frame<<":"<<lastFrameSeen<<std::endl;
    if(!(frame > lastFrameSeen || (frame < 50 && lastFrameSeen > (unsigned short)(-1) - 50))) return;
    lastFrameSeen = frame;
    int counter =0;
    while(pos < len)
    {
      if (command[pos] == '`')//object
      {
        ++pos;
        ++counter;
        //std::cout<<"Getting Object"<<std::endl;
        //std::cout<<"Response found an object!!!!"<<std::endl;
        unsigned int objectID = *static_cast<const unsigned int *>(static_cast<const void *>(&(command[pos])));
        pos += sizeof(unsigned int);
        const unsigned char textureNameLength = *reinterpret_cast<const unsigned char*>(&(command[pos]));
        pos += sizeof(unsigned char);
        std::string textureName = "";
        for(unsigned char i = 0; i < textureNameLength; ++i){
          textureName += (char)command[pos++];
        }
        //std::cout<<"TEXTURE NAME: "<<textureName<<std::endl;
        //for(auto & iter : g->mTextures){
        //  std::cout<<iter.first<<" "<<strcmp(textureName.c_str(), iter.first.c_str())<<std::endl;
        //  if(strcmp(textureName.c_str(), iter.first.c_str()) == 0){
        //    std::cout<<"MATCH FOUND " << iter.second.textureID;
        //  }
        //}
        //std::cout<<"Object with textID "<<textureID<<" #"<<count[textureID]<<std::endl;
        //std::cout<<pos<<"-"<<len <<" TextureID: "<< textureID <<std::endl;
        
        const float xPos = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"+"<<len <<" xPos: "<< xPos <<std::endl;
        pos += sizeof(float);
        const float yPos = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"="<<len <<" yPos: "<< yPos <<std::endl;
        pos += sizeof(float);
        const float zPos = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"]"<<len <<" zPos: "<< zPos <<std::endl;
        pos += sizeof(float);
        const float xSca = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"["<<len <<" xSca: "<< xSca <<std::endl;
        pos += sizeof(float);
        const float ySca = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"*"<<len <<" ySca: "<< ySca <<std::endl;
        pos += sizeof(float);
        const float rot  = *reinterpret_cast<const float*>(&(command[pos]));
        //std::cout<<pos<<"~"<<len <<" rot: "<< rot <<std::endl;
        pos += sizeof(float);
        int textureID = g->mTextures[textureName].textureID;
        if(gObjects[textureID].find(objectID) == gObjects[textureID].end())
        {
          Object * obj = new Object();
          gObjects[textureID].insert({objectID, obj});
          gObjectMap.insert({objectID, obj});
        }
        Object * temp = gObjects[textureID][objectID];
        temp->position[0] = xPos;
        temp->position[1] = yPos;
        temp->position[2] = zPos;
        temp->scale[0] = xSca;
        temp->scale[1] = ySca;
        temp->rotation[2] = rot;
        temp->textureID = textureID;
        temp->inUse = true;
      }
      else if(command[pos] == '%')//DEATH
      {
        ++pos;
        unsigned int objectID = *static_cast<const unsigned int *>(static_cast<const void *>(&(command[pos])));
        pos += sizeof(unsigned int);
        if(gObjectMap.find(objectID) != gObjectMap.end()){
          gObjectMap[objectID]->inUse = false;
        }
      }
    }
    //std::cout<<"\t\t\t\tUPDATING #"<<counter<<" OBJECTS"<<std::endl;
  }
}

int main ( int argc, char *argv[] )
{
  mcp3008Spi a2d("/dev/spidev0.0", SPI_MODE_0, 1000000, 8);
  
  GraphicsSystem g;
  NetworkingSystem n(27015, "192.168.77.106");
  std::cout<<"CONNECTED"<<std::endl;
  int res = n.Send("HELLO", strlen("HELLO"));
  //std::cout<<res<<std::endl;
  //return 0;
  g.LoadPngToTexture("Kakka_Carrot_Veggie.png");
  g.LoadPngToTexture("Key.png");
  //Object a;
  //a.position[0] = 0;
  //a.position[1] = 0;
  //a.scale[0] = 1;
  //a.scale[1] = 1;
  //a.textureID = g.mTextures["Kakka_Carrot_Veggie"].textureID;
  //a.inUse = true;
  //gObjects[a.textureID][0] = a;
  bool toSend = false;
  char buf[1024] = {0};
  int pos = 0;
  int clientNumber = -1;
  int netResult = 0;
  struct timeval t1, t2;
  struct timeval tStart,tEnd;
  struct timezone tz;
  float deltatime, gDt, rDt,sDt,iDt;
  while(true){
    //std::cout<<"loop"<<std::endl;
    gettimeofday ( &t1 , &tz );
    bool updated = false;
    gettimeofday ( &tStart , &tz );
    do{
      memset((void*)buf, 0, 1024);
      //std::cout<<"Tryna recv"<<std::endl;
      netResult = n.Receive((buf),1023);
      
      //std::cout<<"netResult: "<<netResult<<std::endl;
      pos = 0;
      if(netResult > 0)
      {
        ProcessResponse(pos, clientNumber, buf, netResult, &g);
      }
    }while(netResult > 0);
    gettimeofday ( &tEnd , &tz );
    rDt = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    gettimeofday ( &tStart , &tz );
    g.Draw();
    gettimeofday ( &tEnd , &tz );
    gDt = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    toSend = !toSend;
    inputstream = "~";
    gettimeofday ( &tStart , &tz );
    unsigned short x = a2d.GetChannelData(0);
    unsigned short y = a2d.GetChannelData(1);
    for(unsigned i = 0; i < sizeof(unsigned short); ++i){
      inputstream += static_cast<char *>(static_cast<void *>(&x))[i];
    }
    for(unsigned i = 0; i < sizeof(unsigned short); ++i){
      inputstream += static_cast<char *>(static_cast<void *>(&y))[i];
    }
    inputstream += (a2d.GetChannelData(2) > 15) ? '0' : '1';
    if(toSend && inputstream.length() > 0){
      
      //inputstream = "~" + inputstream + "!";
      std::vector<char> v(inputstream.length() + 1);
      for(unsigned i = 0; i < inputstream.length(); ++i)v[i] = inputstream[i];
      char* pc = &v[0];
      //std::cout<<inputstream<<std::endl;
      int sentbytes = n.Send(pc, inputstream.length());
      std::cout<<"Bytes sent: "<<sentbytes<<std::endl;
      inputstream = "";
    }
    gettimeofday ( &tEnd , &tz );
    iDt = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    
    gettimeofday(&t2, &tz);
    deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec)/1000.f);
    //std::cout<<deltatime<<std::endl;
    if(deltatime >= 1.0f/30.f)
    {
      //Frame took too long
      //float total = gDt + rDt + iDt;
      //std::cout<<"Graphics: " << (gDt / total) * 100.f <<"%"<<std::endl
      //<<"Input: " << (iDt / total) * 100.f <<"%"<<std::endl
      //<<"Receiving: " << (rDt / total) * 100.f <<"%"<<std::endl<<std::endl;
      
    }
    //do{
    //  gettimeofday(&t2, &tz);
    //  deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    //}while(deltatime < 1.0f/30.0f);
  }
  return 0;
}
