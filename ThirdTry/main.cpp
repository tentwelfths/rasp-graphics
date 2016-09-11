#include "GraphicsSystem.h"
#include "Object.h"
Object gObjects[50][50];
int main ( int argc, char *argv[] )
{
  
  GraphicsSystem g;
  g.LoadPngToTexture("Kakka_Carrot_Veggie.png");
  Object a;
  a.position[0] = 0;
  a.position[1] = 0;
  a.scale[0] = 1;
  a.scale[1] = 1;
  a.textureID = g.mTextures["Kakka_Carrot_Veggie"].textureID;
  a.inUse = true;
  gObjects[a.textureID][0] = a;
  
  while(true){
    g.Draw();
  }
  return 0;
}
