/*
** GLW_IMP.C
**
** This file contains ALL platform specific stuff having to do with the
** OpenGL refresh.
**
*/

#include "../game/g_headers.h"

#include "../game/b_local.h"
#include "../game/q_shared.h"

#include "../renderer/tr_local.h"
#include "../client/client.h"

#include "sdl_local.h"
#include "sdl_glw.h"

#ifdef USE_VR
#include "../hmd/ClientHmd.h"
#include "../hmd/FactoryHmdDevice.h"
#include "../hmd/HmdDevice/IHmdDevice.h"
#include "../hmd/HmdRenderer/IHmdRenderer.h"
#include "../hmd/HmdRenderer/PlatformInfo.h"

#ifdef USE_OVR_0_5
#include "../hmd/OculusSdk_0.5/HmdRendererOculusSdk.h"
#endif

#ifdef USE_OVR_CURRENT
#include "../hmd/OculusSdk_current/HmdRendererOculusSdk.h"
#endif
#endif //USE_VR

#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#else
#include <SDL.h>
#include <SDL_syswm.h>
#include <algorithm>
#endif

using namespace std;

typedef enum {
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;


glwstate_t glw_state;

SDL_Window*   s_pSdlWindow = NULL;
SDL_Renderer* s_pSdlRenderer = NULL;
SDL_GLContext sGlContext = NULL;

int s_windowWidth = 0;
int s_windowHeight = 0;

static qboolean        mouse_avail;
static int   mx, my;

static cvar_t	*in_mouse;
static cvar_t	*r_fakeFullscreen;

static bool sWindowHasFocus = qtrue;
static qboolean sVideoModeFullscreen = qfalse;
static bool sRelativeMouseMode = false;


// Hack variable for deciding which kind of texture rectangle thing to do (for some
// reason it acts different on radeon! It's against the spec!).
bool g_bTextureRectangleHack = false;



static void		GLW_InitExtensions( void );
int GLW_SetMode(int mode, qboolean fullscreen );

//
// function declaration
//
void	 QGL_EnableLogging( qboolean enable );
qboolean QGL_Init( const char *dllname );
void	 QGL_Shutdown( void );


/*****************************************************************************/


static void InitSig(void)
{
	return;
}


static void QueKeyEvent(int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr)
{
#ifdef USE_OVR_0_5
	IHmdRenderer* pRenderer = ClientHmd::Get()->GetRenderer();
	if (pRenderer)
	{
		static OvrSdk_0_5::HmdRendererOculusSdk* pHmdRenderer = dynamic_cast<OvrSdk_0_5::HmdRendererOculusSdk*>(pRenderer);
		if (pHmdRenderer)
		{
			pHmdRenderer->DismissHealthSafetyWarning();
		}
	}
#endif
	
	Sys_QueEvent(time, type, value, value2, ptrLength, ptr);
}


/*
** GLW_StartDriverAndSetMode
*/
static qboolean GLW_StartDriverAndSetMode( const char *drivername, 
										   int mode, 
										   qboolean fullscreen )
{
	rserr_t err;

	err = (rserr_t) GLW_SetMode(mode, fullscreen );

	switch ( err )
	{
	case RSERR_INVALID_FULLSCREEN:
		VID_Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
		return qfalse;
	case RSERR_INVALID_MODE:
		VID_Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
		return qfalse;
	default:
		break;
	}
	return qtrue;
}



/*
** GLW_SetMode
*/
int GLW_SetMode(int mode, qboolean fullscreen )
{
	int colorbits, depthbits, stencilbits;
	int redbits, greenbits, bluebits;
	int actualWidth, actualHeight;


	r_fakeFullscreen = Cvar_Get( "r_fakeFullscreen", "0", CVAR_ARCHIVE);

	VID_Printf( PRINT_ALL, "Initializing OpenGL display\n");
	VID_Printf (PRINT_ALL, "...setting mode %d:\n", mode );

	float aspect;
	
	if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &aspect, mode ) )
	{
		Com_Error( PRINT_ALL, " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	
	if (mode == 10)
	{
		// use main display resolution
		int display_count = 0, display_index = 0, mode_index = 0;
		SDL_DisplayMode dm = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };

		int ret = SDL_GetDisplayMode(0, mode_index, &dm);
		if (ret == 0)
		{
			glConfig.vidWidth = dm.w;
			glConfig.vidHeight = dm.h;
		}
	}

	//glConfig.vidWidth = 640;
	//glConfig.vidHeight = 480;
	//fullscreen = false;
	
	actualWidth = glConfig.vidWidth;
	actualHeight = glConfig.vidHeight;
	
    s_windowWidth = actualWidth;
    s_windowHeight = actualHeight;

	bool fixedDeviceResolution = false;
	bool fullscreenWindow = false;
	
	bool useWindowPosition = false;
	int xPos = 0;
	int yPos = 0;
	
#ifdef USE_VR
	// check for hmd device
	IHmdDevice* pHmdDevice = ClientHmd::Get()->GetDevice();
	if (pHmdDevice)
	{
		// found hmd device - test if device has a display
		bool displayFound = pHmdDevice->HasDisplay();
		if (displayFound)
		{
			int deviceWidth = 0;
			int deviceHeight = 0;
			bool isRotated = false;
			bool isExtendedMode = false;
			pHmdDevice->GetDeviceResolution(deviceWidth, deviceHeight, isRotated, isExtendedMode);
			
			fixedDeviceResolution = true;
			actualWidth = isRotated ? deviceHeight : deviceWidth;
			actualHeight = isRotated ? deviceWidth : deviceHeight;
	
			s_windowWidth = actualWidth;
			s_windowHeight = actualHeight;

			glConfig.vidWidth = deviceWidth / 2;
			glConfig.vidHeight = deviceHeight;
						
			useWindowPosition = pHmdDevice->GetDisplayPos(xPos, yPos);
			fullscreen = isExtendedMode;

			VID_Printf( PRINT_ALL, "hmd display: %s\n", pHmdDevice->GetDisplayDeviceName().c_str());    
			
			glConfig.stereoEnabled = qtrue; 
			
			Cvar_Set("r_stereo", "1");
		}
	}
#endif
	
	sVideoModeFullscreen = fullscreen || fullscreenWindow;
	mx = 0;
	my = 0;

	VID_Printf( PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight);    


	if (!r_colorbits->value)
		colorbits = 24;
	else
		colorbits = r_colorbits->value;

	if (!r_depthbits->value)
		depthbits = 24;
	else
		depthbits = r_depthbits->value;
	stencilbits = r_stencilbits->value;
	
	if (colorbits == 24) 
	{
		redbits = 8;
		greenbits = 8;
		bluebits = 8;
	} 
	else  
	{
		// must be 16 bit
		redbits = 4;
		greenbits = 4;
		bluebits = 4;
	}
	
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, redbits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, greenbits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bluebits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthbits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilbits);
	
	
	int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	if (fullscreen)
	{
		windowFlags |= SDL_WINDOW_FULLSCREEN;
		windowFlags |= SDL_WINDOW_INPUT_GRABBED;
	}
	
	if (fullscreenWindow)
	{
		windowFlags |= SDL_WINDOW_BORDERLESS;
	}
	
	int displayPosX = useWindowPosition ? xPos : SDL_WINDOWPOS_UNDEFINED;
	int displayPosY = useWindowPosition ? yPos : SDL_WINDOWPOS_UNDEFINED;
	
	#ifdef LINUX
	// fullscreen is ... ARRGG! Legacy worked better on Ubuntu Unity, but gamepad is not working with legacy :-(
	//putenv("SDL_VIDEO_X11_LEGACY_FULLSCREEN=1");
	#endif
	
    VID_Printf(PRINT_ALL, "Create Window %dx%d at %dx%d\n", s_windowWidth, s_windowHeight, displayPosX, displayPosY);
    s_pSdlWindow = SDL_CreateWindow("Jedi Knight 2 SP", displayPosX, displayPosY, s_windowWidth, s_windowHeight, windowFlags);
	

	if (!s_pSdlWindow)
	{
		VID_Printf( PRINT_ALL, "CreateWindow failed: %s\n", SDL_GetError());
		return RSERR_UNKNOWN;
	}
	
#ifdef USE_OVR_0_5
	OvrSdk_0_5::HmdRendererOculusSdk* pHmdRenderer = dynamic_cast<OvrSdk_0_5::HmdRendererOculusSdk*>(ClientHmd::Get()->GetRenderer());
	if (pHmdRenderer)
	{
		SDL_SysWMinfo sysInfo;
		SDL_VERSION(&sysInfo.version); // initialize info structure with SDL version info
		SDL_GetWindowWMInfo(s_pSdlWindow, &sysInfo);

		void* pWindowHandle = NULL;
#ifdef LINUX
		if (sysInfo.subsystem == SDL_SYSWM_X11)
		{
			pWindowHandle = (void*)sysInfo.info.x11.window;
		}
#endif

#ifdef _WINDOWS
		if (sysInfo.subsystem == SDL_VIDEO_DRIVER_WINDOWS)
		{
			pWindowHandle = sysInfo.info.win.window;
		}
#endif

		if (pWindowHandle)
		{
			pHmdRenderer->AttachToWindow(pWindowHandle);
		}
	}
#endif

	sRelativeMouseMode = false;
	sWindowHasFocus = true;
	if (sVideoModeFullscreen)
	{
		VID_Printf( PRINT_ALL, "set relative mouse.");
		int mouseError = SDL_SetRelativeMouseMode(SDL_TRUE);
		if (mouseError != 0)
		{
			VID_Printf( PRINT_ALL, "set relative mouse motion failed: %s\n", SDL_GetError());
		}
		else
		{
			sRelativeMouseMode = true;
		}
	}

	sGlContext = SDL_GL_CreateContext(s_pSdlWindow);

	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &redbits);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &greenbits);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &bluebits);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depthbits);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &stencilbits);
	
	
	VID_Printf( PRINT_ALL, "Using %d/%d/%d Color bits, %d depth, %d stencil display.\n",
		redbits, greenbits, bluebits, depthbits, stencilbits);
	
	
	glConfig.colorBits = colorbits;
	glConfig.depthBits = depthbits;
	glConfig.stencilBits = stencilbits;
	
	if (!fixedDeviceResolution)
	{
		SDL_GL_GetDrawableSize(s_pSdlWindow, &actualWidth, &actualHeight);
		glConfig.vidWidth = actualWidth;
		glConfig.vidHeight = actualHeight;
	}
	
	if (fullscreenWindow)
	{
		SDL_SetWindowPosition(s_pSdlWindow, xPos, yPos);
	}
	
	WG_CheckHardwareGamma();

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
		VID_Printf( PRINT_ALL, "...GL_S3_s3tc available\n" );
	}

	if ( newer_tc )
	{
		VID_Printf( PRINT_ALL, "...GL_EXT_texture_compression_s3tc available\n" );
	}

	if ( !r_ext_compressed_textures->value )
	{
		// Compressed textures are off
		glConfig.textureCompression = TC_NONE;
		VID_Printf( PRINT_ALL, "...ignoring texture compression\n" );
	}
	else if ( !old_tc && !newer_tc )
	{
		// Requesting texture compression, but no method found
		glConfig.textureCompression = TC_NONE;
		VID_Printf( PRINT_ALL, "...no supported texture compression method found\n" );
		VID_Printf( PRINT_ALL, ".....ignoring texture compression\n" );
	}
	else
	{
		// some form of supported texture compression is avaiable, so see if the user has a preference
		if ( r_ext_preferred_tc_method->integer == TC_NONE )
		{
			// No preference, so pick the best
			if ( newer_tc )
			{
				VID_Printf( PRINT_ALL, "...no tc preference specified\n" );
				VID_Printf( PRINT_ALL, ".....using GL_EXT_texture_compression_s3tc\n" );
				glConfig.textureCompression = TC_S3TC_DXT;
			}
			else
			{
				VID_Printf( PRINT_ALL, "...no tc preference specified\n" );
				VID_Printf( PRINT_ALL, ".....using GL_S3_s3tc\n" );
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
					VID_Printf( PRINT_ALL, "...using preferred tc method, GL_S3_s3tc\n" );
					glConfig.textureCompression = TC_S3TC;
				}
				else
				{
					VID_Printf( PRINT_ALL, "...using preferred tc method, GL_EXT_texture_compression_s3tc\n" );
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
						VID_Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
						glConfig.textureCompression = TC_S3TC;
					}
					else
					{
						// Drat, preference can't be honored 
						VID_Printf( PRINT_ALL, "...preferred tc method, GL_S3_s3tc not available\n" );
						VID_Printf( PRINT_ALL, ".....falling back to GL_EXT_texture_compression_s3tc\n" );
						glConfig.textureCompression = TC_S3TC_DXT;
					}
				}
				else
				{
					// Preferring to user newer compression
					if ( newer_tc )
					{
						VID_Printf( PRINT_ALL, "...using GL_EXT_texture_compression_s3tc\n" );
						glConfig.textureCompression = TC_S3TC_DXT;
					}
					else
					{
						// Drat, preference can't be honored 
						VID_Printf( PRINT_ALL, "...preferred tc method, GL_EXT_texture_compression_s3tc not available\n" );
						VID_Printf( PRINT_ALL, ".....falling back to GL_S3_s3tc\n" );
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
static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		VID_Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	VID_Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	// Select our tc scheme
	GLW_InitTextureCompression();

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "EXT_texture_env_add" ) )
	{
		if ( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = qtrue;
			VID_Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		}
		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			VID_Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	}
	else
	{
		VID_Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}


	// GL_EXT_clamp_to_edge
	glConfig.clampToEdgeAvailable = qfalse;
	if ( strstr( glConfig.extensions_string, "GL_EXT_texture_edge_clamp" ) )
	{
		glConfig.clampToEdgeAvailable = qtrue;
		VID_Printf( PRINT_ALL, "...Using GL_EXT_texture_edge_clamp\n" );
	}

	// WGL_EXT_swap_control
	#if 0
	qwglSwapIntervalEXT = ( BOOL (WINAPI *)(int)) GPA( "wglSwapIntervalEXT" );
	if ( qwglSwapIntervalEXT )
	{
		VID_Printf( PRINT_ALL, "...using WGL_EXT_swap_control\n" );
		r_swapInterval->modified = qtrue;	// force a set next frame
	}
	else
	{
		VID_Printf( PRINT_ALL, "...WGL_EXT_swap_control not found\n" );
	}
	#endif

	// GL_ARB_multitexture
	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" )  )
	{
		if ( r_ext_multitexture->integer )
		{
			qglMultiTexCoord2fARB = ( PFNGLMULTITEXCOORD2FARBPROC ) GPA("glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( PFNGLACTIVETEXTUREARBPROC ) GPA( "glActiveTextureARB" );
			qglClientActiveTextureARB = ( PFNGLCLIENTACTIVETEXTUREARBPROC ) GPA( "glClientActiveTextureARB" );

			if ( qglActiveTextureARB )
			{
				qglGetIntegerv( GL_MAX_ACTIVE_TEXTURES_ARB, &glConfig.maxActiveTextures );

				if ( glConfig.maxActiveTextures > 1 )
				{
					VID_Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}
				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					VID_Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}
		else
		{
			VID_Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}
	else
	{
		VID_Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	qglLockArraysEXT = NULL;
	qglUnlockArraysEXT = NULL;
	if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) )
	{
		if ( r_ext_compiled_vertex_array->integer )
		{
			VID_Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( int, int ) ) GPA( "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) GPA( "glUnlockArraysEXT" );
			if (!qglLockArraysEXT || !qglUnlockArraysEXT) {
				Com_Error (ERR_FATAL, "bad getprocaddress");
			}
		}
		else
		{
			VID_Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
	{
		VID_Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}


#ifdef USE_VR
	IHmdRenderer* pHmdRenderer = ClientHmd::Get()->GetRenderer();
	if (pHmdRenderer != NULL)
	{
		// init needed extensions
		
		qglIsRenderbuffer = (PFNglIsRenderbufferPROC) GPA("glIsRenderbuffer");
		qglBindRenderbuffer = (PFNglBindRenderbufferPROC) GPA("glBindRenderbuffer");
		qglDeleteRenderbuffers = (PFNglDeleteRenderbuffersPROC) GPA("glDeleteRenderbuffers");
		qglGenRenderbuffers = (PFNglGenRenderbuffersPROC) GPA("glGenRenderbuffers");
		qglRenderbufferStorage = (PFNglRenderbufferStoragePROC) GPA("glRenderbufferStorage");
		qglRenderbufferStorageMultisample = (PFNglRenderbufferStorageMultisamplePROC) GPA("glRenderbufferStorageMultisample");
		qglGetRenderbufferParameteriv = (PFNglGetRenderbufferParameterivPROC) GPA("glGetRenderbufferParameteriv");
		qglIsFramebuffer = (PFNglIsFramebufferPROC) GPA("glIsFramebuffer");
		qglGenFramebuffers = (PFNglGenFramebuffersPROC) GPA("glGenFramebuffers");
		qglBindFramebuffer = (PFNglBindFramebufferPROC) GPA("glBindFramebuffer");
		qglDeleteFramebuffers = (PFNglDeleteFramebuffersPROC) GPA("glDeleteFramebuffers");
		qglCheckFramebufferStatus = (PFNglCheckFramebufferStatusPROC) GPA("glCheckFramebufferStatus");
		qglFramebufferTexture1D = (PFNglFramebufferTexture1DPROC) GPA("glFramebufferTexture1D");
		qglFramebufferTexture2D = (PFNglFramebufferTexture2DPROC) GPA( "glFramebufferTexture2D");
		qglFramebufferTexture3D = (PFNglFramebufferTexture3DPROC) GPA("glFramebufferTexture3D");
		qglFramebufferTextureLayer = (PFNglFramebufferTextureLayerPROC) GPA("glFramebufferTextureLayer");
		qglFramebufferRenderbuffer = (PFNglFramebufferRenderbufferPROC) GPA( "glFramebufferRenderbuffer");
		qglGetFramebufferAttachmentParameteriv = (PFNglGetFramebufferAttachmentParameterivPROC) GPA( "glGetFramebufferAttachmentParameteriv");
		qglBlitFramebuffer = (PFNglBlitFramebufferPROC) GPA("glBlitFramebuffer");
		qglGenerateMipmap = (PFNglGenerateMipmapPROC) GPA("glGenerateMipmap");
		
		qglCreateShaderObjectARB = (PFNglCreateShaderObjectARBPROC) GPA("glCreateShaderObjectARB");
		qglShaderSourceARB = (PFNglShaderSourceARBPROC) GPA("glShaderSourceARB");
		qglCompileShaderARB = (PFNglCompileShaderARBPROC) GPA("glCompileShaderARB");
		qglCreateProgramObjectARB = (PFNglCreateProgramObjectARBPROC) GPA("glCreateProgramObjectARB");
		qglAttachObjectARB = (PFNglAttachObjectARBPROC) GPA("glAttachObjectARB");
		qglLinkProgramARB = (PFNglLinkProgramARBPROC) GPA("glLinkProgramARB");
		qglUseProgramObjectARB = (PFNglUseProgramObjectARBPROC) GPA("glUseProgramObjectARB");
		qglUniform2fARB = (PFNglUniform2fARBPROC) GPA("glUniform2fARB");
		qglUniform2fvARB = (PFNglUniform2fvARBPROC) GPA("glUniform2fvARB");
		qglGetUniformLocationARB = (PFNglGetUniformLocationARBPROC) GPA("glGetUniformLocationARB");
		
		qglBindBuffer = (PFNglBindBufferPROC) GPA("glBindBuffer");
		qglBindVertexArray = (PFNglBindVertexArrayPROC) GPA("glBindVertexArray");
		
		
		// try to initialize hmd renderer
		
		PlatformInfo platformInfo;
		platformInfo.WindowWidth = s_windowWidth;
		platformInfo.WindowHeight = s_windowHeight;
	
		SDL_SysWMinfo sysInfo;
		SDL_VERSION(&sysInfo.version); // initialize info structure with SDL version info
		SDL_GetWindowWMInfo(s_pSdlWindow, &sysInfo);
#ifdef LINUX
		if (sysInfo.subsystem == SDL_SYSWM_X11)
		{
			platformInfo.pDisplay = sysInfo.info.x11.display;
			platformInfo.WindowId = sysInfo.info.x11.window;
		}
#endif

#ifdef _WINDOWS
		if (sysInfo.subsystem == SDL_VIDEO_DRIVER_WINDOWS)
		{
			platformInfo.Window = sysInfo.info.win.window;
			platformInfo.DC = NULL;// GetDC(sysInfo.info.win.window);
		}
#endif
		bool worked = pHmdRenderer->Init(s_windowWidth, s_windowHeight, platformInfo);
		if (worked)
		{  
			pHmdRenderer->GetRenderResolution(glConfig.vidWidth, glConfig.vidHeight);
		}
		else
		{
			// renderer could not be initialized -> set NULL
			pHmdRenderer = NULL;
			ClientHmd::Get()->SetRenderer(NULL);
		}
	}
#endif
	
	// Figure out which texture rectangle extension to use.
	bool bTexRectSupported = false;
	if ( strnicmp( glConfig.vendor_string, "ATI Technologies",16 )==0
		&& strnicmp( glConfig.version_string, "1.3.3",5 )==0 
		&& glConfig.version_string[5] < '9' ) //1.3.34 and 1.3.37 and 1.3.38 are broken for sure, 1.3.39 is not
	{
		g_bTextureRectangleHack = true;
	}
	
	if ( strstr( glConfig.extensions_string, "GL_NV_texture_rectangle" )
		   || strstr( glConfig.extensions_string, "GL_EXT_texture_rectangle" ) )
	{
		bTexRectSupported = true;
	}
	
	
	// Find out how many general combiners they have.
	#define GL_MAX_GENERAL_COMBINERS_NV       0x854D
	GLint iNumGeneralCombiners = 0;
	qglGetIntegerv( GL_MAX_GENERAL_COMBINERS_NV, &iNumGeneralCombiners );
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

	VID_Printf( PRINT_ALL, "...loading %s: ", buffer );


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
		VID_Printf( PRINT_ALL, "failed\n" );
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
	}

	bool doSwap = true;
#ifdef USE_VR
	IHmdRenderer* pHmdRenderer = ClientHmd::Get()->GetRenderer();
	if (pHmdRenderer != NULL)
	{
		pHmdRenderer->EndFrame();
		doSwap = !pHmdRenderer->HandlesSwap();
	}
#endif
	
	if (doSwap)
	{
		// don't flip if drawing to front buffer
		//if ( stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
		{
			SDL_GL_SwapWindow(s_pSdlWindow);
		}
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
		Com_Error( ERR_FATAL, "GLW_StartOpenGL() - could not load OpenGL subsystem\n" );
	}
	

}

SDL_Joystick* pSdlJoystick = NULL;
SDL_GameController* pSdlGameController = NULL;

void IN_StartupGameController( void )
{
	int joystickCount = SDL_NumJoysticks();
	if (joystickCount <= 0)
	{
		return;
	}
	
	
	int joystickIndex = 0;
	pSdlGameController = NULL;
	for (int i = 0; i < joystickCount; ++i) 
	{
		if (SDL_IsGameController(i)) 
		{
			pSdlGameController = SDL_GameControllerOpen(i);
			if (pSdlGameController) 
			{
				joystickIndex = i;
				break;
			}
		}
	}
	
	
	if (pSdlGameController == NULL)
	{
		pSdlJoystick = SDL_JoystickOpen(joystickIndex);
		if (!pSdlJoystick)
		{
			return;
		}
	}
	
	if (pSdlJoystick)
	{
		VID_Printf (PRINT_ALL, "Opened Joystick %d\n", joystickIndex);
		VID_Printf (PRINT_ALL,"Number of Axes: %d\n", SDL_JoystickNumAxes(pSdlJoystick));
		VID_Printf (PRINT_ALL,"Number of Buttons: %d\n", SDL_JoystickNumButtons(pSdlJoystick));
		VID_Printf (PRINT_ALL,"Number of Balls: %d\n", SDL_JoystickNumBalls(pSdlJoystick));        
	}
	else
	{
		VID_Printf (PRINT_ALL, "Opened GameController %d\n", joystickIndex);
	}

	VID_Printf (PRINT_ALL,"Name: %s\n", SDL_JoystickNameForIndex(joystickIndex)); 
}


void IN_ShutdownGameController( void )
{
	if (pSdlJoystick)
	{
		if (SDL_JoystickGetAttached(pSdlJoystick)) 
		{
			SDL_JoystickClose(pSdlJoystick);
		}
	}
	
	if (pSdlGameController)
	{
		SDL_GameControllerClose(pSdlGameController);
	}
}

#ifdef USE_VR
void InitHmdDevice()
{
	// try to create a hmd device
	ClientHmd::Get()->SetDevice(NULL);
	ClientHmd::Get()->SetRenderer(NULL);
	
	cvar_t* pHmdEnabled = Cvar_Get("hmd_enabled", "1", CVAR_ARCHIVE);
	if (pHmdEnabled->integer == 1)
	{
		cvar_t* pHmdLib = Cvar_Get("hmd_forceLibrary", "", CVAR_ARCHIVE);
		std::string hmdLibName = pHmdLib->string;
		std::transform(hmdLibName.begin(), hmdLibName.end(), hmdLibName.begin(), ::tolower);
	
		FactoryHmdDevice::HmdLibrary lib = FactoryHmdDevice::LIB_UNDEFINED;
		if (hmdLibName == "ovr")
		{
			lib = FactoryHmdDevice::LIB_OVR;
		}
		else if (hmdLibName == "openhmd")
		{
			lib = FactoryHmdDevice::LIB_OPENHMD;
		}
		else if (hmdLibName == "mouse_dummy")
		{
			lib = FactoryHmdDevice::LIB_MOUSE_DUMMY;
		}
	
		cvar_t* pAllowDummyDevice = Cvar_Get ("hmd_allowdummydevice", "0", CVAR_ARCHIVE);
		bool allowDummyDevice = pAllowDummyDevice->integer == 1;


		IHmdDevice* pHmdDevice = FactoryHmdDevice::CreateHmdDevice(lib, allowDummyDevice);
		if (pHmdDevice)
		{
			VID_Printf(PRINT_ALL, "HMD Device found: %s\n", pHmdDevice->GetInfo().c_str());
			ClientHmd::Get()->SetDevice(pHmdDevice);

			Cvar_Set("cg_activeHmd", "1");
			Cvar_Set("cg_useHmd", "1");
			Cvar_Set("cg_thirdPerson", "0");
	
			IHmdRenderer* pHmdRenderer = FactoryHmdDevice::CreateRendererForDevice(pHmdDevice);
	
			if (pHmdRenderer)
			{
				VID_Printf(PRINT_ALL, "HMD Renderer created: %s\n", pHmdRenderer->GetInfo().c_str());
				ClientHmd::Get()->SetRenderer(pHmdRenderer);
			}
		}
	}
}
#endif

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
	cvar_t *lastValidRenderer = Cvar_Get( "r_lastValidRenderer", "(uninitialized)", CVAR_ARCHIVE );

	VID_Printf( PRINT_ALL, "Initializing OpenGL subsystem\n" );

	//glConfig.deviceSupportsGamma = qfalse;

	InitSig();

	int sdlRet = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
	if (sdlRet < 0)
	{
		Com_Error(PRINT_ALL, "GLimp_Init: Can't initialize SDL. Error=%s\n", SDL_GetError());
		return;
	}
	
#ifdef USE_VR	
	InitHmdDevice();
#endif

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
		Com_Error( ERR_FATAL, "GLimp_Init() - Invalid GL Driver\n" );
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
	//strcpy( buf, glConfig.renderer_string );
	//strlwr( buf );

	////
	//// NOTE: if changing cvars, do it within this block.  This allows them
	//// to be overridden when testing driver fixes, etc. but only sets
	//// them to their default state when the hardware is first installed/run.
	////

	//if ( Q_stricmp( lastValidRenderer->string, glConfig.renderer_string ) )
	//{
	//	//reset to defaults
	//	Cvar_Set( "r_picmip", "1" );
	//	
	//	if ( strstr( buf, "matrox" )) {
 //           Cvar_Set( "r_allowExtensions", "0");			
	//	}


	//	Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
	//	
	//	if ( strstr( buf, "intel" ) )
	//	{
	//		// disable dynamic glow as default
	//		Cvar_Set( "r_DynamicGlow","0" );
	//	}		

	//	if ( strstr( buf, "kyro" ) )	
	//	{
	//		Cvar_Set( "r_ext_texture_filter_anisotropic", "0");	//KYROs have it avail, but suck at it!
	//		Cvar_Set( "r_ext_preferred_tc_method", "1");			//(Use DXT1 instead of DXT5 - same quality but much better performance on KYRO)
	//	}

	//	GLW_InitExtensions();
	//	
	//	//this must be a really sucky card!
	//	if ( (glConfig.textureCompression == TC_NONE) || (glConfig.maxActiveTextures < 2)  || (glConfig.maxTextureSize <= 512) )
	//	{
	//		Cvar_Set( "r_picmip", "2");
	//		Cvar_Set( "r_colorbits", "16");
	//		Cvar_Set( "r_texturebits", "16");
	//		Cvar_Set( "r_mode", "3");	//force 640
	//		Cmd_ExecuteString ("exec low.cfg\n");	//get the rest which can be pulled in after init
	//	}
	//}
	//
	//Cvar_Set( "r_lastValidRenderer", glConfig.renderer_string );

	GLW_InitExtensions();
	InitSig();

	IN_StartupGameController();
	SDL_StartTextInput();
}


/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
//void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
//{
//}


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


	VID_Printf( PRINT_ALL, "Shutting down OpenGL subsystem\n" );

	// restore gamma.  We do this first because 3Dfx's extension needs a valid OGL subsystem
	WG_RestoreGamma();


	IN_DeactivateMouse();

	SDL_SetRelativeMouseMode(SDL_FALSE);
	
	//SDL_DestroyRenderer(s_pSdlRenderer);
	SDL_GL_DeleteContext(sGlContext);
	SDL_DestroyWindow(s_pSdlWindow);

	// close the r_logFile
	if ( glw_state.log_fp )
	{
		fclose( glw_state.log_fp );
		glw_state.log_fp = 0;
	}

	IN_ShutdownGameController();
	
#ifdef USE_VR	
	IHmdRenderer* pHmdRenderer = ClientHmd::Get()->GetRenderer();
	if (pHmdRenderer != NULL)
	{
		ClientHmd::Get()->SetRenderer(NULL);
		
		pHmdRenderer->Shutdown();
		delete pHmdRenderer;
		pHmdRenderer = NULL;
	}
	
	IHmdDevice* pHmdDevice = ClientHmd::Get()->GetDevice();
	if (pHmdDevice != NULL)
	{
		ClientHmd::Get()->SetDevice(NULL);
		
		pHmdDevice->Shutdown();
		delete pHmdDevice;
		pHmdDevice = NULL;
	}
#endif
	
	SDL_Quit();

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



void Input_Init(void);
void Input_GetState( void );


/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/


static char *XLateKey(const SDL_Keysym& keysym, int *key)
{
	static char buf[1];
	buf[0] = 0;
	*key = 0;

	//const char* keyName = SDL_GetKeyName(keysym.sym);
	
	if (keysym.sym < 0x40000000)
	{
		char keyName = keysym.sym;
		//if (keysym.mod & KMOD_SHIFT && keyName >= 'a' && keyName <= 'z')
		//{
		//	keyName = keyName - 'a' + 'A';
		//}

		strncpy(buf, &keyName, 1);

		//Com_Printf("key: %s (%d)\n", buf, *buf);
	}

	switch(keysym.scancode)
	{
		//case XK_KP_Page_Up:	
		case SDL_SCANCODE_KP_9:	 *key = A_KP_9; break;
		case SDL_SCANCODE_PAGEUP:	 *key = A_PAGE_UP; break;

		//case XK_KP_Page_Down: 
		case SDL_SCANCODE_KP_3: *key = A_KP_3; break;
		case SDL_SCANCODE_PAGEDOWN:	 *key = A_PAGE_DOWN; break;

		//case XK_KP_Home:
		case SDL_SCANCODE_KP_7: *key = A_KP_7; break;
		case SDL_SCANCODE_HOME:	 *key = A_HOME; break;

		//case XK_KP_End:
		case SDL_SCANCODE_KP_1:	  *key = A_KP_1; break;
		case SDL_SCANCODE_END:	 *key = A_END; break;

		//case XK_KP_Left: 
		case SDL_SCANCODE_KP_4: *key = A_KP_4; break;
		case SDL_SCANCODE_LEFT:	 *key = A_CURSOR_LEFT; break;

		//case XK_KP_Right:
		case SDL_SCANCODE_KP_6: *key = A_KP_6; break;
		case SDL_SCANCODE_RIGHT:	*key = A_CURSOR_RIGHT;		break;

		//case XK_KP_Down:
		case SDL_SCANCODE_KP_2: 	 *key = A_KP_2; break;
		case SDL_SCANCODE_DOWN:	 *key = A_CURSOR_DOWN; break;

		//case XK_KP_Up:   
		case SDL_SCANCODE_KP_8:    *key = A_KP_8; break;
		case SDL_SCANCODE_UP:		 *key = A_CURSOR_UP;	 break;

		case SDL_SCANCODE_ESCAPE: *key = A_ESCAPE;		break;

		case SDL_SCANCODE_KP_ENTER: *key = A_KP_ENTER;	break;
		case SDL_SCANCODE_RETURN: *key = A_ENTER;		 break;

		case SDL_SCANCODE_TAB:		*key = A_TAB;			 break;

		case SDL_SCANCODE_F1:		 *key = A_F1;				break;

		case SDL_SCANCODE_F2:		 *key = A_F2;				break;

		case SDL_SCANCODE_F3:		 *key = A_F3;				break;

		case SDL_SCANCODE_F4:		 *key = A_F4;				break;

		case SDL_SCANCODE_F5:		 *key = A_F5;				break;

		case SDL_SCANCODE_F6:		 *key = A_F6;				break;

		case SDL_SCANCODE_F7:		 *key = A_F7;				break;

		case SDL_SCANCODE_F8:		 *key = A_F8;				break;

		case SDL_SCANCODE_F9:		 *key = A_F9;				break;

		case SDL_SCANCODE_F10:		*key = A_F10;			 break;

		case SDL_SCANCODE_F11:		*key = A_F11;			 break;

		case SDL_SCANCODE_F12:		*key = A_F12;			 break;

		case SDL_SCANCODE_BACKSPACE: *key = A_BACKSPACE; break; // ctrl-h

		//case XK_KP_Delete:
		case SDL_SCANCODE_KP_PERIOD: *key = A_KP_PERIOD; break;
		case SDL_SCANCODE_DELETE: *key = A_DELETE; break;

		case SDL_SCANCODE_PAUSE:	*key = A_PAUSE;		 break;

		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT:	*key = A_SHIFT;		break;

		case SDL_SCANCODE_EXECUTE: 
		case SDL_SCANCODE_LCTRL: 
		case SDL_SCANCODE_RCTRL:	*key = A_CTRL;		 break;

		case SDL_SCANCODE_LALT:	
		case SDL_SCANCODE_LGUI: 
		case SDL_SCANCODE_RALT:	
		case SDL_SCANCODE_RGUI: *key = A_ALT;			break;

		//case XK_KP_Begin: *key = A_KP_5;	break;

		//case XK_Insert:		*key = K_INS; break;
		case SDL_SCANCODE_INSERT:
		case SDL_SCANCODE_KP_0: *key = A_KP_0; break;

		case SDL_SCANCODE_KP_MULTIPLY: *key = '*'; break;
		case SDL_SCANCODE_KP_PLUS:  *key = A_KP_PLUS; break;
		case SDL_SCANCODE_KP_MINUS: *key = A_KP_MINUS; break;
		//case XK_KP_Divide: *key = K_KP_SLASH; break;
		case SDL_SCANCODE_SPACE: *key = A_SPACE; break;

		default:
			*key = *(unsigned char *)buf;
			if (*key >= 'A' && *key <= 'Z')
				*key = *key - 'A' + 'a';
			break;
	} 

	if (keysym.sym != '`' && keysym.sym > ' ')
	{
		buf[0] = 0;
	}

	return buf;
}


float JoyToF( int value, float threshold) {
	float	fValue;

	// convert range from -32768..32767 to -1..1 
	fValue = (float)value / 32768.0;
	float sign = fValue >= 0 ? 1.0f : -1.0f;
	
	// remove the threshold
	fValue = fabs(fValue) - threshold;
	fValue = max(fValue, 0.0f);
	
	// scale the value to the full range
	fValue *= (1.0f / (1.0f - threshold));
	fValue = min(fValue, 1.0f);
	
	fValue *= sign;

	return fValue;
}

int joyDirectionKeys[16] = {
	A_CURSOR_LEFT, A_CURSOR_RIGHT,
	A_CURSOR_UP, A_CURSOR_DOWN,
	A_JOY16, A_JOY17,
	A_JOY18, A_JOY19,
	A_JOY20, A_JOY21,
	A_JOY22, A_JOY23,
	
	A_JOY24, A_JOY25,
	A_JOY26, A_JOY27
};




void IN_GameControllerMove(int axis, int value) {
	float	fAxisValue;
	
	int i = axis;
	float threshold = 0.15f; //joy_threshold->value;
	
	// get the floating point zero-centered, potentially-inverted data for the current axis
	fAxisValue = JoyToF(value, threshold);
	
	if (i == 0) {
		QueKeyEvent(0, SE_JOYSTICK_AXIS, AXIS_SIDE, (int) (fAxisValue*127.0), 0, NULL );
	}
	
	if (i == 1) {
		QueKeyEvent(0, SE_JOYSTICK_AXIS, AXIS_FORWARD, (int) -(fAxisValue*127.0), 0, NULL );
	}
	
	if (i == 2) {
		QueKeyEvent( 0, SE_JOYSTICK_AXIS, AXIS_YAW, (int) -(fAxisValue*127.0), 0, NULL );
	}
	
	if (i == 3) {
		QueKeyEvent( 0, SE_JOYSTICK_AXIS, AXIS_PITCH, (int) (fAxisValue*127.0), 0, NULL );
	}
	


//    if ( fAxisValue < -threshold) {
//        povstate |= (1<<(i*2));
//    } else if ( fAxisValue > threshold) {
//        povstate |= (1<<(i*2+1));
//    }


//    // determine which bits have changed and key an auxillary event for each change
//    for (i=0 ; i < 16 ; i++) {
//        if ( (povstate & (1<<i)) && !(joy.oldpovstate & (1<<i)) ) {
//            QueKeyEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qtrue, 0, NULL );
//        }
    
//        if ( !(povstate & (1<<i)) && (joy.oldpovstate & (1<<i)) ) {
//            QueKeyEvent( g_wv.sysMsgTime, SE_KEY, joyDirectionKeys[i], qfalse, 0, NULL );
//        }
//    }
}


void IN_JoyMove_Old(SDL_JoyAxisEvent event)
{
	/* Store instantaneous joystick state. Hack to get around
	* event model used in Linux joystick driver.
	 */
	static int axes_state[16];
	/* Old bits for Quake-style input compares. */
	static unsigned int old_axes = 0;
	/* Our current goodies. */
	
	float threshold = 0.15f;//joy_threshold->value;
	
	unsigned int axes = 0;
	int i = 0;
	
	if( event.axis < 16 ) {
		axes_state[event.axis] = event.value;
	}
	
	
	/* Translate our instantaneous state to bits. */
	for( i = 0; i < 16; i++ ) {
	float f = ( (float) axes_state[i] ) / 32767.0f;
	
	if( f < -threshold ) 
	{
		axes |= ( 1 << ( i * 2 ) );
	} 
	else if( f > threshold ) 
	{
		axes |= ( 1 << ( ( i * 2 ) + 1 ) );
	}
	
	}
	
	/* Time to update axes state based on old vs. new. */
	for( i = 0; i < 16; i++ ) 
	{
		if( ( axes & ( 1 << i ) ) && !( old_axes & ( 1 << i ) ) ) 
		{
			QueKeyEvent( 0, SE_KEY, joyDirectionKeys[i], qtrue, 0, NULL );
		}
		
		if( !( axes & ( 1 << i ) ) && ( old_axes & ( 1 << i ) ) ) 
		{
			QueKeyEvent( 0, SE_KEY, joyDirectionKeys[i], qfalse, 0, NULL );
		}
	}
	
	/* Save for future generations. */
	old_axes = axes;
}



#define MAX_MOUSE_BUTTONS 9

static int mouseConvert[MAX_MOUSE_BUTTONS] =
{
	A_MOUSE1,
	A_MOUSE3,
	A_MOUSE2,	
	A_MWHEELUP,
	A_MWHEELDOWN,
	A_UNDEFINED_7,
	A_UNDEFINED_8,
	A_MOUSE4,
	A_MOUSE5,
};



static void HandleEvents(void)
{
    const int mouseDefaultPosX = s_windowWidth / 2;
    const int mouseDefaultPosY = s_windowHeight / 2;
	int key;
	qboolean dowarp = qfalse;

    int lastMousePosX = mouseDefaultPosX;
    int lastMousePosY = mouseDefaultPosY;

	qboolean forceRelMouse = qfalse;
#ifdef __APPLE__
	//forceRelMouse = qtrue;
#endif
	char *p;
	
	SDL_Event event;
	
	while(SDL_PollEvent(&event)) 
	{
		switch(event.type)
		{
		case SDL_QUIT:
			Com_Quit_f();
			break;
		
		case SDL_KEYDOWN:
			p = XLateKey(event.key.keysym, &key);
			if (key)
				QueKeyEvent( 0, SE_KEY, key, qtrue, 0, NULL );
			//handle control chars
			while (*p)
				QueKeyEvent( 0, SE_CHAR, *p++, 0, 0, NULL );
			break;                
		case SDL_KEYUP:
			XLateKey(event.key.keysym, &key);
			
			QueKeyEvent( 0, SE_KEY, key, qfalse, 0, NULL );
			break;

		case SDL_TEXTINPUT:
			p = event.text.text;
			if (*p != '`' && *p != '^')
			{
				while (*p)
					QueKeyEvent(0, SE_CHAR, *p++, 0, 0, NULL);
			}
			break;
		case SDL_MOUSEMOTION:
			if (sRelativeMouseMode || forceRelMouse)
			{
				mx += event.motion.xrel;
				my += event.motion.yrel;
			}
			else
			{
                //VID_Printf(PRINT_ALL, "event x=%d y=%d motionx=%d motiony=%d\n", event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
				//VID_Printf (PRINT_ALL, "mxOld=%d myOld=%d mx=%d my=%d lastMousePosX=%d lastMousePosX=%d\n", mx, my, mx + event.motion.x -lastMousePosX, my + event.motion.y - lastMousePosY, lastMousePosX, lastMousePosY);
                mx += (event.motion.x - lastMousePosX);
                my += (event.motion.y - lastMousePosY);
                lastMousePosX = event.motion.x;
                lastMousePosY = event.motion.y;
	
				if (mx || my)
				{
					dowarp = qtrue;
				}
			}
			
			break;
				
		case SDL_MOUSEBUTTONDOWN:
		{
			//VID_Printf (PRINT_ALL, "button.y=%d\n", event.button.button);
			
			int buttonNr = event.button.button - 1;
			if (buttonNr >= 0 && buttonNr < MAX_MOUSE_BUTTONS)
			{
				QueKeyEvent( 0, SE_KEY, mouseConvert[buttonNr], qtrue, 0, NULL );
			}
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			int buttonNr = event.button.button - 1;
			if (buttonNr >= 0 && buttonNr < MAX_MOUSE_BUTTONS)
			{
				QueKeyEvent( 0, SE_KEY, mouseConvert[buttonNr], qfalse, 0, NULL );
			}

			break;
		}
// mouse wheel is handled as mouse buttons            
//        case SDL_MOUSEWHEEL:        
//            if (event.wheel.y != 0)
//            {
//                int wheel = event.wheel.y < 0 ? A_MWHEELDOWN : A_MWHEELUP;
//                //VID_Printf (PRINT_ALL, "wheel.y=%d\n", event.wheel.y);
                
//                int wheelCount = abs(event.wheel.y);
//                for (int i=0; i<wheelCount; i++)
//                {
//                    QueKeyEvent( 0, SE_KEY, wheel, qtrue, 0, NULL );
//                    QueKeyEvent( 0, SE_KEY, wheel, qfalse, 0, NULL );
//                }
               
//            }
//            break;

		case SDL_JOYBUTTONDOWN:
			if (pSdlJoystick)
			{
				QueKeyEvent( 0, SE_KEY, A_JOY0 + event.jbutton.button, qtrue, 0, NULL );
			}
			break;            
		case SDL_JOYBUTTONUP:
			if (pSdlJoystick)
			{
				QueKeyEvent( 0, SE_KEY, A_JOY0 + event.jbutton.button, qfalse, 0, NULL );
			}
			break;
		case SDL_JOYAXISMOTION:
			if (pSdlJoystick)
			{
				IN_GameControllerMove(event.jaxis.axis, event.jaxis.value);
			}
			break;
			
		case SDL_CONTROLLERBUTTONDOWN:
			QueKeyEvent( 0, SE_KEY, A_JOY0 + event.cbutton.button, qtrue, 0, NULL );
			break;            
		case SDL_CONTROLLERBUTTONUP:
			QueKeyEvent( 0, SE_KEY, A_JOY0 + event.cbutton.button, qfalse, 0, NULL );
			break;
		case SDL_CONTROLLERAXISMOTION:
			IN_GameControllerMove(event.caxis.axis, event.caxis.value);
			break;            
			
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				//VID_Printf (PRINT_ALL, "SDL_WINDOWEVENT_FOCUS_GAINED\n");
				sWindowHasFocus = true;
				SDL_ShowCursor(0);
				break;
			case SDL_WINDOWEVENT_FOCUS_LOST:
				//VID_Printf (PRINT_ALL, "SDL_WINDOWEVENT_FOCUS_LOST\n");
				sWindowHasFocus = false;
				SDL_ShowCursor(1);
				break;
			}
			break;
		
		}
		
		
	}

	if (dowarp && sWindowHasFocus) {
        SDL_WarpMouseInWindow(s_pSdlWindow, mouseDefaultPosX, mouseDefaultPosY);
	}
	
	if (!sRelativeMouseMode && !sWindowHasFocus)
	{
		mx = 0;
		my = 0;
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

}

void IN_DeactivateMouse( void ) 
{

}




/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/

void IN_Init(void)
{
	// mouse variables
	in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE);

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
	if (!mouse_avail)
		return;


	if (mx || my)
		QueKeyEvent( 0, SE_MOUSE, mx, my, 0, NULL );
	mx = my = 0;
}

void IN_Frame (void)
{
	IN_ActivateMouse();

	// post events to the system que
	IN_MouseMove();
#ifdef _WINDOWS
	HandleEvents();
#endif
}

void IN_Activate(void)
{
}

void Sys_SendKeyEvents (void)
{
	HandleEvents();
}

#ifdef _WINDOWS


/*
===========================================================

SMP acceleration

===========================================================
*/

HANDLE	renderCommandsEvent;
HANDLE	renderCompletedEvent;
HANDLE	renderActiveEvent;

void (*glimpRenderThread)( void );

void GLimp_RenderThreadWrapper( void ) {
	glimpRenderThread();

	SDL_GL_MakeCurrent(s_pSdlWindow, nullptr);
}

/*
=======================
GLimp_SpawnRenderThread
=======================
*/
HANDLE	renderThreadHandle;
int		renderThreadId;
qboolean GLimp_SpawnRenderThread( void (*function)( void ) ) {

	renderCommandsEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	renderCompletedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	renderActiveEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	glimpRenderThread = function;

	renderThreadHandle = CreateThread(
	   NULL,	// LPSECURITY_ATTRIBUTES lpsa,
	   0,		// DWORD cbStack,
	   (LPTHREAD_START_ROUTINE)GLimp_RenderThreadWrapper,	// LPTHREAD_START_ROUTINE lpStartAddr,
	   0,			// LPVOID lpvThreadParm,
	   0,			//   DWORD fdwCreate,
	   (unsigned long *) &renderThreadId );

	if ( !renderThreadHandle ) {
		return qfalse;
	}

	return qtrue;
}

static	void	*smpData;
static	int		wglErrors;

void *GLimp_RendererSleep( void ) {
	void	*data;

	if ( !SDL_GL_MakeCurrent(s_pSdlWindow, nullptr) ) {
		wglErrors++;
	}

	ResetEvent( renderActiveEvent );

	// after this, the front end can exit GLimp_FrontEndSleep
	SetEvent( renderCompletedEvent );

	WaitForSingleObject( renderCommandsEvent, INFINITE );

	if ( !SDL_GL_MakeCurrent(s_pSdlWindow, sGlContext) ) {
		wglErrors++;
	}

	ResetEvent( renderCompletedEvent );
	ResetEvent( renderCommandsEvent );

	data = smpData;

	// after this, the main thread can exit GLimp_WakeRenderer
	SetEvent( renderActiveEvent );

	return data;
}


void GLimp_FrontEndSleep( void ) {
	WaitForSingleObject( renderCompletedEvent, INFINITE );

	if ( !SDL_GL_MakeCurrent(s_pSdlWindow, sGlContext) ) {
		wglErrors++;
	}
}


void GLimp_WakeRenderer( void *data ) {
	smpData = data;

	if ( !SDL_GL_MakeCurrent(s_pSdlWindow, nullptr) ) {
		wglErrors++;
	}

	// after this, the renderer can continue through GLimp_RendererSleep
	SetEvent( renderCommandsEvent );

	WaitForSingleObject( renderActiveEvent, INFINITE );
}


#endif
