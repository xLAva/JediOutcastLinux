#include "../client/client.h"
#include "sdl_local.h"


/*
==================
Sys_InitConsole
==================
*/
void	Sys_InitConsole( void ) {

}

/*
==================
Sys_ShowConsole
==================
*/
void	Sys_ShowConsole( int level, qboolean quitOnClose ) {
	
}



/*
================
Sys_ConsoleInput

Checks for a complete line of text typed in at the console.
Return NULL if a complete line is not ready.
================
*/
char *Sys_ConsoleInput( void ) {

	return NULL;
}

#ifdef _WINDOWS
void	Sys_CreateConsole() {

}

void	Sys_DestroyConsole() {

}

void Sys_SetErrorText(const char* msg)
{
	// open message box
}

void Conbuf_AppendText(char const*)
{

}

#endif