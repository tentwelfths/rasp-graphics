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

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <vector>
#include <unordered_map>

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

/// esCreateWindow flag - RGB color buffer
#define ES_WINDOW_RGB           0
/// esCreateWindow flag - ALPHA color buffer
#define ES_WINDOW_ALPHA         1 
/// esCreateWindow flag - depth buffer
#define ES_WINDOW_DEPTH         2 
/// esCreateWindow flag - stencil buffer
#define ES_WINDOW_STENCIL       4
/// esCreateWindow flat - multi-sample buffer
#define ES_WINDOW_MULTISAMPLE   8


///
// Types
//

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

struct TextureType{
  std::string name;
  GLuint textureID;
};

struct Camera{
  float posx,posy,fov;
};

class GraphicsSystem
{
public:
  GraphicsSystem();
  GLboolean CreateWindow();
  void Draw();
  void KeyPress();
  void Update(float dt);
  
  GLuint LoadProgram(const char * vertSrc,const char * fragSrc);
  
  void LoadPngToTexture(const char * filename);
  
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;

  GLint       width;

  /// Window height
  GLint       height;

  GLuint View,Projection,Position_modelspace,Position_worldspace,Scale_,Rotation_,Texture,VertexUV;
  
  /// Window handle
  EGLNativeWindowType  hWnd;

  /// EGL display
  EGLDisplay  eglDisplay;
     
  /// EGL context
  EGLContext  eglContext;

  /// EGL surface
  EGLSurface  eglSurface;
  
  std::unordered_map<std::string, TextureType> mTextures;
  
  GLfloat vVerts_[20];
  
  GLushort indices_[6];
private:
  GLint loadpng(const char * filename);
  bool WinCreate();
  GLboolean userInterrupt();
};
