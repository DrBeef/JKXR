#include <dlfcn.h>
#ifdef DEDICATED
#include <sys/fcntl.h>
#endif
#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"
#include "qcommon/q_platform.h"

#include "sys_loadlib.h"
#ifdef DEDICATED
#include "unix_local.h"
#else
#include "sys_local.h"
#endif

#if defined(MACOS_X) || defined(__linux__) || defined(__FreeBSD_kernel__)
#include <unistd.h>
#endif

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH - 1] = 0;

	return cwd;
}

#if defined( DO_LOADDLL_WRAP )
void *Sys_LoadDll_Wrapped( const char *name,
						   int( **entryPoint ) ( int, ... ),
						   int ( *systemcalls )( int, ... ) )
#else
void *Sys_LoadDll( const char *name)
#endif
{
	void *libHandle;
	//void ( *dllEntry )( intptr_t ( *syscallptr )( intptr_t, ... ) );
	char fname[MAX_OSPATH];
	char  *homepath;
	char  *basepath;
	char  *pwdpath;
	char  *gamedir;
	char  *fn;
	const char*  err = NULL; // bk001206 // rb0101023 - now const

	// bk001206 - let's have some paranoia
	assert( name );

	snprintf( fname, sizeof( fname ), "../%sarm.so", name );

// bk001129 - was RTLD_LAZY
#define Q_RTLD    RTLD_NOW

	homepath = Cvar_VariableString( "fs_homepath" );
	basepath = Cvar_VariableString( "fs_basepath" );
	gamedir = Cvar_VariableString( "fs_game" );

	pwdpath = Sys_Cwd();
	fn = FS_BuildOSPath( pwdpath, gamedir, fname );
	// bk001206 - verbose
	Com_Printf( "Sys_LoadDll(%s)... ", fn );

#ifdef __ANDROID__
	char path[500];
	char *libdir = (char*)getenv("RTCW_GAMELIBDIR");

#ifdef WOLF_SP_DEMO
	snprintf( path, sizeof( path ), "%s/lib%sarm_d.so", getLibPath(), name );
#else
	snprintf( path, sizeof( path ), "%s/lib%sarm.so", libdir, name );
#endif

	//LOGI("Trying to load Android lib: %s",path);
	libHandle = dlopen (path, RTLD_LAZY );


#else
	// bk001129 - from cvs1.17 (mkv), was fname not fn
	libHandle = dlopen( fn, Q_RTLD );
#endif


	if ( !libHandle ) {
		Com_Printf( "failed (%s)\n", dlerror() );
		// homepath
		fn = FS_BuildOSPath( homepath, gamedir, fname );
		Com_Printf( "Sys_LoadDll(%s)... ", fn );
		libHandle = dlopen( fn, Q_RTLD );

		if ( !libHandle ) {
			Com_Printf( "failed (%s)\n", dlerror() );
			// basepath
			fn = FS_BuildOSPath( basepath, gamedir, fname );
			Com_Printf( "Sys_LoadDll(%s)... ", fn );
			libHandle = dlopen( fn, Q_RTLD );

			if ( !libHandle ) {
				Com_Printf( "failed (%s)\n", dlerror() );

				if ( strlen( gamedir ) && Q_stricmp( gamedir, BASEGAME ) ) { // begin BASEGAME != fs_game section

					// media-only mods: no DLL whatsoever in the fs_game
					// start the loop again using the hardcoded BASEDIRNAME
					fn = FS_BuildOSPath( pwdpath, BASEGAME, fname );
					Com_Printf( "Sys_LoadDll(%s)... ", fn );
					libHandle = dlopen( fn, Q_RTLD );

					if ( !libHandle ) {
						Com_Printf( "failed (%s)\n", dlerror() );
						// homepath
						fn = FS_BuildOSPath( homepath, BASEGAME, fname );
						Com_Printf( "Sys_LoadDll(%s)... ", fn );
						libHandle = dlopen( fn, Q_RTLD );

						if ( !libHandle ) {
							Com_Printf( "failed (%s)\n", dlerror() );
							// homepath
							fn = FS_BuildOSPath( basepath, BASEGAME, fname );
							Com_Printf( "Sys_LoadDll(%s)... ", fn );
							libHandle = dlopen( fn, Q_RTLD );

							if ( !libHandle ) {
								// ok, this time things are really fucked
								Com_Printf( "failed (%s)\n", dlerror() );
							} else {
								Com_Printf( "ok\n" );
							}
						} else {
							Com_Printf( "ok\n" );
						}
					} else {
						Com_Printf( "ok\n" );
					}
				} // end BASEGAME != fs_game section
			} else {
				Com_Printf( "ok\n" );
			}
		} else {
			Com_Printf( "ok\n" );
		}
	} else {
		Com_Printf( "ok\n" );
	}

	if ( !libHandle ) {
#ifndef NDEBUG // in debug, abort on failure
		Com_Error( ERR_FATAL, "Sys_LoadDll(%s) failed dlopen() completely!\n", name  );
#else
		Com_Printf( "Sys_LoadDll(%s) failed dlopen() completely!\n", name );
#endif
		return NULL;
	}

	Com_Printf( "Sys_LoadDll handle = %p", libHandle );

	typedef void QDECL DllEntryProc( SystemCallProc *syscallptr );

	DllEntryProc *dllEntry = (DllEntryProc *)dlsym( libHandle, "dllEntry" );
	void *entryPoint = dlsym( libHandle, "vmMain" );


//	dllEntry = dlsym( libHandle, "dllEntry" );
//	*entryPoint = dlsym( libHandle, "vmMain" );
	if ( !entryPoint || !dllEntry ) {
		err = dlerror();
#ifndef NDEBUG // bk001206 - in debug abort on failure
		Com_Error( ERR_FATAL, "Sys_LoadDll(%s) failed dlsym(vmMain):\n\"%s\" !\n", name, err );
#else
		Com_Printf( "Sys_LoadDll(%s) failed dlsym(vmMain):\n\"%s\" !\n", name, err );
#endif
		dlclose( libHandle );
		err = dlerror();
		if ( err != NULL ) {
			Com_Printf( "Sys_LoadDll(%s) failed dlcose:\n\"%s\"\n", name, err );
		}
		return NULL;
	}
	Com_Printf( "Sys_LoadDll(%s) found **vmMain** at  %p  \n", name, entryPoint ); // bk001212
	//dllEntry( systemcalls );
	Com_Printf( "Sys_LoadDll(%s) succeeded!\n", name );
	return libHandle;
}


/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
	Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath(void)
{
	return binaryPath;
}

/*
=================
Sys_SetDefaultInstallPath
=================
*/
void Sys_SetDefaultInstallPath(const char *path)
{
	Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
Sys_DefaultInstallPath
=================
*/
char *Sys_DefaultInstallPath(void)
{
	if (*installPath)
		return installPath;
	else
		return Sys_Cwd();
}

/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath(void)
{
	return Sys_BinaryPath();
}

// We now expect newlines instead of always appending
// otherwise sectioned prints get messed up.
#define MAXPRINTMSG		4096
void Conbuf_AppendText( const char *pMsg )
{
	char msg[MAXPRINTMSG] = {0};
	Q_strncpyz(msg, pMsg, sizeof(msg));
	Q_StripColor(msg);
	//((void)__android_log_print(ANDROID_LOG_INFO,"JK3","%s", msg));
	//printf("%s", msg);
}

void Sys_Print( const char *msg ) {
	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if ( !Q_strncmp( msg, "[skipnotify]", 12 ) ) {
		msg += 12;
	}
	if ( msg[0] == '*' ) {
		msg += 1;
	}
	Conbuf_AppendText( msg );
}

/*
=================
Sys_In_Restart_f
=================
*/
void Sys_In_Restart_f( void )
{
#ifdef DEDICATED
	IN_Shutdown();
	IN_Init();
#else
	//IN_Restart( );
#endif
}

void	Sys_Init (void) {
	Cmd_AddCommand ("in_restart", Sys_In_Restart_f);
	Cvar_Set( "arch", "arm" );
	Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

void Sys_Exit( int ex ) __attribute__((noreturn));
void Sys_Exit( int ex ) {
#ifndef DEDICATED
	//SDL_Quit( );
#endif

#ifdef NDEBUG // regular behavior
	// We can't do this
    //  as long as GL DLL's keep installing with atexit...
    //exit(ex);
    _exit(ex);
#else
	// Give me a backtrace on error exits.
	assert( ex == 0 );
	exit(ex);
#endif
}

void Sys_Error( const char *error, ... )
{
	va_list argptr;
	char    string[1024];

	va_start (argptr,error);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);
	//((void)__android_log_print(ANDROID_LOG_ERROR,"JK3","%s", string));
	//Sys_ErrorDialog( string );
	Sys_Print( string );

	Sys_Exit( 3 );
}

void Sys_Quit (void) {
	//IN_Shutdown();

	Com_ShutdownZoneMemory();
	Com_ShutdownHunkMemory();

#ifdef DEDICATED
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
#endif
	Sys_Exit(0);
}

/*
=================
Sys_UnloadDll
=================
*/
void Sys_UnloadDll( void *dllHandle )
{
	if( !dllHandle )
	{
		Com_Printf("Sys_UnloadDll(NULL)\n");
		return;
	}

	//Sys_UnloadLibrary(dllHandle);
}



extern "C"
{
extern const char * getLibPath();
}

/*
=================
Sys_LoadDll

First try to load library name from system library path,
from executable path, then fs_basepath.
=================


void *Sys_LoadDll(const char *name, qboolean useSystemLib)
{
	void *dllhandle = NULL;


	char  lib_path[512];
	sprintf(lib_path,"%s/lib%s", getLibPath(),name);
	//LOGI("Trying to load Android lib: %s",lib_path);
	dllhandle = dlopen (lib_path, RTLD_LAZY );

	return dllhandle;

	if(useSystemLib)
		Com_Printf("Trying to load \"%s\"...\n", name);

	//if(!useSystemLib || !(dllhandle = Sys_LoadLibrary(name)))
	{
		const char *topDir;
		char libPath[MAX_OSPATH];

		topDir = Sys_BinaryPath();

		if(!*topDir)
			topDir = ".";

		Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, topDir);
		Com_sprintf(libPath, sizeof(libPath), "%s%c%s", topDir, PATH_SEP, name);

		if(!(dllhandle = Sys_LoadLibrary(libPath)))
		{
			const char *basePath = Cvar_VariableString("fs_basepath");

			if(!basePath || !*basePath)
				basePath = ".";

			if(FS_FilenameCompare(topDir, basePath))
			{
				Com_Printf("Trying to load \"%s\" from \"%s\"...\n", name, basePath);
				Com_sprintf(libPath, sizeof(libPath), "%s%c%s", basePath, PATH_SEP, name);
				dllhandle = Sys_LoadLibrary(libPath);
			}

			if(!dllhandle)
			{
				Com_Printf("Loading \"%s\" failed\n", name);
			}
		}
	}

	return dllhandle;
}*/

#ifdef MACOS_X
void *Sys_LoadMachOBundle( const char *name )
{
	if ( !FS_LoadMachOBundle(name) )
		return NULL;

	char *homepath = Cvar_VariableString( "fs_homepath" );
	char *gamedir = Cvar_VariableString( "fs_game" );
	char dllName[MAX_QPATH];

	Com_sprintf( dllName, sizeof(dllName), "%s_pk3" DLL_EXT, name );

	//load the unzipped library
	char *fn = FS_BuildOSPath( homepath, gamedir, dllName );

	void    *libHandle = Sys_LoadLibrary( fn );

	if ( libHandle != NULL ) {
		Com_Printf( "Loaded pk3 bundle %s.\n", name );
	}

	return libHandle;
}
#endif

/*
 =================
 Sys_LoadGameDll

 Used to load a development dll instead of a virtual machine
 =================
 */

//TODO: load mac dlls that are inside zip things inside pk3s.

void *Sys_LoadLegacyGameDll( const char *name, intptr_t (QDECL **vmMain)(int, ...), intptr_t (QDECL *systemcalls)(intptr_t, ...) )
{
	void	*libHandle = NULL;
	void	(QDECL *dllEntry)( intptr_t (QDECL *syscallptr)(intptr_t, ...) );
	char	*basepath;
	char	*homepath;
	char	*cdpath;
	char	*gamedir;
#ifdef MACOS_X
	char    *apppath;
#endif
	char	*fn;
	char	filename[MAX_OSPATH];

	Com_sprintf (filename, sizeof(filename), "%sarm" DLL_EXT, name);

#if 0
	libHandle = Sys_LoadLibrary( filename );
#endif

#ifdef MACOS_X
	//First, look for the old-style mac .bundle that's inside a pk3
    //It's actually zipped, and the zipfile has the same name as 'name'
    libHandle = Sys_LoadMachOBundle( name );
#endif

	if (!libHandle) {
		//Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", filename, Sys_LibraryError() );

		basepath = Cvar_VariableString( "fs_basepath" );
		homepath = Cvar_VariableString( "fs_homepath" );
		cdpath = Cvar_VariableString( "fs_cdpath" );
		gamedir = Cvar_VariableString( "fs_game" );
#ifdef MACOS_X
		apppath = Cvar_VariableString( "fs_apppath" );
#endif

		fn = FS_BuildOSPath( basepath, gamedir, filename );
		libHandle = Sys_LoadLibrary( fn );

		if ( !libHandle ) {
			Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			if( homepath[0] ) {
				Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
				fn = FS_BuildOSPath( homepath, gamedir, filename );
				libHandle = Sys_LoadLibrary( fn );
			}
			if ( !libHandle ) {
				Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#ifdef MACOS_X
				if( apppath[0] ) {
					fn = FS_BuildOSPath( apppath, gamedir, filename );
					libHandle = Sys_LoadLibrary( fn );
				}
                if ( !libHandle ) {
                    Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#endif
				if( cdpath[0] ) {
					fn = FS_BuildOSPath( cdpath, gamedir, filename );
					libHandle = Sys_LoadLibrary( fn );
				}
				if ( !libHandle ) {
					Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
					// now we try base
					fn = FS_BuildOSPath( basepath, BASEGAME, filename );
					libHandle = Sys_LoadLibrary( fn );
					if ( !libHandle ) {
						if( homepath[0] ) {
							Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
							fn = FS_BuildOSPath( homepath, BASEGAME, filename );
							libHandle = Sys_LoadLibrary( fn );
						}
						if ( !libHandle ) {
							Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#ifdef MACOS_X
							if( apppath[0] ) {
                                    fn = FS_BuildOSPath( apppath, BASEGAME, filename);
                                    libHandle = Sys_LoadLibrary( fn );
                                }
                                if ( !libHandle ) {
                                    Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#endif
							if( cdpath[0] ) {
								fn = FS_BuildOSPath( cdpath, BASEGAME, filename );
								libHandle = Sys_LoadLibrary( fn );
							}
							if ( !libHandle ) {
								Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
								return NULL;
							}
#ifdef MACOS_X
							}
#endif
						}
					}
				}
#ifdef MACOS_X
				}
#endif
			}
		}
	}

	dllEntry = ( void (QDECL *)( intptr_t (QDECL *)( intptr_t, ... ) ) )Sys_LoadFunction( libHandle, "dllEntry" );
	*vmMain = (intptr_t (QDECL *)(int,...))Sys_LoadFunction( libHandle, "vmMain" );
	if ( !*vmMain || !dllEntry ) {
		Com_Printf ( "Sys_LoadGameDll(%s) failed to find vmMain function:\n\"%s\" !\n", name, Sys_LibraryError() );
		Sys_UnloadLibrary( libHandle );
		return NULL;
	}

	Com_Printf ( "Sys_LoadGameDll(%s) found vmMain function at %p\n", name, *vmMain );
	dllEntry( systemcalls );

	return libHandle;
}

/*
=================
Sys_LoadDll

First try to load library name from system library path,
from executable path, then fs_basepath.
=================
*/
void *Sys_LoadDll( const char *name, qboolean useSystemLib )
{
	void *dllhandle = NULL;

	// Don't load any DLLs that end with the pk3 extension
	if ( COM_CompareExtension( name, ".pk3" ) )
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: Rejecting DLL named \"%s\"", name );
		return NULL;
	}

	if ( useSystemLib )
	{
		Com_Printf( "Trying to load \"%s\"...\n", name );

		dllhandle = Sys_LoadLibrary( name );
		if ( dllhandle )
			return dllhandle;

		Com_Printf( "%s(%s) failed: \"%s\"\n", __FUNCTION__, name, Sys_LibraryError() );
	}

	const char *binarypath = Sys_BinaryPath();
	const char *basepath = Cvar_VariableString( "fs_basepath" );

	if ( !*binarypath )
		binarypath = ".";

	const char *searchPaths[] = {
			binarypath,
			basepath,
	};
	const size_t numPaths = ARRAY_LEN( searchPaths );

	for ( size_t i = 0; i < numPaths; i++ )
	{
		const char *libDir = searchPaths[i];
		if ( !libDir[0] )
			continue;

		Com_Printf( "Trying to load \"%s\" from \"%s\"...\n", name, libDir );
		char *fn = va( "%s%c%s", libDir, PATH_SEP, name );
		dllhandle = Sys_LoadLibrary( fn );
		if ( dllhandle )
			return dllhandle;

		Com_Printf( "%s(%s) failed: \"%s\"\n", __FUNCTION__, fn, Sys_LibraryError() );
	}
	return NULL;
}

void *Sys_LoadGameDll( const char *name, void *(QDECL **moduleAPI)(int, ...) )
{
	void	*libHandle = NULL;
	char	*basepath;
	char	*homepath;
	char	*cdpath;
	char	*gamedir;
#ifdef MACOS_X
	char    *apppath;
#endif
	char	*fn;
	char	filename[MAX_OSPATH];

	//Com_sprintf (filename, sizeof(filename), "%s" ARCH_STRING DLL_EXT, name);

#if 0
	libHandle = Sys_LoadLibrary( filename );
#endif

#ifdef MACOS_X
	//First, look for the old-style mac .bundle that's inside a pk3
    //It's actually zipped, and the zipfile has the same name as 'name'
    libHandle = Sys_LoadMachOBundle( name );
#endif


	char  lib_path[512];
	sprintf(lib_path,"%s/lib%s", getLibPath(),filename);
	//LOGI("Trying to load Android lib: %s",lib_path);
	libHandle = dlopen (lib_path, RTLD_LAZY );



	if (!libHandle) {
		//Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", filename, Sys_LibraryError() );

		basepath = Cvar_VariableString( "fs_basepath" );
		homepath = Cvar_VariableString( "fs_homepath" );
		cdpath = Cvar_VariableString( "fs_cdpath" );
		gamedir = Cvar_VariableString( "fs_game" );
#ifdef MACOS_X
		apppath = Cvar_VariableString( "fs_apppath" );
#endif

		fn = FS_BuildOSPath( basepath, gamedir, filename );
		libHandle = Sys_LoadLibrary( fn );

		if ( !libHandle ) {
			Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
			if( homepath[0] ) {
				Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
				fn = FS_BuildOSPath( homepath, gamedir, filename );
				libHandle = Sys_LoadLibrary( fn );
			}
			if ( !libHandle ) {
				Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#ifdef MACOS_X
				if( apppath[0] ) {
					fn = FS_BuildOSPath( apppath, gamedir, filename );
					libHandle = Sys_LoadLibrary( fn );
				}
                if ( !libHandle ) {
                    Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#endif
				if( cdpath[0] ) {
					fn = FS_BuildOSPath( cdpath, gamedir, filename );
					libHandle = Sys_LoadLibrary( fn );
				}
				if ( !libHandle ) {
					Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
					// now we try base
					fn = FS_BuildOSPath( basepath, BASEGAME, filename );
					libHandle = Sys_LoadLibrary( fn );
					if ( !libHandle ) {
						if( homepath[0] ) {
							Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
							fn = FS_BuildOSPath( homepath, BASEGAME, filename );
							libHandle = Sys_LoadLibrary( fn );
						}
						if ( !libHandle ) {
							Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#ifdef MACOS_X
							if( apppath[0] ) {
                                    fn = FS_BuildOSPath( apppath, BASEGAME, filename);
                                    libHandle = Sys_LoadLibrary( fn );
                                }
                                if ( !libHandle ) {
                                    Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
#endif
							if( cdpath[0] ) {
								fn = FS_BuildOSPath( cdpath, BASEGAME, filename );
								libHandle = Sys_LoadLibrary( fn );
							}
							if ( !libHandle ) {
								Com_Printf( "Sys_LoadGameDll(%s) failed: \"%s\"\n", fn, Sys_LibraryError() );
								return NULL;
							}
#ifdef MACOS_X
							}
#endif
						}
					}
				}
#ifdef MACOS_X
				}
#endif
			}
		}
	}

	*moduleAPI = (void *(QDECL *)(int,...))Sys_LoadFunction( libHandle, "GetModuleAPI" );
	if ( !*moduleAPI ) {
		Com_Printf ( "Sys_LoadGameDll(%s) failed to find GetModuleAPI function:\n\"%s\" !\n", name, Sys_LibraryError() );
		Sys_UnloadLibrary( libHandle );
		return NULL;
	}

	return libHandle;
}

void    Sys_ConfigureFPU() { // bk001213 - divide by zero
#ifdef __linux2__
	#ifdef __i386
#ifndef NDEBUG
    // bk0101022 - enable FPE's in debug mode
    static int fpu_word = _FPU_DEFAULT & ~(_FPU_MASK_ZM | _FPU_MASK_IM);
    int current = 0;
    _FPU_GETCW(current);
    if ( current!=fpu_word) {
#if 0
        Com_Printf("FPU Control 0x%x (was 0x%x)\n", fpu_word, current );
        _FPU_SETCW( fpu_word );
        _FPU_GETCW( current );
        assert(fpu_word==current);
#endif
    }
#else // NDEBUG
    static int fpu_word = _FPU_DEFAULT;
    _FPU_SETCW( fpu_word );
#endif // NDEBUG
#endif // __i386
#endif // __linux
}

#ifdef MACOS_X
/*
 =================
 Sys_StripAppBundle

 Discovers if passed dir is suffixed with the directory structure of a Mac OS X
 .app bundle. If it is, the .app directory structure is stripped off the end and
 the result is returned. If not, dir is returned untouched.
 =================
 */
char *Sys_StripAppBundle( char *dir )
{
	static char cwd[MAX_OSPATH];

	Q_strncpyz(cwd, dir, sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "MacOS"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "Contents"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(!strstr(Sys_Basename(cwd), ".app"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	return cwd;
}
#endif

#ifndef DEFAULT_BASEDIR
#	ifdef MACOS_X
#		define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#	else
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#	endif
#endif

int main ( int argc, char* argv[] )
{
	int		i;
	char	commandLine[ MAX_STRING_CHARS ] = { 0 };

	// done before Com/Sys_Init since we need this for error output
	//Sys_CreateConsole();

	// no abort/retry/fail errors
	//SetErrorMode (SEM_FAILCRITICALERRORS);

	// get the initial time base
	Sys_Milliseconds();

#ifdef MACOS_X
	// This is passed if we are launched by double-clicking
	if ( argc >= 2 && Q_strncmp ( argv[1], "-psn", 4 ) == 0 )
		argc = 1;
#endif

	Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );
	Sys_SetDefaultInstallPath( DEFAULT_BASEDIR );

	// Concatenate the command line for passing to Com_Init
	for( i = 1; i < argc; i++ )
	{
		const bool containsSpaces = (strchr(argv[i], ' ') != NULL);
		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );

		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), " " );
	}

	Com_Init (commandLine);

	NET_Init();

#ifdef DEDICATED
	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
#else
	// hide the early console since we've reached the point where we
	// have a working graphics subsystems
/*	if (!com_dedicated->integer && !com_viewlog->integer)
	{
		Sys_ShowConsole(0, qfalse);
	}*/
#endif

	// main game loop
	while (1)
	{
#if defined __linux__ && defined DEDICATED
		Sys_ConfigureFPU();//FIXME: what's this for?
#endif
		// make sure mouse and joystick are only called once a frame
		IN_Frame();

		// run the game
		Com_Frame();
	}

	// never gets here
}
