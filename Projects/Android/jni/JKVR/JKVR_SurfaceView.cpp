/************************************************************************************

Filename	:	JKVR_SurfaceView.c based on VrCubeWorld_SurfaceView.c
Content		:	This sample uses a plain Android SurfaceView and handles all
				Activity and Surface life cycle events in native code. This sample
				does not use the application framework and also does not use LibOVR.
				This sample only uses the VrApi.
Created		:	March, 2015
Authors		:	J.M.P. van Waveren / Simon Brown

Copyright	:	Copyright 2015 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>					// for prctl( PR_SET_NAME )
#include <android/log.h>
#include <android/native_window_jni.h>	// for native window JNI
#include <android/input.h>

#include "argtable3.h"
#include "VrInput.h"
#include "VrCvars.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES/gl2ext.h>

extern "C" {
#include "src/gl/loader.h"
}

#include <client/client.h>

#include "VrCompositor.h"
#include "VrInput.h"

#if !defined( EGL_OPENGL_ES3_BIT_KHR )
#define EGL_OPENGL_ES3_BIT_KHR		0x0040
#endif

// EXT_texture_border_clamp
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER			0x812D
#endif

#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR		0x1004
#endif

#ifndef GLAPI
#define GLAPI extern
#endif
//#define ENABLE_GL_DEBUG
#define ENABLE_GL_DEBUG_VERBOSE 1


// Must use EGLSyncKHR because the VrApi still supports OpenGL ES 2.0
#define EGL_SYNC

#if defined EGL_SYNC
// EGL_KHR_reusable_sync
PFNEGLCREATESYNCKHRPROC			eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC		eglDestroySyncKHR;
PFNEGLCLIENTWAITSYNCKHRPROC		eglClientWaitSyncKHR;
PFNEGLSIGNALSYNCKHRPROC			eglSignalSyncKHR;
PFNEGLGETSYNCATTRIBKHRPROC		eglGetSyncAttribKHR;
#endif

//Let's go to the maximum!
int CPU_LEVEL			= 4;
int GPU_LEVEL			= 4;
int NUM_MULTI_SAMPLES	= 1;
int REFRESH	            = 0;
float SS_MULTIPLIER    = 0.0f;

jclass clazz;

float radians(float deg) {
	return (deg * M_PI) / 180.0;
}

float degrees(float rad) {
	return (rad * 180.0) / M_PI;
}

/* global arg_xxx structs */
struct arg_dbl *ss;
struct arg_int *cpu;
struct arg_int *gpu;
struct arg_int *msaa;
struct arg_int *refresh;
struct arg_end *end;

char **argv;
int argc=0;


const char* const requiredExtensionNames[] = {
		XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME,
		XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME,
		XR_KHR_ANDROID_THREAD_SETTINGS_EXTENSION_NAME,
		XR_KHR_COMPOSITION_LAYER_CYLINDER_EXTENSION_NAME,
		XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME};
const uint32_t numRequiredExtensions =
		sizeof(requiredExtensionNames) / sizeof(requiredExtensionNames[0]);


/*
================================================================================

System Clock Time in millis

================================================================================
*/

double GetTimeInMilliSeconds()
{
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );
	return ( now.tv_sec * 1e9 + now.tv_nsec ) * (double)(1e-6);
}

/*
================================================================================

LAMBDA1VR Stuff

================================================================================
*/

bool JKVR_useScreenLayer()
{
	vr.using_screen_layer = (bool)((vr.cin_camera && !vr.immersive_cinematics) ||
			vr.misc_camera ||
			(CL_IsRunningInGameCinematic() || CL_InGameCinematicOnStandBy()) ||
            (cls.state == CA_CINEMATIC) ||
            (cls.state == CA_LOADING) ||
            ( Key_GetCatcher( ) & KEYCATCH_UI ) ||
            ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ));

	return vr.using_screen_layer;
}

int runStatus = -1;
void JKVR_exit(int exitCode)
{
	runStatus = exitCode;
}

static void UnEscapeQuotes( char *arg )
{
	char *last = NULL;
	while( *arg ) {
		if( *arg == '"' && *last == '\\' ) {
			char *c_curr = arg;
			char *c_last = last;
			while( *c_curr ) {
				*c_last = *c_curr;
				c_last = c_curr;
				c_curr++;
			}
			*c_last = '\0';
		}
		last = arg;
		arg++;
	}
}

static int ParseCommandLine(char *cmdline, char **argv)
{
	char *bufp;
	char *lastp = NULL;
	int argc, last_argc;
	argc = last_argc = 0;
	for ( bufp = cmdline; *bufp; ) {
		while ( isspace(*bufp) ) {
			++bufp;
		}
		if ( *bufp == '"' ) {
			++bufp;
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			while ( *bufp && ( *bufp != '"' || *lastp == '\\' ) ) {
				lastp = bufp;
				++bufp;
			}
		} else {
			if ( *bufp ) {
				if ( argv ) {
					argv[argc] = bufp;
				}
				++argc;
			}
			while ( *bufp && ! isspace(*bufp) ) {
				++bufp;
			}
		}
		if ( *bufp ) {
			if ( argv ) {
				*bufp = '\0';
			}
			++bufp;
		}
		if( argv && last_argc != argc ) {
			UnEscapeQuotes( argv[last_argc] );
		}
		last_argc = argc;
	}
	if ( argv ) {
		argv[argc] = NULL;
	}
	return(argc);
}

/*
================================================================================

OpenGL-ES Utility Functions

================================================================================
*/

typedef struct
{
	bool multi_view;						// GL_OVR_multiview, GL_OVR_multiview2
	bool EXT_texture_border_clamp;			// GL_EXT_texture_border_clamp, GL_OES_texture_border_clamp
} OpenGLExtensions_t;

OpenGLExtensions_t glExtensions;

static void EglInitExtensions()
{
#if defined EGL_SYNC
	eglCreateSyncKHR		= (PFNEGLCREATESYNCKHRPROC)			eglGetProcAddress( "eglCreateSyncKHR" );
	eglDestroySyncKHR		= (PFNEGLDESTROYSYNCKHRPROC)		eglGetProcAddress( "eglDestroySyncKHR" );
	eglClientWaitSyncKHR	= (PFNEGLCLIENTWAITSYNCKHRPROC)		eglGetProcAddress( "eglClientWaitSyncKHR" );
	eglSignalSyncKHR		= (PFNEGLSIGNALSYNCKHRPROC)			eglGetProcAddress( "eglSignalSyncKHR" );
	eglGetSyncAttribKHR		= (PFNEGLGETSYNCATTRIBKHRPROC)		eglGetProcAddress( "eglGetSyncAttribKHR" );
#endif

	const char * allExtensions = (const char *)glGetString( GL_EXTENSIONS );
	if ( allExtensions != NULL )
	{
		glExtensions.multi_view = strstr( allExtensions, "GL_OVR_multiview2" ) &&
								  strstr( allExtensions, "GL_OVR_multiview_multisampled_render_to_texture" );

		glExtensions.EXT_texture_border_clamp = false;//strstr( allExtensions, "GL_EXT_texture_border_clamp" ) ||
												//strstr( allExtensions, "GL_OES_texture_border_clamp" );
	}
}

static const char * EglErrorString( const EGLint error )
{
	switch ( error )
	{
		case EGL_SUCCESS:				return "EGL_SUCCESS";
		case EGL_NOT_INITIALIZED:		return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS:			return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC:				return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE:			return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONTEXT:			return "EGL_BAD_CONTEXT";
		case EGL_BAD_CONFIG:			return "EGL_BAD_CONFIG";
		case EGL_BAD_CURRENT_SURFACE:	return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY:			return "EGL_BAD_DISPLAY";
		case EGL_BAD_SURFACE:			return "EGL_BAD_SURFACE";
		case EGL_BAD_MATCH:				return "EGL_BAD_MATCH";
		case EGL_BAD_PARAMETER:			return "EGL_BAD_PARAMETER";
		case EGL_BAD_NATIVE_PIXMAP:		return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW:		return "EGL_BAD_NATIVE_WINDOW";
		case EGL_CONTEXT_LOST:			return "EGL_CONTEXT_LOST";
		default:						return "unknown";
	}
}

static const char * GlFrameBufferStatusString( GLenum status )
{
	switch ( status )
	{
		case GL_FRAMEBUFFER_UNDEFINED:						return "GL_FRAMEBUFFER_UNDEFINED";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_UNSUPPORTED:					return "GL_FRAMEBUFFER_UNSUPPORTED";
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
		default:											return "unknown";
	}
}


/*
================================================================================

ovrEgl

================================================================================
*/

typedef struct
{
	EGLint		MajorVersion;
	EGLint		MinorVersion;
	EGLDisplay	Display;
	EGLConfig	Config;
	EGLSurface	TinySurface;
	EGLSurface	MainSurface;
	EGLContext	Context;
} ovrEgl;

static void ovrEgl_Clear( ovrEgl * egl )
{
	egl->MajorVersion = 0;
	egl->MinorVersion = 0;
	egl->Display = 0;
	egl->Config = 0;
	egl->TinySurface = EGL_NO_SURFACE;
	egl->MainSurface = EGL_NO_SURFACE;
	egl->Context = EGL_NO_CONTEXT;
}

static void ovrEgl_CreateContext( ovrEgl * egl, const ovrEgl * shareEgl )
{
	if ( egl->Display != 0 )
	{
		return;
	}

	egl->Display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	ALOGV( "        eglInitialize( Display, &MajorVersion, &MinorVersion )" );
	eglInitialize( egl->Display, &egl->MajorVersion, &egl->MinorVersion );
	// Do NOT use eglChooseConfig, because the Android EGL code pushes in multisample
	// flags in eglChooseConfig if the user has selected the "force 4x MSAA" option in
	// settings, and that is completely wasted for our warp target.
	const int MAX_CONFIGS = 1024;
	EGLConfig configs[MAX_CONFIGS];
	EGLint numConfigs = 0;
	if ( eglGetConfigs( egl->Display, configs, MAX_CONFIGS, &numConfigs ) == EGL_FALSE )
	{
		ALOGE( "        eglGetConfigs() failed: %s", EglErrorString( eglGetError() ) );
		return;
	}
	const EGLint configAttribs[] =
	{
		EGL_RED_SIZE,		8,
		EGL_GREEN_SIZE,		8,
		EGL_BLUE_SIZE,		8,
		EGL_ALPHA_SIZE,		8, // need alpha for the multi-pass timewarp compositor
		EGL_DEPTH_SIZE,		0,
		EGL_STENCIL_SIZE,	0,
		EGL_SAMPLES,		0,
		EGL_NONE
	};
	egl->Config = 0;
	for ( int i = 0; i < numConfigs; i++ )
	{
		EGLint value = 0;

		eglGetConfigAttrib( egl->Display, configs[i], EGL_RENDERABLE_TYPE, &value );
		if ( ( value & EGL_OPENGL_ES3_BIT_KHR ) != EGL_OPENGL_ES3_BIT_KHR )
		{
			continue;
		}

		// The pbuffer config also needs to be compatible with normal window rendering
		// so it can share textures with the window context.
		eglGetConfigAttrib( egl->Display, configs[i], EGL_SURFACE_TYPE, &value );
		if ( ( value & ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) ) != ( EGL_WINDOW_BIT | EGL_PBUFFER_BIT ) )
		{
			continue;
		}

		int	j = 0;
		for ( ; configAttribs[j] != EGL_NONE; j += 2 )
		{
			eglGetConfigAttrib( egl->Display, configs[i], configAttribs[j], &value );
			if ( value != configAttribs[j + 1] )
			{
				break;
			}
		}
		if ( configAttribs[j] == EGL_NONE )
		{
			egl->Config = configs[i];
			break;
		}
	}
	if ( egl->Config == 0 )
	{
		ALOGE( "        eglChooseConfig() failed: %s", EglErrorString( eglGetError() ) );
		return;
	}
	EGLint contextAttribs[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	ALOGV( "        Context = eglCreateContext( Display, Config, EGL_NO_CONTEXT, contextAttribs )" );
	egl->Context = eglCreateContext( egl->Display, egl->Config, ( shareEgl != NULL ) ? shareEgl->Context : EGL_NO_CONTEXT, contextAttribs );
	if ( egl->Context == EGL_NO_CONTEXT )
	{
		ALOGE( "        eglCreateContext() failed: %s", EglErrorString( eglGetError() ) );
		return;
	}
	const EGLint surfaceAttribs[] =
	{
		EGL_WIDTH, 16,
		EGL_HEIGHT, 16,
		EGL_NONE
	};
	ALOGV( "        TinySurface = eglCreatePbufferSurface( Display, Config, surfaceAttribs )" );
	egl->TinySurface = eglCreatePbufferSurface( egl->Display, egl->Config, surfaceAttribs );
	if ( egl->TinySurface == EGL_NO_SURFACE )
	{
		ALOGE( "        eglCreatePbufferSurface() failed: %s", EglErrorString( eglGetError() ) );
		eglDestroyContext( egl->Display, egl->Context );
		egl->Context = EGL_NO_CONTEXT;
		return;
	}
	ALOGV( "        eglMakeCurrent( Display, TinySurface, TinySurface, Context )" );
	if ( eglMakeCurrent( egl->Display, egl->TinySurface, egl->TinySurface, egl->Context ) == EGL_FALSE )
	{
		ALOGE( "        eglMakeCurrent() failed: %s", EglErrorString( eglGetError() ) );
		eglDestroySurface( egl->Display, egl->TinySurface );
		eglDestroyContext( egl->Display, egl->Context );
		egl->Context = EGL_NO_CONTEXT;
		return;
	}
}

static void ovrEgl_DestroyContext( ovrEgl * egl )
{
	if ( egl->Display != 0 )
	{
		ALOGE( "        eglMakeCurrent( Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT )" );
		if ( eglMakeCurrent( egl->Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ) == EGL_FALSE )
		{
			ALOGE( "        eglMakeCurrent() failed: %s", EglErrorString( eglGetError() ) );
		}
	}
	if ( egl->Context != EGL_NO_CONTEXT )
	{
		ALOGE( "        eglDestroyContext( Display, Context )" );
		if ( eglDestroyContext( egl->Display, egl->Context ) == EGL_FALSE )
		{
			ALOGE( "        eglDestroyContext() failed: %s", EglErrorString( eglGetError() ) );
		}
		egl->Context = EGL_NO_CONTEXT;
	}
	if ( egl->TinySurface != EGL_NO_SURFACE )
	{
		ALOGE( "        eglDestroySurface( Display, TinySurface )" );
		if ( eglDestroySurface( egl->Display, egl->TinySurface ) == EGL_FALSE )
		{
			ALOGE( "        eglDestroySurface() failed: %s", EglErrorString( eglGetError() ) );
		}
		egl->TinySurface = EGL_NO_SURFACE;
	}
	if ( egl->Display != 0 )
	{
		ALOGE( "        eglTerminate( Display )" );
		if ( eglTerminate( egl->Display ) == EGL_FALSE )
		{
			ALOGE( "        eglTerminate() failed: %s", EglErrorString( eglGetError() ) );
		}
		egl->Display = 0;
	}
}

/*
================================================================================

ovrFramebuffer

================================================================================
*/


static void ovrFramebuffer_Clear(ovrFramebuffer* frameBuffer) {
	frameBuffer->Width = 0;
	frameBuffer->Height = 0;
	frameBuffer->TextureSwapChainLength = 0;
	frameBuffer->TextureSwapChainIndex = 0;
	frameBuffer->ColorSwapChain.Handle = XR_NULL_HANDLE;
	frameBuffer->ColorSwapChain.Width = 0;
	frameBuffer->ColorSwapChain.Height = 0;
	frameBuffer->ColorSwapChainImage = NULL;
	frameBuffer->DepthBuffers = NULL;
	frameBuffer->FrameBuffers = NULL;
}

static bool ovrFramebuffer_Create( XrSession session,
								   ovrFramebuffer * frameBuffer, const GLenum colorFormat, const int width, const int height )
{
	frameBuffer->Width = width;
	frameBuffer->Height = height;

	frameBuffer->DepthBuffers = (GLuint *)malloc( frameBuffer->TextureSwapChainLength * sizeof( GLuint ) );
	frameBuffer->FrameBuffers = (GLuint *)malloc( frameBuffer->TextureSwapChainLength * sizeof( GLuint ) );

	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
			(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress("glRenderbufferStorageMultisampleEXT");
	PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
			(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");

	XrSwapchainCreateInfo swapChainCreateInfo;
	memset(&swapChainCreateInfo, 0, sizeof(swapChainCreateInfo));
	swapChainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
	swapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.format = GL_RGBA8;
	swapChainCreateInfo.sampleCount = 1;
	swapChainCreateInfo.width = width;
	swapChainCreateInfo.height = height;
	swapChainCreateInfo.faceCount = 1;
	swapChainCreateInfo.arraySize = 2;
	swapChainCreateInfo.mipCount = 1;

	frameBuffer->ColorSwapChain.Width = swapChainCreateInfo.width;
	frameBuffer->ColorSwapChain.Height = swapChainCreateInfo.height;

	// Create the swapchain.
	OXR(xrCreateSwapchain(session, &swapChainCreateInfo, &frameBuffer->ColorSwapChain.Handle));
	// Get the number of swapchain images.
	OXR(xrEnumerateSwapchainImages(
			frameBuffer->ColorSwapChain.Handle, 0, &frameBuffer->TextureSwapChainLength, NULL));
	// Allocate the swapchain images array.
	frameBuffer->ColorSwapChainImage = (XrSwapchainImageOpenGLESKHR*)malloc(
			frameBuffer->TextureSwapChainLength * sizeof(XrSwapchainImageOpenGLESKHR));

	// Populate the swapchain image array.
	for (uint32_t i = 0; i < frameBuffer->TextureSwapChainLength; i++) {
		frameBuffer->ColorSwapChainImage[i].type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR;
		frameBuffer->ColorSwapChainImage[i].next = NULL;
	}
	OXR(xrEnumerateSwapchainImages(
			frameBuffer->ColorSwapChain.Handle,
			frameBuffer->TextureSwapChainLength,
			&frameBuffer->TextureSwapChainLength,
			(XrSwapchainImageBaseHeader*)frameBuffer->ColorSwapChainImage));


	for ( int i = 0; i < frameBuffer->TextureSwapChainLength; i++ )
	{
		// Create the color buffer texture.
		const GLuint colorTexture = frameBuffer->ColorSwapChainImage[i].image;

		GL(glGenRenderbuffers(1, &frameBuffer->DepthBuffers[i]));
		GL(glBindRenderbuffer(GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));
		GL(glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 1, GL_DEPTH_COMPONENT24, width, height));
		GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));

		// Create the frame buffer.
		GL(glGenFramebuffers(1, &frameBuffer->FrameBuffers[i]));
		GL(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->FrameBuffers[i]));
		GL(glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0, 1));
		GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBuffer->DepthBuffers[i]));

		GL(GLenum renderFramebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
		GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		if (renderFramebufferStatus != GL_FRAMEBUFFER_COMPLETE)
		{
			ALOGE("OVRHelper::Incomplete frame buffer object: %s", GlFrameBufferStatusString(renderFramebufferStatus));
			return false;
		}
	}

	return true;
}

void ovrFramebuffer_Destroy( ovrFramebuffer * frameBuffer )
{
    //LOAD_GLES2(glDeleteFramebuffers);
    //LOAD_GLES2(glDeleteRenderbuffers);

	GL( glDeleteFramebuffers( frameBuffer->TextureSwapChainLength, frameBuffer->FrameBuffers ) );
	GL( glDeleteRenderbuffers( frameBuffer->TextureSwapChainLength, frameBuffer->DepthBuffers ) );

	vrapi_DestroyTextureSwapChain( frameBuffer->ColorTextureSwapChain );

	free( frameBuffer->DepthBuffers );
	free( frameBuffer->FrameBuffers );

	ovrFramebuffer_Clear( frameBuffer );
}

void GPUWaitSync()
{
	GLsync syncBuff = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	GLenum status = glClientWaitSync(syncBuff, GL_SYNC_FLUSH_COMMANDS_BIT, 1000 * 1000 * 50); // Wait for a max of 50ms...
	if (status != GL_CONDITION_SATISFIED)
	{
		LOGE("Error on glClientWaitSync: %d\n", status);
	}
	glDeleteSync(syncBuff);
}

void ovrFramebuffer_SetCurrent( ovrFramebuffer * frameBuffer )
{
    //LOAD_GLES2(glBindFramebuffer);
	GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, frameBuffer->FrameBuffers[frameBuffer->TextureSwapChainIndex] ) );
}

void ovrFramebuffer_SetNone()
{
    //LOAD_GLES2(glBindFramebuffer);
	GL( glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
}

void ovrFramebuffer_Resolve( ovrFramebuffer * frameBuffer )
{
	// Discard the depth buffer, so the tiler won't need to write it back out to memory.
//	const GLenum depthAttachment[1] = { GL_DEPTH_ATTACHMENT };
//	glInvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, 1, depthAttachment );

    // Flush this frame worth of commands.
    glFlush();
}

void ovrFramebuffer_Advance( ovrFramebuffer * frameBuffer )
{
	// Advance to the next texture from the set.
	frameBuffer->TextureSwapChainIndex = ( frameBuffer->TextureSwapChainIndex + 1 ) % frameBuffer->TextureSwapChainLength;
}


void ovrFramebuffer_ClearEdgeTexels( ovrFramebuffer * frameBuffer )
{
	//LOAD_GLES2(glEnable);
	//LOAD_GLES2(glDisable);
	//LOAD_GLES2(glViewport);
	//LOAD_GLES2(glScissor);
	//LOAD_GLES2(glClearColor);
	//LOAD_GLES2(glClear);

	GL( glEnable( GL_SCISSOR_TEST ) );
	GL( glViewport( 0, 0, frameBuffer->Width, frameBuffer->Height ) );

	// Explicitly clear the border texels to black because OpenGL-ES does not support GL_CLAMP_TO_BORDER.
	// Clear to fully opaque black.
	GL( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );

	// bottom
	GL( glScissor( 0, 0, frameBuffer->Width, 1 ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );
	// top
	GL( glScissor( 0, frameBuffer->Height - 1, frameBuffer->Width, 1 ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );
	// left
	GL( glScissor( 0, 0, 1, frameBuffer->Height ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );
	// right
	GL( glScissor( frameBuffer->Width - 1, 0, 1, frameBuffer->Height ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );


	GL( glScissor( 0, 0, 0, 0 ) );
	GL( glDisable( GL_SCISSOR_TEST ) );
}


/*
================================================================================

ovrRenderer

================================================================================
*/


void ovrRenderer_Clear( ovrRenderer * renderer )
{
	for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
	{
		ovrFramebuffer_Clear( &renderer->FrameBuffer[eye] );
	}
	renderer->ProjectionMatrix = ovrMatrix4f_CreateIdentity();
	renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;
}


void ovrRenderer_Create( int width, int height, ovrRenderer * renderer )
{
	renderer->NumBuffers = VRAPI_FRAME_LAYER_EYE_MAX;

	// Create the render Textures.
	for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
	{
		ovrFramebuffer_Create( &renderer->FrameBuffer[eye],
							   GL_RGBA8,
							   width,
							   height,
							   NUM_MULTI_SAMPLES );
	}
}

void ovrRenderer_Destroy( ovrRenderer * renderer )
{
	for ( int eye = 0; eye < renderer->NumBuffers; eye++ )
	{
		ovrFramebuffer_Destroy( &renderer->FrameBuffer[eye] );
	}
	renderer->ProjectionMatrix = ovrMatrix4f_CreateIdentity();
}


#ifndef EPSILON
#define EPSILON 0.001f
#endif

static ovrVector3f normalizeVec(ovrVector3f vec) {
    //NOTE: leave w-component untouched
    //@@const float EPSILON = 0.000001f;
    float xxyyzz = vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
    //@@if(xxyyzz < EPSILON)
    //@@    return *this; // do nothing if it is zero vector

    //float invLength = invSqrt(xxyyzz);
    ovrVector3f result;
    float invLength = 1.0f / sqrtf(xxyyzz);
    result.x = vec.x * invLength;
    result.y = vec.y * invLength;
    result.z = vec.z * invLength;
    return result;
}

void NormalizeAngles(vec3_t angles)
{
	while (angles[0] >= 90) angles[0] -= 180;
	while (angles[1] >= 180) angles[1] -= 360;
	while (angles[2] >= 180) angles[2] -= 360;
	while (angles[0] < -90) angles[0] += 180;
	while (angles[1] < -180) angles[1] += 360;
	while (angles[2] < -180) angles[2] += 360;
}

void GetAnglesFromVectors(const ovrVector3f forward, const ovrVector3f right, const ovrVector3f up, vec3_t angles)
{
	float sr, sp, sy, cr, cp, cy;

	sp = -forward.z;

	float cp_x_cy = forward.x;
	float cp_x_sy = forward.y;
	float cp_x_sr = -right.z;
	float cp_x_cr = up.z;

	float yaw = atan2(cp_x_sy, cp_x_cy);
	float roll = atan2(cp_x_sr, cp_x_cr);

	cy = cos(yaw);
	sy = sin(yaw);
	cr = cos(roll);
	sr = sin(roll);

	if (fabs(cy) > EPSILON)
	{
	cp = cp_x_cy / cy;
	}
	else if (fabs(sy) > EPSILON)
	{
	cp = cp_x_sy / sy;
	}
	else if (fabs(sr) > EPSILON)
	{
	cp = cp_x_sr / sr;
	}
	else if (fabs(cr) > EPSILON)
	{
	cp = cp_x_cr / cr;
	}
	else
	{
	cp = cos(asin(sp));
	}

	float pitch = atan2(sp, cp);

	angles[0] = pitch / (M_PI*2.f / 360.f);
	angles[1] = yaw / (M_PI*2.f / 360.f);
	angles[2] = roll / (M_PI*2.f / 360.f);

	NormalizeAngles(angles);
}


void QuatToYawPitchRoll(XrQuaternionf q, vec3_t rotation, vec3_t out) {

	ovrMatrix4f mat = ovrMatrix4f_CreateFromQuaternion( &q );

	if (rotation[0] != 0.0f || rotation[1] != 0.0f || rotation[2] != 0.0f)
	{
		ovrMatrix4f rot = ovrMatrix4f_CreateRotation(radians(rotation[0]), radians(rotation[1]), radians(rotation[2]));
		mat = ovrMatrix4f_Multiply(&mat, &rot);
	}

	XrVector4f v1 = {0, 0, -1, 0};
	XrVector4f v2 = {1, 0, 0, 0};
	XrVector4f v3 = {0, 1, 0, 0};

	XrVector4f forwardInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v1);
	XrVector4f rightInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v2);
	XrVector4f upInVRSpace = XrVector4f_MultiplyMatrix4f(&mat, &v3);

	XrVector3f forward = {-forwardInVRSpace.z, -forwardInVRSpace.x, forwardInVRSpace.y};
	XrVector3f right = {-rightInVRSpace.z, -rightInVRSpace.x, rightInVRSpace.y};
	XrVector3f up = {-upInVRSpace.z, -upInVRSpace.x, upInVRSpace.y};

	XrVector3f forwardNormal = normalizeVec(forward);
	XrVector3f rightNormal = normalizeVec(right);
	XrVector3f upNormal = normalizeVec(up);

	GetAnglesFromVectors(forwardNormal, rightNormal, upNormal, out);
}

void updateHMDOrientation()
{
	//Position
    VectorSubtract(vr.hmdposition_last, vr.hmdposition, vr.hmdposition_delta);

    //Keep this for our records
    VectorCopy(vr.hmdposition, vr.hmdposition_last);

	//Orientation
	VectorSubtract(vr.hmdorientation_last, vr.hmdorientation, vr.hmdorientation_delta);

	//Keep this for our records
	VectorCopy(vr.hmdorientation, vr.hmdorientation_last);

	if (!vr.third_person && !vr.remote_npc){
		VectorCopy(vr.hmdorientation, vr.hmdorientation_first);
	}

	if (!vr.remote_turret)
	{
		VectorCopy(vr.weaponangles, vr.weaponangles_first);
	}

	// View yaw delta
	float clientview_yaw = vr.clientviewangles[YAW] - vr.hmdorientation[YAW];
	vr.clientview_yaw_delta = vr.clientview_yaw_last - clientview_yaw;
	vr.clientview_yaw_last = clientview_yaw;
}

void setHMDPosition( float x, float y, float z )
{
	static bool s_useScreen = qfalse;

	VectorSet(vr.hmdposition, x, y, z);

    if (s_useScreen != JKVR_useScreenLayer())
    {
		s_useScreen = JKVR_useScreenLayer();

		//Record player position on transition
		VectorSet(vr.hmdposition_snap, x, y, z);
		VectorCopy(vr.hmdorientation, vr.hmdorientation_snap);
    }

	VectorSubtract(vr.hmdposition, vr.hmdposition_snap, vr.hmdposition_offset);
}

/*
========================
JKVR_Vibrate
========================
*/

//0 = left, 1 = right
float vibration_channel_duration[2] = {0.0f, 0.0f};
float vibration_channel_intensity[2] = {0.0f, 0.0f};

void JKVR_Vibrate( int duration, int chan, float intensity )
{
	for (int i = 0; i < 2; ++i)
	{
		int channel = (i + 1) & chan;
		if (channel)
		{
			if (vibration_channel_duration[channel-1] > 0.0f)
				return;

			if (vibration_channel_duration[channel-1] == -1.0f && duration != 0.0f)
				return;

			vibration_channel_duration[channel-1] = duration;
			vibration_channel_intensity[channel-1] = intensity * vr_haptic_intensity->value;
		}
	}
}

void JKVR_GetMove(float *forward, float *side, float *pos_forward, float *pos_side, float *up,
                    float *yaw, float *pitch, float *roll)
{
	if (vr.remote_turret) {
		*forward = 0.0f;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = 0.0f;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation_first[YAW] +
				vr.weaponangles[YAW] - vr.weaponangles_first[YAW];
		*pitch = vr.weaponangles[PITCH];
		*roll = 0.0f;
	}
	else if (vr.cgzoommode == 2 || vr.cgzoommode == 4)
	{
		*forward = 0.0f;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = 0.0f;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn;
		*pitch = vr.weaponangles[PITCH];
		*roll = vr.hmdorientation[ROLL];
	}
	else if (vr.remote_npc) {
		*forward = remote_movementForward;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation[YAW] - vr.hmdorientation_first[YAW];
		*pitch = vr.hmdorientation[PITCH];
		*roll = 0.0f;
	}
	else if (!vr.third_person) {
		*forward = remote_movementForward;
		*pos_forward = positional_movementForward;
		*up = remote_movementUp;
		*side = remote_movementSideways;
		*pos_side = positional_movementSideways;
		*yaw = vr.hmdorientation[YAW] + vr.snapTurn;
		*pitch = vr.hmdorientation[PITCH];
		*roll = vr.hmdorientation[ROLL];
	} else {
		//in third person just send the bare minimum
		*forward = remote_movementForward;
		*pos_forward = 0.0f;
		*up = 0.0f;
		*side = remote_movementSideways;
		*pos_side = 0.0f;
		*yaw = vr.snapTurn + vr.hmdorientation_first[YAW];
		*pitch = 0.0f;
		*roll = 0.0f;
	}
}

/*
================================================================================

ovrRenderThread

================================================================================
*/



void ovrApp_Clear(ovrApp* app) {
	app->Focused = false;
	app->Instance = XR_NULL_HANDLE;
	app->Session = XR_NULL_HANDLE;
	memset(&app->ViewportConfig, 0, sizeof(XrViewConfigurationProperties));
	memset(&app->ViewConfigurationView, 0, ovrMaxNumEyes * sizeof(XrViewConfigurationView));
	app->SystemId = XR_NULL_SYSTEM_ID;
	app->HeadSpace = XR_NULL_HANDLE;
	app->StageSpace = XR_NULL_HANDLE;
	app->FakeStageSpace = XR_NULL_HANDLE;
	app->CurrentSpace = XR_NULL_HANDLE;
	app->SessionActive = false;
	app->SupportedDisplayRefreshRates = NULL;
	app->RequestedDisplayRefreshRateIndex = 0;
	app->NumSupportedDisplayRefreshRates = 0;
	app->pfnGetDisplayRefreshRate = NULL;
	app->pfnRequestDisplayRefreshRate = NULL;
	app->SwapInterval = 1;
	memset(app->Layers, 0, sizeof(ovrCompositorLayer_Union) * ovrMaxLayerCount);
	app->LayerCount = 0;
	app->MainThreadTid = 0;
	app->RenderThreadTid = 0;
	ovrEgl_Clear( &app->Egl );
	ovrScene_Clear( &app->Scene );
	ovrRenderer_Clear(&app->Renderer);
}

static void ovrApp_PushBlackFinal( ovrApp * app )
{
	int frameFlags = 0;
	frameFlags |= VRAPI_FRAME_FLAG_FLUSH | VRAPI_FRAME_FLAG_FINAL;

	ovrLayerProjection2 layer = vrapi_DefaultLayerBlackProjection2();
	layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

	const ovrLayerHeader2 * layers[] =
	{
		&layer.Header
	};

	ovrSubmitFrameDescription2 frameDesc = {};
	frameDesc.Flags = frameFlags;
	frameDesc.SwapInterval = 1;
	frameDesc.FrameIndex = app->FrameIndex;
	frameDesc.DisplayTime = app->DisplayTime;
	frameDesc.LayerCount = 1;
	frameDesc.Layers = layers;

	vrapi_SubmitFrame2( app->Ovr, &frameDesc );
}



void ovrApp_HandleSessionStateChanges(ovrApp* app, XrSessionState state) {
	if (state == XR_SESSION_STATE_READY) {
		assert(app->SessionActive == false);

		XrSessionBeginInfo sessionBeginInfo;
		memset(&sessionBeginInfo, 0, sizeof(sessionBeginInfo));
		sessionBeginInfo.type = XR_TYPE_SESSION_BEGIN_INFO;
		sessionBeginInfo.next = NULL;
		sessionBeginInfo.primaryViewConfigurationType = app->ViewportConfig.viewConfigurationType;

		XrResult result;
		OXR(result = xrBeginSession(app->Session, &sessionBeginInfo));

		app->SessionActive = (result == XR_SUCCESS);

		// Set session state once we have entered VR mode and have a valid session object.
		if (app->SessionActive) {
			XrPerfSettingsLevelEXT cpuPerfLevel = XR_PERF_SETTINGS_LEVEL_BOOST_EXT;
			XrPerfSettingsLevelEXT gpuPerfLevel = XR_PERF_SETTINGS_LEVEL_BOOST_EXT;

			PFN_xrPerfSettingsSetPerformanceLevelEXT pfnPerfSettingsSetPerformanceLevelEXT = NULL;
			OXR(xrGetInstanceProcAddr(
					app->Instance,
					"xrPerfSettingsSetPerformanceLevelEXT",
					(PFN_xrVoidFunction*)(&pfnPerfSettingsSetPerformanceLevelEXT)));

			OXR(pfnPerfSettingsSetPerformanceLevelEXT(
					app->Session, XR_PERF_SETTINGS_DOMAIN_CPU_EXT, cpuPerfLevel));
			OXR(pfnPerfSettingsSetPerformanceLevelEXT(
					app->Session, XR_PERF_SETTINGS_DOMAIN_GPU_EXT, gpuPerfLevel));

			PFN_xrSetAndroidApplicationThreadKHR pfnSetAndroidApplicationThreadKHR = NULL;
			OXR(xrGetInstanceProcAddr(
					app->Instance,
					"xrSetAndroidApplicationThreadKHR",
					(PFN_xrVoidFunction*)(&pfnSetAndroidApplicationThreadKHR)));

			OXR(pfnSetAndroidApplicationThreadKHR(
					app->Session, XR_ANDROID_THREAD_TYPE_APPLICATION_MAIN_KHR, app->MainThreadTid));
			OXR(pfnSetAndroidApplicationThreadKHR(
					app->Session, XR_ANDROID_THREAD_TYPE_RENDERER_MAIN_KHR, app->RenderThreadTid));
		}
	} else if (state == XR_SESSION_STATE_STOPPING) {
		assert(app->SessionActive);

		OXR(xrEndSession(app->Session));
		app->SessionActive = false;
	}
}

GLboolean ovrApp_HandleXrEvents(ovrApp* app) {
	XrEventDataBuffer eventDataBuffer = {};
	GLboolean recenter = GL_FALSE;

	// Poll for events
	for (;;) {
		XrEventDataBaseHeader* baseEventHeader = (XrEventDataBaseHeader*)(&eventDataBuffer);
		baseEventHeader->type = XR_TYPE_EVENT_DATA_BUFFER;
		baseEventHeader->next = NULL;
		XrResult r;
		OXR(r = xrPollEvent(app->Instance, &eventDataBuffer));
		if (r != XR_SUCCESS) {
			break;
		}

		switch (baseEventHeader->type) {
			case XR_TYPE_EVENT_DATA_EVENTS_LOST:
				ALOGV("xrPollEvent: received XR_TYPE_EVENT_DATA_EVENTS_LOST event");
				break;
			case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
				const XrEventDataInstanceLossPending* instance_loss_pending_event =
						(XrEventDataInstanceLossPending*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING event: time %f",
						FromXrTime(instance_loss_pending_event->lossTime));
			} break;
			case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
				ALOGV("xrPollEvent: received XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED event");
				break;
			case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT: {
				const XrEventDataPerfSettingsEXT* perf_settings_event =
						(XrEventDataPerfSettingsEXT*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT event: type %d subdomain %d : level %d -> level %d",
						perf_settings_event->type,
						perf_settings_event->subDomain,
						perf_settings_event->fromLevel,
						perf_settings_event->toLevel);
			} break;
			case XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB: {
				const XrEventDataDisplayRefreshRateChangedFB* refresh_rate_changed_event =
						(XrEventDataDisplayRefreshRateChangedFB*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB event: fromRate %f -> toRate %f",
						refresh_rate_changed_event->fromDisplayRefreshRate,
						refresh_rate_changed_event->toDisplayRefreshRate);
			} break;
			case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
				XrEventDataReferenceSpaceChangePending* ref_space_change_event =
						(XrEventDataReferenceSpaceChangePending*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING event: changed space: %d for session %p at time %f",
						ref_space_change_event->referenceSpaceType,
						(void*)ref_space_change_event->session,
						FromXrTime(ref_space_change_event->changeTime));
				recenter = GL_TRUE;
			} break;
			case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
				const XrEventDataSessionStateChanged* session_state_changed_event =
						(XrEventDataSessionStateChanged*)(baseEventHeader);
				ALOGV(
						"xrPollEvent: received XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: %d for session %p at time %f",
						session_state_changed_event->state,
						(void*)session_state_changed_event->session,
						FromXrTime(session_state_changed_event->time));

				switch (session_state_changed_event->state) {
					case XR_SESSION_STATE_FOCUSED:
						app->Focused = true;
						break;
					case XR_SESSION_STATE_VISIBLE:
						app->Focused = false;
						break;
					case XR_SESSION_STATE_READY:
					case XR_SESSION_STATE_STOPPING:
						ovrApp_HandleSessionStateChanges(app, session_state_changed_event->state);
						break;
					default:
						break;
				}
			} break;
			default:
				ALOGV("xrPollEvent: Unknown event");
				break;
		}
	}
	return recenter;
}


/*
================================================================================

ovrMessageQueue

================================================================================
*/

typedef enum
{
	MQ_WAIT_NONE,		// don't wait
	MQ_WAIT_RECEIVED,	// wait until the consumer thread has received the message
	MQ_WAIT_PROCESSED	// wait until the consumer thread has processed the message
} ovrMQWait;

#define MAX_MESSAGE_PARMS	8
#define MAX_MESSAGES		1024

typedef struct
{
	int			Id;
	ovrMQWait	Wait;
	long long	Parms[MAX_MESSAGE_PARMS];
} ovrMessage;

static void ovrMessage_Init( ovrMessage * message, const int id, const int wait )
{
	message->Id = id;
	message->Wait = (ovrMQWait)wait;
	memset( message->Parms, 0, sizeof( message->Parms ) );
}

static void		ovrMessage_SetPointerParm( ovrMessage * message, int index, void * ptr ) { *(void **)&message->Parms[index] = ptr; }
static void *	ovrMessage_GetPointerParm( ovrMessage * message, int index ) { return *(void **)&message->Parms[index]; }
static void		ovrMessage_SetIntegerParm( ovrMessage * message, int index, int value ) { message->Parms[index] = value; }
static int		ovrMessage_GetIntegerParm( ovrMessage * message, int index ) { return (int)message->Parms[index]; }
static void		ovrMessage_SetFloatParm( ovrMessage * message, int index, float value ) { *(float *)&message->Parms[index] = value; }
static float	ovrMessage_GetFloatParm( ovrMessage * message, int index ) { return *(float *)&message->Parms[index]; }

// Cyclic queue with messages.
typedef struct
{
	ovrMessage	 		Messages[MAX_MESSAGES];
	volatile int		Head;	// dequeue at the head
	volatile int		Tail;	// enqueue at the tail
	ovrMQWait			Wait;
	volatile bool		EnabledFlag;
	volatile bool		PostedFlag;
	volatile bool		ReceivedFlag;
	volatile bool		ProcessedFlag;
	pthread_mutex_t		Mutex;
	pthread_cond_t		PostedCondition;
	pthread_cond_t		ReceivedCondition;
	pthread_cond_t		ProcessedCondition;
} ovrMessageQueue;

static void ovrMessageQueue_Create( ovrMessageQueue * messageQueue )
{
	messageQueue->Head = 0;
	messageQueue->Tail = 0;
	messageQueue->Wait = MQ_WAIT_NONE;
	messageQueue->EnabledFlag = false;
	messageQueue->PostedFlag = false;
	messageQueue->ReceivedFlag = false;
	messageQueue->ProcessedFlag = false;

	pthread_mutexattr_t	attr;
	pthread_mutexattr_init( &attr );
	pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
	pthread_mutex_init( &messageQueue->Mutex, &attr );
	pthread_mutexattr_destroy( &attr );
	pthread_cond_init( &messageQueue->PostedCondition, NULL );
	pthread_cond_init( &messageQueue->ReceivedCondition, NULL );
	pthread_cond_init( &messageQueue->ProcessedCondition, NULL );
}

static void ovrMessageQueue_Destroy( ovrMessageQueue * messageQueue )
{
	pthread_mutex_destroy( &messageQueue->Mutex );
	pthread_cond_destroy( &messageQueue->PostedCondition );
	pthread_cond_destroy( &messageQueue->ReceivedCondition );
	pthread_cond_destroy( &messageQueue->ProcessedCondition );
}

static void ovrMessageQueue_Enable( ovrMessageQueue * messageQueue, const bool set )
{
	messageQueue->EnabledFlag = set;
}

static void ovrMessageQueue_PostMessage( ovrMessageQueue * messageQueue, const ovrMessage * message )
{
	if ( !messageQueue->EnabledFlag )
	{
		return;
	}
	while ( messageQueue->Tail - messageQueue->Head >= MAX_MESSAGES )
	{
		usleep( 1000 );
	}
	pthread_mutex_lock( &messageQueue->Mutex );
	messageQueue->Messages[messageQueue->Tail & ( MAX_MESSAGES - 1 )] = *message;
	messageQueue->Tail++;
	messageQueue->PostedFlag = true;
	pthread_cond_broadcast( &messageQueue->PostedCondition );
	if ( message->Wait == MQ_WAIT_RECEIVED )
	{
		while ( !messageQueue->ReceivedFlag )
		{
			pthread_cond_wait( &messageQueue->ReceivedCondition, &messageQueue->Mutex );
		}
		messageQueue->ReceivedFlag = false;
	}
	else if ( message->Wait == MQ_WAIT_PROCESSED )
	{
		while ( !messageQueue->ProcessedFlag )
		{
			pthread_cond_wait( &messageQueue->ProcessedCondition, &messageQueue->Mutex );
		}
		messageQueue->ProcessedFlag = false;
	}
	pthread_mutex_unlock( &messageQueue->Mutex );
}

static void ovrMessageQueue_SleepUntilMessage( ovrMessageQueue * messageQueue )
{
	if ( messageQueue->Wait == MQ_WAIT_PROCESSED )
	{
		messageQueue->ProcessedFlag = true;
		pthread_cond_broadcast( &messageQueue->ProcessedCondition );
		messageQueue->Wait = MQ_WAIT_NONE;
	}
	pthread_mutex_lock( &messageQueue->Mutex );
	if ( messageQueue->Tail > messageQueue->Head )
	{
		pthread_mutex_unlock( &messageQueue->Mutex );
		return;
	}
	while ( !messageQueue->PostedFlag )
	{
		pthread_cond_wait( &messageQueue->PostedCondition, &messageQueue->Mutex );
	}
	messageQueue->PostedFlag = false;
	pthread_mutex_unlock( &messageQueue->Mutex );
}

static bool ovrMessageQueue_GetNextMessage( ovrMessageQueue * messageQueue, ovrMessage * message, bool waitForMessages )
{
	if ( messageQueue->Wait == MQ_WAIT_PROCESSED )
	{
		messageQueue->ProcessedFlag = true;
		pthread_cond_broadcast( &messageQueue->ProcessedCondition );
		messageQueue->Wait = MQ_WAIT_NONE;
	}
	if ( waitForMessages )
	{
		ovrMessageQueue_SleepUntilMessage( messageQueue );
	}
	pthread_mutex_lock( &messageQueue->Mutex );
	if ( messageQueue->Tail <= messageQueue->Head )
	{
		pthread_mutex_unlock( &messageQueue->Mutex );
		return false;
	}
	*message = messageQueue->Messages[messageQueue->Head & ( MAX_MESSAGES - 1 )];
	messageQueue->Head++;
	pthread_mutex_unlock( &messageQueue->Mutex );
	if ( message->Wait == MQ_WAIT_RECEIVED )
	{
		messageQueue->ReceivedFlag = true;
		pthread_cond_broadcast( &messageQueue->ReceivedCondition );
	}
	else if ( message->Wait == MQ_WAIT_PROCESSED )
	{
		messageQueue->Wait = MQ_WAIT_PROCESSED;
	}
	return true;
}

/*
================================================================================

ovrAppThread

================================================================================
*/

enum
{
	MESSAGE_ON_CREATE,
	MESSAGE_ON_START,
	MESSAGE_ON_RESUME,
	MESSAGE_ON_PAUSE,
	MESSAGE_ON_STOP,
	MESSAGE_ON_DESTROY,
	MESSAGE_ON_SURFACE_CREATED,
	MESSAGE_ON_SURFACE_DESTROYED
};

typedef struct
{
	JavaVM *		JavaVm;
	jobject			ActivityObject;
	jclass          ActivityClass;
	pthread_t		Thread;
	ovrMessageQueue	MessageQueue;
	ANativeWindow * NativeWindow;
} ovrAppThread;

long shutdownCountdown;

int m_width;
int m_height;

bool R_SetMode( void );

void JKVR_GetScreenRes(int *width, int *height)
{
    *width = m_width;
    *height = m_height;
}

void Android_MessageBox(const char *title, const char *text)
{
    ALOGE("%s %s", title, text);
}

extern "C" {
void initialize_gl4es();
}

void JKVR_Init()
{
	//Initialise all our variables
	remote_movementSideways = 0.0f;
	remote_movementForward = 0.0f;
	remote_movementUp = 0.0f;
	positional_movementSideways = 0.0f;
	positional_movementForward = 0.0f;
	vr.snapTurn = 0.0f;
	vr.immersive_cinematics = true;
	ducked = DUCK_NOTDUCKED;

	//init randomiser
	srand(time(NULL));

	//Create Cvars
	vr_turn_mode = Cvar_Get( "vr_turn_mode", "0", CVAR_ARCHIVE); // 0 = snap, 1 = smooth (3rd person only), 2 = smooth (all modes)
	vr_turn_angle = Cvar_Get( "vr_turn_angle", "45", CVAR_ARCHIVE);
	vr_positional_factor = Cvar_Get( "vr_positional_factor", "12", CVAR_ARCHIVE);
    vr_walkdirection = Cvar_Get( "vr_walkdirection", "1", CVAR_ARCHIVE);
	vr_weapon_pitchadjust = Cvar_Get( "vr_weapon_pitchadjust", "-20.0", CVAR_ARCHIVE);
    vr_virtual_stock = Cvar_Get( "vr_virtual_stock", "0", CVAR_ARCHIVE);

    //Defaults
	vr_control_scheme = Cvar_Get( "vr_control_scheme", "0", CVAR_ARCHIVE);
	vr_switch_sticks = Cvar_Get( "vr_switch_sticks", "0", CVAR_ARCHIVE);

	vr_immersive_cinematics = Cvar_Get("vr_immersive_cinematics", "1", CVAR_ARCHIVE);
	vr_screen_dist = Cvar_Get( "vr_screen_dist", "2.5", CVAR_ARCHIVE);
	vr_weapon_velocity_trigger = Cvar_Get( "vr_weapon_velocity_trigger", "2.0", CVAR_ARCHIVE);
	vr_force_velocity_trigger = Cvar_Get( "vr_force_velocity_trigger", "2.0", CVAR_ARCHIVE);
	vr_force_distance_trigger = Cvar_Get( "vr_force_distance_trigger", "0.15", CVAR_ARCHIVE);
    vr_two_handed_weapons = Cvar_Get ("vr_two_handed_weapons", "1", CVAR_ARCHIVE);
	vr_force_motion_controlled = Cvar_Get ("vr_force_motion_controlled", "1", CVAR_ARCHIVE);
	vr_crouch_toggle = Cvar_Get ("vr_crouch_toggle", "0", CVAR_ARCHIVE);
	vr_irl_crouch_enabled = Cvar_Get ("vr_irl_crouch_enabled", "0", CVAR_ARCHIVE);
	vr_irl_crouch_to_stand_ratio = Cvar_Get ("vr_irl_crouch_to_stand_ratio", "0.65", CVAR_ARCHIVE);
	vr_saber_block_debounce_time = Cvar_Get ("vr_saber_block_debounce_time", "200", CVAR_ARCHIVE);
	vr_haptic_intensity = Cvar_Get ("vr_haptic_intensity", "1.0", CVAR_ARCHIVE);
	vr_comfort_vignette = Cvar_Get ("vr_comfort_vignette", "0.0", CVAR_ARCHIVE);
	vr_saber_3rdperson_mode = Cvar_Get ("vr_saber_3rdperson_mode", "1", CVAR_ARCHIVE);
	vr_gesture_triggered_use = Cvar_Get ("vr_gesture_triggered_use", "0", CVAR_ARCHIVE);
	vr_use_gesture_boundary = Cvar_Get ("vr_use_gesture_boundary", "0.35", CVAR_ARCHIVE);

	cvar_t *expanded_menu_enabled = Cvar_Get ("expanded_menu_enabled", "0", CVAR_ARCHIVE);
	if (FS_FileExists("expanded_menu.pk3")) {
		Cvar_Set( "expanded_menu_enabled", "1" );
	} else {
		Cvar_Set( "expanded_menu_enabled", "0" );
	}
}


static ovrAppThread * gAppThread = NULL;
static ovrApp gAppState;
static ovrJava java;
static bool destroyed = qfalse;

void JKVR_prepareEyeBuffer(int eye )
{
	ovrRenderer *renderer = JKVR_useScreenLayer() ? &gAppState.Scene.CylinderRenderer : &gAppState.Renderer;

	ovrFramebuffer *frameBuffer = &(renderer->FrameBuffer[eye]);
	ovrFramebuffer_SetCurrent(frameBuffer);

	GL(glEnable(GL_SCISSOR_TEST));
	GL(glDepthMask(GL_TRUE));
	GL(glEnable(GL_DEPTH_TEST));
	GL(glDepthFunc(GL_LEQUAL));

	//Weusing the size of the render target
	GL(glViewport(0, 0, frameBuffer->Width, frameBuffer->Height));
	GL(glScissor(0, 0, frameBuffer->Width, frameBuffer->Height));

	GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GL(glDisable(GL_SCISSOR_TEST));
}

void JKVR_finishEyeBuffer(int eye )
{
	ovrRenderer *renderer = JKVR_useScreenLayer() ? &gAppState.Scene.CylinderRenderer : &gAppState.Renderer;

	ovrFramebuffer *frameBuffer = &(renderer->FrameBuffer[eye]);

	//Clear edge to prevent smearing
	ovrFramebuffer_ClearEdgeTexels(frameBuffer);
	ovrFramebuffer_Resolve(frameBuffer);
}

void JKVR_processMessageQueue() {
	for ( ; ; )
	{
		ovrMessage message;
		const bool waitForMessages = ( gAppState.Ovr == NULL && destroyed == false );
		if ( !ovrMessageQueue_GetNextMessage( &gAppThread->MessageQueue, &message, waitForMessages ) )
		{
			break;
		}

		switch ( message.Id )
		{
			case MESSAGE_ON_CREATE:
			{
				break;
			}
			case MESSAGE_ON_START:
			{
				openjk_initialised = qtrue;
				break;
			}
			case MESSAGE_ON_RESUME:
			{
				//If we get here, then user has opted not to quit
				gAppState.Resumed = true;
				break;
			}
			case MESSAGE_ON_PAUSE:
			{
				gAppState.Resumed = false;
				break;
			}
			case MESSAGE_ON_STOP:
			{
				break;
			}
			case MESSAGE_ON_DESTROY:
			{
				gAppState.NativeWindow = NULL;
				destroyed = true;
				//shutdown = true;
				break;
			}
			case MESSAGE_ON_SURFACE_CREATED:	{ gAppState.NativeWindow = (ANativeWindow *)ovrMessage_GetPointerParm( &message, 0 ); break; }
			case MESSAGE_ON_SURFACE_DESTROYED:	{ gAppState.NativeWindow = NULL; break; }
		}

		ovrApp_HandleXrEvents(&gAppState);
	}
}

void showLoadingIcon();
extern "C" void jni_shutdown();
void JKVR_incrementFrameIndex();
void shutdownVR();
int VR_main( int argc, char* argv[] );

int GetRefresh()
{
    return vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DISPLAY_REFRESH_RATE);
}

void GL_APIENTRYP VR_GLDebugLog(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,GLvoid *userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_PERFORMANCE || ENABLE_GL_DEBUG_VERBOSE)
	{
		char typeStr[128];
		switch (type) {
			case GL_DEBUG_TYPE_ERROR: sprintf(typeStr, "ERROR"); break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: sprintf(typeStr, "DEPRECATED_BEHAVIOR"); break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: sprintf(typeStr, "UNDEFINED_BEHAVIOR"); break;
			case GL_DEBUG_TYPE_PORTABILITY: sprintf(typeStr, "PORTABILITY"); break;
			case GL_DEBUG_TYPE_PERFORMANCE: sprintf(typeStr, "PERFORMANCE"); break;
			case GL_DEBUG_TYPE_MARKER: sprintf(typeStr, "MARKER"); break;
			case GL_DEBUG_TYPE_PUSH_GROUP: sprintf(typeStr, "PUSH_GROUP"); break;
			case GL_DEBUG_TYPE_POP_GROUP: sprintf(typeStr, "POP_GROUP"); break;
			default: sprintf(typeStr, "OTHER"); break;
		}

		char severinityStr[128];
		switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH: sprintf(severinityStr, "HIGH"); break;
			case GL_DEBUG_SEVERITY_MEDIUM: sprintf(severinityStr, "MEDIUM"); break;
			case GL_DEBUG_SEVERITY_LOW: sprintf(severinityStr, "LOW"); break;
			default: sprintf(severinityStr, "VERBOSE"); break;
		}

		ALOGE("[%s] GL issue - %s: %s\n", severinityStr, typeStr, message);
	}
}

void VR_GetResolution(int *pWidth, int *pHeight)
{
	static int width = 0;
	static int height = 0;

	// Enumerate the viewport configurations.
	uint32_t viewportConfigTypeCount = 0;
	OXR(xrEnumerateViewConfigurations(
			gAppState.Instance, gAppState.SystemId, 0, &viewportConfigTypeCount, NULL));

	XrViewConfigurationType* viewportConfigurationTypes =
			(XrViewConfigurationType*)malloc(viewportConfigTypeCount * sizeof(XrViewConfigurationType));

	OXR(xrEnumerateViewConfigurations(
			gAppState.Instance,
			gAppState.SystemId,
			viewportConfigTypeCount,
			&viewportConfigTypeCount,
			viewportConfigurationTypes));

	ALOGV("Available Viewport Configuration Types: %d", viewportConfigTypeCount);

	for (uint32_t i = 0; i < viewportConfigTypeCount; i++) {
		const XrViewConfigurationType viewportConfigType = viewportConfigurationTypes[i];

		ALOGV(
				"Viewport configuration type %d : %s",
				viewportConfigType,
				viewportConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO ? "Selected" : "");

		XrViewConfigurationProperties viewportConfig;
		viewportConfig.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;
		OXR(xrGetViewConfigurationProperties(
				gAppState.Instance, gAppState.SystemId, viewportConfigType, &viewportConfig));
		ALOGV(
				"FovMutable=%s ConfigurationType %d",
				viewportConfig.fovMutable ? "true" : "false",
				viewportConfig.viewConfigurationType);

		uint32_t viewCount;
		OXR(xrEnumerateViewConfigurationViews(
				gAppState.Instance, gAppState.SystemId, viewportConfigType, 0, &viewCount, NULL));

		if (viewCount > 0) {
			XrViewConfigurationView* elements =
					(XrViewConfigurationView*)malloc(viewCount * sizeof(XrViewConfigurationView));

			for (uint32_t e = 0; e < viewCount; e++) {
				elements[e].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
				elements[e].next = NULL;
			}

			OXR(xrEnumerateViewConfigurationViews(
					gAppState.Instance,
					gAppState.SystemId,
					viewportConfigType,
					viewCount,
					&viewCount,
					elements));

			// Cache the view config properties for the selected config type.
			if (viewportConfigType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO) {
				assert(viewCount == ovrMaxNumEyes);
				for (uint32_t e = 0; e < viewCount; e++) {
					gAppState.ViewConfigurationView[e] = elements[e];
				}
			}

			free(elements);
		} else {
			ALOGE("Empty viewport configuration type: %d", viewCount);
		}
	}

	free(viewportConfigurationTypes);

	*pWidth = width = gAppState.ViewConfigurationView[0].recommendedImageRectWidth;
	*pHeight = height = gAppState.ViewConfigurationView[0].recommendedImageRectHeight;
}

void VR_EnterVR( ) {

	if (gAppState.Session) {
		Com_Printf("VR_EnterVR called with existing session");
		return;
	}

	// Create the OpenXR Session.
	XrGraphicsBindingOpenGLESAndroidKHR graphicsBindingAndroidGLES = {};
	graphicsBindingAndroidGLES.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR;
	graphicsBindingAndroidGLES.next = NULL;
	graphicsBindingAndroidGLES.display = eglGetCurrentDisplay();
	graphicsBindingAndroidGLES.config = eglGetCurrentSurface(EGL_DRAW);
	graphicsBindingAndroidGLES.context = eglGetCurrentContext();

	XrSessionCreateInfo sessionCreateInfo = {};
	memset(&sessionCreateInfo, 0, sizeof(sessionCreateInfo));
	sessionCreateInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	sessionCreateInfo.next = &graphicsBindingAndroidGLES;
	sessionCreateInfo.createFlags = 0;
	sessionCreateInfo.systemId = gAppState.SystemId;

	XrResult initResult;
	OXR(initResult = xrCreateSession(gAppState.Instance, &sessionCreateInfo, &gAppState.Session));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to create XR session: %d.", initResult);
		exit(1);
	}

	// Create a space to the first path
	XrReferenceSpaceCreateInfo spaceCreateInfo = {};
	spaceCreateInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	spaceCreateInfo.poseInReferenceSpace.orientation.w = 1.0f;
	OXR(xrCreateReferenceSpace(gAppState.Session, &spaceCreateInfo, &gAppState.HeadSpace));
}

void VR_LeaveVR( ) {
	if (gAppState.Session) {
		OXR(xrDestroySpace(gAppState.HeadSpace));
		// StageSpace is optional.
		if (gAppState.StageSpace != XR_NULL_HANDLE) {
			OXR(xrDestroySpace(gAppState.StageSpace));
		}
		OXR(xrDestroySpace(gAppState.FakeStageSpace));
		gAppState.CurrentSpace = XR_NULL_HANDLE;
		OXR(xrDestroySession(gAppState.Session));
		gAppState.Session = NULL;
	}
}

void VR_InitRenderer(  ) {
	int eyeW, eyeH;
	VR_GetResolution(&eyeW, &eyeH);

	// Get the viewport configuration info for the chosen viewport configuration type.
	gAppState.ViewportConfig.type = XR_TYPE_VIEW_CONFIGURATION_PROPERTIES;

	OXR(xrGetViewConfigurationProperties(
			gAppState.Instance, gAppState.SystemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, &gAppState.ViewportConfig));

	// Get the supported display refresh rates for the system.
	{
		PFN_xrEnumerateDisplayRefreshRatesFB pfnxrEnumerateDisplayRefreshRatesFB = NULL;
		OXR(xrGetInstanceProcAddr(
				gAppState.Instance,
				"xrEnumerateDisplayRefreshRatesFB",
				(PFN_xrVoidFunction*)(&pfnxrEnumerateDisplayRefreshRatesFB)));

		OXR(pfnxrEnumerateDisplayRefreshRatesFB(
				gAppState.Session, 0, &gAppState.NumSupportedDisplayRefreshRates, NULL));

		gAppState.SupportedDisplayRefreshRates =
				(float*)malloc(gAppState.NumSupportedDisplayRefreshRates * sizeof(float));
		OXR(pfnxrEnumerateDisplayRefreshRatesFB(
				gAppState.Session,
				gAppState.NumSupportedDisplayRefreshRates,
				&gAppState.NumSupportedDisplayRefreshRates,
				gAppState.SupportedDisplayRefreshRates));
		ALOGV("Supported Refresh Rates:");
		for (uint32_t i = 0; i < gAppState.NumSupportedDisplayRefreshRates; i++) {
			ALOGV("%d:%f", i, gAppState.SupportedDisplayRefreshRates[i]);
		}

		OXR(xrGetInstanceProcAddr(
				gAppState.Instance,
				"xrGetDisplayRefreshRateFB",
				(PFN_xrVoidFunction*)(&gAppState.pfnGetDisplayRefreshRate)));

		float currentDisplayRefreshRate = 0.0f;
		OXR(gAppState.pfnGetDisplayRefreshRate(gAppState.Session, &currentDisplayRefreshRate));
		ALOGV("Current System Display Refresh Rate: %f", currentDisplayRefreshRate);

		OXR(xrGetInstanceProcAddr(
				gAppState.Instance,
				"xrRequestDisplayRefreshRateFB",
				(PFN_xrVoidFunction*)(&gAppState.pfnRequestDisplayRefreshRate)));

		// Test requesting the system default.
		OXR(gAppState.pfnRequestDisplayRefreshRate(gAppState.Session, 0.0f));
		ALOGV("Requesting system default display refresh rate");
	}

	uint32_t numOutputSpaces = 0;
	OXR(xrEnumerateReferenceSpaces(gAppState.Session, 0, &numOutputSpaces, NULL));

	XrReferenceSpaceType* referenceSpaces =
			(XrReferenceSpaceType*)malloc(numOutputSpaces * sizeof(XrReferenceSpaceType));

	OXR(xrEnumerateReferenceSpaces(
			gAppState.Session, numOutputSpaces, &numOutputSpaces, referenceSpaces));

	for (uint32_t i = 0; i < numOutputSpaces; i++) {
		if (referenceSpaces[i] == XR_REFERENCE_SPACE_TYPE_STAGE) {
			stageSupported = GL_TRUE;
			break;
		}
	}

	free(referenceSpaces);

	if (gAppState.CurrentSpace == XR_NULL_HANDLE) {
		VR_Recenter(engine);
	}

	projections = (XrView*)(malloc(ovrMaxNumEyes * sizeof(XrView)));

	ovrRenderer_Create(
			gAppState.Session,
			&gAppState.Renderer,
			gAppState.ViewConfigurationView[0].recommendedImageRectWidth,
			gAppState.ViewConfigurationView[0].recommendedImageRectHeight);
}

void VR_DestroyRenderer(  )
{
	ovrRenderer_Destroy(&gAppState.Renderer);
	free(projections);
}

void * AppThreadFunction(void * parm ) {
	gAppThread = (ovrAppThread *) parm;

	java.Vm = gAppThread->JavaVm;
	java.Vm->AttachCurrentThread(&java.Env, NULL);
	java.ActivityObject = gAppThread->ActivityObject;

	jclass cls = java.Env->GetObjectClass(java.ActivityObject);

	// Note that AttachCurrentThread will reset the thread name.
	prctl(PR_SET_NAME, (long) "OVR::Main", 0, 0, 0);

	openjk_initialised = false;
	vr_screen_dist = NULL;

	ovrApp_Clear(&gAppState);
	gAppState.Java = java;


	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddr(
			XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);
	if (xrInitializeLoaderKHR != NULL) {
		XrLoaderInitInfoAndroidKHR loaderInitializeInfoAndroid;
		memset(&loaderInitializeInfoAndroid, 0, sizeof(loaderInitializeInfoAndroid));
		loaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
		loaderInitializeInfoAndroid.next = NULL;
		loaderInitializeInfoAndroid.applicationVM = java.Vm;
		loaderInitializeInfoAndroid.applicationContext = java.ActivityObject;
		xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&loaderInitializeInfoAndroid);
	}

	// Create the OpenXR instance.
	XrApplicationInfo appInfo;
	memset(&appInfo, 0, sizeof(appInfo));
	strcpy(appInfo.applicationName, "JKQUest");
	appInfo.applicationVersion = 0;
	strcpy(appInfo.engineName, "JKQuest");
	appInfo.engineVersion = 0;
	appInfo.apiVersion = XR_CURRENT_API_VERSION;

	XrInstanceCreateInfo instanceCreateInfo;
	memset(&instanceCreateInfo, 0, sizeof(instanceCreateInfo));
	instanceCreateInfo.type = XR_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.next = NULL;
	instanceCreateInfo.createFlags = 0;
	instanceCreateInfo.applicationInfo = appInfo;
	instanceCreateInfo.enabledApiLayerCount = 0;
	instanceCreateInfo.enabledApiLayerNames = NULL;
	instanceCreateInfo.enabledExtensionCount = numRequiredExtensions;
	instanceCreateInfo.enabledExtensionNames = requiredExtensionNames;

	XrResult initResult;
	OXR(initResult = xrCreateInstance(&instanceCreateInfo, &gAppState.Instance));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to create XR instance: %d.", initResult);
		exit(1);
	}

	XrInstanceProperties instanceInfo;
	instanceInfo.type = XR_TYPE_INSTANCE_PROPERTIES;
	instanceInfo.next = NULL;
	OXR(xrGetInstanceProperties(gAppState.Instance, &instanceInfo));
	ALOGV(
			"Runtime %s: Version : %u.%u.%u",
			instanceInfo.runtimeName,
			XR_VERSION_MAJOR(instanceInfo.runtimeVersion),
			XR_VERSION_MINOR(instanceInfo.runtimeVersion),
			XR_VERSION_PATCH(instanceInfo.runtimeVersion));

	XrSystemGetInfo systemGetInfo;
	memset(&systemGetInfo, 0, sizeof(systemGetInfo));
	systemGetInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	systemGetInfo.next = NULL;
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	OXR(initResult = xrGetSystem(gAppState.Instance, &systemGetInfo, &gAppState.SystemId));
	if (initResult != XR_SUCCESS) {
		ALOGE("Failed to get system.");
		exit(1);
	}

	// Get the graphics requirements.
	PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnGetOpenGLESGraphicsRequirementsKHR = NULL;
	OXR(xrGetInstanceProcAddr(
			vr_engine.appState.Instance,
			"xrGetOpenGLESGraphicsRequirementsKHR",
			(PFN_xrVoidFunction*)(&pfnGetOpenGLESGraphicsRequirementsKHR)));

	XrGraphicsRequirementsOpenGLESKHR graphicsRequirements = {};
	graphicsRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR;
	OXR(pfnGetOpenGLESGraphicsRequirementsKHR(gAppState.Instance, gAppState.SystemId, &graphicsRequirements));



	//Set device defaults
	if (SS_MULTIPLIER == 0.0f)
	{
		//GB Override as refresh is now 72 by default as we decided a higher res is better as 90hz has stutters
		SS_MULTIPLIER = 1.25f;
	}
	else if (SS_MULTIPLIER > 1.5f)
	{
		SS_MULTIPLIER = 1.5f;
	}

	//Using a symmetrical render target
	VR_GetResolution(m_height, m_width);

	gAppState.CpuLevel = CPU_LEVEL;
	gAppState.GpuLevel = GPU_LEVEL;
	gAppState.MainThreadTid = gettid();

	ovrEgl_CreateContext(&gAppState.Egl, NULL);

	EglInitExtensions();

	//First handle any messages in the queue
	while ( gAppState.Ovr == NULL ) {
		JKVR_processMessageQueue();
	}

	VR_EnterVR();
	ovrRenderer_Create(m_width, m_height, &gAppState.Renderer, &java);


	// Create the scene if not yet created.
	ovrScene_Create( m_width, m_height, &gAppState.Scene, &java );

#ifdef JK2_MODE
		chdir("/sdcard/JKQuest/JK2");
#else
		chdir("/sdcard/JKQuest/JK3");
#endif


	//Run loading loop until we are ready to start JKVR
	while (!destroyed && !openjk_initialised) {
		JKVR_processMessageQueue();
		JKVR_incrementFrameIndex();
		showLoadingIcon();
	}

    int maximumSupportRefresh = 0;
    //AmmarkoV : Query Refresh rates and select maximum..!
    //-----------------------------------------------------------------------------------------------------------
    int numberOfRefreshRates = vrapi_GetSystemPropertyInt(&java,
                                                          VRAPI_SYS_PROP_NUM_SUPPORTED_DISPLAY_REFRESH_RATES);
    float refreshRatesArray[16]; //Refresh rates are currently (12/2020) the following 4 : 60.0 / 72.0 / 80.0 / 90.0
    if (numberOfRefreshRates > 16) { numberOfRefreshRates = 16; }
    vrapi_GetSystemPropertyFloatArray(&java, VRAPI_SYS_PROP_SUPPORTED_DISPLAY_REFRESH_RATES,
                                      &refreshRatesArray[0], numberOfRefreshRates);
    for (int i = 0; i < numberOfRefreshRates; i++) {
        ALOGV("Supported refresh rate : %d Hz", refreshRatesArray[i]);
        if (maximumSupportRefresh < refreshRatesArray[i]) {
            maximumSupportRefresh = refreshRatesArray[i];
        }
    }

    if (maximumSupportRefresh > 90.0) {
        ALOGV("Soft limiting to 90.0 Hz as per John carmack's request ( https://www.onlinepeeps.org/oculus-quest-2-according-to-carmack-in-the-future-also-at-120-hz/ );P");
        maximumSupportRefresh = 90.0;
    }

    if (REFRESH == 0 || REFRESH > maximumSupportRefresh)
    {
        REFRESH = 72.0;
    }
    //-----------------------------------------------------------------------------------------------------------


    //start
	VR_main(argc, argv);

	//We are done, shutdown cleanly
	shutdownVR();

	//Ask Java to shut down
	jni_shutdown();

	return NULL;
}

//All the stuff we want to do each frame
void JKVR_FrameSetup()
{
	//Use floor based tracking space
	vrapi_SetTrackingSpace(gAppState.Ovr, VRAPI_TRACKING_SPACE_LOCAL_FLOOR);

	//Set framerate so VrApi doesn't change it on us..
    vrapi_SetDisplayRefreshRate(gAppState.Ovr, REFRESH);

	vrapi_SetExtraLatencyMode(gAppState.Ovr, VRAPI_EXTRA_LATENCY_MODE_ON);

	//get any cvar values required here
	vr.immersive_cinematics = (vr_immersive_cinematics->value != 0.0f);
}

void JKVR_processHaptics() {
	static float lastFrameTime = 0.0f;
	float timestamp = (float)(GetTimeInMilliSeconds());
	float frametime = timestamp - lastFrameTime;
	lastFrameTime = timestamp;

	for (int i = 0; i < 2; ++i) {
		if (vibration_channel_duration[i] > 0.0f ||
			vibration_channel_duration[i] == -1.0f) {
			vrapi_SetHapticVibrationSimple(gAppState.Ovr, controllerIDs[1 - i],
										   vibration_channel_intensity[i]);

			if (vibration_channel_duration[i] != -1.0f) {
				vibration_channel_duration[i] -= frametime;

				if (vibration_channel_duration[i] < 0.0f) {
					vibration_channel_duration[i] = 0.0f;
					vibration_channel_intensity[i] = 0.0f;
				}
			}
		} else {
			vrapi_SetHapticVibrationSimple(gAppState.Ovr, controllerIDs[1 - i], 0.0f);
		}
	}
}

extern "C" {
void jni_haptic_event(const char *event, int position, int flags, int intensity, float angle,
					  float yHeight);
void jni_haptic_updateevent(const char *event, int intensity, float angle);
void jni_haptic_stopevent(const char *event);
void jni_haptic_endframe();
void jni_haptic_enable();
void jni_haptic_disable();
};

void JKVR_ExternalHapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight )
{
	jni_haptic_event(event, position, flags, intensity, angle, yHeight);
}

void JKVR_HapticUpdateEvent(const char* event, int intensity, float angle )
{
	jni_haptic_updateevent(event, intensity, angle);
}

void JKVR_HapticEndFrame()
{
	jni_haptic_endframe();
}

void JKVR_HapticStopEvent(const char* event)
{
	jni_haptic_stopevent(event);
}

void JKVR_HapticEnable()
{
	static bool firstTime = true;
	if (firstTime) {
		jni_haptic_enable();
		firstTime = false;
		jni_haptic_event("fire_pistol", 0, 0, 100, 0, 0);
	}
}

void JKVR_HapticDisable()
{
	jni_haptic_disable();
}

/*
 *  event - name of event
 *  position - for the use of external haptics providers to indicate which bit of haptic hardware should be triggered
 *  flags - a way for the code to specify which controller to produce haptics on, if 0 then weaponFireChannel is calculated in this function
 *  intensity - 0-100
 *  angle - yaw angle (again for external haptics devices) to place the feedback correctly
 *  yHeight - for external haptics devices to place the feedback correctly
 */
void JKVR_HapticEvent(const char* event, int position, int flags, int intensity, float angle, float yHeight )
{
	if (vr_haptic_intensity->value == 0.0f)
	{
		return;
	}

	//Pass on to any external services
	JKVR_ExternalHapticEvent(event, position, flags, intensity, angle, yHeight);

	//Controller Haptic Support
	int weaponFireChannel = vr.weapon_stabilised ? 3 : (vr_control_scheme->integer ? 2 : 1);
	if (flags != 0)
	{
		weaponFireChannel = flags;
	}
	if (strcmp(event, "pickup_shield") == 0 ||
		strcmp(event, "pickup_weapon") == 0 ||
		strstr(event, "pickup_item") != NULL)
	{
		JKVR_Vibrate(100, 3, 1.0);
	}
	else if (strcmp(event, "weapon_switch") == 0)
	{
		JKVR_Vibrate(250, vr_control_scheme->integer ? 2 : 1, 0.8);
	}
	else if (strcmp(event, "shotgun") == 0 || strcmp(event, "fireball") == 0)
	{
		JKVR_Vibrate(400, 3, 1.0);
	}
	else if (strcmp(event, "bullet") == 0)
	{
		JKVR_Vibrate(150, 3, 1.0);
	}
	else if (strcmp(event, "chainsaw_fire") == 0 ||
			 strcmp(event, "RTCWQuest:fire_tesla") == 0)
	{
		JKVR_Vibrate(500, weaponFireChannel, 1.0);
	}
	else if (strcmp(event, "machinegun_fire") == 0 || strcmp(event, "plasmagun_fire") == 0)
	{
		JKVR_Vibrate(90, weaponFireChannel, 0.8);
	}
	else if (strcmp(event, "shotgun_fire") == 0)
	{
		JKVR_Vibrate(250, weaponFireChannel, 1.0);
	}
	else if (strcmp(event, "rocket_fire") == 0 ||
			 strcmp(event, "RTCWQuest:fire_sniper") == 0 ||
			 strcmp(event, "bfg_fire") == 0 ||
			 strcmp(event, "handgrenade_fire") == 0 )
	{
		JKVR_Vibrate(400, weaponFireChannel, 1.0);
	}
	else if (strcmp(event, "selector_icon") == 0)
	{
		//Quick blip
		JKVR_Vibrate(50, flags, 1.0);
	}
}

void showLoadingIcon()
{
	int frameFlags = 0;
	frameFlags |= VRAPI_FRAME_FLAG_FLUSH;

	ovrLayerProjection2 blackLayer = vrapi_DefaultLayerBlackProjection2();
	blackLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

	ovrLayerLoadingIcon2 iconLayer = vrapi_DefaultLayerLoadingIcon2();
	iconLayer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_INHIBIT_SRGB_FRAMEBUFFER;

	const ovrLayerHeader2 * layers[] =
			{
					&blackLayer.Header,
					&iconLayer.Header,
			};

	ovrSubmitFrameDescription2 frameDesc = {};
	frameDesc.Flags = frameFlags;
	frameDesc.SwapInterval = 1;
	frameDesc.FrameIndex = gAppState.FrameIndex;
	frameDesc.DisplayTime = gAppState.DisplayTime;
	frameDesc.LayerCount = 2;
	frameDesc.Layers = layers;

	vrapi_SubmitFrame2( gAppState.Ovr, &frameDesc );
}

void JKVR_getHMDOrientation() {//Get orientation

	// Get the HMD pose, predicted for the middle of the time period during which
	// the new eye images will be displayed. The number of frames predicted ahead
	// depends on the pipeline depth of the engine and the synthesis rate.
	// The better the prediction, the less black will be pulled in at the edges.
	tracking = vrapi_GetPredictedTracking2(gAppState.Ovr, gAppState.DisplayTime);


	// We extract Yaw, Pitch, Roll instead of directly using the orientation
	// to allow "additional" yaw manipulation with mouse/controller.
	const ovrQuatf quatHmd = tracking.Pose.orientation;
	const ovrVector3f positionHmd = tracking.Pose.position;
	vec3_t rotation = {0, 0, 0};
	QuatToYawPitchRoll(quatHmd, rotation, vr.hmdorientation);
	setHMDPosition(positionHmd.x, positionHmd.y, positionHmd.z);

	updateHMDOrientation();

	// Max-height is set only once on start, or after re-calibration
	// (ignore too low value which is sometimes provided on start)
	if (!vr.maxHeight || vr.maxHeight < 1.0) {
		vr.maxHeight = positionHmd.y;
	}
	vr.curHeight = positionHmd.y;

	///ALOGV("        HMD-Position: %f, %f, %f", positionHmd.x, positionHmd.y, positionHmd.z);
}

void shutdownVR() {
	ovrRenderer_Destroy( &gAppState.Renderer );
	ovrEgl_DestroyContext( &gAppState.Egl );
	java.Vm->DetachCurrentThread( );
	vrapi_Shutdown();
}

long long JKVR_getFrameIndex()
{
	return gAppState.FrameIndex;
}

void JKVR_incrementFrameIndex()
{
	// This is the only place the frame index is incremented, right before
	// calling vrapi_GetPredictedDisplayTime().
	gAppState.FrameIndex++;
	gAppState.DisplayTime = vrapi_GetPredictedDisplayTime(gAppState.Ovr,
														  gAppState.FrameIndex);
}

void JKVR_getTrackedRemotesOrientation() {//Get info for tracked remotes
	acquireTrackedRemotesData(gAppState.Ovr, gAppState.DisplayTime);

	//Call additional control schemes here
	switch (vr_control_scheme->integer)
	{
		case RIGHT_HANDED_DEFAULT:
			HandleInput_Default(&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								xrButton_A, xrButton_B, xrButton_X, xrButton_Y);
			break;
		case LEFT_HANDED_DEFAULT:
			HandleInput_Default(&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								xrButton_X, xrButton_Y, xrButton_A, xrButton_B);
			break;
/*		case WEAPON_ALIGN:
			HandleInput_WeaponAlign(&rightTrackedRemoteState_new, &rightTrackedRemoteState_old, &rightRemoteTracking_new,
								&leftTrackedRemoteState_new, &leftTrackedRemoteState_old, &leftRemoteTracking_new,
								xrButton_A, xrButton_B, xrButton_X, xrButton_Y);
			break;
			*/
	}
}

void JKVR_submitFrame()
{
	ovrSubmitFrameDescription2 frameDesc = {0};

	if (!JKVR_useScreenLayer()) {

		ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();
		layer.HeadPose = tracking.HeadPose;
		for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
		{
			ovrFramebuffer * frameBuffer = &gAppState.Renderer.FrameBuffer[gAppState.Renderer.NumBuffers == 1 ? 0 : eye];
			layer.Textures[eye].ColorSwapChain = frameBuffer->ColorTextureSwapChain;
			layer.Textures[eye].SwapChainIndex = frameBuffer->TextureSwapChainIndex;

			ovrMatrix4f projectionMatrix;

			if (vr.cgzoommode)
			{
				projectionMatrix = ovrMatrix4f_CreateProjectionFov(vr.fov / 1.3, vr.fov / 1.3,
																   0.0f, 0.0f, 0.1f, 0.0f);
			}
			else
			{
				projectionMatrix = ovrMatrix4f_CreateProjectionFov(vr.fov, vr.fov,
																   0.0f, 0.0f, 0.1f, 0.0f);
			}

			layer.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection(&projectionMatrix);

			layer.Textures[eye].TextureRect.x = 0;
			layer.Textures[eye].TextureRect.y = 0;
			layer.Textures[eye].TextureRect.width = 1.0f;
			layer.Textures[eye].TextureRect.height = 1.0f;

			ovrFramebuffer_Advance(frameBuffer);
		}

		ovrFramebuffer_SetNone();

		layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

		// Set up the description for this frame.
		const ovrLayerHeader2 *layers[] =
				{
						&layer.Header
				};

		frameDesc.Flags = 0;
		frameDesc.SwapInterval = gAppState.SwapInterval;
		frameDesc.FrameIndex = gAppState.FrameIndex;
		frameDesc.DisplayTime = gAppState.DisplayTime;
		frameDesc.LayerCount = 1;
		frameDesc.Layers = layers;

		// Hand over the eye images to the time warp.
		vrapi_SubmitFrame2(gAppState.Ovr, &frameDesc);

	} else {
		// Set-up the compositor layers for this frame.
		// NOTE: Multiple independent layers are allowed, but they need to be added
		// in a depth consistent order.
		memset( gAppState.Layers, 0, sizeof( ovrLayer_Union2 ) * ovrMaxLayerCount );
		gAppState.LayerCount = 0;

		// Add a simple cylindrical layer
		gAppState.Layers[gAppState.LayerCount++].Cylinder =
				BuildCylinderLayer(&gAppState.Scene.CylinderRenderer,
								   gAppState.Scene.CylinderWidth, gAppState.Scene.CylinderHeight, &tracking, radians(vr.hmdorientation_snap[YAW]) );

		for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
		{
			ovrFramebuffer * frameBuffer = &gAppState.Scene.CylinderRenderer.FrameBuffer[gAppState.Renderer.NumBuffers == 1 ? 0 : eye];
			ovrFramebuffer_Advance(frameBuffer);
		}

		ovrFramebuffer_SetNone();

		// Compose the layers for this frame.
		const ovrLayerHeader2 * layerHeaders[ovrMaxLayerCount] = { 0 };
		for ( int i = 0; i < gAppState.LayerCount; i++ )
		{
			layerHeaders[i] = &gAppState.Layers[i].Header;
		}

		// Set up the description for this frame.
		frameDesc.Flags = 0;
		frameDesc.SwapInterval = gAppState.SwapInterval;
		frameDesc.FrameIndex = gAppState.FrameIndex;
		frameDesc.DisplayTime = gAppState.DisplayTime;
		frameDesc.LayerCount = gAppState.LayerCount;
		frameDesc.Layers = layerHeaders;

		// Hand over the eye images to the time warp.
		vrapi_SubmitFrame2(gAppState.Ovr, &frameDesc);
	}


	JKVR_incrementFrameIndex();
}

static void ovrAppThread_Create( ovrAppThread * appThread, JNIEnv * env, jobject activityObject, jclass activityClass )
{
	env->GetJavaVM( &appThread->JavaVm );
	appThread->ActivityObject = env->NewGlobalRef( activityObject );
	appThread->ActivityClass = (jclass)env->NewGlobalRef( activityClass );
	appThread->Thread = 0;
	appThread->NativeWindow = NULL;
	ovrMessageQueue_Create( &appThread->MessageQueue );

	const int createErr = pthread_create( &appThread->Thread, NULL, AppThreadFunction, appThread );
	if ( createErr != 0 )
	{
		ALOGE( "pthread_create returned %i", createErr );
	}
}

static void ovrAppThread_Destroy( ovrAppThread * appThread, JNIEnv * env )
{
	pthread_join( appThread->Thread, NULL );
	env->DeleteGlobalRef( appThread->ActivityObject );
	env->DeleteGlobalRef( appThread->ActivityClass );
	ovrMessageQueue_Destroy( &appThread->MessageQueue );
}

/*
================================================================================

Activity lifecycle

================================================================================
*/

extern "C" {

jmethodID android_shutdown;
static JavaVM *jVM;
static jobject jniCallbackObj=0;

void jni_shutdown()
{
	ALOGV("Calling: jni_shutdown");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}
	return env->CallVoidMethod(jniCallbackObj, android_shutdown);
}

jmethodID android_haptic_event;
jmethodID android_haptic_updateevent;
jmethodID android_haptic_stopevent;
jmethodID android_haptic_endframe;
jmethodID android_haptic_enable;
jmethodID android_haptic_disable;

void jni_haptic_event(const char* event, int position, int flags, int intensity, float angle, float yHeight)
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_event, StringArg1, position, flags, intensity, angle, yHeight);
}

void jni_haptic_updateevent(const char* event, int intensity, float angle)
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_updateevent, StringArg1, intensity, angle);
}

void jni_haptic_stopevent(const char* event)
{
	ALOGV("Calling: jni_haptic_stopevent");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	jstring StringArg1 = env->NewStringUTF(event);

	return env->CallVoidMethod(jniCallbackObj, android_haptic_stopevent, StringArg1);
}

void jni_haptic_endframe()
{
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_endframe);
}

void jni_haptic_enable()
{
	ALOGV("Calling: jni_haptic_enable");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_enable);
}

void jni_haptic_disable()
{
	ALOGV("Calling: jni_haptic_disable");
	JNIEnv *env;
	jobject tmp;
	if ((jVM->GetEnv((void**) &env, JNI_VERSION_1_4))<0)
	{
		jVM->AttachCurrentThread(&env, NULL);
	}

	return env->CallVoidMethod(jniCallbackObj, android_haptic_disable);
}

int JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv *env;
    jVM = vm;
	if(vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		ALOGE("Failed JNI_OnLoad");
		return -1;
	}

	return JNI_VERSION_1_4;
}

JNIEXPORT jlong JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onCreate( JNIEnv * env, jclass activityClass, jobject activity,
																	   jstring commandLineParams)
{
	ALOGV( "    GLES3JNILib::onCreate()" );

	/* the global arg_xxx structs are initialised within the argtable */
	void *argtable[] = {
			ss    = arg_dbl0("s", "supersampling", "<double>", "super sampling value (default: Q1: 1.2, Q2: 1.35)"),
            cpu   = arg_int0("c", "cpu", "<int>", "CPU perf index 1-4 (default: 2)"),
            gpu   = arg_int0("g", "gpu", "<int>", "GPU perf index 1-4 (default: 3)"),
            msaa  = arg_int0("m", "msaa", "<int>", "MSAA (default: 1)"),
            refresh  = arg_int0("r", "refresh", "<int>", "Refresh Rate (default: Q1: 72, Q2: 72)"),
            end   = arg_end(20)
	};

	jboolean iscopy;
	const char *arg = env->GetStringUTFChars(commandLineParams, &iscopy);

	char *cmdLine = NULL;
	if (arg && strlen(arg))
	{
		cmdLine = strdup(arg);
	}

	env->ReleaseStringUTFChars(commandLineParams, arg);

	ALOGV("Command line %s", cmdLine);
	argv = (char**)malloc(sizeof(char*) * 255);
	argc = ParseCommandLine(strdup(cmdLine), argv);

	/* verify the argtable[] entries were allocated sucessfully */
	if (arg_nullcheck(argtable) == 0) {
		/* Parse the command line as defined by argtable[] */
		arg_parse(argc, argv, argtable);

        if (ss->count > 0 && ss->dval[0] > 0.0)
        {
            SS_MULTIPLIER = ss->dval[0];
        }

        if (cpu->count > 0 && cpu->ival[0] > 0 && cpu->ival[0] < 10)
        {
            CPU_LEVEL = cpu->ival[0];
        }

        if (gpu->count > 0 && gpu->ival[0] > 0 && gpu->ival[0] < 10)
        {
            GPU_LEVEL = gpu->ival[0];
        }

        if (msaa->count > 0 && msaa->ival[0] > 0 && msaa->ival[0] < 10)
        {
            NUM_MULTI_SAMPLES = msaa->ival[0];
        }

        if (refresh->count > 0 && refresh->ival[0] > 0 && refresh->ival[0] <= 120)
        {
            REFRESH = refresh->ival[0];
        }
	}

	initialize_gl4es();

	ovrAppThread * appThread = (ovrAppThread *) malloc( sizeof( ovrAppThread ) );
	ovrAppThread_Create( appThread, env, activity, activityClass );

	ovrMessageQueue_Enable( &appThread->MessageQueue, true );
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_CREATE, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );

	return (jlong)((size_t)appThread);
}


JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onStart( JNIEnv * env, jobject obj, jlong handle, jobject obj1)
{
	ALOGV( "    GLES3JNILib::onStart()" );


    jniCallbackObj = (jobject)env->NewGlobalRef( obj1);
	jclass callbackClass = env->GetObjectClass( jniCallbackObj);

	android_shutdown = env->GetMethodID(callbackClass,"shutdown","()V");
	android_haptic_event = env->GetMethodID(callbackClass, "haptic_event", "(Ljava/lang/String;IIIFF)V");
	android_haptic_updateevent = env->GetMethodID(callbackClass, "haptic_updateevent", "(Ljava/lang/String;IF)V");
	android_haptic_stopevent = env->GetMethodID(callbackClass, "haptic_stopevent", "(Ljava/lang/String;)V");
	android_haptic_endframe = env->GetMethodID(callbackClass, "haptic_endframe", "()V");
	android_haptic_enable = env->GetMethodID(callbackClass, "haptic_enable", "()V");
	android_haptic_disable = env->GetMethodID(callbackClass, "haptic_disable", "()V");

	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_START, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onResume( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onResume()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_RESUME, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onPause( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onPause()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_PAUSE, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onStop( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onStop()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_STOP, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onDestroy( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onDestroy()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_DESTROY, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
	ovrMessageQueue_Enable( &appThread->MessageQueue, false );

	ovrAppThread_Destroy( appThread, env );
	free( appThread );
}

/*
================================================================================

Surface lifecycle

================================================================================
*/

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onSurfaceCreated( JNIEnv * env, jobject obj, jlong handle, jobject surface )
{
	ALOGV( "    GLES3JNILib::onSurfaceCreated()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);

	ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
	if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
	{
		// An app that is relaunched after pressing the home button gets an initial surface with
		// the wrong orientation even though android:screenOrientation="landscape" is set in the
		// manifest. The choreographer callback will also never be called for this surface because
		// the surface is immediately replaced with a new surface with the correct orientation.
		ALOGE( "        Surface not in landscape mode!" );
	}

	ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
	appThread->NativeWindow = newNativeWindow;
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED );
	ovrMessage_SetPointerParm( &message, 0, appThread->NativeWindow );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onSurfaceChanged( JNIEnv * env, jobject obj, jlong handle, jobject surface )
{
	ALOGV( "    GLES3JNILib::onSurfaceChanged()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);

	ANativeWindow * newNativeWindow = ANativeWindow_fromSurface( env, surface );
	if ( ANativeWindow_getWidth( newNativeWindow ) < ANativeWindow_getHeight( newNativeWindow ) )
	{
		// An app that is relaunched after pressing the home button gets an initial surface with
		// the wrong orientation even though android:screenOrientation="landscape" is set in the
		// manifest. The choreographer callback will also never be called for this surface because
		// the surface is immediately replaced with a new surface with the correct orientation.
		ALOGE( "        Surface not in landscape mode!" );
	}

	if ( newNativeWindow != appThread->NativeWindow )
	{
		if ( appThread->NativeWindow != NULL )
		{
			ovrMessage message;
			ovrMessage_Init( &message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED );
			ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
			ALOGV( "        ANativeWindow_release( NativeWindow )" );
			ANativeWindow_release( appThread->NativeWindow );
			appThread->NativeWindow = NULL;
		}
		if ( newNativeWindow != NULL )
		{
			ALOGV( "        NativeWindow = ANativeWindow_fromSurface( env, surface )" );
			appThread->NativeWindow = newNativeWindow;
			ovrMessage message;
			ovrMessage_Init( &message, MESSAGE_ON_SURFACE_CREATED, MQ_WAIT_PROCESSED );
			ovrMessage_SetPointerParm( &message, 0, appThread->NativeWindow );
			ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
		}
	}
	else if ( newNativeWindow != NULL )
	{
		ANativeWindow_release( newNativeWindow );
	}
}

JNIEXPORT void JNICALL Java_com_drbeef_jkquest_GLES3JNILib_onSurfaceDestroyed( JNIEnv * env, jobject obj, jlong handle )
{
	ALOGV( "    GLES3JNILib::onSurfaceDestroyed()" );
	ovrAppThread * appThread = (ovrAppThread *)((size_t)handle);
	ovrMessage message;
	ovrMessage_Init( &message, MESSAGE_ON_SURFACE_DESTROYED, MQ_WAIT_PROCESSED );
	ovrMessageQueue_PostMessage( &appThread->MessageQueue, &message );
	ALOGV( "        ANativeWindow_release( NativeWindow )" );
	ANativeWindow_release( appThread->NativeWindow );
	appThread->NativeWindow = NULL;
}

}

