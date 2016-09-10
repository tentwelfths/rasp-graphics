#include "GraphicsSystem.h"
std::vector<Object*> gObjects;
int main ( int argc, char *argv[] )
{
  
  GraphicsSystem g;
  g.LoadPngToTexture("Kakka_Carrot_Veggie.png");
  Object * a = new Object();
  a->position[0] = 1;
  a->position[1] = 4;
  a->scale[0] = 3;
  a->scale[1] 1;
  a->textureID = g.mTextures["Kakka_Carrot_Veggie.png"];
  gObjects.push_back(a);
  
  while(true){
    g.Draw();
  }
  return 0;
}
