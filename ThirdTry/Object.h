#pragma once
class Object
{
  public:
    float position[3];
    float scale[3];
    float rotation[3];
    unsigned int textureID;
};

extern std::vector<Object*> gObjects;