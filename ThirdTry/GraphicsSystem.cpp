#include "GraphicsSystem.h"
#include "Object.h"

void setUpRotationMatrix(GLfloat ** rotationMatrix, float angle, float u, float v, float w);

bool CreateEGLContext ( EGLNativeWindowType hWnd, EGLDisplay* eglDisplay,
                              EGLContext* eglContext, EGLSurface* eglSurface,
                              EGLint attribList[])
{
   EGLint numConfigs;
   EGLint majorVersion;
   EGLint minorVersion;
   EGLDisplay display;
   EGLContext context;
   EGLSurface surface;
   EGLConfig config;
   #ifndef RPI_NO_X
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
   #else
   EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
   #endif
   
   
   // Get Display
   #ifndef RPI_NO_X
   display = eglGetDisplay((EGLNativeDisplayType)x_display);
   if ( display == EGL_NO_DISPLAY )
   {
      return false;
   }
   #else
   display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if ( display == EGL_NO_DISPLAY )
   {
      return false;
   }
   #endif

   // Initialize EGL
   if ( !eglInitialize(display, &majorVersion, &minorVersion) )
   {
      return false;
   }

   // Get configs
   if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
   {
      return false;
   }

   // Choose config
   if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
   {
      return false;
   }

   // Create a surface
   surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)hWnd, NULL);
   if ( surface == EGL_NO_SURFACE )
   {
      return false;
   }

   // Create a GL context
   context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
   if ( context == EGL_NO_CONTEXT )
   {
      return false;
   }   
   
   // Make the context current
   if ( !eglMakeCurrent(display, surface, surface, context) )
   {
      return false;
   }
   
   *eglDisplay = display;
   *eglSurface = surface;
   *eglContext = context;
   return true;
} 

GLuint ESUTIL_API esLoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = new char[infoLen];

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

GraphicsSystem::GraphicsSystem()
{
#ifdef RPI_NO_X
   bcm_host_init();
#endif
  int32_t success = 0;
  
  //CreateWindow();
  
  char  vShaderStr[] =  
    "attribute vec3 vertexPosition_modelspace;       \n"
    "attribute vec2 vertexUV;       \n"
    "varying vec2 UV;         \n"
    "uniform mat4 Position;\n"
    "uniform mat4 Scale;\n"
    "uniform mat4 Rotation;\n"
    "uniform mat4 View;\n"
    "uniform mat4 Projection;\n"
    "\n"
    "void main(){\n"
    "  mat4 MVP = Projection * View * Position * Rotation * Scale;\n"
    "  // Output position of the vertex, in clip space : MVP * position\n"
    "  gl_Position =  MVP * vec4(vertexPosition_modelspace,1);\n"
    "  UV = vec2(vertexUV.x, 1.0 - vertexUV.y);  \n"
    "}                                \n";
   
  char fShaderStr[] =  
    "precision mediump float;                            \n"
    "varying vec2 UV;                            \n"
    "uniform sampler2D myTextureSampler;                        \n"
    "void main()                                         \n"
    "{                                                   \n"
    "  gl_FragColor = texture2D( myTextureSampler, UV ).rgba;\n"
    "}                                                   \n";
  

  
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor ( 1.0f, 0.0f, 1.0f, 1.0f );
  GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
  for(unsigned i = 0; i < 20; ++i)
  {
    vVerts_[i] = vVertices[i];
  }
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
  for(unsigned i = 0; i < 6; ++i)
  {
    indices_[i] = indices[i];
  }
  // Set the viewport
  glViewport ( 0, 0, width, height );
    program = LoadProgram(vShaderStr, fShaderStr);
  
  Position_worldspace = glGetUniformLocation(program, "Position");
  Scale_ = glGetUniformLocation(program, "Scale");
  Rotation_ = glGetUniformLocation(program, "Rotation");
  View = glGetUniformLocation(program, "View");
  Projection = glGetUniformLocation(program, "Projection");
  Position_modelspace = glGetAttribLocation(program, "vertexPosition_modelspace");
  VertexUV = glGetAttribLocation(program, "vertexUV");
  Texture = glGetUniformLocation(program, "myTextureSampler");
  
  // Use the program object
  glUseProgram ( program );
  
    // Load the vertex position
  glVertexAttribPointer ( Position_modelspace, 3, GL_FLOAT, 
                          GL_FALSE, 5 * sizeof(GLfloat), vVerts_ );
  // Load the texture coordinate
  glVertexAttribPointer ( VertexUV, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVerts_[3] );

  glEnableVertexAttribArray ( Position_modelspace );
  glEnableVertexAttribArray ( VertexUV );
   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
}

void GraphicsSystem::Draw()
{
  for(int i = 0; i < gObjects.size(); ++i)
  {
    GLfloat Position[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    Position[3] = gObjects[i]->scale[0];
    Position[7] = gObjects[i]->scale[1];
    Position[11] = gObjects[i]->scale[2];

    GLfloat Rotation[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    GLfloat Scale[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    Scale[0] = gObjects[i]->scale[0];
    Scale[5] = gObjects[i]->scale[1];
    Scale[10] = gObjects[i]->scale[2];
    
    setUpRotationMatrix(reinterpret_cast<GLfloat**>(&Rotation), gObjects[i]->rotation[0], 1, 0, 0);
    setUpRotationMatrix(reinterpret_cast<GLfloat**>(&Rotation), gObjects[i]->rotation[1], 0, 1, 0);
    setUpRotationMatrix(reinterpret_cast<GLfloat**>(&Rotation), gObjects[i]->rotation[2], 0, 0, 1);
    
    glBindTexture ( GL_TEXTURE_2D, gObjects[i]->textureID );

    // Set the sampler texture unit to 0
    glUniform1i ( Texture, 0 );

    glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices_ );
  }
}

GLuint GraphicsSystem::LoadProgram(const char * vertSrc, const char * fragSrc)
{
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = esLoadShader ( GL_VERTEX_SHADER, vertSrc );
   if ( vertexShader == 0 )
      return 0;

   fragmentShader = esLoadShader ( GL_FRAGMENT_SHADER, fragSrc );
   if ( fragmentShader == 0 )
   {
      glDeleteShader( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = new char[infoLen];

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}

GLboolean GraphicsSystem::CreateWindow()
{
  GLuint flags  = ES_WINDOW_RGB;
  EGLint attribList[] =
   {
       EGL_RED_SIZE,       5,
       EGL_GREEN_SIZE,     6,
       EGL_BLUE_SIZE,      5,
       EGL_ALPHA_SIZE,     (flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
       EGL_DEPTH_SIZE,     (flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
       EGL_STENCIL_SIZE,   (flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
       EGL_SAMPLE_BUFFERS, (flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
       EGL_NONE
   };
   

   if ( !WinCreate () )
   {
      return GL_FALSE;
   }

  
   if ( !CreateEGLContext ( hWnd,
                            &eglDisplay,
                            &eglContext,
                            &eglSurface,
                            attribList) )
   {
      return GL_FALSE;
   }
   

   return GL_TRUE;
}


#ifdef RPI_NO_X
///
//  WinCreate() - RaspberryPi, direct surface (No X, Xlib)
//
//      This function initialized the display and window for EGL
//
bool GraphicsSystem::WinCreate() 
{
   int32_t success = 0;

   static EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;
   

   unsigned int display_width;
   unsigned int display_height;

   // create an EGL window surface, passing context width/height
   success = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
   if ( success < 0 )
   {
      return false;
   }
   
   // You can hardcode the resolution here:
   //display_width = 640;
   //display_height = 480;
   width = display_width;
   height = display_height;

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = display_width;
   dst_rect.height = display_height;
      
   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = display_width << 16;
   src_rect.height = display_height << 16;   

   dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );
         
   dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);
      
   nativewindow.element = dispman_element;
   nativewindow.width = display_width;
   nativewindow.height = display_height;
   vc_dispmanx_update_submit_sync( dispman_update );
   
   hWnd = &nativewindow;

	return true;
}
///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keypress, or
//      window close action.
//
GLboolean GraphicsSystem::userInterrupt()
{
	//GLboolean userinterrupt = GL_FALSE;
    //return userinterrupt;
    
    // Ctrl-C for now to stop
    
    return GL_FALSE;
}
#else
///
//  WinCreate()
//
//      This function initialized the native X11 display and window for EGL
//
bool GraphicsSystem::GraphicsSystem::WinCreate()
{
    Window root;
    XSetWindowAttributes swa;
    XSetWindowAttributes  xattr;
    Atom wm_state;
    XWMHints hints;
    XEvent xev;
    EGLConfig ecfg;
    EGLint num_config;
    Window win;

    /*
     * X11 native display initialization
     */

    x_display = XOpenDisplay(NULL);
    if ( x_display == NULL )
    {
        return false;
    }

    root = DefaultRootWindow(x_display);

    swa.event_mask  =  ExposureMask | PointerMotionMask | KeyPressMask;
    win = XCreateWindow(
               x_display, root,
               0, 0, width, height, 0,
               CopyFromParent, InputOutput,
               CopyFromParent, CWEventMask,
               &swa );

    xattr.override_redirect = FALSE;
    XChangeWindowAttributes ( x_display, win, CWOverrideRedirect, &xattr );

    hints.input = TRUE;
    hints.flags = InputHint;
    XSetWMHints(x_display, win, &hints);

    // make the window visible on the screen
    XMapWindow (x_display, win);
    XStoreName (x_display, win, title);

    // get identifiers for the provided atom name strings
    wm_state = XInternAtom (x_display, "_NET_WM_STATE", FALSE);

    memset ( &xev, 0, sizeof(xev) );
    xev.type                 = ClientMessage;
    xev.xclient.window       = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format       = 32;
    xev.xclient.data.l[0]    = 1;
    xev.xclient.data.l[1]    = FALSE;
    XSendEvent (
       x_display,
       DefaultRootWindow ( x_display ),
       FALSE,
       SubstructureNotifyMask,
       &xev );

    hWnd = (EGLNativeWindowType) win;
    return true;
}


///
//  userInterrupt()
//
//      Reads from X11 event loop and interrupt program if there is a keypress, or
//      window close action.
//
GLboolean GraphicsSystem::userInterrupt(ESContext *esContext)
{
    XEvent xev;
    KeySym key;
    GLboolean userinterrupt = GL_FALSE;
    char text;

    // Pump all messages from X server. Keypresses are directed to keyfunc (if defined)
    while ( XPending ( x_display ) )
    {
        XNextEvent( x_display, &xev );
        if ( xev.type == KeyPress )
        {
            if (XLookupString(&xev.xkey,&text,1,&key,0)==1)
            {
                if (esContext->keyFunc != NULL)
                    esContext->keyFunc(esContext, text, 0, 0);
            }
        }
        if ( xev.type == DestroyNotify )
            userinterrupt = GL_TRUE;
    }
    return userinterrupt;
}
#endif

void GraphicsSystem::LoadPngToTexture(const char * filename)
{
  GLint myPNG = loadpng(filename);
  if(myPNG == 0){
    std::cout<<"PNG did not load"<<std::endl;
    return;
  }
  std::string temp = filename;
  temp = temp.substr(0, temp.find_last_of('.'));
  TextureType t;
  t.name = temp;
  t.textureID = myPNG;
  mTextures.insert({temp, t});
}


// This one file (png_texture.cpp) is free and unencumbered software
// released into the public domain.

GLint GraphicsSystem::loadpng(const char * file_name)
{
    // This function was originally written by David Grayson for
    // https://github.com/DavidEGrayson/ahrs-visualizer

    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        return 0;
    }

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);

    //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);

    if (bit_depth != 8)
    {
        fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
        return 0;
    }

    GLint format;
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        break;
    default:
        fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
        return 0;
    }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);
    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return texture;
}

void setUpRotationMatrix(GLfloat ** rotationMatrix, float angle, float u, float v, float w)
{
  float L = (u*u + v * v + w * w);
  angle = angle * 3.14159 / 180.0; //converting to radian value
  float u2 = u * u;
  float v2 = v * v;
  float w2 = w * w;

  (*rotationMatrix)[0] = (u2 + (v2 + w2) * cos(angle)) / L;
  (*rotationMatrix)[4] = (u * v * (1 - cos(angle)) - w * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[8] = (u * w * (1 - cos(angle)) + v * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[12] = 0.0;

  (*rotationMatrix)[1] = (u * v * (1 - cos(angle)) + w * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[5] = (v2 + (u2 + w2) * cos(angle)) / L;
  (*rotationMatrix)[9] = (v * w * (1 - cos(angle)) - u * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[13] = 0.0;

  (*rotationMatrix)[2] = (u * w * (1 - cos(angle)) - v * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[6] = (v * w * (1 - cos(angle)) + u * sqrt(L) * sin(angle)) / L;
  (*rotationMatrix)[10] = (w2 + (u2 + v2) * cos(angle)) / L;
  (*rotationMatrix)[14] = 0.0;

  (*rotationMatrix)[3] = 0.0;
  (*rotationMatrix)[7] = 0.0;
  (*rotationMatrix)[11] = 0.0;
  (*rotationMatrix)[15] = 1.0;
  return;
}
