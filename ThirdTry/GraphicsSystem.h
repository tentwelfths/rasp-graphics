#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <GLES/gl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <EGL/eglext.h>
#include <png.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>

#ifdef RPI_NO_X
#include  "bcm_host.h"
#else
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
#endif

#ifndef RPI_NO_X
// X11 related local variables
static Display *x_display = NULL;
#endif

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
  EGLboolean WinCreate();
  GLboolean userInterrupt();
};
