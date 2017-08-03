/*
** LINUX_GAMMA.C
*/

#include "../server/exe_headers.h"


#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "sdl_local.h"
#include <stdint.h>

#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#define HIBYTE(W)       (unsigned char)(((W) >> 8) & 0xFF)

//static x11drv_gamma_ramp s_oldHardwareGamma;
static uint16_t sOriginalGammaRed[256];
static uint16_t sOriginalGammaGreen[256];
static uint16_t sOriginalGammaBlue[256];

/*
** WG_CheckHardwareGamma
**
** Determines if the underlying hardware supports gamma correction API.
*/

void WG_CheckHardwareGamma( void )
{
	//SDL_GetWindowGammaRamp
	
	
	glConfig.deviceSupportsGamma = qfalse;
	
	// [LAva] gamma is not working yet
	return;
	
	if (s_pSdlWindow == NULL)
	{
		return;
	}
	
	int ret = SDL_GetWindowGammaRamp(s_pSdlWindow, &sOriginalGammaRed[0], &sOriginalGammaGreen[0], &sOriginalGammaBlue[0]);
	if (ret != 0)
	{
		return;
	}

	glConfig.deviceSupportsGamma = qtrue;

//	if ( !r_ignorehwgamma->integer )
//	{
//		glConfig.deviceSupportsGamma = X11DRV_XF86VM_GetGammaRamp(&s_oldHardwareGamma );

//		if ( glConfig.deviceSupportsGamma )
//		{
//			//
//			// do a sanity check on the gamma values
//			//
//			if ( ( HIBYTE( s_oldHardwareGamma.red[255] ) <= HIBYTE( s_oldHardwareGamma.red[0] ) ) ||
//				 ( HIBYTE( s_oldHardwareGamma.green[255] ) <= HIBYTE( s_oldHardwareGamma.green[0] ) ) ||
//				 ( HIBYTE( s_oldHardwareGamma.blue[255] ) <= HIBYTE( s_oldHardwareGamma.blue[0] ) ) )
//			{
//				glConfig.deviceSupportsGamma = qfalse;
//				VID_Printf( PRINT_WARNING, "WARNING: device has broken gamma support, generated gamma.dat\n" );
//			}

//			//
//			// make sure that we didn't have a prior crash in the game, and if so we need to
//			// restore the gamma values to at least a linear value
//			//
//			if ( ( HIBYTE( s_oldHardwareGamma.red[181] ) == 255 ) )
//			{
//				int g;

//				VID_Printf( PRINT_WARNING, "WARNING: suspicious gamma tables, using linear ramp for restoration\n" );

//				for ( g = 0; g < 255; g++ )
//				{
//					s_oldHardwareGamma.red[g] = g << 8;
//					s_oldHardwareGamma.green[g] = g << 8;
//					s_oldHardwareGamma.blue[g] = g << 8;
//				}
//			}
//		}
//	}
}

/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
//	x11drv_gamma_ramp table;
//	int		i, j;
//    int		ret;


//	if ( !glConfig.deviceSupportsGamma || r_ignorehwgamma->integer) {
//		return;
//	}

////mapGammaMax();

//	for ( i = 0; i < 256; i++ ) {
//		table.red[i] = ( ( ( unsigned short ) red[i] ) << 8 ) | red[i];
//		table.green[i] = ( ( ( unsigned short ) green[i] ) << 8 ) | green[i];
//		table.blue[i] = ( ( ( unsigned short ) blue[i] ) << 8 ) | blue[i];
//	}



//	// enforce constantly increasing

//	for ( i = 1 ; i < 256 ; i++ ) {
//		if ( table.red[i] < table.red[i-1] ) {
//			table.red[i] = table.red[i-1];
//		}
//		if ( table.green[i] < table.green[i-1] ) {
//			table.green[i] = table.green[i-1];
//		}
//		if ( table.blue[i] < table.blue[i-1] ) {
//			table.blue[i] = table.blue[i-1];
//		}
//	}


	bool worked = false;
	
	if (glConfig.deviceSupportsGamma && s_pSdlWindow != NULL)
	{
		uint16_t sdlRed[256];
		uint16_t sdlGreen[256];
		uint16_t sdlBlue[256];
		
		for (int i = 0; i < 256; i++ ) {
			sdlRed[i] = red[i];//( ( ( unsigned short ) red[i] ) << 8 ) | red[i];
			sdlGreen[i] = green[i];//( ( ( unsigned short ) green[i] ) << 8 ) | green[i];
			sdlBlue[i] = blue[i];//( ( ( unsigned short ) blue[i] ) << 8 ) | blue[i];
		}
		
		int ret = SDL_SetWindowGammaRamp(s_pSdlWindow, &sdlRed[0], &sdlGreen[0], &sdlBlue[0]);
		worked = ret == 0;
	}
	
	
	if (!worked) 
	{
		Com_Printf( "SetDeviceGammaRamp failed.\n" );
	}
}

/*
** WG_RestoreGamma
*/
void WG_RestoreGamma( void )
{
	if (!glConfig.deviceSupportsGamma || s_pSdlWindow == NULL)
	{
		return;
	}

	SDL_SetWindowGammaRamp(s_pSdlWindow, &sOriginalGammaRed[0], &sOriginalGammaGreen[0], &sOriginalGammaBlue[0]);
}

