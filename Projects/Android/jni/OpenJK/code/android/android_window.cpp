/*
===========================================================================
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "qcommon/qcommon.h"
#include "rd-common/tr_types.h"
#include "sys/sys_local.h"
#include "qgl.h"

enum rserr_t
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
};

cvar_t *r_allowSoftwareGL;

// Window cvars
cvar_t	*r_fullscreen = 0;
cvar_t	*r_noborder;
cvar_t	*r_centerWindow;
cvar_t	*r_customwidth;
cvar_t	*r_customheight;
cvar_t	*r_swapInterval;
cvar_t	*r_stereo;
cvar_t	*r_mode;
cvar_t	*r_displayRefresh;

// Window surface cvars
cvar_t	*r_stencilbits;
cvar_t	*r_depthbits;
cvar_t	*r_colorbits;
cvar_t	*r_ignorehwgamma;
cvar_t  *r_ext_multisample;

void TBXR_GetScreenRes(int *width, int *height);


/*
** R_GetModeInfo
*/
typedef struct vidmode_s
{
    const char *description;
    int         width, height;
} vidmode_t;

const vidmode_t r_vidModes[] = {
    { "Mode  0: 320x240",		320,	240 },
    { "Mode  1: 400x300",		400,	300 },
    { "Mode  2: 512x384",		512,	384 },
    { "Mode  3: 640x480",		640,	480 },
    { "Mode  4: 800x600",		800,	600 },
    { "Mode  5: 960x720",		960,	720 },
    { "Mode  6: 1024x768",		1024,	768 },
    { "Mode  7: 1152x864",		1152,	864 },
    { "Mode  8: 1280x1024",		1280,	1024 },
    { "Mode  9: 1600x1200",		1600,	1200 },
    { "Mode 10: 2048x1536",		2048,	1536 },
    { "Mode 11: 856x480 (wide)", 856,	 480 },
    { "Mode 12: 2400x600(surround)",2400,600 }
};
static const int	s_numVidModes = ARRAY_LEN( r_vidModes );

#define R_MODE_FALLBACK (4) // 640x480

qboolean R_GetModeInfo( int *width, int *height, int mode ) {
	const vidmode_t	*vm;

    if ( mode < -1 ) {
        return qfalse;
	}
	if ( mode >= s_numVidModes ) {
		return qfalse;
	}

	if ( mode == -1 ) {
		*width = r_customwidth->integer;
		*height = r_customheight->integer;
		return qtrue;
	}

	vm = &r_vidModes[mode];

    *width  = vm->width;
    *height = vm->height;

    return qtrue;
}

/*
** R_ModeList_f
*/
static void R_ModeList_f( void )
{
	int i;

	Com_Printf( "\n" );
	Com_Printf( "Mode -2: Use desktop resolution\n" );
	Com_Printf( "Mode -1: Use r_customWidth and r_customHeight variables\n" );
	for ( i = 0; i < s_numVidModes; i++ )
	{
		Com_Printf( "%s\n", r_vidModes[i].description );
	}
	Com_Printf( "\n" );
}

/*
===============
GLimp_Minimize

Minimize the game so that user is back at the desktop
===============
*/
void GLimp_Minimize(void)
{
	
}

void TBXR_submitFrame();

void WIN_Present( window_t *window )
{
	TBXR_submitFrame();
}



window_t WIN_Init( const windowDesc_t *windowDesc, glconfig_t *glConfig )
{
	Cmd_AddCommand("modelist", R_ModeList_f);
	Cmd_AddCommand("minimize", GLimp_Minimize);

	r_allowSoftwareGL	= Cvar_Get( "r_allowSoftwareGL",	"0",		CVAR_ARCHIVE_ND|CVAR_LATCH );

	// Window cvars
	r_fullscreen		= Cvar_Get( "r_fullscreen",			"0",		CVAR_ARCHIVE|CVAR_LATCH );
	r_noborder			= Cvar_Get( "r_noborder",			"0",		CVAR_ARCHIVE|CVAR_LATCH );
	r_centerWindow		= Cvar_Get( "r_centerWindow",		"0",		CVAR_ARCHIVE|CVAR_LATCH );
	r_customwidth		= Cvar_Get( "r_customwidth",		"1600",		CVAR_ARCHIVE|CVAR_LATCH );
	r_customheight		= Cvar_Get( "r_customheight",		"1024",		CVAR_ARCHIVE|CVAR_LATCH );
	r_swapInterval		= Cvar_Get( "r_swapInterval",		"0",		CVAR_ARCHIVE_ND );
	r_stereo			= Cvar_Get( "r_stereo",				"0",		CVAR_ARCHIVE_ND|CVAR_LATCH );
	r_mode				= Cvar_Get( "r_mode",				"4",		CVAR_ARCHIVE|CVAR_LATCH );
	r_displayRefresh	= Cvar_Get( "r_displayRefresh",		"0",		CVAR_LATCH );
	Cvar_CheckRange( r_displayRefresh, 0, 240, qtrue );

	// Window render surface cvars
	r_stencilbits		= Cvar_Get( "r_stencilbits",		"8",		CVAR_ARCHIVE_ND|CVAR_LATCH );
	r_depthbits			= Cvar_Get( "r_depthbits",			"24",		CVAR_ARCHIVE_ND|CVAR_LATCH );
	r_colorbits			= Cvar_Get( "r_colorbits",			"32",		CVAR_ARCHIVE_ND|CVAR_LATCH );
	r_ignorehwgamma		= Cvar_Get( "r_ignorehwgamma",		"1",		CVAR_ARCHIVE|CVAR_LATCH );
	r_ext_multisample	= Cvar_Get( "r_ext_multisample",	"0",		CVAR_ARCHIVE_ND|CVAR_LATCH );
	Cvar_Get( "r_availableModes", "", CVAR_ROM );


	glConfig->deviceSupportsGamma =
		(qboolean)(!r_ignorehwgamma->integer);

	// This depends on SDL_INIT_VIDEO, hence having it here
	//IN_Init( screen );

	// window_t is only really useful for Windows if the renderer wants to create a D3D context.
	window_t window = {};

	window.api = windowDesc->api;

	int android_screen_width;
	int android_screen_height;
	TBXR_GetScreenRes(&android_screen_width, &android_screen_height);
	glConfig->vidWidth = android_screen_width;
	glConfig->vidHeight = android_screen_height;
	glConfig->colorBits = 32;
	glConfig->depthBits = 16;
	glConfig->stencilBits = 8;

	return window;
}

/*
===============
GLimp_Shutdown
===============
*/
void WIN_Shutdown( void )
{
	Cmd_RemoveCommand("modelist");
	Cmd_RemoveCommand("minimize");

	//IN_Shutdown();
}

void GLimp_EnableLogging( qboolean enable )
{
}

void GLimp_LogComment( char *comment )
{
}

void WIN_SetGamma( glconfig_t *glConfig, byte red[256], byte green[256], byte blue[256] )
{

}

void *WIN_GL_GetProcAddress( const char *proc )
{
	return NULL;//SDL_GL_GetProcAddress( proc );
}

qboolean WIN_GL_ExtensionSupported( const char *extension )
{
	return ((Q_stristr( (const char *)glGetString (GL_EXTENSIONS), extension) ) != NULL) ? qtrue : qfalse;
}
