/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the RTCW SP Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
 */

/*
 ** GLW_IMP.C
 **
 ** This file contains ALL Linux specific stuff having to do with the
 ** OpenGL refresh.  When a port is being made the following functions
 ** must be implemented by the port:
 **
 ** GLimp_EndFrame
 ** GLimp_Init
 ** GLimp_Shutdown
 ** GLimp_SwitchFullscreen
 ** GLimp_SetGamma
 **
 */

#include <termios.h>
#include <sys/ioctl.h>
#ifdef __linux__
#include <sys/stat.h>
#include <sys/vt.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

// bk001204
#include <dlfcn.h>

// bk001206 - from my Heretic2 by way of Ryan's Fakk2
// Needed for the new X11_PendingInput() function.
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "android_local.h" // bk001130
#include "EGL/egl.h"

#define WINDOW_CLASS_NAME   "Return to Castle Wolfenstein"

typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;



void myglMultiTexCoord2f( GLenum texture, GLfloat s, GLfloat t )
{
	glMultiTexCoord4f(texture, s, t, 0, 1);
}



static cvar_t *in_mouse;
static cvar_t *in_dgamouse;

// bk001130 - from cvs1.17 (mkv), but not static
cvar_t   *in_joystick      = NULL;
cvar_t   *in_joystickDebug = NULL;
cvar_t   *joy_threshold    = NULL;

cvar_t  *r_allowSoftwareGL;   // don't abort out if the pixelformat claims software
cvar_t  *r_previousglDriver;

//static int default_dotclock_vidmode; // bk001204 - unused
static int num_vidmodes;
static qboolean vidmode_active = qfalse;


/*
 * Find the first occurrence of find in s.
 */
// bk001130 - from cvs1.17 (mkv), const
// bk001130 - made first argument const
static const char *Q_stristr( const char *s, const char *find ) {
	register char c, sc;
	register size_t len;

	if ( ( c = *find++ ) != 0 ) {
		if ( c >= 'a' && c <= 'z' ) {
			c -= ( 'a' - 'A' );
		}
		len = strlen( find );
		do
		{
			do
			{
				if ( ( sc = *s++ ) == 0 ) {
					return NULL;
				}
				if ( sc >= 'a' && sc <= 'z' ) {
					sc -= ( 'a' - 'A' );
				}
			} while ( sc != c );
		} while ( Q_stricmpn( s, find, len ) != 0 );
		s--;
	}
	return s;
}



void IN_ActivateMouse( void ) {

}

void IN_DeactivateMouse( void ) {

}
/*****************************************************************************/

static qboolean signalcaught = qfalse;;

void Sys_Exit( int ); // bk010104 - abstraction

static void signal_handler( int sig ) { // bk010104 - replace this... (NOTE TTimo huh?)
	if ( signalcaught ) {
		printf( "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n", sig );
		Sys_Exit( 1 ); // bk010104 - abstraction
	}

	signalcaught = qtrue;
	printf( "Received signal %d, exiting...\n", sig );
	GLimp_Shutdown(); // bk010104 - shouldn't this be CL_Shutdown
	Sys_Exit( 0 ); // bk010104 - abstraction NOTE TTimo send a 0 to avoid DOUBLE SIGNAL FAULT
}

static void InitSig( void ) {

	return;


	signal( SIGHUP, signal_handler );
	signal( SIGQUIT, signal_handler );
	signal( SIGILL, signal_handler );
	signal( SIGTRAP, signal_handler );
	signal( SIGIOT, signal_handler );
	signal( SIGBUS, signal_handler );
	signal( SIGFPE, signal_handler );
	signal( SIGSEGV, signal_handler );
	signal( SIGTERM, signal_handler );
}

/*
 ** GLimp_SetGamma
 **
 ** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
 */
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {

}

/*
 ** GLimp_Shutdown
 **
 ** This routine does all OS specific shutdown procedures for the OpenGL
 ** subsystem.  Under OpenGL this means NULLing out the current DC and
 ** HGLRC, deleting the rendering context, and releasing the DC acquired
 ** for the window.  The state structure is also nulled out.
 **
 */
void GLimp_Shutdown( void ) {

	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );

	QGL_Shutdown();
}

/*
 ** GLimp_LogComment
 */
void GLimp_LogComment( char *comment ) {

}

/*
 ** GLW_StartDriverAndSetMode
 */
// bk001204 - prototype needed
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen );
static qboolean GLW_StartDriverAndSetMode( const char *drivername,
		int mode,
		qboolean fullscreen ) {
	rserr_t err;

	// don't ever bother going into fullscreen with a voodoo card
#if 1   // JDC: I reenabled this
	if ( Q_stristr( drivername, "Voodoo" ) ) {
		ri.Cvar_Set( "r_fullscreen", "0" );
		r_fullscreen->modified = qfalse;
		fullscreen = qfalse;
	}
#endif

	err = GLW_SetMode( drivername, mode, fullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}
	return qtrue;
}


void VR_GetScreenRes(int *width, int *height);

/*
 ** GLW_InitExtensions
 */
static void GLW_InitExtensions( void ) {
	if ( !r_allowExtensions->integer ) {
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

#ifndef HAVE_GLES
	// GL_S3_s3tc
	if ( Q_stristr( glConfig.extensions_string, "GL_S3_s3tc" ) ) {
		if ( r_ext_compressed_textures->value ) {
			glConfig.textureCompression = TC_S3TC;
			ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
		} else
		{
			glConfig.textureCompression = TC_NONE;
			ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
		}
	} else
#endif
	{
		glConfig.textureCompression = TC_NONE;
		ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
	}

	// GL_EXT_texture_env_add
#ifdef HAVE_GLES
	glConfig.textureEnvAddAvailable = qtrue;
	ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
#else
	glConfig.textureEnvAddAvailable = qfalse;
	if ( Q_stristr( glConfig.extensions_string, "EXT_texture_env_add" ) ) {
		if ( r_ext_texture_env_add->integer ) {
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		} else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}
#endif

	// GL_ARB_multitexture
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
#ifdef HAVE_GLES
	qglGetIntegerv( GL_MAX_TEXTURE_UNITS, &glConfig.maxActiveTextures );
	//ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, %i texture units\n", glConfig.maxActiveTextures );
	//glConfig.maxActiveTextures=4;
	qglMultiTexCoord2fARB = myglMultiTexCoord2f;
	qglActiveTextureARB = glActiveTexture;
	qglClientActiveTextureARB = glClientActiveTexture;
	if ( glConfig.maxActiveTextures > 1 )
	{
		ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture (%i texture units)\n", glConfig.maxActiveTextures );
	}
	else
	{
		qglMultiTexCoord2fARB = NULL;
		qglActiveTextureARB = NULL;
		qglClientActiveTextureARB = NULL;
		ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
	}
#else
	if ( Q_stristr( glConfig.extensions_string, "GL_ARB_multitexture" ) ) {
		if ( r_ext_multitexture->value ) {
			qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) dlsym( glw_state.OpenGLLib, "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glActiveTextureARB" );
			qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glClientActiveTextureARB" );

			if ( qglActiveTextureARB ) {
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );

				if ( glConfig.maxActiveTextures > 1 ) {
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				} else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	} else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}
#endif

	// GL_EXT_compiled_vertex_array
#ifndef HAVE_GLES
	if ( Q_stristr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) ) {
		if ( r_ext_compiled_vertex_array->value ) {
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( int, int ) )dlsym( glw_state.OpenGLLib, "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) )dlsym( glw_state.OpenGLLib, "glUnlockArraysEXT" );
			if ( !qglLockArraysEXT || !qglUnlockArraysEXT ) {
				ri.Error( ERR_FATAL, "bad getprocaddress" );
			}
		} else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	} else
#endif
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	// GL_NV_fog_distance
#ifndef HAVE_GLES
	if ( Q_stristr( glConfig.extensions_string, "GL_NV_fog_distance" ) ) {
		if ( r_ext_NV_fog_dist->integer ) {
			glConfig.NVFogAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_NV_fog_distance\n" );
		} else {
			ri.Printf( PRINT_ALL, "...ignoring GL_NV_fog_distance\n" );
			ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
		}
	} else 
#endif
	{
		ri.Printf( PRINT_ALL, "...GL_NV_fog_distance not found\n" );
		ri.Cvar_Set( "r_ext_NV_fog_dist", "0" );
	}

}

static void GLW_InitGamma() {

	glConfig.deviceSupportsGamma = qfalse;

}

/*
 ** GLW_LoadOpenGL
 **
 ** GLimp_win.c internal function that that attempts to load and use
 ** a specific OpenGL DLL.
 */
static qboolean GLW_LoadOpenGL( const char *name ) {
	qboolean fullscreen;

	ri.Printf( PRINT_ALL, "...loading %s: ", name );

#ifndef HAVE_GLES
	// disable the 3Dfx splash screen and set gamma
	// we do this all the time, but it shouldn't hurt anything
	// on non-3Dfx stuff
	putenv( "FX_GLIDE_NO_SPLASH=0" );

	// Mesa VooDoo hacks
	putenv( "MESA_GLX_FX=fullscreen\n" );
#endif
	// load the QGL layer
	if ( QGL_Init( name ) ) {
#ifdef PANDORA
		fullscreen = 1;
#else
		fullscreen = r_fullscreen->integer;
#endif

		// create the window and set up the context
		if ( !GLW_StartDriverAndSetMode( name, r_mode->integer, fullscreen ) ) {
			if ( r_mode->integer != 3 ) {
				if ( !GLW_StartDriverAndSetMode( name, 3, fullscreen ) ) {
					goto fail;
				}
			} else {
				goto fail;
			}
		}

		return qtrue;
	} else
	{
		ri.Printf( PRINT_ALL, "failed\n" );
	}
	fail:

	QGL_Shutdown();

	return qfalse;
}



/*
 ** GLimp_Init
 **
 ** This routine is responsible for initializing the OS specific portions
 ** of OpenGL.
 */
void GLimp_Init( void ) {
	qboolean attemptedlibGL = qfalse;
	qboolean attempted3Dfx = qfalse;
	qboolean success = qfalse;
	char buf[1024];
	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
	// cvar_t	*cv; // bk001204 - unused

	r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

	r_previousglDriver = ri.Cvar_Get( "r_previousglDriver", "", CVAR_ROM );

	InitSig();

	// Hack here so that if the UI
	if ( *r_previousglDriver->string ) {
		// The UI changed it on us, hack it back
		// This means the renderer can't be changed on the fly
		ri.Cvar_Set( "r_glDriver", r_previousglDriver->string );
	}

    int android_screen_width;
    int android_screen_height;
	VR_GetScreenRes(&android_screen_width, &android_screen_height);
	glConfig.vidWidth = android_screen_width;
	glConfig.vidHeight = android_screen_height;
	glConfig.colorBits = 32;
	glConfig.depthBits = 16;
	glConfig.stencilBits = 8;



	// Save it in case the UI stomps it
	ri.Cvar_Set( "r_previousglDriver", r_glDriver->string );

	// This values force the UI to disable driver selection
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, qglGetString( GL_VENDOR ), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, qglGetString( GL_RENDERER ), sizeof( glConfig.renderer_string ) );
	if ( *glConfig.renderer_string && glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] == '\n' ) {
		glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] = 0;
	}
	Q_strncpyz( glConfig.version_string, qglGetString( GL_VERSION ), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, qglGetString( GL_EXTENSIONS ), sizeof( glConfig.extensions_string ) );

	//
	// chipset specific configuration
	//
	strcpy( buf, glConfig.renderer_string );
	strlwr( buf );

	//
	// NOTE: if changing cvars, do it within this block.  This allows them
	// to be overridden when testing driver fixes, etc. but only sets
	// them to their default state when the hardware is first installed/run.
	//
	/*if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) ) {
		glConfig.hardwareType = GLHW_GENERIC;

		ri.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_LINEAR" );

		// VOODOO GRAPHICS w/ 2MB
		if ( Q_stristr( buf, "voodoo graphics/1 tmu/2 mb" ) ) {
			ri.Cvar_Set( "r_picmip", "2" );
			ri.Cvar_Get( "r_picmip", "1", CVAR_ARCHIVE | CVAR_LATCH );
		} else
		{
			ri.Cvar_Set( "r_picmip", "1" );

			if ( Q_stristr( buf, "rage 128" ) || Q_stristr( buf, "rage128" ) ) {
				ri.Cvar_Set( "r_finish", "0" );
			}
			// Savage3D and Savage4 should always have trilinear enabled
			else if ( Q_stristr( buf, "savage3d" ) || Q_stristr( buf, "s3 savage4" ) ) {
				ri.Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
			}
		}
	}*/

	//
	// this is where hardware specific workarounds that should be
	// detected/initialized every startup should go.
	//
	if ( Q_stristr( buf, "banshee" ) || Q_stristr( buf, "Voodoo_Graphics" ) ) {
		glConfig.hardwareType = GLHW_3DFX_2D3D;
	} else if ( Q_stristr( buf, "rage pro" ) || Q_stristr( buf, "RagePro" ) ) {
		glConfig.hardwareType = GLHW_RAGEPRO;
	} else if ( Q_stristr( buf, "permedia2" ) ) {
		glConfig.hardwareType = GLHW_PERMEDIA2;
	} else if ( Q_stristr( buf, "riva 128" ) ) {
		glConfig.hardwareType = GLHW_RIVA128;
	} else if ( Q_stristr( buf, "riva tnt " ) ) {
	}

	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	//TEST!!!
	//ri.Cvar_Set( "r_picmip", "0" );
	//ri.Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
	//!!!!


	// initialize extensions
	GLW_InitExtensions();
	GLW_InitGamma();

	InitSig();

	return;
}

void RTCWVR_submitFrame();

//int androidSwapped = 1; //If loading, then draw frame does not return, so detect this
/*
 ** GLimp_EndFrame
 **
 ** Responsible for doing a swapbuffers and possibly for other stuff
 ** as yet to be determined.  Probably better not to make this a GLimp
 ** function and instead do a call to GLimp_SwapBuffers.
 */
void GLimp_EndFrame( void ) {

	// check logging
	//QGL_EnableLogging( (qboolean)r_logFile->integer ); // bk001205 - was ->value
//	if (!androidSwapped)
//		eglSwapBuffers( eglGetCurrentDisplay(), eglGetCurrentSurface( EGL_DRAW ) );

	//androidSwapped = 0;
    RTCWVR_submitFrame();
}

#ifdef SMP
/*
===========================================================

SMP acceleration

===========================================================
 */

sem_t renderCommandsEvent;
sem_t renderCompletedEvent;
sem_t renderActiveEvent;

void ( *glimpRenderThread )( void );

void *GLimp_RenderThreadWrapper( void *stub ) {
	glimpRenderThread();
	return NULL;
}


/*
=======================
GLimp_SpawnRenderThread
=======================
 */
pthread_t renderThreadHandle;
qboolean GLimp_SpawnRenderThread( void ( *function )( void ) ) {

	sem_init( &renderCommandsEvent, 0, 0 );
	sem_init( &renderCompletedEvent, 0, 0 );
	sem_init( &renderActiveEvent, 0, 0 );

	glimpRenderThread = function;

	if ( pthread_create( &renderThreadHandle, NULL,
			GLimp_RenderThreadWrapper, NULL ) ) {
		return qfalse;
	}

	return qtrue;
}

static void  *smpData;
//static	int		glXErrors; // bk001204 - unused

void *GLimp_RendererSleep( void ) {
	void  *data;

	// after this, the front end can exit GLimp_FrontEndSleep
	sem_post( &renderCompletedEvent );

	sem_wait( &renderCommandsEvent );

	data = smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	sem_post( &renderActiveEvent );

	return data;
}


void GLimp_FrontEndSleep( void ) {
	sem_wait( &renderCompletedEvent );
}


void GLimp_WakeRenderer( void *data ) {
	smpData = data;

	// after this, the renderer can continue through GLimp_RendererSleep
	sem_post( &renderCommandsEvent );

	sem_wait( &renderActiveEvent );
}

#else

void GLimp_RenderThreadWrapper( void *stub ) {}
qboolean GLimp_SpawnRenderThread( void ( *function )( void ) ) {
	return qfalse;
}
void *GLimp_RendererSleep( void ) {
	return NULL;
}
void GLimp_FrontEndSleep( void ) {}
void GLimp_WakeRenderer( void *data ) {}

#endif

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

void IN_Init( void ) {
	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_ARCHIVE );
	in_dgamouse = Cvar_Get( "in_dgamouse", "1", CVAR_ARCHIVE );

}

void IN_Shutdown( void ) {

}
/*
void IN_Frame( void ) {


	if ( cls.keyCatchers & KEYCATCH_CONSOLE ) {
		// temporarily deactivate if not in the game and
		// running on the desktop
		// voodoo always counts as full screen
		if ( Cvar_VariableValue( "r_fullscreen" ) == 0
				&& strcmp( Cvar_VariableString( "r_glDriver" ), _3DFX_DRIVER_NAME ) ) {
			IN_DeactivateMouse();
			return;
		}
	}

	IN_ActivateMouse();
}
 */

void IN_Activate( void ) {
}

// bk001130 - cvs1.17 joystick code (mkv) was here, no linux_joystick.c

void Sys_SendKeyEvents( void ) {
	// XEvent event; // bk001204 - unused


	//HandleEvents();
}


// bk010216 - added stubs for non-Linux UNIXes here
// FIXME - use NO_JOYSTICK or something else generic

#if defined( __FreeBSD__ ) // rb010123
void IN_StartupJoystick( void ) {}
void IN_JoyMove( void ) {}
#endif
