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

#include <X11/extensions/xf86vmode.h>


extern	int		sys_ticBase;
extern	int		sys_msecBase;
extern	int		sys_lastEventTic;

extern	cvar_t	*sys_waitNextEvent;

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );


extern	int	vkeyToQuakeKey[256];
void Sys_SendKeyEvents (void);


void IN_Init(void);
void IN_DeactivateMouse( void );
void IN_Frame (void);

extern	qboolean	inputActive;

extern glconfig_t glConfig;

void WG_CheckHardwareGamma();
void WG_RestoreGamma();


struct x11drv_gamma_ramp {
	unsigned short red[256];
	unsigned short green[256];
	unsigned short blue[256];
};

void X11DRV_XF86VM_Init(Display* dpy);
BOOL X11DRV_XF86VM_GetGammaRamp(struct x11drv_gamma_ramp *ramp);
BOOL X11DRV_XF86VM_SetGammaRamp(struct x11drv_gamma_ramp *ramp);





