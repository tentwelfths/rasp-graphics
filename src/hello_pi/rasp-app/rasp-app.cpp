#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#include "bcm_host.h"

#include "GLES/gl.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

class GraphicsSystem
{
public:
	void Initialize()
	{
		int32_t success = 0;
		EGLBoolean result;
		EGLint num_config;

		static EGL_DISPMANX_WINDOW_T nativewindow;

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

		EGLConfig config;

		// get an EGL display connection
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		assert(display!=EGL_NO_DISPLAY);

		// initialize the EGL display connection
		result = eglInitialize(display, NULL, NULL);
		assert(EGL_FALSE != result);

		// get an appropriate EGL frame buffer configuration
		result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
		assert(EGL_FALSE != result);

		// create an EGL rendering context
		context = eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
		assert(context!=EGL_NO_CONTEXT);

		// create an EGL window surface
		success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
		assert( success >= 0 );

		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.width = screen_width;
		dst_rect.height = screen_height;
		  
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.width = screen_width << 16;
		src_rect.height = screen_height << 16;        

		dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
		dispman_update = vc_dispmanx_update_start( 0 );
		     
		state->dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
		  0/*layer*/, &dst_rect, 0/*src*/,
		  &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
		  
		nativewindow.element = dispman_element;
		nativewindow.width =   screen_width;
		nativewindow.height =  screen_height;
		vc_dispmanx_update_submit_sync( dispman_update );
		  
		state->surface = eglCreateWindowSurface( display, config, &nativewindow, NULL );
		assert(surface != EGL_NO_SURFACE);

		// connect the context to the surface
		result = eglMakeCurrent(display, surface, surface, context);
		assert(EGL_FALSE != result);

		// Set background color and clear buffers
		glClearColor(0.15f, 0.25f, 0.35f, 1.0f);

		// Enable back face culling.
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		glMatrixMode(GL_MODELVIEW);

		float nearp = 1.0f;
		float farp = 500.0f;
		float hht;
		float hwd;

		glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

		glViewport(0, 0, (GLsizei)state->   glLoadIdentity();

		hht = nearp * (float)tan(45.0 / 2.0 / 180.0 * M_PI);
		hwd = hht * (float)state->screen_width / (float)state->screen_height;

		glFrustumf(-hwd, hwd, -hht, hht, nearp, farp);

		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 3, GL_BYTE, 0, quadx );
		screen_width, (GLsizei)state->screen_height);
		  
		glMatrixMode(GL_PROJECTION);

		
	}
private:
	uint32_t screen_width;
	uint32_t screen_height;
	// OpenGL|ES objects
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
};

int main(void)
{
	bcm_host_init();



	return 0;
}