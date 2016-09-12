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



void ProcessResponse(int& pos, int & clientNumber, const char * buf, int len)
{
  for(; pos < len;)
  {
    if(buf[pos] == '~')//client number
    {
      GetClientNumber(pos,clientNumber,buf);
      ++pos
    }
    else if(buf[pos] == '!') //object
    {
      int textureID = *reinterpret_cast<int*>(&(buf[pos]));
      pos += sizeof(int);
      int xPos = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      int yPos = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      int zPos = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      int xSca = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      int ySca = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      int rot  = reinterpret_cast<float*>(&(buf[pos]));
      pos += sizeof(float);
      gObjects[textureID][count[textureID]].position[0] = xPos;
      gObjects[textureID][count[textureID]].position[1] = yPos;
      gObjects[textureID][count[textureID]].position[2] = zPos;
      gObjects[textureID][count[textureID]].scale[0] = xSca;
      gObjects[textureID][count[textureID]].scale[1] = ySca;
      gObjects[textureID][count[textureID]].rotation[2] = rot;
      gObjects[textureID][count[textureID]].textureID = textureID;
      gObjects[textureID][count[textureID]].inUse = true;
      count[textureID]++;
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
  while(true){
    if(Input())break;
    do{
      memset((void*)buf, 0, 1024);
      netResult = n.Receive(buf,1023);
      pos = 0;
      if(netResult > 0)
      {
        for(int i = 0; i < 50; ++i)
        {
          gObjects[i][0].inUse = false;
          count[i] = 0;
        }
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
  }
  return 0;
}
