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

std::string inputstream = "";

Object gObjects[50][50];
int count[50];

bool Input ( void )
{
  static int first = 1;
  static int mouseFd = -1;
  static int keyboardFd = -1;
  struct input_event ev[64];
  int rd;

  bool ret = false;


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
        return true;

    }
    if(regcomp(&mouse,"event-mouse",0)!=0)
    {
        printf("regcomp for mouse failed\n");
        return true;

    }

    if ((dirp = opendir(dirName)) == NULL) {
        perror("couldn't open '/dev/input/by-id'");
        return true;
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
            printf("Getting exclusive access: ");
            result = ioctl(keyboardFd, EVIOCGRAB, 1);
            printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");

        }
        if(regexec (&mouse, dp->d_name, 0, NULL, 0) == 0)
        {
            printf("match for the kbd = %s\n",dp->d_name);
            sprintf(fullPath,"%s/%s",dirName,dp->d_name);
            mouseFd = open(fullPath,O_RDONLY | O_NONBLOCK);
            printf("%s Fd = %d\n",fullPath,mouseFd);
            printf("Getting exclusive access: ");
            result = ioctl(mouseFd, EVIOCGRAB, 1);
            printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");

        }
      }
    } while (dp != NULL);

    closedir(dirp);


    regfree(&kbd);
    regfree(&mouse);

    
    first = 0;
    if((keyboardFd == -1) || (mouseFd == -1)) return true;

    }

    
    // Read events from mouse

    rd = read(mouseFd,ev,sizeof(ev));
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

          if(evp->code == BTN_LEFT)  
          {
              if(evp->value == 1)   // Press
              {
                printf("Left button pressed\n");

              }
              else
              {
                printf("Left button released\n");
              }
          }
        }
  
        if(evp->type == 2)
        {

          if(evp->code == 0)
          {
              // Mouse Left/Right

              printf("Mouse moved left/right %d\n",evp->value);
          }
      
          if(evp->code == 1)
          {
              // Mouse Up/Down
              printf("Mouse moved up/down %d\n",evp->value);
              
          }
        }
      }
    }

    // Read events from keyboard

    rd = read(keyboardFd,ev,sizeof(ev));
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
          inputstream += (unsigned char)(evp->code);
          if(evp->value == 0)
          {
            inputstream += '0';
          }
          else if(evp->value == 1)
          {
            inputstream += '1';
          }
          //if(evp->value == 1) 
          //{
          //    if(evp->code == KEY_LEFTCTRL)
          //    {
          //      printf("Left Control key pressed\n");
          //    }
          //    if(evp->code == KEY_LEFTMETA )
          //    {
          //      printf("Left Meta key pressed\n");
          //    }
          //    if(evp->code == KEY_LEFTSHIFT)
          //    {
          //      printf("Left Shift key pressed\n");
          //    }
          //}
          if((evp->code == KEY_Q) && (evp->value == 1))
              ret = true;
        }
      }

    }

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

struct oldData
{
  int size;
  char buf[1023];
};
oldData old;

void ProcessResponse(int& pos, int & clientNumber, const char * buf, int len)
{
  for(; pos < len;)
  {
    if(buf[pos] == '~')//client number
    {
      GetClientNumber(pos,clientNumber,buf);
      std::cout<<"Client NUmber is " <<clientNumber<<std::endl;
      ++pos;
    }
    else if(buf[pos] == '!') //object
    {
      int totalNeeded = sizeof(unsigned int) + (sizeof(float) * 6) + 2;
      if(pos + totalNeeded >= len)
      {
        old.size = len - pos;
        for(int i = 0; pos<len; ++pos, ++i)
        {
          old.buf[i] = buf[pos];
        }
        return;
      }
      old.size = 0;
      ++pos;
      std::cout<<"Response found an object!!!!"<<std::endl;
      const unsigned int textureID = *reinterpret_cast<const unsigned int*>(&(buf[pos]));
      std::cout<<pos<<"-"<<len <<" TextureID: "<< textureID <<std::endl;
      pos += sizeof(unsigned int);
      const float xPos = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"+"<<len <<" xPos: "<< xPos <<std::endl;
      pos += sizeof(float);
      const float yPos = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"="<<len <<" yPos: "<< yPos <<std::endl;
      pos += sizeof(float);
      const float zPos = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"]"<<len <<" zPos: "<< zPos <<std::endl;
      pos += sizeof(float);
      const float xSca = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"["<<len <<" xSca: "<< xSca <<std::endl;
      pos += sizeof(float);
      const float ySca = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"*"<<len <<" ySca: "<< ySca <<std::endl;
      pos += sizeof(float);
      const float rot  = *reinterpret_cast<const float*>(&(buf[pos]));
      std::cout<<pos<<"~"<<len <<" rot: "<< rot <<std::endl;
      pos += sizeof(float);
      
      //gObjects[textureID][count[textureID]].position[0] = xPos;
      //gObjects[textureID][count[textureID]].position[1] = yPos;
      //gObjects[textureID][count[textureID]].position[2] = zPos;
      //gObjects[textureID][count[textureID]].scale[0] = xSca;
      //gObjects[textureID][count[textureID]].scale[1] = ySca;
      //gObjects[textureID][count[textureID]].rotation[2] = rot;
      //gObjects[textureID][count[textureID]].textureID = textureID;
      //gObjects[textureID][count[textureID]].inUse = true;
      //count[textureID]++;
      ++pos;
    }
    else if(buf[pos] == '@')//audio cue
    {
      //???????????????????????????
    }
    else{ //the fuck?
      ++pos;
    }
    
  }
}


int main ( int argc, char *argv[] )
{
  
  GraphicsSystem g;
  NetworkingSystem n(27015, "192.168.77.106");
  g.LoadPngToTexture("Kakka_Carrot_Veggie.png");
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
  struct timezone tz;
  float deltatime;
  while(true){
    gettimeofday ( &t1 , &tz );
    if(Input())break;
    bool updated = false;
    do{
      memset((void*)buf, 0, 1024);
      netResult = n.Receive((buf + old.size),1023 - old.size);
      for(int i = 0; i < old.size; ++i)
      {
        buf[i] = old.buf[i];
      }
      std::cout<<"netResult: "<<netResult<<std::endl;
      pos = 0;
      if(netResult > 0)
      {
        for(int i = 0; i < 50 && !updated; ++i)
        {
          gObjects[i][0].inUse = false;
          count[i] = 0;
        }
        updated = true;
        ProcessResponse(pos, clientNumber, buf, netResult);
        for(int i = 0; i < 50; ++i)
        {
          gObjects[i][count[i]].inUse = false;
        }
      }
    }while(netResult > 0);
    g.Draw();
    toSend = !toSend;
    if(toSend){
      std::vector<char> v(inputstream.length() + 1);
      std::strcpy(&v[0], inputstream.c_str());
      char* pc = &v[0];
      //std::cout<<inputstream<<std::endl;
      std::cout<<"Bytes sent: "<<n.Send(pc, inputstream.length())<<std::endl;
      inputstream = "";
    }
    
    do{
      gettimeofday(&t2, &tz);
      deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
    }while(deltatime > 1.0f/30.0f);
  }
  return 0;
}
