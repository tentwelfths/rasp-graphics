#pragma once
class Object
{
  public:
  Object():position(), scale(), rotation(), textureID(0) ,inUse(false){}
  float position[3];
  float scale[3];
  float rotation[3];
  unsigned int textureID;
  bool inUse;
};

extern Object gObjects[50][50];