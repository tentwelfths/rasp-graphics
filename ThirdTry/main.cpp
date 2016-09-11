#include "GraphicsSystem.h"
#include "Object.h"
std::vector<Object*> gObjects;
int main ( int argc, char *argv[] )
{
  
  GraphicsSystem g;
  g.LoadPngToTexture("Kakka_Carrot_Veggie.png");
  Object * a = new Object();
  a->position[0] = -3;
  a->position[1] = 2;
  a->scale[0] = 3;
  a->scale[1]  = 5;
  a->textureID = g.mTextures["Kakka_Carrot_Veggie"].textureID;
  gObjects.push_back(a);
  
  while(true){
    g.Draw();
  }
  return 0;
}
