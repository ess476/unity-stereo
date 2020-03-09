#include "RenderAPI.h"
#include "PlatformBase.h"

// OpenGL Core profile (desktop) or OpenGL ES (mobile) implementation of RenderAPI.
// Supports several flavors: Core, ES2, ES3


//#if SUPPORT_OPENGL_UNIFIED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xlib.h>


unsigned int width = 1920, height = 1080;

static int att[] = { 
	GLX_LEVEL, 0,
	GLX_DOUBLEBUFFER,
	GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_STEREO,
	None
};

Display                 *dpy;
Window                  root;
XVisualInfo             *vi;
XSetWindowAttributes    swa;
Window                  win;
GLXContext              glc;

Display* unityDisplay;
GLXDrawable unityWindow;
GLXContext unityContext;

class RenderAPI_OpenGLCoreES : public RenderAPI
{
public:
	RenderAPI_OpenGLCoreES(UnityGfxRenderer apiType);
	virtual ~RenderAPI_OpenGLCoreES() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ() { return false; }

	virtual void Draw(GLuint ltex, GLuint rtex);

private:
	void CreateResources(GLXContext unityContext);

private:
	UnityGfxRenderer m_APIType;
	GLuint texture_id;
};


RenderAPI* CreateRenderAPI_OpenGLCoreES(UnityGfxRenderer apiType)
{
	return new RenderAPI_OpenGLCoreES(apiType);
}

void RenderAPI_OpenGLCoreES::CreateResources(GLXContext unityContext)
{
	XEvent xev;

	dpy = XOpenDisplay(NULL);
	if(dpy == NULL) {
		printf("\n\tcannot open display\n\n");
		exit(0); 
	}
		
	root = DefaultRootWindow(dpy);

	vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(0); 
	}
		
	swa.event_mask = ExposureMask | KeyPressMask;
	swa.colormap   = XCreateColormap(dpy, root, vi->visual, AllocNone);

	win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWEventMask  | CWColormap, &swa);
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "Yurt Window");

	glc = glXCreateContext(dpy, vi, unityContext, GL_TRUE);

	if(glc == NULL) {
		printf("\n\tcannot create gl context\n\n");
		exit(0); 
	}

	glXMakeCurrent(dpy, win, glc);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}


RenderAPI_OpenGLCoreES::RenderAPI_OpenGLCoreES(UnityGfxRenderer apiType)
	: m_APIType(apiType)
{
}


void RenderAPI_OpenGLCoreES::ProcessDeviceEvent(UnityGfxDeviceEventType type, IUnityInterfaces* interfaces)
{
	if (type == kUnityGfxDeviceEventInitialize)
	{
		// Get Unity window
		unityDisplay = glXGetCurrentDisplay();
		unityWindow = glXGetCurrentReadDrawable();
		unityContext = glXGetCurrentContext();

		// Initialize window and resources
		CreateResources(unityContext);

		// Switch back to Unity
		glXMakeCurrent(unityDisplay, unityWindow, unityContext);
	}
	else if (type == kUnityGfxDeviceEventShutdown)
	{
		glXMakeCurrent(dpy, None, NULL);
		glXDestroyContext(dpy, glc);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		exit(0); 
	}
}

void RenderAPI_OpenGLCoreES::Draw(GLuint ltex, GLuint rtex)
{
	// Switch to rendering window
	glXMakeCurrent(dpy, win, glc);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Left eye
	glDrawBuffer(GL_BACK_LEFT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, ltex);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glEnd();

	// Right eye
	glDrawBuffer(GL_BACK_RIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, rtex);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0,  1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f( 1.0,  1.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f( 1.0, -1.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, 0.0);
	glEnd();

	// Swap buffers
	glXSwapBuffers(dpy, win);

	// Switch back to Unity
	glXMakeCurrent(unityDisplay, unityWindow, unityContext);
}

//#endif // #if SUPPORT_OPENGL_UNIFIED
