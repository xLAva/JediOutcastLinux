#include "sdl_glw.h"

#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
void IN_DeactivateMouse( void );


void WG_CheckHardwareGamma();
void WG_RestoreGamma();

extern SDL_Window* s_pSdlWindow;




