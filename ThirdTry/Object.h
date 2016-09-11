#pragma once
class Object
{
  public:
  Object():position({0,0,0}), scale({1,1,1,}), rotation({0,0,0}), textureID(0) ,inUse(false){}
  float position[3];
  float scale[3];
  float rotation[3];
  unsigned int textureID;
  bool inUse;
};

extern Object gObjects[50][50];