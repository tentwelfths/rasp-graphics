#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <GLES/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <png.h>

struct TextureType{
  std::string name;
  GLuint textureID;
};

class GraphicsSystem
{
public:
  GraphicsSystem();
  GLboolean CreateWindow();
  void Draw();
  void KeyPress();
  void Update(float dt);
  
  void LoadProgram(const char * vertSrc,const char * fragSrc);
  
  void LoadPngToTexture(const char * filename);
  
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;

  GLint       width;

  /// Window height
  GLint       height;

  GLuint View,Projection,Position_modelspace,Position_worldspace,Scale,Rotation,Texture;
  
  /// Window handle
  EGLNativeWindowType  hWnd;

  /// EGL display
  EGLDisplay  eglDisplay;
     
  /// EGL context
  EGLContext  eglContext;

  /// EGL surface
  EGLSurface  eglSurface;
  
  std::vector<TextureType> mTextures;
  
  GLfloat vVerts_[20];
  
  GLushort indices_[6];
private:
  GLint loadpng(const char * filename);
  GLboolean WinCreate();
};
