/*
** linux_joystick.c
**
** This file contains ALL Linux specific stuff having to do with the
** Joystick input.  When a port is being made the following functions
** must be implemented by the port:
**
** Authors: mkv, bk
**
*/

#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>  // bk001204

#include "../client/client.h"
#include "linux_local.h"

// TTimo moved joystick.h include here, conflicts in client.h with wkey_t
#include <linux/joystick.h>

/* We translate axes movement into keypresses. */
int joy_keys[16] = {
	A_CURSOR_LEFT, A_CURSOR_RIGHT,
	A_CURSOR_UP, A_CURSOR_DOWN,
	A_JOY16, A_JOY17,
	A_JOY18, A_JOY19,
	A_JOY20, A_JOY21,
	A_JOY22, A_JOY23,
	
	A_JOY24, A_JOY25,
	A_JOY26, A_JOY27
};

/* Our file descriptor for the joystick device. */
static int joy_fd = -1;

/*
#ifdef CROUCH
static int joyMoving = 0;
#endif
*/


// bk001130 - from linux_glimp.c
extern cvar_t *  in_joystick;
extern cvar_t *  in_joystickDebug;
extern cvar_t *  joy_threshold;


/**********************************************/
/* Joystick routines.                         */
/**********************************************/
// bk001130 - from cvs1.17 (mkv), removed from linux_glimp.c
void IN_StartupJoystick( void ) {
	int i = 0;

	joy_fd = -1;

	if ( !in_joystick->integer ) {
		Com_DPrintf( "Joystick is not active.\n" );
		return;
	}

	for ( i = 0; i < 4; i++ ) {
		char filename[255];

		#ifdef PANDORA
		snprintf( filename, 255, "/dev/input/js%d", i );
		#else
		snprintf( filename, 255, "/dev/js%d", i );
		#endif

		joy_fd = open( filename, O_RDONLY | O_NONBLOCK );

		if ( joy_fd != -1 ) {
			struct js_event event;
			char axes = 0;
			char buttons = 0;
			char name[128];
			int n = -1;

			Com_Printf( "Joystick %s found\n", filename );

			/* Get rid of initialization messages. */
			do {
				n = read( joy_fd, &event, sizeof( event ) );

				if ( n == -1 ) {
					break;
				}

			} while ( ( event.type & JS_EVENT_INIT ) );

			/* Get joystick statistics. */
			ioctl( joy_fd, JSIOCGAXES, &axes );
			ioctl( joy_fd, JSIOCGBUTTONS, &buttons );

			if ( ioctl( joy_fd, JSIOCGNAME( sizeof( name ) ), name ) < 0 ) {
				strncpy( name, "Unknown", sizeof( name ) );
			}

			Com_Printf( "Name:    %s\n", name );
			Com_Printf( "Axes:    %d\n", axes );
			Com_Printf( "Buttons: %d\n", buttons );

			/* Our work here is done. */
			return;
		}

	}

	/* No soup for you. */
	if ( joy_fd == -1 ) {
		Com_Printf( "No joystick found.\n" );
		return;
	}

}

void IN_JoyMove( void ) {
	/* Store instantaneous joystick state. Hack to get around
	 * event model used in Linux joystick driver.
	   */
	static int axes_state[16];
/*
#ifdef CROUCH
	static int axes_actif[2];
#endif
*/
	/* Old bits for Quake-style input compares. */
	static unsigned int old_axes = 0;
	/* Our current goodies. */
	unsigned int axes = 0;
	int i = 0;

	if ( joy_fd == -1 ) {
		return;
	}

	/* Empty the queue, dispatching button presses immediately
	   * and updating the instantaneous state for the axes.
	   */
	do {
		int n = -1;
		struct js_event event;

		n = read( joy_fd, &event, sizeof( event ) );

		if ( n == -1 ) {
			/* No error, we're non-blocking. */
			break;
		}

		if ( event.type & JS_EVENT_BUTTON ) {
			Sys_QueEvent( 0, SE_KEY, A_JOY1 + event.number, event.value, 0, NULL );
		} else if ( event.type & JS_EVENT_AXIS ) {

			if ( event.number >= 16 ) {
				continue;
			}

			axes_state[event.number] = event.value;
#if 1
			int value = ( axes_state[event.number] ) / (32768/128);
			if (value>127) value = 127;
			if (value<-127) value = -127;
#ifdef PANDORA
			Sys_QueEvent(0, SE_JOYSTICK_AXIS, event.number, (event.number==1)?-value:value, 0, NULL );
#else
			Sys_QueEvent(0, SE_JOYSTICK_AXIS, event.number, value, 0, NULL );
#endif
/*
#ifdef CROUCH
			if (value<0) value=-value;
			if (event.number<2) {
				axis_actif[event.number]=(value<10)?0:1;
			}
			joyMoving=axis_actif[0]|axis_actif[1];
#endif
*/
#endif
		} else {
			Com_Printf( "Unknown joystick event type\n" );
		}

	} while ( 1 );

#if 0
	/* Translate our instantaneous state to bits. */
	for ( i = 0; i < 16; i++ ) {
		float f = ( (float) axes_state[i] ) / 32767.0f;

		if ( f < -joy_threshold->value ) {
			axes |= ( 1 << ( i * 2 ) );
		} else if ( f > joy_threshold->value ) {
			axes |= ( 1 << ( ( i * 2 ) + 1 ) );
		}

	}

	/* Time to update axes state based on old vs. new. */
	for ( i = 0; i < 16; i++ ) {

		if ( ( axes & ( 1 << i ) ) && !( old_axes & ( 1 << i ) ) ) {
			Sys_QueEvent( 0, SE_KEY, joy_keys[i], qtrue, 0, NULL );
		}

		if ( !( axes & ( 1 << i ) ) && ( old_axes & ( 1 << i ) ) ) {
			Sys_QueEvent( 0, SE_KEY, joy_keys[i], qfalse, 0, NULL );
		}
	}

	/* Save for future generations. */
	old_axes = axes;
#endif
}


