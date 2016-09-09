#include <stdlib.h>
#include <stdio.h>

#include <GLES/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <png.h>

struct Texture{
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
  
  void LoadVertShader(const char * shaderSrc);
  void LoadFragShader(const char * shaderSrc);
  
  void LoadPngToTexture(const char * filename);
  
  GLuint vertShader;
  GLuint fragShader;

  GLint       width;

  /// Window height
  GLint       height;

  /// Window handle
  EGLNativeWindowType  hWnd;

  /// EGL display
  EGLDisplay  eglDisplay;
     
  /// EGL context
  EGLContext  eglContext;

  /// EGL surface
  EGLSurface  eglSurface;
  
  std::vector<Texture> mTextures;
private:
  GLint loadpng(const char * filename);
  GLboolean WinCreate();
};
