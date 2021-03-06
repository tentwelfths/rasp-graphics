/*
 * code stolen from openGL-RPi-tutorial-master/encode_OGL/
 * and from OpenGL® ES 2.0 Programming Guide
 */
/*
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/time.h>
//#include "jpeg.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>


// from esUtil.h
#define TRUE 1
#define FALSE 0

class GraphicsSystem{
public:
   GraphicsSystem(){

   }
   void Draw(){
      GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };

   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
   //GLushort indices[] = {1, 0, 3, 0, 2, 0, 1 };
      
   // Set the viewport
   glViewport ( 0, 0, width, height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( programObject );

   // Load the vertex position
   glVertexAttribPointer ( gl, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( positionLoc );
   glEnableVertexAttribArray ( texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );

   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
   //glDrawElements ( GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, indices );
   }
private:
    // Handle to a program object
    GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Texture handle
   GLuint textureId;
   uint32_t width;
    uint32_t height;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    EGL_DISPMANX_WINDOW_T nativewindow;
};

char *image;
int tex;


char* esLoadTGA ( char *fileName, int *width, int *height )
{
    char *buffer = NULL;
    FILE *f;
    unsigned char tgaheader[12];
    unsigned char attributes[6];
    unsigned int imagesize;

    f = fopen(fileName, "rb");
    if(f == NULL) return NULL;

    if(fread(&tgaheader, sizeof(tgaheader), 1, f) == 0)
    {
        fclose(f);
        return NULL;
    }

    if(fread(attributes, sizeof(attributes), 1, f) == 0)
    {
        fclose(f);
        return 0;
    }

    *width = attributes[1] * 256 + attributes[0];
    *height = attributes[3] * 256 + attributes[2];
    imagesize = attributes[4] / 8 * *width * *height;
    //imagesize *= 4/3;
    printf("Origin bits: %d\n", attributes[5] & 030);
    printf("Pixel depth %d\n", attributes[4]);
    buffer = malloc(imagesize);
    if (buffer == NULL)
    {
        fclose(f);
        return 0;
    }

#if 1
    // invert - should be reflect, easier is 180 rotate
    int n = 1;
    while (n <= imagesize) {
   fread(&buffer[imagesize - n], 1, 1, f);
   n++;
    }
#else
    // as is - upside down
    if(fread(buffer, 1, imagesize, f) != imagesize)
    {
        free(buffer);
        return NULL;
    }
#endif
    fclose(f);
    return buffer;
}

///
// Create a simple width x height texture image with four different colors
//
GLuint CreateSimpleTexture2D(int width, int height )
{
   // Texture object handle
   GLuint textureId;

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load the texture


   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 
        width, height, 
        0, GL_RGB, GL_UNSIGNED_BYTE, image );

   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   return textureId;
}



///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader(GLenum type, const char *shaderSrc)
{
    GLuint shader;
    GLint compiled;
    // Create the shader object
    shader = glCreateShader(type);
    if(shader == 0)
   return 0;
    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);
    // Compile the shader
    glCompileShader(shader);
    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
   {
       GLint infoLen = 0;
       glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
       if(infoLen > 1)
      {
          char* infoLog = malloc(sizeof(char) * infoLen);
          glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
          fprintf(stderr, "Error compiling shader:\n%s\n", infoLog);
          free(infoLog);
      }
       glDeleteShader(shader);
       return 0;
   }
    return shader;
}

GLuint LoadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vertShaderSrc );
   if ( vertexShader == 0 )
      return 0;

   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
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
         char* infoLog = malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         fprintf (stderr, "Error linking program:\n%s\n", infoLog );            
         
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

///
// Initialize the shader and program object
//
int Init(CUBE_STATE_T *p_state)
{

   p_state->user_data = malloc(sizeof(UserData));      
   UserData *userData = p_state->user_data;
   GLbyte vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";

   // Load the shaders and get a linked program object
   userData->programObject = LoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );
   // Load the texture
   userData->textureId = CreateSimpleTexture2D (p_state->width, p_state->height);

   glClearColor ( 1.0f, 0.0f, 0.0f, 1.0f );
   return GL_TRUE;
}

///
// Draw triangles using the shader pair created in Init()
//
void Draw(CUBE_STATE_T *p_state)
{
   UserData *userData = p_state->user_data;

   
}

CUBE_STATE_T state, *p_state = &state;

void init_ogl(CUBE_STATE_T *state, int width, int height)
{
    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    bcm_host_init();

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] =
   {
       EGL_RED_SIZE, 8,
       EGL_GREEN_SIZE, 8,
       EGL_BLUE_SIZE, 8,
       EGL_ALPHA_SIZE, 8,
       EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
       EGL_NONE
   };

    static const EGLint context_attributes[] =
   {
       EGL_CONTEXT_CLIENT_VERSION, 2,
       EGL_NONE
   };

    EGLConfig config;

    // get an EGL display connection
    state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    // initialize the EGL display connection
    result = eglInitialize(state->display, NULL, NULL);

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);

    // get an appropriate EGL frame buffer configuration
    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);


    // create an EGL rendering context
    state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
    assert(state->context!=EGL_NO_CONTEXT);

    // create an EGL window surface
    success = graphics_get_display_size(0 , &state->width, &state->height);
    assert( success >= 0 );

    //state->width = width;
    //state->height = height;

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = state->width;
    dst_rect.height = state->height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = state->width<<16;
    src_rect.height = state->height<<16;        

    dispman_display = vc_dispmanx_display_open( 0 );
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = 
   vc_dispmanx_element_add(dispman_update, dispman_display,
            0, &dst_rect, 0,
            &src_rect, DISPMANX_PROTECTION_NONE, 
            0 , 0, 0);

    state->nativewindow.element = dispman_element;
    state->nativewindow.width = state->width;
    state->nativewindow.height = state->height;
    vc_dispmanx_update_submit_sync( dispman_update );

    state->surface = eglCreateWindowSurface( state->display, config, &(state->nativewindow), NULL );
    assert(state->surface != EGL_NO_SURFACE);

    // connect the context to the surface
    result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
    assert(EGL_FALSE != result);
}

void esInitContext ( CUBE_STATE_T *p_state )
{
   if ( p_state != NULL )
   {
      memset( p_state, 0, sizeof( CUBE_STATE_T) );
   }
}

void esRegisterDrawFunc(CUBE_STATE_T *p_state, void (*draw_func) (CUBE_STATE_T* ) )
{
   p_state->draw_func = draw_func;
}

void  esMainLoop (CUBE_STATE_T *esContext )
{
    struct timeval t1, t2;
    struct timezone tz;
    float deltatime;
    float totaltime = 0.0f;
    unsigned int frames = 0;

    gettimeofday ( &t1 , &tz );

    while(1)
    {
        gettimeofday(&t2, &tz);
        deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
        t1 = t2;

        if (esContext->draw_func != NULL)
            esContext->draw_func(esContext);

        eglSwapBuffers(esContext->display, esContext->surface);

        totaltime += deltatime;
        frames++;
        if (totaltime >  2.0f)
        {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
            totaltime -= 2.0f;
            frames = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    UserData user_data;
    int width, height;

    image = esLoadTGA("jan.tga", &width, &height);
    if (image == NULL) {
   fprintf(stderr, "No such image\n");
   exit(1);
    }
    fprintf(stderr, "Image is %d x %d\n", width, height);

    bcm_host_init();
    esInitContext(p_state);

    init_ogl(p_state, width, height);

    p_state->user_data = &user_data;
    p_state->width = width;
    p_state->height = height;

    if(!Init(p_state))
   return 0;

    esRegisterDrawFunc(p_state, Draw);

    eglSwapBuffers(p_state->display, p_state->surface);
    esMainLoop(p_state);
}*/

#include "esUtil.h"
typedef struct
{
 // Handle to a program object
 GLuint programObject;
} UserData;
///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader(const char *shaderSrc, GLenum type)
{
 GLuint shader;
 GLint compiled;

 // Create the shader object
 shader = glCreateShader(type);
 if(shader == 0)
 return 0;
 // Load the shader source
 glShaderSource(shader, 1, &shaderSrc, NULL);

 // Compile the shader
 glCompileShader(shader);
 // Check the compile status
 glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
ch02.fm Page 21 Thursday, June 19, 2008 3:21 PM
22 Chapter 2: Hello Triangle: An OpenGL ES 2.0 Example
 if(!compiled)
 {
 GLint infoLen = 0;
 glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

 if(infoLen > 1)
 {
 char* infoLog = malloc(sizeof(char) * infoLen);
 glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
 esLogMessage("Error compiling shader:\n%s\n", infoLog);
 free(infoLog);
 }
 glDeleteShader(shader);
 return 0;
 }
 return shader;
}
///
// Initialize the shader and program object
//
int Init(ESContext *esContext)
{
 UserData *userData = esContext->userData;
 GLbyte vShaderStr[] =
 "attribute vec4 vPosition; \n"
 "void main() \n"
 "{ \n"
 " gl_Position = vPosition; \n"
 "} \n";

 GLbyte fShaderStr[] =
 "precision mediump float; \n"
 "void main() \n"
 "{ \n"
 " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
 "} \n";
 GLuint vertexShader;
 GLuint fragmentShader;
 GLuint programObject;
 GLint linked;
ch02.fm Page 22 Thursday, June 19, 2008 3:21 PM
Hello Triangle Example 23
 // Load the vertex/fragment shaders
 vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
 fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);
 // Create the program object
 programObject = glCreateProgram();
 if(programObject == 0)
 return 0;
 glAttachShader(programObject, vertexShader);
 glAttachShader(programObject, fragmentShader);
 // Bind vPosition to attribute 0
 glBindAttribLocation(programObject, 0, "vPosition");
 // Link the program
 glLinkProgram(programObject);
 // Check the link status
 glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
 if(!linked)
 {
 GLint infoLen = 0;
 glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

 if(infoLen > 1)
 {
 char* infoLog = malloc(sizeof(char) * infoLen);
 glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
 esLogMessage("Error linking program:\n%s\n", infoLog);

 free(infoLog);
 }
 glDeleteProgram(programObject);
 return FALSE;
 }
 // Store the program object
 userData->programObject = programObject;
 glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
 return TRUE;
}
ch02.fm Page 23 Thursday, June 19, 2008 3:21 PM
24 Chapter 2: Hello Triangle: An OpenGL ES 2.0 Example
///
// Draw a triangle using the shader pair created in Init()
//
void Draw(ESContext *esContext)
{
 UserData *userData = esContext->userData;
 GLfloat vVertices[] = {0.0f, 0.5f, 0.0f,
 -0.5f, -0.5f, 0.0f,
 0.5f, -0.5f, 0.0f};

 // Set the viewport
 glViewport(0, 0, esContext->width, esContext->height);

 // Clear the color buffer
 glClear(GL_COLOR_BUFFER_BIT);
 // Use the program object
 glUseProgram(userData->programObject);
 // Load the vertex data
 glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
 glEnableVertexAttribArray(0);
 glDrawArrays(GL_TRIANGLES, 0, 3);
 eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}
int main(int argc, char *argv[])
{
 ESContext esContext;
 UserData userData;
 esInitialize(&esContext);
 esContext.userData = &userData;
 esCreateWindow(&esContext, "Hello Triangle", 320, 240,
 ES_WINDOW_RGB);

 if(!Init(&esContext))
 return 0;
 esRegisterDrawFunc(&esContext, Draw);

 esMainLoop(&esContext);
}
