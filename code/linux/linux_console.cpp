#include "../client/client.h"
#include "linux_local.h"


#define	CONSOLE_MASK	1023
static char	consoleChars[CONSOLE_MASK+1];
static int consoleHead, consoleTail;
static qboolean consoleDisplayed;

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
	
	if ( level ) {
		consoleDisplayed = qtrue;
		printf( "\n" );
	} else {
		// FIXME: I don't know how to hide this window...
		consoleDisplayed = qfalse;
	}	
}


/*
================
Sys_Print

This is called for all console output, even if the game is running
full screen and the dedicated console window is hidden.
================
*/
//void	Sys_Print( const char *text ) {
//	if ( !consoleDisplayed ) {
//		return;
//	}
//	printf( "%s", text );
//}


/*
==================
Sys_ConsoleEvent
==================
*/
  /* LAvaPort

qboolean Sys_ConsoleEvent( EventRecord *event ) {
	qboolean flag;
	
	flag = SIOUXHandleOneEvent(event);
	
	// track keyboard events so we can do console input,
	// because SIOUX doesn't offer a polled read as far
	// as I can tell...
	if ( flag && event->what == keyDown ) {
		int		myCharCode;
	
		myCharCode	= BitAnd( event->message, charCodeMask );
		if ( myCharCode == 8 || myCharCode == 28 ) {
			if ( consoleHead > consoleTail ) {
				consoleHead--;
			}
		} else if ( myCharCode >= 32 || myCharCode == 13 ) {
			consoleChars[ consoleHead & CONSOLE_MASK ] = myCharCode;
			consoleHead++;
		}
	}
		
	return flag;
	
}
*/


/*
================
Sys_ConsoleInput

Checks for a complete line of text typed in at the console.
Return NULL if a complete line is not ready.
================
*/
char *Sys_ConsoleInput( void ) {
  /*LAvaPort
	static char	string[1024];
	int		i;

	if ( consoleTail == consoleHead ) {
		return NULL;
	}
	
	for ( i = 0 ; i + consoleTail < consoleHead ; i++ ) {
		string[i] = consoleChars[ ( consoleTail + i ) & CONSOLE_MASK ];
		if ( string[i] == 13 ) {
			consoleTail += i + 1;
			string[i] = 0;
			return string;
		}
	}
		*/
	return NULL;
}

