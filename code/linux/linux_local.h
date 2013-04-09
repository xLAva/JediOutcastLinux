



#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#include "linux_glw.h"

#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>


// mac_main.c
extern	int		sys_ticBase;
extern	int		sys_msecBase;
extern	int		sys_lastEventTic;

extern	cvar_t	*sys_waitNextEvent;

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );

// mac_event.c
extern	int	vkeyToQuakeKey[256];
void Sys_SendKeyEvents (void);

// mac_net.c
void Sys_InitNetworking( void );
void Sys_ShutdownNetworking( void );

// linux_input.c
void IN_Init(void);
void IN_Shutdown(void);
void Sys_Input( void );
void IN_ActivateMouse( void ) ;
void IN_DeactivateMouse( void );

extern	qboolean			inputActive;

// mac_glimp.c
extern glconfig_t glConfig;

// mac_console.c

void	Sys_InitConsole( void );
void	Sys_ShowConsole( int level, qboolean quitOnClose );
void	Sys_Print( const char *text );
char	*Sys_ConsoleInput( void );
//qboolean Sys_ConsoleEvent( EventRecord *event );


