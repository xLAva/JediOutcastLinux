#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <dirent.h> 
#include <stdio.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <mntent.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>

#include <dlfcn.h>

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderer/tr_public.h"
#include "linux_local.h"

/*
==================
Sys_LowPhysicalMemory()
==================
*/
qboolean Sys_LowPhysicalMemory() 
{
	return qfalse;
}


/*
==================
Sys_BeginProfiling
==================
*/
void Sys_BeginProfiling( void ) {
	// this is just used on the mac build
}




/*
================
Sys_Error
================
*/
void Sys_Error( const char *error, ... ) {
  va_list     argptr;
  char        string[1024];

  // change stdin to non blocking
  fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);

  //Sys_Shutdown(); -> LAvaPort - we might need it

  va_start (argptr,error);
  vsprintf (string,error,argptr);
  va_end (argptr);
  fprintf(stderr, "Error: %s\n", string);

  exit (1);
	
}


/*
================
Sys_Quit
================
*/
void Sys_Quit( void ) {
	//Sys_Shutdown(); -> LAvaPort - we might need it
  fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	exit (0);
}


/*
==============
Sys_Print
==============
*/
void	Sys_Print( const char *msg )
{
	fputs(msg, stderr);
}


/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path ) {
	char	ospath[MAX_OSPATH];
	int		err;
	
	Com_sprintf( ospath, sizeof(ospath), "%s", path );
	
	err = mkdir( ospath, 0777 );
}

/*
==============
Sys_Cwd
==============
*/
char *Sys_Cwd( void ) {
	static char dir[MAX_OSPATH];
	int			l;
	
	getcwd( dir, sizeof( dir ) );
	dir[MAX_OSPATH-1] = 0;
	
	return dir;
}


/*
================
Sys_Milliseconds
================
*/
int curtime;
int	sys_timeBase;
int Sys_Milliseconds (void)
{
	//struct timeval tp;
	//struct timezone tzp;

	//gettimeofday(&tp, &tzp);
	
	//if (!sys_timeBase)
	//{
	//	sys_timeBase = tp.tv_sec;
	//	return tp.tv_usec/1000;
	//}

	//curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;

	//LAvaPort
	//try out higher resolution timer (not sure if we need this)
	
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	
	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_nsec/1000000;
	}

	curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_nsec/1000000;
	
	return curtime;
}

/*
==============
Sys_DefaultCDPath
==============
*/
char *Sys_DefaultCDPath( void ) {
	return "";
}

/*
==============
Sys_DefaultBasePath
==============
*/
char *Sys_DefaultBasePath( void ) {
	return Sys_Cwd();
}

static char programpath[MAX_OSPATH];

void SetProgramPath(char *path)
{
	char *p;

	Q_strncpyz(programpath, path, sizeof(programpath));
	if ((p = strrchr(programpath, '/')) != NULL)
		*p = 0; // remove program name, leave only path
}


int		sys_ticBase;
int		sys_msecBase;
int		sys_lastEventTic;

cvar_t	*sys_profile;
cvar_t	*sys_waitNextEvent;


/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/
#define	MAX_FOUND_FILES	0x1000

#define	MAX_FOUND_FILES	0x1000

char **Sys_ListFiles( const char *directory, const char *extension, int *numfiles, qboolean wantsubs )
{
	struct dirent *d;
	char *p;
	DIR		*fdir;
	qboolean dironly = wantsubs;
	char		search[MAX_OSPATH];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	int			flag;
	int         scancount;
	int         scanloop;
	struct dirent **scanlist;
	struct stat st;

	int			extLen;

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = qtrue;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

    scancount = scandir(directory, &scanlist, NULL, alphasort);
    if (scancount < 0) {
		*numfiles = 0;
		return NULL;
	}
    for (scanloop=0; scanloop < scancount; scanloop++)
    {
        d = scanlist[scanloop];
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < strlen( extension ) ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - strlen( extension ),
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = 0;

    for (scanloop=0; scanloop < scancount; scanloop++) {
        free(scanlist[scanloop]);
    }
    free(scanlist);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

  int i;
	listCopy = (char **) Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_LISTFILES, qfalse);
	for (i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	return listCopy;
}

void	Sys_FreeFileList( char **list ) {
	int		i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
================
Sys_CheckCD

Return true if the proper CD is in the drive
================
*/
qboolean	Sys_CheckCD( void ) {
	return qtrue;
}


/*
================
Sys_GetClipboardData

================
*/
char *Sys_GetClipboardData(void)
{
	return NULL;
}


/*
========================================================================

GAME DLL

========================================================================
*/
static void *game_library;

/*
=================
Sys_UnloadGame
=================
*/

void Sys_UnloadGame (void)
{
	if (game_library) 
		dlclose (game_library);
	game_library = NULL;
}

/*
=================
Sys_GetGameAPI

Loads the game dll
=================
*/
void *Sys_GetGameAPI (void *parms)
{
	void	*(*GetGameAPI) (void *);
	char	name[MAX_OSPATH];
	char	cwd[MAX_OSPATH];
#ifdef __i386__
	const char *gamename = "jk2gamex86.so";

#ifdef NDEBUG
	const char *debugdir = "Release";
#else
	const char *debugdir = "Debug";
#endif	//NDEBUG

#endif //__i386__

	if (game_library)
		Com_Error (ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	// check the current debug directory first for development purposes
	getcwd (cwd, sizeof(cwd));
	Com_sprintf (name, sizeof(name), "%s/%s/%s", cwd, debugdir, gamename);
	game_library = dlopen (name, RTLD_LAZY );
	if (game_library)
	{
		Com_DPrintf ("LoadLibrary (%s)\n", name);
	}
	else
	{
		// check the current directory for other development purposes
		Com_sprintf (name, sizeof(name), "%s/%s", cwd, gamename);
		game_library = dlopen (name, RTLD_LAZY );
		if (game_library)
		{
			Com_DPrintf ("LoadLibrary (%s)\n", name);
		} else {

			Com_Error( ERR_FATAL, "Couldn't load game: %s\n", dlerror() );
		}
	}

	GetGameAPI = (void *(*)(void *))dlsym (game_library, "GetGameAPI");
	if (!GetGameAPI)
	{
	  const char* error = dlerror();
	  printf("game_library error: %s\n", error);
	  Com_Error( ERR_FATAL, "dlsym() failed on GetGameAPI" );
		Sys_UnloadGame ();		
		return NULL;
	}
	return GetGameAPI (parms);
}


/*
=================
Sys_LoadCgame

Used to hook up a development dll
=================
*/
void * Sys_LoadCgame( int (**entryPoint)(int, ...), int (*systemcalls)(int, ...) ) 
{
	void	(*dllEntry)( int (*syscallptr)(int, ...) );

	dllEntry = ( void (*)( int (*)( int, ... ) ) )dlsym( game_library, "dllEntry" ); 
	*entryPoint = (int (*)(int,...))dlsym( game_library, "vmMain" );
	if ( !*entryPoint || !dllEntry ) {
		//FreeLibrary( game_library );
		printf("Sys_LoadCgame failed.");
		return NULL;
	}

	dllEntry( systemcalls );
	return game_library;
}

/*
========================================================================

BACKGROUND FILE STREAMING

========================================================================
*/

void Sys_InitStreamThread( void ) {
}

void Sys_ShutdownStreamThread( void ) {
}

void Sys_BeginStreamedFile( fileHandle_t f, int readAhead ) {
}

void Sys_EndStreamedFile( fileHandle_t f ) {
}

int Sys_StreamedRead( void *buffer, int size, int count, fileHandle_t f ) {
   return FS_Read( buffer, size * count, f );
}

void Sys_StreamSeek( fileHandle_t f, int offset, int origin ) {
   FS_Seek( f, offset, origin );
}



//===================================================================

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define	MAX_QUED_EVENTS		256
#define	MASK_QUED_EVENTS	( MAX_QUED_EVENTS - 1 )

sysEvent_t	eventQue[MAX_QUED_EVENTS];
int			eventHead, eventTail;
byte		sys_packetReceived[MAX_MSGLEN];

/*
================
Sys_QueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr ) {
	sysEvent_t	*ev;

	ev = &eventQue[ eventHead & MASK_QUED_EVENTS ];
	if ( eventHead - eventTail >= MAX_QUED_EVENTS ) {
		Com_Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr ) {
			Z_Free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 ) {
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}


/*
================
Sys_GetEvent

================
*/


sysEvent_t Sys_GetEvent( void ) {
	sysEvent_t	ev;
	char		*s;
	msg_t		netmsg;

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// pump the message loop
	// in vga this calls KBD_Update, under X, it calls GetEvent
  
	Sys_SendKeyEvents ();

	// check for console commands
	
  //LAvaPort ... work in progress	
	//s = Sys_ConsoleInput();
	s = NULL;
	if ( s ) {
		char	*b;
		int		len;

		len = strlen( s ) + 1;
		b = (char *) Z_Malloc( len, TAG_EVENT, qfalse);
		strcpy( b, s );
		Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// check for other input devices
	IN_Frame();

	// check for network packets
	MSG_Init( &netmsg, sys_packetReceived, sizeof( sys_packetReceived ) );

	// return if we have data
	if ( eventHead > eventTail ) {
		eventTail++;
		return eventQue[ ( eventTail - 1 ) & MASK_QUED_EVENTS ];
	}

	// create an empty event to return

	memset( &ev, 0, sizeof( ev ) );

	ev.evTime = Sys_Milliseconds();

  
	return ev;
}

//================================================================

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( void ) {
  //LAvaPort ... work in progress
	//IN_Shutdown();
	//IN_Init();
}

static bool Sys_IsExpired()
{
#if 0
//								sec min Hr Day Mon Yr
    struct tm t_valid_start	= { 0, 0, 17, 23, 9, 101 };	//zero based months!
//								sec min Hr Day Mon Yr
    struct tm t_valid_end	= { 0, 0, 19, 2, 10, 101 };
//    struct tm t_valid_end	= t_valid_start;
//	t_valid_end.tm_mday += 8;
	time_t startTime  = mktime( &t_valid_start);
	time_t expireTime = mktime( &t_valid_end);
	time_t now;
	time(&now);
	if((now < startTime) || (now> expireTime))
	{
		return true;
	}
#endif
	return false;
}

/*
================
Sys_Init

Called after the common systems (cvars, files, etc)
are initialized
================
*/
void Sys_Init(void)
{
	Cmd_AddCommand ("in_restart", Sys_In_Restart_f);

#if id386
  //LAvaPort ... work in progress
	//Sys_SetFPCW();
#endif

#if defined __linux__
#if defined __i386__
	Cvar_Set( "arch", "linux i386" );
#elif defined __alpha__
	Cvar_Set( "arch", "linux alpha" );
#elif defined __sparc__
	Cvar_Set( "arch", "linux sparc" );
#else
	Cvar_Set( "arch", "linux unknown" );
#endif
#elif defined __sun__
#if defined __i386__
	Cvar_Set( "arch", "solaris x86" );
#elif defined __sparc__
	Cvar_Set( "arch", "solaris sparc" );
#else
	Cvar_Set( "arch", "solaris unknown" );
#endif
#elif defined __sgi__
#if defined __mips__
	Cvar_Set( "arch", "sgi mips" );
#else
	Cvar_Set( "arch", "sgi unknown" );
#endif
#else
	Cvar_Set( "arch", "unknown" );
#endif

  Cvar_Set( "sys_cpustring", "generic" );

  //LAvaPort ... work in progress
	IN_Init();

}

// do a quick mem test to check for any potential future mem problems...
//
static void QuickMemTest(void)
{
  //left empty for now
}





/*
=============
main
=============
*/
uid_t saved_euid;
cvar_t *nostdout;

int main (int argc, char **argv)
{
	int 	oldtime, newtime;
	int		len, i;
	char	*cmdline;
	void SetProgramPath(char *path);

	// go back to real user for config loads
	saved_euid = geteuid();
	seteuid(getuid());

	SetProgramPath(argv[0]);

	// merge the command line, this is kinda silly
	for (len = 1, i = 1; i < argc; i++)
		len += strlen(argv[i]) + 1;
	cmdline = malloc(len);
	*cmdline = 0;
	for (i = 1; i < argc; i++) {
		if (i > 1)
			strcat(cmdline, " ");
		strcat(cmdline, argv[i]);
	}
	Com_Init(cmdline);

  //LAva Port in progress ...
	//NET_Init();

	fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);

	nostdout = Cvar_Get("nostdout", "0", 0);
	if (!nostdout->value) {
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
//		printf ("Linux Quake -- Version %0.3f\n", LINUX_VERSION);
	}

    while (1)
    {
		// set low precision every frame, because some system calls
		// reset it arbitrarily
		//Sys_LowFPPrecision ();    

        Com_Frame ();
    }
}
