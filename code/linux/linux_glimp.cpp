/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.
**
*/

#include "../game/g_headers.h"

#include "../game/b_local.h"
#include "../game/q_shared.h"

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <dlfcn.h>

#include "../renderer/tr_local.h"
#include "../client/client.h"

#include "linux_glw.h"
#include "linux_local.h"


#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/Xrandr.h>


typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;


glwstate_t glw_state;

static Display *dpy = NULL;
static int scrnum;
static Window win = 0;
static GLXContext ctx = NULL;

int num_sizes;
XRRScreenSize *xrrs;
XRRScreenConfiguration  *conf;
XRRScreenResources  *res;
short possible_frequencies[64][64];
short original_rate;
Rotation original_rotation;
SizeID original_size_id;

static qboolean autorepeaton = qtrue;

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )

static qboolean        mouse_avail;
static qboolean        mouse_active;
static int   mx, my;

static cvar_t	*in_mouse;
static cvar_t	*in_dgamouse;

static cvar_t	*r_fakeFullscreen;

Window root;
qboolean dgamouse = qfalse;
qboolean vidmode_ext = qfalse;

static int win_x, win_y;

static bool default_safed = false;
static XF86VidModeModeLine default_vidmode;
static XF86VidModeModeInfo default_vidmodeInfo;
static XF86VidModeModeInfo **vidmodes;
static int default_dotclock_vidmode;
static int num_vidmodes;
static qboolean vidmode_active = qfalse;
static qboolean vidmode_xrandr = qfalse;
static qboolean vidmode_fullscreen = qfalse;

static int mouse_accel_numerator;
static int mouse_accel_denominator;
static int mouse_threshold;   


static void		GLW_InitExtensions( void );
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen );

//
// function declaration
//
void	 QGL_EnableLogging( qboolean enable );
qboolean QGL_Init( const char *dllname );
void     QGL_Shutdown( void );


/*****************************************************************************/

static qboolean signalcaught = qfalse;

static void signal_handler(int sig)
{
	if (signalcaught) {
		printf("DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n", sig);
		exit(1);
	}

	signalcaught = qtrue;
	printf("Received signal %d, exiting...\n", sig);
	GLimp_Shutdown();
	exit(1);
}

static void InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}

/*
** GLW_StartDriverAndSetMode
*/
static qboolean GLW_StartDriverAndSetMode( const char *drivername, 
										   int mode, 
										   qboolean fullscreen )
{
	rserr_t err;


	err = (rserr_t) GLW_SetMode( drivername, mode, fullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}
	return qtrue;
}

// borrowed from SDL (changing to SDL would be good too)
static int CheckXRandR()
{
	int major;
	int minor;
	int	r_useXRandr = (int) ri.Cvar_Get( "r_useXRandr", "1", CVAR_ARCHIVE);
	if (r_useXRandr)
	{
    /* Query the extension version */
    if ( !XRRQueryVersion(dpy, &major, &minor) ) {
        return 0;
    }
		return 1;
	}
	return 0;
}


void GLW_SetModeXRandr(int* actualWidth, int* actualHeight, qboolean* fullscreen, bool usePrimaryRes)
{
	int primaryWidth = -1;
	int primaryHeight = -1;
	
	res = XRRGetScreenResources (dpy, root);
	if (res)
	{
		XRRCrtcInfo* crtcInfo = NULL;
		
		RROutput primaryDisplay = XRRGetOutputPrimary(dpy, root);
		if (primaryDisplay > 0)
		{
			XRROutputInfo *output_info = XRRGetOutputInfo (dpy, res, primaryDisplay);
			if (output_info && output_info->crtc && output_info->connection != RR_Disconnected)
			{
				// get info from the primary display
				crtcInfo = XRRGetCrtcInfo (dpy, res, output_info->crtc);
			}
			XRRFreeOutputInfo(output_info);
			
		}
		
		if (!crtcInfo)
		{
			// primary display not found or it is not connected
			// just get the first active monitor
			XRROutputInfo *output_info = NULL;
			for (int i=0; i<res->noutput && !crtcInfo; i++)
			{
				output_info = XRRGetOutputInfo (dpy, res, res->outputs[i]);
				if (output_info && output_info->crtc && output_info->connection != RR_Disconnected)
				{
					crtcInfo = XRRGetCrtcInfo (dpy, res, output_info->crtc);
				}
				
				XRRFreeOutputInfo(output_info);
			}
		}
		
		if (crtcInfo)
		{
			primaryWidth = crtcInfo->width;
			primaryHeight = crtcInfo->height;

			XRRFreeCrtcInfo(crtcInfo);
		}
		
	}

  bool disablePrimary = false;
	if (primaryWidth == -1 || primaryHeight == -1)
	{
		disablePrimary = true;
		ri.Printf( PRINT_ALL, "...WARNING: xrandr query for the primary display resolution failed.\n" );
	}

	int best_fit, best_dist, dist, x, y;

	if (!default_safed)
	{
		default_safed = true;

		conf = XRRGetScreenInfo(dpy, root);
		original_rate = XRRConfigCurrentRate(conf);
		original_size_id = XRRConfigCurrentConfiguration(conf, &original_rotation);

	}


	xrrs   = XRRSizes(dpy, 0, &num_sizes);


	// Are we going fullscreen?  If so, let's change video mode
	if (fullscreen && !r_fakeFullscreen->integer) {
		best_dist = 9999999;
		best_fit = -1;

		if (disablePrimary || !usePrimaryRes)
		{
			for (int i = 0; i < num_sizes; i++) {
				//printf("width %d height %d \n",xrrs[i].width, xrrs[i].height);
				
				if (glConfig.vidWidth > xrrs[i].width ||
					glConfig.vidHeight > xrrs[i].height)
					continue;

				x = glConfig.vidWidth - xrrs[i].width;
				y = glConfig.vidHeight - xrrs[i].height;
				dist = (x * x) + (y * y);
				if (dist < best_dist) {
					best_dist = dist;
					best_fit = i;
					//printf("best width %d height\n",xrrs[i].width, xrrs[i].height);

				}
			}
		}

		if (!disablePrimary && best_fit == -1)
		{
			for (int i = 0; i < num_sizes; i++) 
			{
				//printf("width %d height %d pwidht %d pheight %d\n",xrrs[i].width, xrrs[i].height, primaryWidth, primaryHeight)
				
				if (primaryWidth == xrrs[i].width && primaryHeight == xrrs[i].height)
				{
					best_fit = i;
					glConfig.windowAspect = 1;
					break;
				}
			}			
		}

		if (best_fit != -1) {
			(*actualWidth) = xrrs[best_fit].width;
			(*actualHeight) = xrrs[best_fit].height;

			printf("change res to %dx%d\n", xrrs[best_fit].width, xrrs[best_fit].height);

			// change to the mode
			XRRSetScreenConfig(dpy, conf, root, best_fit, original_rotation, CurrentTime);
			vidmode_active = qtrue;
			vidmode_xrandr = qtrue;

			// Move the viewport to top left
			//XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
		} else{
			(*fullscreen) = 0;
			ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		}
	}
}

/*
** GLW_SetMode
*/
int GLW_SetMode( const char *drivername, int mode, qboolean fullscreen )
{
	int attrib[] = {
		GLX_RGBA,					// 0
		GLX_RED_SIZE, 4,			// 1, 2
		GLX_GREEN_SIZE, 4,			// 3, 4
		GLX_BLUE_SIZE, 4,			// 5, 6
		GLX_DOUBLEBUFFER,			// 7
		GLX_DEPTH_SIZE, 1,			// 8, 9
		GLX_STENCIL_SIZE, 1,		// 10, 11
		None
	};
// these match in the array
#define ATTR_RED_IDX 2
#define ATTR_GREEN_IDX 4
#define ATTR_BLUE_IDX 6
#define ATTR_DEPTH_IDX 9
#define ATTR_STENCIL_IDX 11

	XVisualInfo *visinfo;
	XSetWindowAttributes attr;
	unsigned long mask;
	int colorbits, depthbits, stencilbits;
	int tcolorbits, tdepthbits, tstencilbits;
	int MajorVersion, MinorVersion;
	int actualWidth, actualHeight;
	int i;


	r_fakeFullscreen = ri.Cvar_Get( "r_fakeFullscreen", "0", CVAR_ARCHIVE);

	ri.Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode ) )
	{
		ri.Printf( PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	ri.Printf( PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight);

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Error couldn't open the X display\n");
		return RSERR_INVALID_MODE;
	}

	scrnum = DefaultScreen(dpy);
	root = RootWindow(dpy, scrnum);
	
	actualWidth = glConfig.vidWidth;
	actualHeight = glConfig.vidHeight;
	
	// Are we going fullscreen?  If so, let's change video mode
	if (fullscreen && !r_fakeFullscreen->integer) {

		vidmode_xrandr = qfalse;
		if (CheckXRandR())
		{
			bool usePrimary = false;
			if (mode == 10)
			{
				printf("use primary resolution\n");
				usePrimary = true;
			}
			GLW_SetModeXRandr(&actualWidth, &actualHeight, &fullscreen, usePrimary);
		}
		else
		{
			// Get video mode list
			MajorVersion = MinorVersion = 0;
			if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion)) { 
				vidmode_ext = qfalse;
			} else {
				ri.Printf(PRINT_ALL, "Using XFree86-VidModeExtension Version %d.%d\n",
					MajorVersion, MinorVersion);
				vidmode_ext = qtrue;
			}

			if (vidmode_ext) {
				int best_fit, best_dist, dist, x, y;
				
				if (!default_safed)
				{
					default_safed = true;
					XF86VidModeGetModeLine(dpy, scrnum, (int*) &default_vidmodeInfo.dotclock, (XF86VidModeModeLine*)&default_vidmodeInfo.hdisplay);
					printf("default:\nwidth %d height %d htotal %d vtotal %d hskew %d flags %d\n",default_vidmodeInfo.hdisplay, default_vidmodeInfo.vdisplay, default_vidmodeInfo.htotal, default_vidmodeInfo.vtotal, default_vidmodeInfo.hskew, default_vidmodeInfo.flags);

				}

				
				XF86VidModeGetAllModeLines(dpy, scrnum, &num_vidmodes, &vidmodes);

				best_dist = 9999999;
				best_fit = -1;

				for (i = 0; i < num_vidmodes; i++) {
					//printf("width %d height %d htotal %d vtotal %d hskew %d flags %d\n",vidmodes[i]->hdisplay, vidmodes[i]->vdisplay, vidmodes[i]->htotal, vidmodes[i]->vtotal, vidmodes[i]->hskew, vidmodes[i]->flags);
					
					if (glConfig.vidWidth > vidmodes[i]->hdisplay ||
						glConfig.vidHeight > vidmodes[i]->vdisplay)
						continue;

					x = glConfig.vidWidth - vidmodes[i]->hdisplay;
					y = glConfig.vidHeight - vidmodes[i]->vdisplay;
					dist = (x * x) + (y * y);
					if (dist < best_dist) {
						best_dist = dist;
						best_fit = i;
						printf("best width %d height %d htotal %d vtotal %d hskew %d flags %d\n",vidmodes[i]->hdisplay, vidmodes[i]->vdisplay, vidmodes[i]->htotal, vidmodes[i]->vtotal, vidmodes[i]->hskew, vidmodes[i]->flags);

					}
				}

				if (best_fit != -1) {
					actualWidth = vidmodes[best_fit]->hdisplay;
					actualHeight = vidmodes[best_fit]->vdisplay;

					// change to the mode
					XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
					vidmode_active = qtrue;

					// Move the viewport to top left
					XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
				} else{
					fullscreen = 0;
					actualWidth = default_vidmodeInfo.hdisplay;
					actualHeight = default_vidmodeInfo.vdisplay;

					// change to the mode
					//XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
					//vidmode_active = qtrue;
					
				}
			}
		}		
	}

	vidmode_fullscreen = fullscreen;
	glConfig.vidWidth = actualWidth;
	glConfig.vidHeight = actualHeight;

	if (!r_colorbits->value)
		colorbits = 24;
	else
		colorbits = r_colorbits->value;

  //LAvaPort
	//if ( !Q_stricmp( r_glDriver->string, _3DFX_DRIVER_NAME ) )
	//	colorbits = 16;

	if (!r_depthbits->value)
		depthbits = 24;
	else
		depthbits = r_depthbits->value;
	stencilbits = r_stencilbits->value;

	for (i = 0; i < 16; i++) {
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i) {
			// one pass, reduce
			switch (i / 4) {
			case 2 :
				if (colorbits == 24)
					colorbits = 16;
				break;
			case 1 :
				if (depthbits == 24)
					depthbits = 16;
				else if (depthbits == 16)
					depthbits = 8;
			case 3 :
				if (stencilbits == 24)
					stencilbits = 16;
				else if (stencilbits == 16)
					stencilbits = 8;
			}
		}

		tcolorbits = colorbits;
		tdepthbits = depthbits;
		tstencilbits = stencilbits;

		if ((i % 4) == 3) { // reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}	

		if ((i % 4) == 2) { // reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1) { // reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		if (tcolorbits == 24) {
			attrib[ATTR_RED_IDX] = 8;
			attrib[ATTR_GREEN_IDX] = 8;
			attrib[ATTR_BLUE_IDX] = 8;
		} else  {
			// must be 16 bit
			attrib[ATTR_RED_IDX] = 4;
			attrib[ATTR_GREEN_IDX] = 4;
			attrib[ATTR_BLUE_IDX] = 4;
		}

		attrib[ATTR_DEPTH_IDX] = tdepthbits; // default to 24 depth
		attrib[ATTR_STENCIL_IDX] = tstencilbits;

#if 0
		ri.Printf( PRINT_DEVELOPER, "Attempting %d/%d/%d Color bits, %d depth, %d stencil display...", 
			attrib[ATTR_RED_IDX], attrib[ATTR_GREEN_IDX], attrib[ATTR_BLUE_IDX],
			attrib[ATTR_DEPTH_IDX], attrib[ATTR_STENCIL_IDX]);
#endif

		visinfo = qglXChooseVisual(dpy, scrnum, attrib);
		if (!visinfo) {
#if 0
			ri.Printf( PRINT_DEVELOPER, "failed\n");
#endif
			continue;
		}

#if 0
		ri.Printf( PRINT_DEVELOPER, "Successful\n");
#endif

		ri.Printf( PRINT_ALL, "Using %d/%d/%d Color bits, %d depth, %d stencil display.\n", 
			attrib[ATTR_RED_IDX], attrib[ATTR_GREEN_IDX], attrib[ATTR_BLUE_IDX],
			attrib[ATTR_DEPTH_IDX], attrib[ATTR_STENCIL_IDX]);

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		break;
	}

	if (!visinfo) {
		ri.Printf( PRINT_ALL, "Couldn't get a visual\n" );
		return RSERR_INVALID_MODE;
	}

	/* window attributes */
	attr.background_pixel = BlackPixel(dpy, scrnum);
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	if (vidmode_active) {
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore | 
			CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	
	} else
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(dpy, root, 0, 0, 
			actualWidth, actualHeight, 
			0, visinfo->depth, InputOutput,
			visinfo->visual, mask, &attr);
	XMapWindow(dpy, win);

	if (vidmode_active)
		XMoveWindow(dpy, win, 0, 0);

  // Check for DGA
	//if (in_dgamouse->value) {
		if (!XF86DGAQueryVersion(dpy, &MajorVersion, &MinorVersion)) { 
			// unable to query, probalby not supported
			ri.Printf( PRINT_ALL, "Failed to detect XF86DGA Mouse\n" );
			ri.Cvar_Set( "in_dgamouse", "0" );
		} else
			ri.Printf( PRINT_ALL, "XF86DGA Mouse (Version %d.%d) initialized\n",
				MajorVersion, MinorVersion);
	//}

	XFlush(dpy);

	ctx = qglXCreateContext(dpy, visinfo, NULL, True);

	qglXMakeCurrent(dpy, win, ctx);

	return RSERR_OK;
}


//--------------------------------------------
static void GLW_InitTextureCompression( void )
{
	qboolean newer_tc, old_tc;

	// Check for available tc methods.
	newer_tc = ( strstr( glConfig.extensions_string, "ARB_texture_compression" )
		&& strstr( glConfig.extensions_string, "EXT_texture_compression_s3tc" )) ? qtrue : qfalse;
	old_tc = ( strstr( glConfig.extensions_string, "GL_S3_s3tc" )) ? qtrue : qfalse;

	if ( old_tc )
	{
		ri.Printf( PRINT_ALL, "...GL_S3_s3tc available\n" );
	}

	if ( newer_tc )
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_compression_s3tc available\n" );
	}

	if ( !r_ext_compressed_textures->value )
	{
		// Compressed textures are off
		glConfig.textureCompression = TC_NONE;
		ri.Printf( PRINT_ALL, "...ignoring texture compression\n" );
	}
	else if ( !old_tc && !newer_tc )
	{
		// Requesting texture compression, but no method found
		glConfig.textureCompression = TC_NONE;
		ri.Printf( PRINT_ALL, "...no supported texture compression method found\n" );
		ri.Printf( PRINT_ALL, ".....ignoring texture compression\n" );
	}
	else
	{
		// some form of supported texture compression is avaiable, so see if the user has a preference
		if ( r_ext_preferred_tc_method->integer == TC_NONE )
		{
			// No preference, so pick the best
			if ( newer_tc )
			{
				ri.Printf( PRINT_ALL, "...no tc preference specified\n" );
				ri.Printf( PRINT_ALL, ".....using GL_EXT_texture_compression_s3tc\n" );
				glConfig.textureCompression = TC_S3TC_DXT;
			}
			else
			{
				ri.Printf( PRINT_ALL, "...no tc preference specified\n" );
				ri.Printf( PRINT_ALL, ".....using GL_S3_s3tc\n" );
				glConfig.textureCompression = TC_S3TC;
			}
		}
		else
		{
			// User has specified a preference, now see if this request can be honored
			if ( old_tc && newer_tc )
			{
				// both are avaiable, so we can use the desired tc method
				if ( r_ext_preferred_tc_method->integer == TC_S3TC )
				{
					ri.Printf( PRINT_ALL, "...using preferred tc method, GL_S3_s3tc\n" );
					glConfig.textureCompression = TC_S3TC;
				}
				else
				{
					ri.Printf( PRINT_ALL, "...using preferred tc method, GL_EXT_texture_compression_s3tc\n" );
					glConfig.textureCompression = TC_S3TC_DXT;
				}
			}
			else
			{
				// Both methods are not available, so this gets trickier
				if ( r_ext_preferred_tc_method->integer == TC_S3TC )
				{
					// Preferring to user older compression
					if ( old_tc )
					{
						ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
						glConfig.textureCompression = TC_S3TC;
					}
					else
					{
						// Drat, preference can't be honored 
						ri.Printf( PRINT_ALL, "...preferred tc method, GL_S3_s3tc not available\n" );
						ri.Printf( PRINT_ALL, ".....falling back to GL_EXT_texture_compression_s3tc\n" );
						glConfig.textureCompression = TC_S3TC_DXT;
					}
				}
				else
				{
					// Preferring to user newer compression
					if ( newer_tc )
					{
						ri.Printf( PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n" );
						glConfig.textureCompression = TC_S3TC_DXT;
					}
					else
					{
						// Drat, preference can't be honored 
						ri.Printf( PRINT_ALL, "...preferred tc method, GL_EXT_texture_compression_s3tc not available\n" );
						ri.Printf( PRINT_ALL, ".....falling back to GL_S3_s3tc\n" );
						glConfig.textureCompression = TC_S3TC;
					}
				}
			}
		}
	}
}

/*
** GLW_InitExtensions
*/
cvar_t *r_ATI_NPATCH_available = NULL;
static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	// Select our tc scheme
	GLW_InitTextureCompression();

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "EXT_texture_env_add" ) )
	{
		if ( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		}
		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	// GL_EXT_texture_filter_anisotropic
	glConfig.textureFilterAnisotropicAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "EXT_texture_filter_anisotropic" ) )
	{
		glConfig.textureFilterAnisotropicAvailable = qtrue;
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic available\n" );

		if ( r_ext_texture_filter_anisotropic->integer )
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_filter_anisotropic\n" );
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_filter_anisotropic\n" );
		}
		ri.Cvar_Set( "r_ext_texture_filter_anisotropic_avail", "1" );
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_filter_anisotropic not found\n" );
		ri.Cvar_Set( "r_ext_texture_filter_anisotropic_avail", "0" );
	}

	// GL_EXT_clamp_to_edge
	glConfig.clampToEdgeAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "GL_EXT_texture_edge_clamp" ) )
	{
		glConfig.clampToEdgeAvailable = qtrue;
		ri.Printf( PRINT_ALL, "...Using GL_EXT_texture_edge_clamp\n" );
	}

	// WGL_EXT_swap_control
	// LAvaPort - what is this - do we need it?
//	qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) qwglGetProcAddress( "wglSwapIntervalEXT" );
//	if ( qwglSwapIntervalEXT )
//	{
//		ri.Printf( PRINT_ALL, "...using WGL_EXT_swap_control\n" );
//		r_swapInterval->modified = qtrue;	// force a set next frame
//	}
//	else
//	{
//		ri.Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
//	}

	// GL_ARB_multitexture
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" )  )
	{
		if ( r_ext_multitexture->integer )
		{
			qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) dlsym( glw_state.OpenGLLib, "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glActiveTextureARB" );
			qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) dlsym( glw_state.OpenGLLib, "glClientActiveTextureARB" );

			if ( qglActiveTextureARB )
			{
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );

				if ( glConfig.maxActiveTextures > 1 )
				{
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}
				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	qglLockArraysEXT = NULL;
	qglUnlockArraysEXT = NULL;
	if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) )
	{
		if ( r_ext_compiled_vertex_array->integer )
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( int, int ) ) dlsym( glw_state.OpenGLLib, "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) dlsym( glw_state.OpenGLLib, "glUnlockArraysEXT" );
			if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
				ri.Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}

	qglPointParameterfEXT = NULL;
	qglPointParameterfvEXT = NULL;
	if ( strstr( glConfig.extensions_string, "GL_EXT_point_parameters" ) )
	{
		if ( r_ext_compiled_vertex_array->integer || 1)
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_point_parameters\n" );
			qglPointParameterfEXT = ( void ( APIENTRY * )( GLenum, GLfloat) ) dlsym( glw_state.OpenGLLib, "glPointParameterfEXT" );
			qglPointParameterfvEXT = ( void ( APIENTRY * )( GLenum, GLfloat *) ) dlsym( glw_state.OpenGLLib, "glPointParameterfvEXT" );
			if (!qglPointParameterfEXT || !qglPointParameterfvEXT) 
			{
				ri.Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_point_parameters\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_point_parameters not found\n" );
	}

#ifdef _NPATCH
	// GL_ATI_pn_triangles
	qglPNTrianglesiATI = NULL;
	r_ATI_NPATCH_available = ri.Cvar_Get( "r_ATI_NPATCH_available", "0",CVAR_ROM );
/*	if ( strstr( glConfig.extensions_string, "GL_ATI_pn_triangles" ) )
	{
		ri.Cvar_Set( "r_ATI_NPATCH_available", "1" );
		if ( r_ati_pn_triangles->integer )
		{	
			ri.Printf( PRINT_ALL, "...using GL_ATI_pn_triangles\n" );
			qglPNTrianglesiATI = ( void ( APIENTRY * )( GLenum, GLint ) ) qwglGetProcAddress( "glPNTrianglesiATI" );
			if (!qglPNTrianglesiATI) {
				ri.Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ATI_pn_triangles\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_ATI_pn_triangles not found\n" );
	}
*/
#endif // _NPATCH
}


/*
** GLW_LoadOpenGL
**
** GLimp_win.c internal function that that attempts to load and use 
** a specific OpenGL DLL.
*/
static qboolean GLW_LoadOpenGL()
{
	char buffer[1024];
	qboolean fullscreen;

	strcpy( buffer, OPENGL_DRIVER_NAME );

	ri.Printf( PRINT_ALL, "...loading %s: ", buffer );

	// load the QGL layer
	if ( QGL_Init( buffer ) ) 
	{
		fullscreen = r_fullscreen->integer;

		// create the window and set up the context
		if ( !GLW_StartDriverAndSetMode( buffer, r_mode->integer, fullscreen ) )
		{
			if (r_mode->integer != 3) {
				if ( !GLW_StartDriverAndSetMode( buffer, 3, fullscreen ) ) {
					goto fail;
				}
			} else
				goto fail;
		}

		return qtrue;
	}
	else
	{
		ri.Printf( PRINT_ALL, "failed\n" );
	}
fail:

	QGL_Shutdown();

	return qfalse;
}

/*
** GLimp_EndFrame
*/
void GLimp_EndFrame (void)
{
	//
	// swapinterval stuff
	//
	if ( r_swapInterval->modified ) {
		r_swapInterval->modified = qfalse;

		if ( !glConfig.stereoEnabled ) {	// why?
			//if ( qwglSwapIntervalEXT ) { // LAvaPort
			//	qwglSwapIntervalEXT( r_swapInterval->integer );
			//}
		}
	}


	// don't flip if drawing to front buffer
	if ( stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		qglXSwapBuffers(dpy, win);
	}

	// check logging
	QGL_EnableLogging( r_logFile->integer );
}

static void GLW_StartOpenGL( void )
{
	//
	// load and initialize the specific OpenGL driver
	//
	if ( !GLW_LoadOpenGL() )
	{
		ri.Error( ERR_FATAL, "GLW_StartOpenGL() - could not load OpenGL subsystem\n" );
	}
	

}

/*
** GLimp_Init
**
** This is the platform specific OpenGL initialization function.  It
** is responsible for loading OpenGL, initializing it, setting
** extensions, creating a window of the appropriate size, doing
** fullscreen manipulations, etc.  Its overall responsibility is
** to make sure that a functional OpenGL subsystem is operating
** when it returns to the ref.
*/
void GLimp_Init( void )
{
	char	buf[1024];
	cvar_t *lastValidRenderer = ri.Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );
	cvar_t	*cv;

	ri.Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );

	glConfig.deviceSupportsGamma = qfalse;

	InitSig();



	//r_allowSoftwareGL = ri.Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );

	// load appropriate DLL and initialize subsystem
	GLW_StartOpenGL();

	// get our config strings
	glConfig.vendor_string = (const char *) qglGetString (GL_VENDOR);
	glConfig.renderer_string = (const char *) qglGetString (GL_RENDERER);
	glConfig.version_string = (const char *) qglGetString (GL_VERSION);
	glConfig.extensions_string = (const char *) qglGetString (GL_EXTENSIONS);
	
	if (!glConfig.vendor_string || !glConfig.renderer_string || !glConfig.version_string || !glConfig.extensions_string)
	{
		ri.Error( ERR_FATAL, "GLimp_Init() - Invalid GL Driver\n" );
	}

	// OpenGL driver constants
	qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &glConfig.maxTextureSize );
	// stubbed or broken drivers may have reported 0...
	if ( glConfig.maxTextureSize <= 0 ) 
	{
		glConfig.maxTextureSize = 0;
	}

	//
	// chipset specific configuration
	//
	strcpy( buf, glConfig.renderer_string );
	strlwr( buf );

	//
	// NOTE: if changing cvars, do it within this block.  This allows them
	// to be overridden when testing driver fixes, etc. but only sets
	// them to their default state when the hardware is first installed/run.
	//

	if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) )
	{
		//reset to defaults
		ri.Cvar_Set( "r_picmip", "1" );
		
		if ( strstr( buf, "matrox" )) {
            ri.Cvar_Set( "r_allowExtensions", "0");			
		}
		else
		// Savage3D and Savage4 should always have trilinear enabled
		if ( strstr( buf, "savage3d" ) || strstr( buf, "s3 savage4" ) || strstr( buf, "geforce" ))
		{
			ri.Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
		}
		else
		{
			ri.Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
		}

		if ( strstr( buf, "kyro" ) )	
		{
			ri.Cvar_Set( "r_ext_texture_filter_anisotropic", "0");	//KYROs have it avail, but suck at it!
			ri.Cvar_Set( "r_ext_preferred_tc_method", "1");			//(Use DXT1 instead of DXT5 - same quality but much better performance on KYRO)
		}

		GLW_InitExtensions();
		
		//this must be a really sucky card!
		if ( (glConfig.textureCompression == TC_NONE) || (glConfig.maxActiveTextures < 2)  || (glConfig.maxTextureSize <= 512) )
		{
			ri.Cvar_Set( "r_picmip", "2");
			ri.Cvar_Set( "r_colorbits", "16");
			ri.Cvar_Set( "r_texturebits", "16");
			ri.Cvar_Set( "r_mode", "3");	//force 640
			Cmd_ExecuteString ("exec low.cfg\n");	//get the rest which can be pulled in after init
		}
	}
	
	ri.Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	GLW_InitExtensions();
	InitSig();
}


/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
}


/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.
*/
void GLimp_Shutdown( void )
{
//	const char *strings[] = { "soft", "hard" };
	const char *success[] = { "failed", "success" };
	int retVal;


	ri.Printf( PRINT_ALL, "Shutting down OpenGL subsystem\n" );

	// restore gamma.  We do this first because 3Dfx's extension needs a valid OGL subsystem
	//WG_RestoreGamma();


	if (!ctx || !dpy)
		return;
	IN_DeactivateMouse();
	XAutoRepeatOn(dpy);
	if (dpy) {
		if (ctx)
			qglXDestroyContext(dpy, ctx);
		if (win)
			XDestroyWindow(dpy, win);
		if (vidmode_active)
		{
			if (!vidmode_xrandr)
			{
				XF86VidModeSwitchToMode(dpy, scrnum, &default_vidmodeInfo);
			}
			else
			{
				XRRSetScreenConfig(dpy, conf, root, original_size_id, original_rotation, CurrentTime);
			}
		}
		XCloseDisplay(dpy);
	}
	vidmode_active = qfalse;
	dpy = NULL;
	win = 0;
	ctx = NULL;

	// close the r_logFile
	if ( glw_state.log_fp )
	{
		fclose( glw_state.log_fp );
		glw_state.log_fp = 0;
	}

	// shutdown QGL subsystem
	QGL_Shutdown();

	memset( &glConfig, 0, sizeof( glConfig ) );
	memset( &glState, 0, sizeof( glState ) );
}

/*
** GLimp_LogComment
*/
void GLimp_LogComment( char *comment ) 
{
	if ( glw_state.log_fp ) {
		fprintf( glw_state.log_fp, "%s", comment );
	}
}


/*
===========================================================

SMP acceleration

===========================================================
*/

sem_t	renderCommandsEvent;
sem_t	renderCompletedEvent;
sem_t	renderActiveEvent;

void (*glimpRenderThread)( void );

void* GLimp_RenderThreadWrapper( void *stub ) {
	glimpRenderThread();

#if 0
	// unbind the context before we die
	qglXMakeCurrent(dpy, None, NULL);
#endif
}
/*
=======================
GLimp_SpawnRenderThread
=======================
*/
pthread_t	renderThreadHandle;
qboolean GLimp_SpawnRenderThread( void (*function)( void ) ) {

	sem_init( &renderCommandsEvent, 0, 0 );
	sem_init( &renderCompletedEvent, 0, 0 );
	sem_init( &renderActiveEvent, 0, 0 );

	glimpRenderThread = function;

	if (pthread_create( &renderThreadHandle, NULL,
		GLimp_RenderThreadWrapper, NULL)) {
		return qfalse;
	}

	return qtrue;
}

static	void	*smpData;
static	int		glXErrors;

void *GLimp_RendererSleep( void ) {
	void	*data;

#if 0
	if ( !qglXMakeCurrent(dpy, None, NULL) ) {
		glXErrors++;
	}
#endif

//	ResetEvent( renderActiveEvent );

	// after this, the front end can exit GLimp_FrontEndSleep
	sem_post ( &renderCompletedEvent );

	sem_wait ( &renderCommandsEvent );

#if 0
	if ( !qglXMakeCurrent(dpy, win, ctx) ) {
		glXErrors++;
	}
#endif

//	ResetEvent( renderCompletedEvent );
//	ResetEvent( renderCommandsEvent );

	data = smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	sem_post ( &renderActiveEvent );

	return data;
}


void GLimp_FrontEndSleep( void ) {
	sem_wait ( &renderCompletedEvent );

#if 0
	if ( !qglXMakeCurrent(dpy, win, ctx) ) {
		glXErrors++;
	}
#endif
}


void GLimp_WakeRenderer( void *data ) {
	smpData = data;

#if 0
	if ( !qglXMakeCurrent(dpy, None, NULL) ) {
		glXErrors++;
	}
#endif

	// after this, the renderer can continue through GLimp_RendererSleep
	sem_post( &renderCommandsEvent );

	sem_wait( &renderActiveEvent );
}



qboolean			inputActive;
qboolean			inputSuspended;

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | \
		    PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask )




void Input_Init(void);
void Input_GetState( void );


/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/

static unsigned int	keyshift[256];		// key to map to if shift held down in console
static qboolean shift_down=qfalse;

static char *XLateKey(XKeyEvent *ev, int *key)
{
	static char buf[64];
	KeySym keysym;
	static qboolean setup = qfalse;
	int i;

	*key = 0;

	XLookupString(ev, buf, sizeof buf, &keysym, 0);

// ri.Printf( PRINT_ALL, "keysym=%04X\n", (int)keysym);
	switch(keysym)
	{
		case XK_KP_Page_Up:	
		case XK_KP_9:	 *key = A_KP_9; break;
		case XK_Page_Up:	 *key = A_PAGE_UP; break;

		case XK_KP_Page_Down: 
		case XK_KP_3: *key = A_KP_3; break;
		case XK_Page_Down:	 *key = A_PAGE_DOWN; break;

		case XK_KP_Home:
		case XK_KP_7: *key = A_KP_7; break;
		case XK_Home:	 *key = A_HOME; break;

		case XK_KP_End:
		case XK_KP_1:	  *key = A_KP_1; break;
		case XK_End:	 *key = A_END; break;

		case XK_KP_Left: 
		case XK_KP_4: *key = A_KP_4; break;
		case XK_Left:	 *key = A_CURSOR_LEFT; break;

		case XK_KP_Right:
		case XK_KP_6: *key = A_KP_6; break;
		case XK_Right:	*key = A_CURSOR_RIGHT;		break;

		case XK_KP_Down:
		case XK_KP_2: 	 *key = A_KP_2; break;
		case XK_Down:	 *key = A_CURSOR_DOWN; break;

		case XK_KP_Up:   
		case XK_KP_8:    *key = A_KP_8; break;
		case XK_Up:		 *key = A_CURSOR_UP;	 break;

		case XK_Escape: *key = A_ESCAPE;		break;

		case XK_KP_Enter: *key = A_KP_ENTER;	break;
		case XK_Return: *key = A_ENTER;		 break;

		case XK_Tab:		*key = A_TAB;			 break;

		case XK_F1:		 *key = A_F1;				break;

		case XK_F2:		 *key = A_F2;				break;

		case XK_F3:		 *key = A_F3;				break;

		case XK_F4:		 *key = A_F4;				break;

		case XK_F5:		 *key = A_F5;				break;

		case XK_F6:		 *key = A_F6;				break;

		case XK_F7:		 *key = A_F7;				break;

		case XK_F8:		 *key = A_F8;				break;

		case XK_F9:		 *key = A_F9;				break;

		case XK_F10:		*key = A_F10;			 break;

		case XK_F11:		*key = A_F11;			 break;

		case XK_F12:		*key = A_F12;			 break;

		case XK_BackSpace: *key = A_BACKSPACE; break; // ctrl-h

		case XK_KP_Delete:
		case XK_KP_Decimal: *key = A_KP_PERIOD; break;
		case XK_Delete: *key = A_DELETE; break;

		case XK_Pause:	*key = A_PAUSE;		 break;

		case XK_Shift_L:
		case XK_Shift_R:	*key = A_SHIFT;		break;

		case XK_Execute: 
		case XK_Control_L: 
		case XK_Control_R:	*key = A_CTRL;		 break;

		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: *key = A_ALT;			break;

		case XK_KP_Begin: *key = A_KP_5;	break;

		//case XK_Insert:		*key = K_INS; break;
		case XK_KP_Insert:
		case XK_KP_0: *key = A_KP_0; break;

		case XK_KP_Multiply: *key = '*'; break;
		case XK_KP_Add:  *key = A_KP_PLUS; break;
		case XK_KP_Subtract: *key = A_KP_MINUS; break;
		//case XK_KP_Divide: *key = K_KP_SLASH; break;

		default:
			*key = *(unsigned char *)buf;
			if (*key >= 'A' && *key <= 'Z')
				*key = *key - 'A' + 'a';
			break;
	} 

	return buf;
}

// ========================================================================
// makes a null cursor
// ========================================================================

static Cursor CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
          &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}

static void install_grabs(void)
{
// inviso cursor
	XDefineCursor(dpy, win, CreateNullCursor(dpy, win));

	XGetPointerControl(dpy, &mouse_accel_numerator, &mouse_accel_denominator,
		&mouse_threshold);


	if (vidmode_fullscreen)
	{

		XGrabPointer(dpy, win,
						 False,
						 MOUSE_MASK,
						 GrabModeAsync, GrabModeAsync,
						 win,
						 None,
						 CurrentTime);

		XChangePointerControl(dpy, qtrue, qtrue, 2, 1, 0);

		if (in_dgamouse->value) {
			int MajorVersion, MinorVersion;

			if (!XF86DGAQueryVersion(dpy, &MajorVersion, &MinorVersion)) { 
				// unable to query, probalby not supported
				ri.Printf( PRINT_ALL, "Failed to detect XF86DGA Mouse\n" );
				ri.Cvar_Set( "in_dgamouse", "0" );
			} else {
				dgamouse = qtrue;
				XF86DGADirectVideo(dpy, DefaultScreen(dpy), XF86DGADirectMouse);
				XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
			}
		} else {
			XWarpPointer(dpy, None, win,
						 0, 0, 0, 0,
						 glConfig.vidWidth / 2, glConfig.vidHeight / 2);
		}

		XGrabKeyboard(dpy, win,
				  False,
				  GrabModeAsync, GrabModeAsync,
				  CurrentTime);
	}
//	XSync(dpy, True);
}

static void uninstall_grabs(void)
{
	if (dgamouse) {
		dgamouse = qfalse;
		XF86DGADirectVideo(dpy, DefaultScreen(dpy), 0);
	}

	XChangePointerControl(dpy, qtrue, qtrue, mouse_accel_numerator, 
		mouse_accel_denominator, mouse_threshold);

	XUngrabPointer(dpy, CurrentTime);
	XUngrabKeyboard(dpy, CurrentTime);

// inviso cursor
	XUndefineCursor(dpy, win);

//	XAutoRepeatOn(dpy);

//	XSync(dpy, True);
}

static void HandleEvents(void)
{
	int b;
	int key;
	XEvent event;
	qboolean dowarp = qfalse;
	int mwx = glConfig.vidWidth/2;
	int mwy = glConfig.vidHeight/2;
	char *p;
   
	if (!dpy)
		return;

	while (XPending(dpy)) {
		XNextEvent(dpy, &event);
		switch(event.type) {
		case KeyPress:
			p = XLateKey(&event.xkey, &key);
			if (key)
				Sys_QueEvent( 0, SE_KEY, key, qtrue, 0, NULL );
			while (*p)
				Sys_QueEvent( 0, SE_CHAR, *p++, 0, 0, NULL );
			break;
		case KeyRelease:
			XLateKey(&event.xkey, &key);
			
			Sys_QueEvent( 0, SE_KEY, key, qfalse, 0, NULL );
			break;

#if 0
		case KeyPress:
		case KeyRelease:
			key = XLateKey(&event.xkey);
			
			Sys_QueEvent( 0, SE_KEY, key, event.type == KeyPress, 0, NULL );
			if (key == K_SHIFT)
				shift_down = (event.type == KeyPress);
			if (key < 128 && (event.type == KeyPress)) {
				if (shift_down)
					key = keyshift[key];
				Sys_QueEvent( 0, SE_CHAR, key, 0, 0, NULL );
			}
#endif
			break;

		case MotionNotify:
			if (mouse_active) {
				if (dgamouse) {
					if (abs(event.xmotion.x_root) > 1)
						mx += event.xmotion.x_root * 2;
					else
						mx += event.xmotion.x_root;
					if (abs(event.xmotion.y_root) > 1)
						my += event.xmotion.y_root * 2;
					else
						my += event.xmotion.y_root;
//					ri.Printf(PRINT_ALL, "mouse (%d,%d) (root=%d,%d)\n", event.xmotion.x + win_x, event.xmotion.y + win_y, event.xmotion.x_root, event.xmotion.y_root);
				} 
				else 
				{
//					ri.Printf(PRINT_ALL, "mouse x=%d,y=%d\n", (int)event.xmotion.x - mwx, (int)event.xmotion.y - mwy);
					mx += ((int)event.xmotion.x - mwx);
					my += ((int)event.xmotion.y - mwy);
					mwx = event.xmotion.x;
					mwy = event.xmotion.y;

					if (mx || my)
						dowarp = qtrue;
				}
			}
			break;

		case ButtonPress:
			if (event.xbutton.button == 4) {
				Sys_QueEvent( 0, SE_KEY, A_MWHEELUP, qtrue, 0, NULL );
			} else if (event.xbutton.button == 5) {
				Sys_QueEvent( 0, SE_KEY, A_MWHEELDOWN, qtrue, 0, NULL );
			} else {
				b=-1;
				if (event.xbutton.button == 1)
					b = 0;
				else if (event.xbutton.button == 2)
					b = 2;
				else if (event.xbutton.button == 3)
					b = 1;
				Sys_QueEvent( 0, SE_KEY, A_MOUSE1 + b, qtrue, 0, NULL );
			}
			break;

		case ButtonRelease:
			if (event.xbutton.button == 4) {
				Sys_QueEvent( 0, SE_KEY, A_MWHEELUP, qfalse, 0, NULL );
			} else if (event.xbutton.button == 5) {
				Sys_QueEvent( 0, SE_KEY, A_MWHEELDOWN, qfalse, 0, NULL );
			} else {
				b=-1;
				if (event.xbutton.button == 1)
					b = 0;
				else if (event.xbutton.button == 2)
					b = 2;
				else if (event.xbutton.button == 3)
					b = 1;
				Sys_QueEvent( 0, SE_KEY, A_MOUSE1 + b, qfalse, 0, NULL );
			}
			break;

		case CreateNotify :
			win_x = event.xcreatewindow.x;
			win_y = event.xcreatewindow.y;
			break;

		case ConfigureNotify :
			win_x = event.xconfigure.x;
			win_y = event.xconfigure.y;
			break;
		}
	}

	if (dowarp) {
		// move the mouse to the window center again 
		XWarpPointer(dpy,None,win,0,0,0,0, 
				(glConfig.vidWidth/2),(glConfig.vidHeight/2));
	}

}

void KBD_Init(void)
{
}

void KBD_Close(void)
{
}

void IN_ActivateMouse( void ) 
{
	if (!mouse_avail || !dpy || !win)
		return;

	if (!mouse_active) {
		mx = my = 0; // don't spazz
		install_grabs();
		mouse_active = qtrue;
	}
}

void IN_DeactivateMouse( void ) 
{
	if (!mouse_avail || !dpy || !win)
		return;

	if (mouse_active) {
		uninstall_grabs();
		mouse_active = qfalse;
	}
}

/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

void IN_Init(void)
{
	// mouse variables
    in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);
    in_dgamouse = Cvar_Get ("in_dgamouse", "1", CVAR_ARCHIVE);

	if (in_mouse->value)
		mouse_avail = qtrue;
	else
		mouse_avail = qfalse;
}

void IN_Shutdown(void)
{
	mouse_avail = qfalse;
}

void IN_MouseMove(void)
{
	if (!mouse_avail || !dpy || !win)
		return;

#if 0
	if (!dgamouse) {
		Window root, child;
		int root_x, root_y;
		int win_x, win_y;
		unsigned int mask_return;
		int mwx = glConfig.vidWidth/2;
		int mwy = glConfig.vidHeight/2;

		XQueryPointer(dpy, win, &root, &child, 
			&root_x, &root_y, &win_x, &win_y, &mask_return);

		mx = win_x - mwx;
		my = win_y - mwy;

		XWarpPointer(dpy,None,win,0,0,0,0, mwx, mwy);
	}
#endif

	if (mx || my)
		Sys_QueEvent( 0, SE_MOUSE, mx, my, 0, NULL );
	mx = my = 0;
}

void IN_Frame (void)
{
	if ( cls.keyCatchers || cls.state != CA_ACTIVE ) {
		// temporarily deactivate if not in the game and
		// running on the desktop
		// voodoo always counts as full screen
//		if (Cvar_VariableValue ("r_fullscreen") == 0
//			&& strcmp( Cvar_VariableString("r_glDriver"), _3DFX_DRIVER_NAME ) )	{
//			IN_DeactivateMouse ();
//			return;
//		}
		if (dpy && !autorepeaton) {
			XAutoRepeatOn(dpy);
			autorepeaton = qtrue;
		}
	} else if (dpy && autorepeaton) {
		XAutoRepeatOff(dpy);
		autorepeaton = qfalse;
	}

	IN_ActivateMouse();

	// post events to the system que
	IN_MouseMove();
}

void IN_Activate(void)
{
}

void Sys_SendKeyEvents (void)
{
	XEvent event;

	if (!dpy)
		return;

	HandleEvents();
//	while (XCheckMaskEvent(dpy,KEY_MASK|MOUSE_MASK,&event))
//		HandleEvent(&event);
}
