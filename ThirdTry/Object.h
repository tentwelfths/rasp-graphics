#pragma once
#include <unordered_map>

class Object
{
  public:
  Object():position(), scale(), rotation(), textureID(0) , textureName(""),inUse(false){}
  float position[3];
  float scale[3];
  float rotation[3];
  unsigned int textureID;
  std::string textureName;
  bool inUse;
};

extern std::unordered_map<unsigned int, Object*> gObjects[50];