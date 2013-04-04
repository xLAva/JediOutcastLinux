
/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 * $Archive: /MissionPack/code/client/snd_dma.c $
 * $Author: Mrelusive $ 
 * $Revision: 117 $
 * $Modtime: 6/06/01 2:35p $
 * $Date: 6/06/01 7:11p $
 *
 *****************************************************************************/

#include "snd_local.h"
#include "snd_mp3.h"

#include "client.h"

void S_Play_f(void);
void S_SoundList_f(void);
static void S_Music_f(void);

void S_Update_();
void S_StopAllSounds(void);
static void S_UpdateBackgroundTrack( void );
extern int RE_RegisterMedia_GetLevel(void);


//////////////////////////
//
// vars for bgrnd music track...
//
const int iMP3MusicStream_DiskBytesToRead = 10000;//4096;
const int iMP3MusicStream_DiskBufferSize = iMP3MusicStream_DiskBytesToRead * 2;//10;

typedef struct
{	
	int/*qboolean*/	bIsMP3;	// I wanted this kept as qboolean, but {0} init won't work with a typedef'd enum
	//
	// MP3 specific...
	//
	sfx_t		sfxMP3_Bgrnd;
	MP3STREAM	streamMP3_Bgrnd;	// this one is pointed at by the sfx_t's ptr, and is NOT the one the decoder uses every cycle
	channel_t	chMP3_Bgrnd;		// ... but the one in this struct IS.	
	//
	// MP3 disk streamer stuff... (if music is non-dynamic)
	//
	byte		byMP3MusicStream_DiskBuffer[iMP3MusicStream_DiskBufferSize];
	int			iMP3MusicStream_DiskReadPos;
	int			iMP3MusicStream_DiskWindowPos;
	//
	// MP3 disk-load stuff (for use during dynamic music, which is mem-resident)
	//
	byte		*pLoadedData;	// Z_Malloc, Z_Free
	int			iLoadedDataLen;	
	char		sLoadedDataName[MAX_QPATH];
	//
	// remaining dynamic fields...
	//
	int			iXFadeVolumeSeekTime;
	int			iXFadeVolumeSeekTo;	// when changing this, set the above timer to Sys_Milliseconds(). 
									//	Note that this should be thought of more as an up/down bool rather than as a 
									//	number now, in other words set it only to 0 or 255. I'll probably change this
									//	to actually be a bool later.
	int			iXFadeVolume;		// 0 = silent, 255 = max mixer vol, though still modulated via overall music_volume 
	qboolean	bActive;			
	//
	// Generic...
	//
	fileHandle_t s_backgroundFile;	// valid handle, else -1 if an MP3 (so that NZ compares still work)
	wavinfo_t	s_backgroundInfo;
	int			s_backgroundSamples;	
} MusicInfo_t;

// this now no longer supports dynamic music for MP codebase...
//
typedef enum 
{
	eBGRNDTRACK_SLOW = 0,	// for normal walking around
//	eBGRNDTRACK_FAST,		// for when stuff gets exciting
//	eBGRNDTRACK_FADE,		// the xfade channel
//	//
	eBGRNDTRACK_NUMBEROF
} MusicState_e;

#define fDYNAMIC_XFADE_SECONDS (1.0f)

static MusicInfo_t	tMusic_Info[eBGRNDTRACK_NUMBEROF]	= {0};
static MusicState_e	eMusic_State						= eBGRNDTRACK_SLOW;
static char			sMusic_BackgroundLoop[MAX_QPATH]	= {0};
//
//////////////////////////



// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define		SOUND_FULLVOLUME	80

#define		SOUND_ATTENUATE		0.0008f

channel_t   s_channels[MAX_CHANNELS];
channel_t   loop_channels[MAX_CHANNELS];
int			numLoopChannels;

static int	s_soundStarted;
qboolean	s_soundMuted;

dma_t		dma;

static int			listener_number;
static vec3_t		listener_origin;
static vec3_t		listener_axis[3];

int			s_soundtime;		// sample PAIRS
int   		s_paintedtime; 		// sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define		MAX_SFX			4096
sfx_t		s_knownSfx[MAX_SFX];
int			s_numSfx = 0;

#define		LOOP_HASH		128
static	sfx_t		*sfxHash[LOOP_HASH];

cvar_t		*s_volume;
cvar_t		*s_testsound;
cvar_t		*s_khz;
cvar_t		*s_show;
cvar_t		*s_mixahead;
cvar_t		*s_mixPreStep;
cvar_t		*s_musicVolume;
cvar_t		*s_musicMult;
cvar_t		*s_separation;
cvar_t		*s_doppler;
cvar_t		*s_CPUType;
cvar_t		*s_language;


static loopSound_t		loopSounds[MAX_GENTITIES];
static	channel_t		*freelist = NULL;

int						s_rawend;
portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];


/**************************************************************************************************\
*
*	Open AL Specific
*
\**************************************************************************************************/

int			s_UseOpenAL	= false;		// Determines if using Open AL or the default software mixer
ALfloat		listener_pos[3];		// Listener Position
ALfloat		listener_ori[6];		// Listener Orientation
int			s_numChannels;			// Number of AL Sources == Num of Channels
short		s_rawdata[MAX_RAW_SAMPLES*4];	// Used for Raw Samples (Music etc...)

channel_t *S_OpenALPickChannel(int entnum, int entchannel);
void UpdateSingleShotSounds();
void UpdateLoopingSounds();
void UpdateRawSamples();

// EAX Related
ALboolean				s_bEAX;				// Is EAX 3.0 support is available
bool					s_bEALFileLoaded;	// Has an .eal file been loaded for the current level
bool					s_bInWater;			// Underwater effect currently active 
int						s_EnvironmentID;	// EAGLE ID of current environment
LPEAXMANAGER			s_lpEAXManager;		// Pointer to EAXManager object
HINSTANCE				s_hEAXManInst;		// Handle of EAXManager DLL
EAXSet					s_eaxSet;			// EAXSet() function
EAXGet					s_eaxGet;			// EAXGet() function
bool					s_eaxMorphing;		// Is EAX Morphing in progress
int						s_eaxMorphStartTime;// EAX Morph start time
int						s_eaxMorphCount;	// EAX Morph count (1 ... 10)
EAXLISTENERPROPERTIES	s_eaxLPSource;		// Source EAX Parameters
EAXLISTENERPROPERTIES	s_eaxLPCur;			// Current EAX Parameters
EAXLISTENERPROPERTIES	s_eaxLPDest;		// Destination EAX Parameters
char					s_LevelName[MAX_QPATH];		// Name of current level

void InitEAXManager();
void ReleaseEAXManager();
bool LoadEALFile(char *szEALFilename);
void UnloadEALFile();
void UpdateEAXListener(bool bUseDefault, bool bUseMorphing);
void UpdateEAXBuffer(channel_t *ch);
void EALFileInit(char *level);
void EAXMorph();
bool EAX3ListenerInterpolate(EAXLISTENERPROPERTIES *lpStartEAX3LP, EAXLISTENERPROPERTIES *lpFinishEAX3LP,
			float flRatio, EAXLISTENERPROPERTIES *lpResultEAX3LP, bool bCheckValues = false);
void Clamp(EAXVECTOR *eaxVector);
bool CheckEAX3LP(LPEAXLISTENERPROPERTIES lpEAX3LP);

// EAX 3.0 GUIDS ... confidential information ...

const GUID DSPROPSETID_EAX30_ListenerProperties 
				= { 0xa8fa6882, 0xb476,	0x11d3,	{ 0xbd, 0xb9, 0x00, 0xc0, 0xf0, 0x2d, 0xdf, 0x87} };

const GUID DSPROPSETID_EAX30_BufferProperties
				= { 0xa8fa6881, 0xb476,	0x11d3, { 0xbd, 0xb9, 0x0, 0xc0, 0xf0, 0x2d, 0xdf, 0x87} };

/**************************************************************************************************\
*
*	End of Open AL Specific
*
\**************************************************************************************************/



// instead of clearing a whole channel_t struct, we're going to skip the MP3SlidingDecodeBuffer[] buffer in the middle...
//
#ifndef offsetof
#include <stddef.h>
#endif
static inline void Channel_Clear(channel_t *ch)
{
	// memset (ch, 0, sizeof(*ch));

	memset(ch,0,offsetof(channel_t,MP3SlidingDecodeBuffer));

	byte *const p = (byte *)ch + offsetof(channel_t,MP3SlidingDecodeBuffer) + sizeof(ch->MP3SlidingDecodeBuffer);

	memset(p,0,(sizeof(*ch) - offsetof(channel_t,MP3SlidingDecodeBuffer)) - sizeof(ch->MP3SlidingDecodeBuffer));
}



// ====================================================================
// User-setable variables
// ====================================================================

void S_SoundInfo_f(void) {	
	Com_Printf("----- Sound Info -----\n" );
	if (!s_soundStarted) {
		Com_Printf ("sound system not started\n");
	} else {
		if ( s_soundMuted ) {
			Com_Printf ("sound system is muted\n");
		}

		Com_Printf("%5d stereo\n", dma.channels - 1);
		Com_Printf("%5d samples\n", dma.samples);
		Com_Printf("%5d samplebits\n", dma.samplebits);
		Com_Printf("%5d submission_chunk\n", dma.submission_chunk);
		Com_Printf("%5d speed\n", dma.speed);
		Com_Printf("0x%x dma buffer\n", dma.buffer);
		if ( tMusic_Info[eBGRNDTRACK_SLOW].s_backgroundFile ) {
			Com_Printf("Background file: %s\n", sMusic_BackgroundLoop );
		} else {
			Com_Printf("No background file.\n" );
		}

	}
	S_DisplayFreeMemory();
	Com_Printf("----------------------\n" );
}



/*
================
S_Init
================
*/
void S_Init( void )
{
	ALCcontext *ALCContext = NULL;
	ALCdevice *ALCDevice = NULL;
	ALfloat listenerPos[]={0.0,0.0,0.0};
	ALfloat listenerVel[]={0.0,0.0,0.0};
	ALfloat	listenerOri[]={0.0,0.0,-1.0, 0.0,1.0,0.0};
	cvar_t	*cv;
	qboolean	r;
	int i,j;
	channel_t *ch;

	Com_Printf("\n------- sound initialization -------\n");

	s_volume = Cvar_Get ("s_volume", "0.8", CVAR_ARCHIVE);
	s_musicVolume = Cvar_Get ("s_musicvolume", "0.5", CVAR_ARCHIVE);

	//rww - multiply s_musicVolume by this value. Set in cgame when necessary.
	s_musicMult = Cvar_Get ("s_musicMult", "1", 0);

	s_separation = Cvar_Get ("s_separation", "0.5", CVAR_ARCHIVE);
	s_doppler = Cvar_Get ("s_doppler", "1", CVAR_ARCHIVE);
	s_khz = Cvar_Get ("s_khz", "22", CVAR_ARCHIVE);
	s_mixahead = Cvar_Get ("s_mixahead", "0.2", CVAR_ARCHIVE);

	s_mixPreStep = Cvar_Get ("s_mixPreStep", "0.05", CVAR_ARCHIVE);
	s_show = Cvar_Get ("s_show", "0", CVAR_CHEAT);
	s_testsound = Cvar_Get ("s_testsound", "0", CVAR_CHEAT);

	s_language = Cvar_Get("s_language","english",CVAR_ARCHIVE | CVAR_NORESTART);

	MP3_InitCvars();

	s_CPUType = Cvar_Get("sys_cpuid","",0);

// dontcha just love ID's defines sometimes?...
//
#if !( (defined __linux__ || __FreeBSD__ ) && (defined __i386__) )
#if	!id386
#else
	extern unsigned int uiMMXAvailable;
	uiMMXAvailable = !!(s_CPUType->integer >= CPUID_INTEL_MMX);
#endif
#endif


	cv = Cvar_Get ("s_initsound", "1", 0);
	if ( !cv->integer ) {
		Com_Printf ("not initializing.\n");
		Com_Printf("------------------------------------\n");
		return;
	}

	Cmd_AddCommand("play", S_Play_f);
	Cmd_AddCommand("music", S_Music_f);
	Cmd_AddCommand("soundlist", S_SoundList_f);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);
	Cmd_AddCommand("soundstop", S_StopAllSounds);


	cv = Cvar_Get("s_UseOpenAL" , "0",CVAR_ARCHIVE|CVAR_LATCH);
	s_UseOpenAL = !!(cv->integer);

	if (s_UseOpenAL)
	{
		ALCDevice = alcOpenDevice((ALubyte*)"DirectSound3D");
		if (!ALCDevice)
			return;
 
		//Create context(s)
		ALCContext = alcCreateContext(ALCDevice, NULL);
		if (!ALCContext)
			return;

		//Set active context
		alcMakeContextCurrent(ALCContext);		
		if (alcGetError(ALCDevice) != ALC_NO_ERROR)
			return;
		
		s_soundStarted = 1;
		s_soundMuted = qtrue;
		s_soundtime = 0;
		s_paintedtime = 0;
		s_rawend = 0;

		S_StopAllSounds();

		S_SoundInfo_f();

		// Set default level name
		memset(s_LevelName, 0, sizeof(s_LevelName));

		// Set Listener attributes
		alListenerfv(AL_POSITION,listenerPos);
		alListenerfv(AL_VELOCITY,listenerVel);
		alListenerfv(AL_ORIENTATION,listenerOri);

		InitEAXManager();

		memset(s_channels, 0, sizeof(s_channels));

		s_numChannels = 0;

		// Create as many AL Sources (up to 32) as possible
		for (i = 0; i < 32; i++)
		{
			alGenSources(1, &s_channels[i].alSource);
			if (alGetError() != AL_NO_ERROR)
			{
				// Reached limit of sources
				break;
			}
			alSourcef(s_channels[i].alSource, AL_REFERENCE_DISTANCE, 400.0f);
			if (alGetError() != AL_NO_ERROR)
			{
				break;
			}
			s_numChannels++;
		}

		// Generate AL Buffers for streaming audio playback (used for MP3s)
		ch = s_channels + 1;
		for (i = 1; i < s_numChannels; i++, ch++)
		{
			for (j = 0; j < NUM_STREAMING_BUFFERS; j++)
			{
				alGenBuffers(1, &(ch->buffers[j].BufferID));
				ch->buffers[j].Status = UNQUEUED;
				ch->buffers[j].Data = (char *)Z_Malloc(STREAMING_BUFFER_SIZE, TAG_SND_RAWDATA, qfalse);
			}
		}

		// Open AL will always use 22K
		dma.speed = 22050;

		// These aren't really relevant for Open AL, but for completeness ...
		dma.channels = 2;
		dma.samplebits = 16;
		dma.samples = 0;
		dma.submission_chunk = 0;
		dma.buffer = NULL;	

		// Clamp sound volumes between 0.0f and 1.0f
		if (s_volume->value < 0.f)
			s_volume->value = 0.f;
		if (s_volume->value > 1.f)
			s_volume->value = 1.f;

		if (s_musicVolume->value < 0.f)
			s_musicVolume->value = 0.f;
		if (s_musicVolume->value > 1.f)
			s_musicVolume->value = 1.f;

		return;
	}
	else
	{
		r = SNDDMA_Init();
		Com_Printf("------------------------------------\n");

		if ( r ) {
		s_soundStarted = 1;
		s_soundMuted = (qboolean)1;
//		s_numSfx = 0;

		s_soundtime = 0;
		s_paintedtime = 0;

		S_StopAllSounds ();

		S_SoundInfo_f();
		}
	}
}
 
/*
	Mutes / Unmutes all sound
*/
void S_MuteAllSounds(bool bMute)
{
	if (!s_soundStarted)
		return;

	if (!s_UseOpenAL)
		return;

	if (bMute)
	{
		alListenerf(AL_GAIN, 0.0f);
	}
	else
	{
		alListenerf(AL_GAIN, 1.0f);
	}
}

void S_ChannelFree(channel_t *v)
{
	if (s_UseOpenAL)
		return;

	v->thesfx = NULL;
	*(channel_t **)v = freelist;
	freelist = (channel_t*)v;
}

channel_t*	S_ChannelMalloc()
{
	channel_t *v;

	if (s_UseOpenAL)
		return NULL;

	if (freelist == NULL) {
		return NULL;
	}
	v = freelist;
	freelist = *(channel_t **)freelist;
	v->allocTime = Com_Milliseconds();
	return v;
}

void S_ChannelSetup()
{
	channel_t *p, *q;

	if (s_UseOpenAL)
		return;

	// clear all the sounds so they don't
	Com_Memset( s_channels, 0, sizeof( s_channels ) );

	p = s_channels;;
	q = p + MAX_CHANNELS;
	while (--q > p) {
		*(channel_t **)q = q-1;
	}
	
	*(channel_t **)q = NULL;
	freelist = p + MAX_CHANNELS - 1;
	Com_DPrintf("Channel memory manager started\n");
}

// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Shutdown( void )
{
	ALCcontext *ALCContext;
	ALCdevice *ALCDevice;
	channel_t *ch;
	int i,j;

	if ( !s_soundStarted ) {
		return;
	}

	if (s_UseOpenAL)
	{
		// Release all the AL Sources (including Music channel (Source 0))
		for (i = 0; i < s_numChannels; i++)
		{
			alDeleteSources(1, &(s_channels[i].alSource));
		}

		// Release all the AL Buffers here or not ?
		S_FreeAllSFXMem();

		// Release Streaming AL Buffers
		ch = s_channels + 1;
		for (i = 1; i < s_numChannels; i++, ch++)
		{
			for (j = 0; j < NUM_STREAMING_BUFFERS; j++)
			{
				alDeleteBuffers(1, &(ch->buffers[j].BufferID));
				ch->buffers[j].BufferID = 0;
				ch->buffers[j].Status = UNQUEUED;
				if (ch->buffers[j].Data)
				{
					Z_Free(ch->buffers[j].Data);
					ch->buffers[j].Data = NULL;
				}
			}
		}
		
		// Get active context
		ALCContext = alcGetCurrentContext();
		// Get device for active context
		ALCDevice = alcGetContextsDevice(ALCContext);
		// Release context(s)
		alcDestroyContext(ALCContext);
		// Close device
		alcCloseDevice(ALCDevice);

		ReleaseEAXManager();

		s_numChannels = 0;
	}
	else
	{
		SNDDMA_Shutdown();
	}

	s_soundStarted = 0;

    Cmd_RemoveCommand("play");
	Cmd_RemoveCommand("music");
	Cmd_RemoveCommand("stopsound");
	Cmd_RemoveCommand("soundlist");
	Cmd_RemoveCommand("soundinfo");
}


// =======================================================================
// Load a sound
// =======================================================================

/*
================
return a hash value for the sfx name
================
*/
static long S_HashSFXName(const char *name) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (name[i] != '\0') {
		letter = tolower(name[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (LOOP_HASH-1);
	return hash;
}


/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
sfx_t *S_FindName( const char *name ) {
	int		i;
	int		hash;

	sfx_t	*sfx;

	if (!name) {
		Com_Error (ERR_FATAL, "S_FindName: NULL\n");
	}
	if (!name[0]) {
		Com_Error (ERR_FATAL, "S_FindName: empty name\n");
	}

	if (strlen(name) >= MAX_QPATH) {
		Com_Error (ERR_FATAL, "Sound name too long: %s", name);
	}

	char sSoundNameNoExt[MAX_QPATH];
	COM_StripExtension(name,sSoundNameNoExt);

	hash = S_HashSFXName(sSoundNameNoExt);

	sfx = sfxHash[hash];
	// see if already loaded
	while (sfx) {
		if (!Q_stricmp(sfx->sSoundName, sSoundNameNoExt) ) {
			return sfx;
		}
		sfx = sfx->next;
	}
/*
	// find a free sfx
	for (i=0 ; i < s_numSfx ; i++) {
		if (!s_knownSfx[i].soundName[0]) {
			break;
		}
	}
*/
	i = s_numSfx;	//we don't clear the soundName after failed loads any more, so it'll always be the last entry
		
	if (s_numSfx == MAX_SFX) 
	{
		// ok, no sfx's free, but are there any with defaultSound set? (which the registering ent will never
		//	see because he gets zero returned if it's default...)
		//
		for (i=0 ; i < s_numSfx ; i++) {
			if (s_knownSfx[i].bDefaultSound) {
				break;
			}
		}

		if (i==s_numSfx)
		{	  
			// genuinely out of handles...

			// if we ever reach this, let me know and I'll either boost the array or put in a map-used-on
			//	reference to enable sfx_t recycling. TA codebase relies on being able to have structs for every sound
			//	used anywhere, ever, all at once (though audio bit-buffer gets recycled). SOF1 used about 1900 distinct
			//	events, so current MAX_SFX limit should do, or only need a small boost...	-ste
			//

			Com_Error (ERR_FATAL, "S_FindName: out of sfx_t");
		}
	}
	else
	{
		s_numSfx++;
	}
	
	sfx = &s_knownSfx[i];
	memset (sfx, 0, sizeof(*sfx));
	Q_strncpyz(sfx->sSoundName, sSoundNameNoExt, sizeof(sfx->sSoundName));

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void S_DefaultSound( sfx_t *sfx ) {
	
	int		i;

	sfx->iSoundLengthInSamples	= 512;								// #samples, ie shorts
	sfx->pSoundData				= (short *)	SND_malloc(512*2, sfx);	// ... so *2 for alloc bytes	
	sfx->bInMemory				= qtrue;
	
	for ( i=0 ; i < sfx->iSoundLengthInSamples ; i++ ) 
	{
		sfx->pSoundData[i] = i;
	}
}


/*
===================
S_DisableSounds

Disables sounds until the next S_BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
void S_DisableSounds( void ) {
	S_StopAllSounds();
	s_soundMuted = qtrue;
}

/*
=====================
S_BeginRegistration

=====================
*/
void S_BeginRegistration( void ) 
{
	s_soundMuted = qfalse;		// we can play again

	if (s_numSfx == 0) {
		SND_setup();

		s_numSfx = 0;
		Com_Memset( s_knownSfx, 0, sizeof( s_knownSfx ) );
		Com_Memset(sfxHash, 0, sizeof(sfx_t *)*LOOP_HASH);

		S_RegisterSound(DEFAULT_SOUND_NAME);
		S_DefaultSound( &s_knownSfx[0] );
	}
}

void EALFileInit(char *level)
{
	long		lRoom;
	char		name[MAX_QPATH];
	char		szEALFilename[MAX_QPATH];
	char		*szMapName;

	// If an EAL File is already unloaded, remove it
	if (s_bEALFileLoaded)
	{
		UnloadEALFile();
	}

	// Reset variables
	s_bInWater = false;

	// Try and load an EAL file for the new level
	COM_StripExtension(level, name);

	// Find the last occurence of the '/' character
	szMapName = Q_strrchr(name, '/');
	if (szMapName)
	{
		Com_sprintf(szEALFilename, MAX_QPATH, "eagle/%s.eal", ++szMapName);
	}
	else
	{
		Com_sprintf(szEALFilename, MAX_QPATH, "eagle/%s.eal", name);
	}

	s_bEALFileLoaded = LoadEALFile(szEALFilename);

	if (s_bEALFileLoaded)
	{
		UpdateEAXListener(true, false);
	}
	else
	{
		if ((s_bEAX)&&(s_eaxSet))
		{
			lRoom = -10000;
			s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM,
				NULL, &lRoom, sizeof(long));
		}
	}
}

/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t	S_RegisterSound( const char *name) 
{
	sfx_t	*sfx;

	if (!s_soundStarted) {
		return 0;
	}

	if (!name) 
	{
		Com_Error (ERR_FATAL, "S_RegisterSound: NULL name\n");
	}
	if (!name[0]) 
	{
		Com_Error (ERR_FATAL, "S_RegisterSound: empty name\n");
	}

	if ( strlen( name ) >= MAX_QPATH ) {		
		Com_Error (ERR_FATAL, "S_RegisterSound: Sound name exceeds MAX_QPATH - %s\n", name );
		return 0;
	}

	sfx = S_FindName( name );

	SND_TouchSFX(sfx);

	if ( sfx->bDefaultSound )
		return 0;

	if (s_UseOpenAL)
	{
		if ((sfx->pSoundData) || (sfx->Buffer))
			return sfx - s_knownSfx;
	}
	else
	{
		if ( sfx->pSoundData )
		{		
			return sfx - s_knownSfx;
		}
	}

	sfx->bInMemory = qfalse;

	S_memoryLoad(sfx);

	if ( sfx->bDefaultSound )	{
		// Suppress error for inline sounds
		if(Q_stricmp(sfx->sSoundName, DEFAULT_SOUND_NAME)){
			Com_DPrintf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->sSoundName );
		}
		return 0;
	}

	return sfx - s_knownSfx;
}

void S_memoryLoad(sfx_t	*sfx) 
{
	// load the sound file...
	//
	if ( !S_LoadSound( sfx ) ) 
	{
//		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", sfx->sSoundName );
		sfx->bDefaultSound = qtrue;
	}
	sfx->bInMemory = qtrue;
}


//=============================================================================

/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/
void S_SpatializeOrigin (vec3_t origin, int master_vol, int *left_vol, int *right_vol)
{
    vec_t		dot;
    vec_t		dist;
    vec_t		lscale, rscale, scale;
    vec3_t		source_vec;
    vec3_t		vec;

	float dist_mult = SOUND_ATTENUATE;	// the more you divide it down, the further away the sound can be heard
	
	// calculate stereo seperation and distance attenuation
	VectorSubtract(origin, listener_origin, source_vec);

	dist = VectorNormalize(source_vec);
	dist -= SOUND_FULLVOLUME;
	if (dist < 0)
		dist = 0;			// close enough to be at full volume
	dist *= dist_mult;		// different attenuation levels
	
	VectorRotate( source_vec, listener_axis, vec );

	dot = -vec[1];

	if (dma.channels == 1)
	{ // no attenuation = no spatialization
		rscale = 1.0;
		lscale = 1.0;
	}
	else
	{
		rscale = 0.5 * (1.0 + dot);
		lscale = 0.5 * (1.0 - dot);
		//rscale = s_separation->value + ( 1.0 - s_separation->value ) * dot;
		//lscale = s_separation->value - ( 1.0 - s_separation->value ) * dot;
		if ( rscale < 0 ) {
			rscale = 0;
		}
		if ( lscale < 0 ) {
			lscale = 0;
		}
	}

	// add in distance effect
	scale = (1.0 - dist) * rscale;
	*right_vol = (master_vol * scale);
	if (*right_vol < 0)
		*right_vol = 0;

	scale = (1.0 - dist) * lscale;
	*left_vol = (master_vol * scale);
	if (*left_vol < 0)
		*left_vol = 0;
}

static qboolean S_CheckChannelStomp( int chan1, int chan2 )
{
	if ( chan1 == chan2 )
	{
		return qtrue;
	}

	// Hmmmm. without CHAN_VOICE_ATTEN this logic just dups the above, so for now...
	//
//	if ( ( chan1 == CHAN_VOICE /* || chan1 == CHAN_VOICE_ATTEN*/ ) && ( chan2 == CHAN_VOICE /*|| chan2 == CHAN_VOICE_ATTEN*/ ) )
//	{
//		return qtrue;
//	}

	return qfalse;
}



channel_t *S_PickChannel(int entnum, int entchannel)
{
    int			ch_idx;
	channel_t	*ch, *ch_firstToDie;
	qboolean	foundChan = qfalse;

	if (s_UseOpenAL)
		return S_OpenALPickChannel(entnum, entchannel);

	if ( entchannel<0 ) {
		Com_Error (ERR_DROP, "S_PickChannel: entchannel<0");
	}

	// Check for replacement sound, or find the best one to replace

    ch_firstToDie = &s_channels[0];

	for ( int pass = 0; (pass < ((entchannel == CHAN_AUTO)?1:2)) && !foundChan; pass++ )
	{
		for (ch_idx = 0,
			ch = &s_channels[ch_idx]; ch_idx < MAX_CHANNELS ; ch_idx++, ch++ ) 
		{
			if ( entchannel == CHAN_AUTO || pass > 0 )
			{//if we're on the second pass, or we're doing a CHAN_AUTO then just find the first open chan
				if ( !ch->thesfx )
				{//grab the first open channel
					ch_firstToDie = ch;
					foundChan = qtrue;
					break;
				}
			}
			else if ( ch->entnum == entnum 
					 //&& (/*entchannel != CHAN_AMBIENT*/1 || pass)	// don't override Ambient sounds unless 2nd pass (ie all channels in use)
					 && S_CheckChannelStomp( ch->entchannel, entchannel ) 
					 ) 
			{
				// always override sound from same entity
				if ( s_show->integer == 1 && ch->thesfx ) {
					Com_Printf( S_COLOR_YELLOW"...overrides %s\n", ch->thesfx->sSoundName );
					ch->thesfx = 0;	//just to clear the next error msg
				}
				ch_firstToDie = ch;
				foundChan = qtrue;
				break;
			}

			// don't let anything else override local player sounds
			if ( ch->entnum == listener_number 	&& entnum != listener_number && ch->thesfx) {
				continue;
			}

			// Ignore this, loopSounds are different array under TA codebase...
			//
			//			// don't override loop sounds  
			//			if ( ch->loopSound ) {			
			//				continue;
			//			}

			if ( ch->startSample < ch_firstToDie->startSample ) {
				ch_firstToDie = ch;
			}
		}
	}

	if ( s_show->integer == 1 && ch_firstToDie->thesfx ) {
		Com_Printf( S_COLOR_RED"***kicking %s\n", ch_firstToDie->thesfx->sSoundName );
	}

	Channel_Clear(ch_firstToDie);	// memset (ch_firstToDie, 0, sizeof(*ch_firstToDie));
	
    return ch_firstToDie;
}


/*
	For use with Open AL

	Allows more than one sound of the same type to emanate from the same entity - sounds much better
	on hardware this way esp. rapid fire modes of weapons!
*/
channel_t *S_OpenALPickChannel(int entnum, int entchannel)
{
    int			ch_idx;
	channel_t	*ch, *ch_firstToDie;
	bool	foundChan = false;
	float	source_pos[3];

	if ( entchannel < 0 ) 
	{
		Com_Error (ERR_DROP, "S_PickChannel: entchannel<0");
	}

	// Check for replacement sound, or find the best one to replace

    ch_firstToDie = s_channels + 1;	// channel 0 is reserved for Music

	for (ch_idx = 1, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++)
	{
		// See if the channel is free
		if (!ch->thesfx)
		{
			ch_firstToDie = ch;
			foundChan = true;
			break;
		}
	}

	if (!foundChan)
	{
		for (ch_idx = 1, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++)
		{
			if (	(ch->entnum == entnum) && (ch->entchannel == entchannel) && (ch->entnum != listener_number) ) 
			{
				// Same entity and same type of sound effect (entchannel)
				ch_firstToDie = ch;
				foundChan = true;
				break;
			}
		}
	}

	int longestDist;
	int dist;

	if (!foundChan)
	{
		// Find sound effect furthest from listener
		ch = s_channels + 1;

		if (ch->fixed_origin)
		{
			// Convert to Open AL co-ordinates
			source_pos[0] = ch->origin[0];
			source_pos[1] = ch->origin[2];
			source_pos[2] = -ch->origin[1];

			longestDist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
						  ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
						  ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
		}
		else
		{
			if (ch->entnum == listener_number)
				longestDist = 0;
			else
			{
				// Convert to Open AL co-ordinates
				source_pos[0] = loopSounds[ch->entnum].origin[0];
				source_pos[1] = loopSounds[ch->entnum].origin[2];
				source_pos[2] = -loopSounds[ch->entnum].origin[1];

				longestDist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
							  ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
							  ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
			}
		}

		for (ch_idx = 2, ch = s_channels + ch_idx; ch_idx < s_numChannels; ch_idx++, ch++)
		{
			if (ch->fixed_origin)
			{
				// Convert to Open AL co-ordinates
				source_pos[0] = ch->origin[0];
				source_pos[1] = ch->origin[2];
				source_pos[2] = -ch->origin[1];

				dist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
					   ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
					   ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
			}
			else
			{
				if (ch->entnum == listener_number)
					dist = 0;
				else
				{
					// Convert to Open AL co-ordinates
					source_pos[0] = loopSounds[ch->entnum].origin[0];
					source_pos[1] = loopSounds[ch->entnum].origin[2];
					source_pos[2] = -loopSounds[ch->entnum].origin[1];

					dist = ((listener_pos[0] - source_pos[0]) * (listener_pos[0] - source_pos[0])) +
					       ((listener_pos[1] - source_pos[1]) * (listener_pos[1] - source_pos[1])) +
						   ((listener_pos[2] - source_pos[2]) * (listener_pos[2] - source_pos[2]));
				}
			}

			if (dist > longestDist)
			{
				longestDist = dist;
				ch_firstToDie = ch;
			}
		}
	}

	if (ch_firstToDie->bPlaying)
	{
		if (s_show->integer == 1) 
		{
			Com_Printf(S_COLOR_RED"***kicking %s\n", ch_firstToDie->thesfx->sSoundName );
		}

		// Stop sound
		alSourceStop(ch_firstToDie->alSource);
		ch_firstToDie->bPlaying = false;
	}

	// Reset channel variables
	memset(&ch_firstToDie->MP3StreamHeader, 0, sizeof(MP3STREAM));
	ch_firstToDie->bLooping = false;
	ch_firstToDie->bProcessed = false;
	ch_firstToDie->bStreaming = false;
	
    return ch_firstToDie;
}

/*
====================
S_MuteSound

Gets the specified ent/channel and mutes any sound currently playing on it
====================
*/
void S_MuteSound(int entityNum, int entchannel)
{
	channel_t	*ch;
	int	i;

	if (entchannel < 1)
	{
		return;
	}

	if (s_UseOpenAL)
	{
		ch = s_channels + 1;
		for(i = 1; i < s_numChannels; i++,ch++)
		{
			if ((ch->entnum == entityNum) && (ch->entchannel == entchannel))
			{
				alSourcef(ch->alSource, AL_GAIN, 0.0f);
				break;
			}
		}
	}
	else
	{
		ch = S_PickChannel( entityNum, entchannel );
	
		if (!ch)
		{
			return;
		}

		ch->master_vol = 0; //just kill the volume and leave the rest alone, as to not actually interrupt anything expecting the sound to go through
		ch->leftvol = 0;
		ch->rightvol = 0;
	}
}


// =======================================================================
// Start a sound effect
// =======================================================================

/*
====================
S_StartSound

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
void S_StartSound(vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfxHandle ) {
	channel_t	*ch;
	sfx_t		*sfx;
	int			i;
	int			curTime;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( !origin && ( entityNum < 0 || entityNum > MAX_GENTITIES ) ) {
		Com_Error( ERR_DROP, "S_StartSound: bad entitynum %i", entityNum );
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_StartSound: handle %i out of range", sfxHandle );
	}

	sfx = &s_knownSfx[ sfxHandle ];
	if (sfx->bInMemory == qfalse) {
		S_memoryLoad(sfx);
	}
	

	if ( s_show->integer == 1 ) {
		Com_Printf( "%i : %s\n", s_paintedtime, sfx->sSoundName );
	}

	if (s_UseOpenAL)
	{
		// To avoid playing the same sound multiple times ...
		if ((entityNum == ENTITYNUM_NONE) && (origin))
		{
			// Check if we have already started playing this sound within 50 milliseconds ago
			ch = s_channels + 1;
			curTime = Com_Milliseconds();
			for (i = 1; i < s_numChannels; i++, ch++)
			{
				if ((ch->thesfx == sfx) && (curTime < (ch->thesfx->iLastTimeUsed + 50)))
				{
					return;
				}
			}
		}
		else if (entchannel == CHAN_WEAPON)
		{
			// Check if we are playing a 'charging' sound, if so, stop it now ..
			ch = s_channels + 1;
			for (i = 1; i < s_numChannels; i++, ch++)
			{
				if ((ch->entnum == entityNum) && (ch->entchannel == CHAN_WEAPON) && (ch->thesfx) && (strstr(strlwr(ch->thesfx->sSoundName), "altcharge") != NULL))
				{
					// Stop this sound
					alSourceStop(ch->alSource);
					alSourcei(ch->alSource, AL_BUFFER, NULL);
					ch->bPlaying = false;
					ch->thesfx = NULL;
					break;
				}
			}
		}
		else
		{
			ch = s_channels + 1;
			for (i = 1; i < s_numChannels; i++, ch++)
			{
				if ((ch->entnum == entityNum) && (ch->thesfx) && (strstr(strlwr(ch->thesfx->sSoundName), "falling") != NULL))
				{
					// Stop this sound
					alSourceStop(ch->alSource);
					alSourcei(ch->alSource, AL_BUFFER, NULL);
					ch->bPlaying = false;
					ch->thesfx = NULL;
					break;
				}
			}
		}
	}

	SND_TouchSFX(sfx);

// pick a channel to play on
//---------------------------------	
	ch = S_PickChannel(entityNum, entchannel);
	ch->allocTime = sfx->iLastTimeUsed;
/*	ch = S_ChannelMalloc();	// entityNum, entchannel);
	if (!ch) {
		ch = s_channels;

		oldest = sfx->lastTimeUsed;
		chosen = -1;
		for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
			if (ch->entnum != listener_number && ch->entnum == entityNum && ch->allocTime<oldest && ch->entchannel != CHAN_ANNOUNCER) {
				oldest = ch->allocTime;
				chosen = i;
			}
		}
		if (chosen == -1) {
			ch = s_channels;
			for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
				if (ch->entnum != listener_number && ch->allocTime<oldest && ch->entchannel != CHAN_ANNOUNCER) {
					oldest = ch->allocTime;
					chosen = i;
				}
			}
			if (chosen == -1) {				
				Com_Printf("S_StartSound(): dropping sound \"%s\"\n",sfx->soundName);
				return;
			}
		}
		ch = &s_channels[chosen];
		ch->allocTime = sfx->lastTimeUsed;
	}
*/
//---------------------------------

	if (origin) {
		VectorCopy (origin, ch->origin);
		ch->fixed_origin = qtrue;
	} else {
		ch->fixed_origin = qfalse;
	}

	if (s_UseOpenAL)
		ch->master_vol = 255;
	else
		ch->master_vol = 240;

	ch->entnum = entityNum;
	ch->thesfx = sfx;
	ch->startSample = START_SAMPLE_IMMEDIATE;
	ch->entchannel = entchannel;
	ch->leftvol = ch->master_vol;		// these will get calced at next spatialize
	ch->rightvol = ch->master_vol;		// unless the game isn't running
	ch->doppler = qfalse;

	// EF1 also had this, do we want it?
/*	
	if (entchannel < CHAN_AMBIENT && entityNum == listener_number) {	//only do it for body sounds not local sounds
		ch->master_vol = SOUND_MAXVOL * SOUND_FMAXVOL;	//this won't be attenuated so let it scale down
	}
	(SOUND_FMAXVOL = 0.75f)
	(SOUND_MAXVOL = 255)
*/
//	if ( entchannel == CHAN_VOICE )//|| entchannel == CHAN_VOICE_ATTEN ) 
//	{
//		s_entityWavVol[ ch->entnum ] = -1;	//we've started the sound but it's silent for now
//	}

	if (sfx->pMP3StreamHeader)	// -ste
	{
		memcpy(&ch->MP3StreamHeader,sfx->pMP3StreamHeader,	sizeof(ch->MP3StreamHeader));
		ch->iMP3SlidingDecodeWritePos = 0;
		ch->iMP3SlidingDecodeWindowPos= 0;
	}
	else
	{
		memset(&ch->MP3StreamHeader,0,						sizeof(ch->MP3StreamHeader));
	}

}


/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum ) {
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW, "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}

	S_StartSound (NULL, listener_number, channelNum, sfxHandle );
}


/*
==================
S_StartLocalLoopingSound
==================
*/
void S_StartLocalLoopingSound( sfxHandle_t sfxHandle) {
	vec3_t nullVec = {0,0,0};

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_StartLocalLoopingSound: handle %i out of range", sfxHandle );
	}

	S_AddLoopingSound( listener_number, nullVec, nullVec, sfxHandle);//, CHAN_AUTO );
}


/*
==================
S_ClearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/
void S_ClearSoundBuffer( void ) {
	int		clear;
		
	if (!s_soundStarted)
		return;

	// stop looping sounds
	Com_Memset(loopSounds, 0, MAX_GENTITIES*sizeof(loopSound_t));
	Com_Memset(loop_channels, 0, MAX_CHANNELS*sizeof(channel_t));
	numLoopChannels = 0;

	S_ChannelSetup();

	s_rawend = 0;

	if (!s_UseOpenAL)
	{
		if (dma.samplebits == 8)
			clear = 0x80;
		else
			clear = 0;

		SNDDMA_BeginPainting ();
		if (dma.buffer)
			Com_Memset(dma.buffer, clear, dma.samples * dma.samplebits/8);
		SNDDMA_Submit ();
	}
}


/*
==================
S_StopAllSounds
==================
*/
void S_StopAllSounds(void)
{
	channel_t *ch;
	int i;

	if ( !s_soundStarted ) {
		return;
	}

	// stop the background music
	S_StopBackgroundTrack();

	if (s_UseOpenAL)
	{
		ch = s_channels;
		for (i = 0; i < s_numChannels; i++, ch++)
		{
			alSourceStop(s_channels[i].alSource);
			ch->thesfx = NULL;
			memset(&ch->MP3StreamHeader, 0, sizeof(MP3STREAM));
			ch->bLooping = false;
			ch->bProcessed = false;
			ch->bPlaying = false;
			ch->bStreaming = false;
		}
	}

	S_ClearSoundBuffer ();
}

/*
==============================================================

continuous looping sounds are added each frame

==============================================================
*/

void S_StopLoopingSound(int entityNum) {
	loopSounds[entityNum].active = qfalse;
//	loopSounds[entityNum].sfx = 0;
	loopSounds[entityNum].kill = qfalse;
}

/*
==================
S_ClearLoopingSounds

==================
*/
void S_ClearLoopingSounds( qboolean killall ) {
	int i;
	for ( i = 0 ; i < MAX_GENTITIES ; i++) {
		if (killall || loopSounds[i].kill == qtrue || (loopSounds[i].sfx && loopSounds[i].sfx->iSoundLengthInSamples == 0)) {
			loopSounds[i].kill = qfalse;
			S_StopLoopingSound(i);
		}
	}
	numLoopChannels = 0;
}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle) {
	sfx_t *sfx;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Error( ERR_DROP, "S_AddLoopingSound: handle %i out of range", sfxHandle );
	}

	sfx = &s_knownSfx[ sfxHandle ];
	if (sfx->bInMemory == qfalse) {
		S_memoryLoad(sfx);
	}
	SND_TouchSFX(sfx);

	if ( !sfx->iSoundLengthInSamples ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->sSoundName );
	}

	VectorCopy( origin, loopSounds[entityNum].origin );
	VectorCopy( velocity, loopSounds[entityNum].velocity );
	loopSounds[entityNum].active = qtrue;
	loopSounds[entityNum].kill = qtrue;
	loopSounds[entityNum].doppler = qfalse;
	loopSounds[entityNum].oldDopplerScale = 1.0;
	loopSounds[entityNum].dopplerScale = 1.0;

	if (s_UseOpenAL)
	{
		if ((loopSounds[entityNum].bPlaying) && (loopSounds[entityNum].sfx != sfx))
		{
			// Find the channel that is playing this sound, and stop it
			channel_t *ch;
			ch = s_channels + 1;
			for (int i = 1; i < s_numChannels; i++, ch++)
			{
				if ((ch->bLooping) && (ch->entnum == entityNum))
				{
					alSourceStop(ch->alSource);

					ch->bPlaying = false;
					ch->thesfx = NULL;

					loopSounds[entityNum].bPlaying = false;

					break;
				}
			}
		}
	}
	
	loopSounds[entityNum].sfx = sfx;
/*
	if (VectorLengthSquared(velocity)>0.0) {
		vec3_t	out;
		float	lena, lenb;

		loopSounds[entityNum].doppler = qtrue;
		lena = DistanceSquared(loopSounds[listener_number].origin, loopSounds[entityNum].origin);
		VectorAdd(loopSounds[entityNum].origin, loopSounds[entityNum].velocity, out);
		lenb = DistanceSquared(loopSounds[listener_number].origin, out);
		if ((loopSounds[entityNum].framenum+1) != cls.framecount) {
			loopSounds[entityNum].oldDopplerScale = 1.0;
		} else {
			loopSounds[entityNum].oldDopplerScale = loopSounds[entityNum].dopplerScale;
		}
		loopSounds[entityNum].dopplerScale = lenb/(lena*100);
		if (loopSounds[entityNum].dopplerScale<0.5) {
			loopSounds[entityNum].dopplerScale = 0.5;
		}
	}
*/
	loopSounds[entityNum].framenum = cls.framecount;	
}


/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle ) {
	sfx_t *sfx;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Printf( S_COLOR_YELLOW, "S_AddRealLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &s_knownSfx[ sfxHandle ];

	if (sfx->bInMemory == qfalse) {
		S_memoryLoad(sfx);
	}
	SND_TouchSFX(sfx);

	if ( !sfx->iSoundLengthInSamples ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->sSoundName );
	}
	VectorCopy( origin, loopSounds[entityNum].origin );
	VectorCopy( velocity, loopSounds[entityNum].velocity );
	loopSounds[entityNum].sfx = sfx;
	loopSounds[entityNum].active = qtrue;
	loopSounds[entityNum].kill = qfalse;
	loopSounds[entityNum].doppler = qfalse;
}


// returns qtrue if ok to continue, else qfalse if all channels filled up this frame...
//
static qboolean LoopSound_ChannelInit(loopSound_t *pLoopSound, int iLeftVol, int iRightVol)
{
	// allocate a channel
	//
	channel_t *ch = &loop_channels[numLoopChannels];
	
	if (iLeftVol > 255) {
		iLeftVol = 255;
	}
	if (iRightVol > 255) {
		iRightVol = 255;
	}
	
	ch->master_vol		= 255;
	ch->leftvol			= iLeftVol;
	ch->rightvol		= iRightVol;
	ch->thesfx			= pLoopSound->sfx;
	ch->doppler			= pLoopSound->doppler;
	ch->dopplerScale	= pLoopSound->dopplerScale;
	ch->oldDopplerScale = pLoopSound->oldDopplerScale;

	// you cannot use MP3 files here because they offer only streaming access, not random
	//
	if (pLoopSound->sfx->pMP3StreamHeader)
	{
		Com_Error( ERR_DROP, "LoopSound_ChannelInit(): Cannot use streamed MP3 files here for random access (%s)\n",pLoopSound->sfx->sSoundName );
	}
	else
	{
		memset( &ch->MP3StreamHeader, 0, sizeof(ch->MP3StreamHeader) );
	}

	numLoopChannels++;

	if (s_UseOpenAL)
	{
		if (numLoopChannels == s_numChannels)
			return qfalse;
	}
	else
	{
		if (numLoopChannels == MAX_CHANNELS) {
			return qfalse;
		}
	}

	return qtrue;
}

// returns qfalse if sound would be inaudible, else qtrue for go ahead and play it this frame...
//
static qboolean LoopSound_SetupVolume(loopSound_t *pLoopSound, int time, int &iLeftTotal, int &iRightTotal)
{
	if (pLoopSound->kill) 
	{
		S_SpatializeOrigin( pLoopSound->origin, 240, &iLeftTotal, &iRightTotal);	// 3d
	} 
	else 
	{
		S_SpatializeOrigin( pLoopSound->origin, 180/*90*/,  &iLeftTotal, &iRightTotal);	// sphere
	}

	pLoopSound->sfx->iLastTimeUsed = time;

	if (iLeftTotal == 0 && iRightTotal == 0)
		return qfalse;	// not audible

	return qtrue;
}



/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
void S_AddLoopSounds (void) {
	int			i, time;
	int			left_total, right_total;
	loopSound_t	*loop;
	static int	loopFrame;

	numLoopChannels = 0;

	time = Com_Milliseconds();
	loopFrame++;

	// now do the standard ones...
	//	
	for ( i = 0 ; i < MAX_GENTITIES ; i++) 
	{
		loop = &loopSounds[i];
		
		if ( !loop->active	
			// this next test is pointless, since ->mergeFrame is never set - remove field? -ste.
			// || loop->mergeFrame == loopFrame 	// already merged into an earlier sound
			)
		{
			continue;	
		}

		if (LoopSound_SetupVolume(loop, time, left_total, right_total))
		{
			if (!LoopSound_ChannelInit(loop, left_total, right_total))
				return;	// all looping channels occupied
		}
	}
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endien binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
	int		i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( s_channels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	}
}

portable_samplepair_t *S_GetRawSamplePointer() {
	return s_rawsamples;
}

/*
============
S_RawSamples

Music streaming
============
*/
void S_RawSamples( int samples, int rate, int width, int s_channels, const byte *data, float volume ) {
	int		i;
	int		src, dst;
	float	scale;
	int		intVolume;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	intVolume = 256 * volume;

	if ( s_rawend < s_soundtime ) {
		Com_DPrintf( "S_RawSamples: resetting minimum: %i < %i\n", s_rawend, s_soundtime );
		s_rawend = s_soundtime;
	}

	scale = (float)rate / dma.speed;

//Com_Printf ("%i < %i < %i\n", s_soundtime, s_paintedtime, s_rawend);
	if (s_channels == 2 && width == 2)
	{
		if (scale == 1.0)
		{	// optimized case
			for (i=0 ; i<samples ; i++)
			{
				dst = s_rawend&(MAX_RAW_SAMPLES-1);
				s_rawend++;
				s_rawsamples[dst].left = ((short *)data)[i*2] * intVolume;
				s_rawsamples[dst].right = ((short *)data)[i*2+1] * intVolume;
			}
		}
		else
		{
			for (i=0 ; ; i++)
			{
				src = i*scale;
				if (src >= samples)
					break;
				dst = s_rawend&(MAX_RAW_SAMPLES-1);
				s_rawend++;
				s_rawsamples[dst].left = ((short *)data)[src*2] * intVolume;
				s_rawsamples[dst].right = ((short *)data)[src*2+1] * intVolume;
			}
		}
	}
	else if (s_channels == 1 && width == 2)
	{
		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left = ((short *)data)[src] * intVolume;
			s_rawsamples[dst].right = ((short *)data)[src] * intVolume;
		}
	}
	else if (s_channels == 2 && width == 1)
	{
		intVolume *= 256;

		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left = ((char *)data)[src*2] * intVolume;
			s_rawsamples[dst].right = ((char *)data)[src*2+1] * intVolume;
		}
	}
	else if (s_channels == 1 && width == 1)
	{
		intVolume *= 256;

		for (i=0 ; ; i++)
		{
			src = i*scale;
			if (src >= samples)
				break;
			dst = s_rawend&(MAX_RAW_SAMPLES-1);
			s_rawend++;
			s_rawsamples[dst].left = (((byte *)data)[src]-128) * intVolume;
			s_rawsamples[dst].right = (((byte *)data)[src]-128) * intVolume;
		}
	}

	if ( s_rawend > s_soundtime + MAX_RAW_SAMPLES ) {
		Com_DPrintf( "S_RawSamples: overflowed %i > %i\n", s_rawend, s_soundtime );
	}
}

//=============================================================================

/*
=====================
S_UpdateEntityPosition

let the sound system know where an entity currently is
======================
*/
void S_UpdateEntityPosition( int entityNum, const vec3_t origin )
{
	ALfloat pos[3];
	channel_t *ch;
	int i;

	if ( entityNum < 0 || entityNum > MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "S_UpdateEntityPosition: bad entitynum %i", entityNum );
	}

	if (s_UseOpenAL)
	{
		if (entityNum == listener_number)
			return;

		ch = s_channels;
		for (i = 0; i < s_numChannels; i++, ch++)
		{
			if ((s_channels[i].bPlaying) & (s_channels[i].entnum == entityNum))
			{
				pos[0] = origin[0];
				pos[1] = origin[2];
				pos[2] = -origin[1];
				alSourcefv(s_channels[i].alSource, AL_POSITION, pos);
	
				if (s_bEALFileLoaded)
				{
					UpdateEAXBuffer(ch);
				}
			}
		}
	}

	VectorCopy( origin, loopSounds[entityNum].origin );
}


/*
============
S_Respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
void S_Respatialize( int entityNum, const vec3_t head, vec3_t axis[3], int inwater )
{
	EAXOCCLUSIONPROPERTIES eaxOCProp;
	unsigned int ulEnvironment;
	int			i;
	channel_t	*ch;
	vec3_t		origin;
	char		*mapname;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if (s_UseOpenAL)
	{
		// Check if a new level has been loaded - if so, try and load the appropriate EAL file
		mapname = cl.mapname;
		if ((mapname) && (strcmp(mapname, s_LevelName) != 0))
		{
			EALFileInit(mapname);
			strcpy(s_LevelName, mapname);
		}

		listener_number = entityNum;
		
		listener_pos[0] = head[0];
		listener_pos[1] = head[2];
		listener_pos[2] = -head[1];
		alListenerfv(AL_POSITION, listener_pos);

		listener_ori[0] = axis[0][0];
		listener_ori[1] = axis[0][2];
		listener_ori[2] = -axis[0][1];
		listener_ori[3] = axis[2][0];
		listener_ori[4] = axis[2][2];
		listener_ori[5] = -axis[2][1];
		alListenerfv(AL_ORIENTATION, listener_ori);

		// Update EAX effects here
		if (s_bEALFileLoaded)
		{
			// Check if the Listener is underwater
			if (inwater)
			{
				// Check if we have already applied Underwater effect
				if (!s_bInWater)
				{
					// Apply Underwater Reverb effect, and occlude *all* Sources			
					ulEnvironment = EAX_ENVIRONMENT_UNDERWATER;
					s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENT,
						NULL, &ulEnvironment, sizeof(unsigned int));
					s_EnvironmentID = 999;

					eaxOCProp.lOcclusion = -3000;
					eaxOCProp.flOcclusionLFRatio = 0.0f;
					eaxOCProp.flOcclusionRoomRatio = 1.37f;
					eaxOCProp.flOcclusionDirectRatio = 1.0f;

					ch = s_channels + 1;
					for (i = 1; i < s_numChannels; i++, ch++)
					{
						s_eaxSet(&DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONPARAMETERS,
							ch->alSource, &eaxOCProp, sizeof(EAXOCCLUSIONPROPERTIES));
					}

					s_bInWater = true;
				}
			}
			else
			{
				// Not underwater ... check if the underwater effect is still present
				if (s_bInWater)
				{
					// Remove underwater Reverb effect, and reset Occlusion / Obstruction amount on all Sources
					UpdateEAXListener(false, false);

					ch = s_channels + 1;
					for (i = 1; i < s_numChannels; i++, ch++)
					{
						UpdateEAXBuffer(ch);
					}

					s_bInWater = false;
				}
				else
				{
					UpdateEAXListener(false, true);
				}
			}
		}
	}
	else
	{
		listener_number = entityNum;
		VectorCopy(head, listener_origin);
		VectorCopy(axis[0], listener_axis[0]);
		VectorCopy(axis[1], listener_axis[1]);
		VectorCopy(axis[2], listener_axis[2]);

		// update spatialization for dynamic sounds	
		ch = s_channels;
		for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
			if ( !ch->thesfx ) {
				continue;
			}
			// anything coming from the view entity will always be full volume
			if (ch->entnum == listener_number) {
				ch->leftvol = ch->master_vol;
				ch->rightvol = ch->master_vol;
			} else {
				if (ch->fixed_origin) {
					VectorCopy( ch->origin, origin );
				} else {
					VectorCopy( loopSounds[ ch->entnum ].origin, origin );
				}

				S_SpatializeOrigin (origin, ch->master_vol, &ch->leftvol, &ch->rightvol);
			}
		}

		// add loopsounds
		S_AddLoopSounds ();
	}
}


/*
========================
S_ScanChannelStarts

Returns qtrue if any new sounds were started since the last mix
========================
*/
qboolean S_ScanChannelStarts( void ) {
	channel_t		*ch;
	int				i;
	qboolean		newSamples;

	newSamples = qfalse;
	ch = s_channels;

	for (i=0; i<MAX_CHANNELS ; i++, ch++) {
		if ( !ch->thesfx ) {
			continue;
		}
		// if this channel was just started this frame,
		// set the sample count to it begins mixing
		// into the very first sample
		if ( ch->startSample == START_SAMPLE_IMMEDIATE ) {
			ch->startSample = s_paintedtime;
			newSamples = qtrue;
			continue;
		}

		// if it is completely finished by now, clear it
		if ( ch->startSample + (ch->thesfx->iSoundLengthInSamples) <= s_paintedtime ) {
			S_ChannelFree(ch);
		}
	}

	return newSamples;
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update( void ) {
	int			i;
	int			total;
	channel_t	*ch;

	if ( !s_soundStarted || s_soundMuted ) {
		Com_DPrintf ("not started or muted\n");
		return;
	}

	if (s_UseOpenAL)
	{
		//
		// debugging output
		//
		if ( s_show->integer == 2 )
		{
			total = 0;
			ch = s_channels + 1;
			for (i=1 ; i<s_numChannels; i++, ch++) {
				if (ch->thesfx && (ch->leftvol || ch->rightvol) ) {
					Com_Printf ("%s\n", ch->thesfx->sSoundName);
					total++;
				}
			}
		}
	}
	else
	{
		//
		// debugging output
		//
		if ( s_show->integer == 2 ) {
			total = 0;
			ch = s_channels;
			for (i=0 ; i<MAX_CHANNELS; i++, ch++) {
				if (ch->thesfx && (ch->leftvol || ch->rightvol) ) {
					Com_Printf ("%f %f %s\n", ch->leftvol, ch->rightvol, ch->thesfx->sSoundName);
					total++;
				}
			}
		
			Com_Printf ("----(%i)---- painted: %i\n", total, s_paintedtime);
		}
	}

	// The Open AL code, handles background music in the S_UpdateRawSamples function
	if (!s_UseOpenAL)
	{
		// add raw data from streamed samples
		S_UpdateBackgroundTrack();
	}

	// mix some sound
	S_Update_();
}

void S_GetSoundtime(void)
{
	int		samplepos;
	static	int		buffers;
	static	int		oldsamplepos;
	int		fullsamples;
	
	fullsamples = dma.samples / dma.channels;

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();
	if (samplepos < oldsamplepos)
	{
		buffers++;					// buffer wrapped
		
		if (s_paintedtime > 0x40000000)
		{	// time to chop things off to avoid 32 bit limits
			buffers = 0;
			s_paintedtime = fullsamples;
			S_StopAllSounds ();
		}
	}
	oldsamplepos = samplepos;

	s_soundtime = buffers*fullsamples + samplepos/dma.channels;

#if 0
// check to make sure that we haven't overshot
	if (s_paintedtime < s_soundtime)
	{
		Com_DPrintf ("S_Update_ : overflow\n");
		s_paintedtime = s_soundtime;
	}
#endif

	if ( dma.submission_chunk < 256 ) {
		s_paintedtime = s_soundtime + s_mixPreStep->value * dma.speed;
	} else {
		s_paintedtime = s_soundtime + dma.submission_chunk;
	}
}


void S_Update_(void) {
	unsigned        endtime;
	int				samps;
	static			float	lastTime = 0.0f;
	float			ma, op;
	float			thisTime, sane;
	static			int ot = -1;
	channel_t		*ch;
	int				i, j;
	int				source;
	float			pos[3];

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if (s_UseOpenAL)
	{
		UpdateSingleShotSounds();

		ch = s_channels + 1;
		for ( i = 1; i < s_numChannels; i++, ch++ )
		{
			if ( !ch->thesfx || (ch->bPlaying))
				continue;			
	
			source = ch - s_channels;

			// Get position of source
			if (ch->fixed_origin)
			{
				pos[0] = ch->origin[0];
				pos[1] = ch->origin[2];
				pos[2] = -ch->origin[1];
				alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_FALSE);
			}
			else
			{
				if (ch->entnum == listener_number)
				{
					pos[0] = 0.0f;
					pos[1] = 0.0f;
					pos[2] = 0.0f;
					alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_TRUE);
				}
				else
				{
					// Get position of Entity
					pos[0] = loopSounds[ ch->entnum ].origin[0];
					pos[1] = loopSounds[ ch->entnum ].origin[2];
					pos[2] = -loopSounds[ ch->entnum ].origin[1];
					alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_FALSE);
				}
			}
	
			alSourcefv(s_channels[source].alSource, AL_POSITION, pos);

			alSourcei(s_channels[source].alSource, AL_LOOPING, AL_FALSE);
			alSourcef(s_channels[source].alSource, AL_GAIN, ((float)(ch->master_vol) * s_volume->value) / 255.0f);

			if (s_bEALFileLoaded)
				UpdateEAXBuffer(ch);

			int nBytesDecoded = 0;
			int nTotalBytesDecoded = 0;
			int nBuffersToAdd = 0;

			if (ch->thesfx->pMP3StreamHeader)
			{
				memcpy(&ch->MP3StreamHeader, ch->thesfx->pMP3StreamHeader,	sizeof(ch->MP3StreamHeader));
				ch->iMP3SlidingDecodeWritePos = 0;
				ch->iMP3SlidingDecodeWindowPos= 0;

				// Reset streaming buffers status's
				for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
					ch->buffers[i].Status = UNQUEUED;

				// Decode (STREAMING_BUFFER_SIZE / 1152) MP3 frames for each of the NUM_STREAMING_BUFFERS AL Buffers
				for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
				{
					nTotalBytesDecoded = 0;
					
					for (j = 0; j < (STREAMING_BUFFER_SIZE / 1152); j++)
					{
						nBytesDecoded = C_MP3Stream_Decode(&ch->MP3StreamHeader);

						memcpy(ch->buffers[i].Data + nTotalBytesDecoded, ch->MP3StreamHeader.bDecodeBuffer, nBytesDecoded);

						nTotalBytesDecoded += nBytesDecoded;
					}

					if (nTotalBytesDecoded != STREAMING_BUFFER_SIZE)
					{
						memset(ch->buffers[i].Data + nTotalBytesDecoded, 0, (STREAMING_BUFFER_SIZE - nTotalBytesDecoded));
						break;
					}
				}

				if (i >= NUM_STREAMING_BUFFERS)
					nBuffersToAdd = NUM_STREAMING_BUFFERS;
				else
					nBuffersToAdd = i + 1;

				// Make sure queue is empty first
				alSourcei(s_channels[source].alSource, AL_BUFFER, NULL);

				for (i = 0; i < nBuffersToAdd; i++)
				{
					// Copy decoded data to AL Buffer
					alBufferData(ch->buffers[i].BufferID, AL_FORMAT_MONO16, ch->buffers[i].Data, STREAMING_BUFFER_SIZE, 22050);
			
					// Queue AL Buffer on Source
					alSourceQueueBuffers(s_channels[source].alSource, 1, &(ch->buffers[i].BufferID));
					if (alGetError() == AL_NO_ERROR)
					{
						ch->buffers[i].Status = QUEUED;
					}
				}

				// Clear error state, and check for successful Play call
				alGetError();
				alSourcePlay(s_channels[source].alSource);
				if (alGetError() == AL_NO_ERROR)
					s_channels[source].bPlaying = true;

				// Record start time for Lip-syncing
				s_channels[source].iStartTime = Com_Milliseconds();

				ch->bStreaming = true;

				return;
			}
			else
			{
				// Attach buffer to source
				alSourcei(s_channels[source].alSource, AL_BUFFER, ch->thesfx->Buffer);

				ch->bStreaming = false;

				// Clear error state, and check for successful Play call
				alGetError();
				alSourcePlay(s_channels[source].alSource);
				if (alGetError() == AL_NO_ERROR)
					s_channels[source].bPlaying = true;
			}
		}

		UpdateLoopingSounds();

		UpdateRawSamples();

		EAXMorph();
	}
	else
	{
		thisTime = Com_Milliseconds();

		// Updates s_soundtime
		S_GetSoundtime();

		if (s_soundtime == ot) {
			return;
		}
		ot = s_soundtime;

		// clear any sound effects that end before the current time,
		// and start any new sounds
		S_ScanChannelStarts();

		sane = thisTime - lastTime;
		if (sane<11) {
			sane = 11;			// 85hz
		}

		ma = s_mixahead->value * dma.speed;
		op = s_mixPreStep->value + sane*dma.speed*0.01;

		if (op < ma) {
			ma = op;
		}

		// mix ahead of current position
		endtime = s_soundtime + ma;

		// mix to an even submission block size
		endtime = (endtime + dma.submission_chunk-1)
			& ~(dma.submission_chunk-1);

		// never mix more than the complete buffer
		samps = dma.samples >> (dma.channels-1);
		if (endtime - s_soundtime > samps)
			endtime = s_soundtime + samps;



		SNDDMA_BeginPainting ();

		S_PaintChannels (endtime);

		SNDDMA_Submit ();

		lastTime = thisTime;
	}
}

void UpdateSingleShotSounds()
{
	int i, j, k;
	ALint state;
	ALint processed;
	channel_t *ch;

	// Firstly, check if any single-shot sounds have completed, or if they need more data (for streaming Sources),
	// and/or if any of the currently playing (non-Ambient) looping sounds need to be stopped
	ch = s_channels ;
	for (i = 0; i < s_numChannels; i++, ch++)
	{
		ch->bProcessed = false;

		if (s_channels[i].bPlaying)
		{
			if (ch->bLooping)
			{
				// Looping Sound
				if (loopSounds[ch->entnum].active == false)
				{
					alSourceStop(s_channels[i].alSource);

					s_channels[i].bPlaying = false;
					s_channels[i].thesfx = NULL;
					loopSounds[ch->entnum].bPlaying = false;
				}
			}
			else
			{
				// Single-shot
				if (s_channels[i].bStreaming == false)
				{
					alGetSourcei(s_channels[i].alSource, AL_SOURCE_STATE, &state);
					if (state == AL_STOPPED)
					{
						s_channels[i].thesfx = NULL;
						s_channels[i].bPlaying = false;
					}
				}
				else
				{	
					// Process streaming sample
	
					// Procedure :-
					// if more data to play
					//		if any UNQUEUED Buffers
					//			fill them with data
					//		(else ?)
					//			get number of buffers processed
					//			fill them with data
					//		restart playback if it has stopped (buffer underrun)
					// else
					//		free channel
					
					int nBytesDecoded;

					if (ch->thesfx->pMP3StreamHeader)
					{
						if (ch->MP3StreamHeader.iSourceBytesRemaining == 0)
						{
							// Finished decoding data - if the source has finished playing then we're done
							alGetSourcei(ch->alSource, AL_SOURCE_STATE, &state);
							if (state == AL_STOPPED)
							{
								// Attach NULL buffer to Source to remove any buffers left in the queue
								alSourcei(ch->alSource, AL_BUFFER, NULL);
								ch->thesfx = NULL;
								ch->bPlaying = false;
							}
							// Move on to next channel ...
							continue;
						}

						// Check to see if any Buffers have been processed
						alGetSourcei(ch->alSource, AL_BUFFERS_PROCESSED, &processed);

						ALuint buffer;
						while (processed)
						{
							alSourceUnqueueBuffers(ch->alSource, 1, &buffer);			
							for (j = 0; j < NUM_STREAMING_BUFFERS; j++)
							{
								if (ch->buffers[j].BufferID == buffer)
								{
									ch->buffers[j].Status = UNQUEUED;
									break;
								}
							}
							processed--;
						}

						int nTotalBytesDecoded = 0;

						for (j = 0; j < NUM_STREAMING_BUFFERS; j++)
						{
							if ((ch->buffers[j].Status == UNQUEUED) & (ch->MP3StreamHeader.iSourceBytesRemaining > 0))
							{
								nTotalBytesDecoded = 0;
			
								for (k = 0; k < (STREAMING_BUFFER_SIZE / 1152); k++)
								{
									nBytesDecoded = C_MP3Stream_Decode(&ch->MP3StreamHeader);
									if (nBytesDecoded > 0)
									{
										memcpy(ch->buffers[j].Data + nTotalBytesDecoded, ch->MP3StreamHeader.bDecodeBuffer, nBytesDecoded);
										nTotalBytesDecoded += nBytesDecoded;
									}
									else
									{
										// Make sure that iSourceBytesRemaining is 0
										if (ch->MP3StreamHeader.iSourceBytesRemaining != 0)
										{
											ch->MP3StreamHeader.iSourceBytesRemaining = 0;
											break;
										}
									}
								}

								if (nTotalBytesDecoded != STREAMING_BUFFER_SIZE)
								{
									memset(ch->buffers[j].Data + nTotalBytesDecoded, 0, (STREAMING_BUFFER_SIZE - nTotalBytesDecoded));

									// Move data to buffer
									alBufferData(ch->buffers[j].BufferID, AL_FORMAT_MONO16, ch->buffers[j].Data, STREAMING_BUFFER_SIZE, 22050);

									// Queue Buffer on Source
									alSourceQueueBuffers(ch->alSource, 1, &(ch->buffers[j].BufferID));

									// Update status of Buffer
									ch->buffers[j].Status = QUEUED;

									break;
								}
								else
								{
									// Move data to buffer
									alBufferData(ch->buffers[j].BufferID, AL_FORMAT_MONO16, ch->buffers[j].Data, STREAMING_BUFFER_SIZE, 22050);

									// Queue Buffer on Source
									alSourceQueueBuffers(ch->alSource, 1, &(ch->buffers[j].BufferID));
									
									// Update status of Buffer
									ch->buffers[j].Status = QUEUED;
								}
							}
						}

						// Get state of Buffer
						alGetSourcei(ch->alSource, AL_SOURCE_STATE, &state);
						if (state != AL_PLAYING)
						{
							alSourcePlay(ch->alSource);
#ifdef _DEBUG
							char szString[256];
							sprintf(szString,"[%d] Restarting playback of single-shot streaming MP3 sample - still have %d bytes to decode\n", i, ch->MP3StreamHeader.iSourceBytesRemaining);
							OutputDebugString(szString);
#endif
						}
					}
				}
			}
		}
	}		
}




void UpdateLoopingSounds()
{
	int i;
	ALuint source;
	channel_t *ch;
	loopSound_t	*loop;
	float pos[3];
	float fVolume = 0.003922;	// 1.f / 255.f

#ifdef _DEBUG
	// Clear AL Error State
	alGetError();
#endif

	for ( i = 0 ; i < MAX_GENTITIES ; i++) 
	{
		loop = &loopSounds[i];

		if ((loop->bPlaying)|(!loop->active))
			continue;

 		ch = S_PickChannel(i, CHAN_AUTO);

		// Play sound on channel
		ch->master_vol = 255;
		ch->entnum = i;
		ch->thesfx = loop->sfx;
		ch->entchannel = CHAN_AUTO;

		ch->fixed_origin = qfalse;
		ch->origin[0] = 0.f;
		ch->origin[1] = 0.f;
		ch->origin[2] = 0.f;

		ch->bLooping = true;
			
		source = ch - s_channels;
		alSourcei(s_channels[source].alSource, AL_BUFFER, ch->thesfx->Buffer);

		if (ch->entnum == listener_number)
		{
			// Make Source Head Relative and set position to 0,0,0 (on top of the listener)
			alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcefv(s_channels[source].alSource, AL_POSITION, ch->origin);
		}
		else
		{
			pos[0] = loop->origin[0];
			pos[1] = loop->origin[2];
			pos[2] = -loop->origin[1];
			alSourcefv(s_channels[source].alSource, AL_POSITION, pos);
			alSourcei(s_channels[source].alSource, AL_SOURCE_RELATIVE, AL_FALSE);
		}

		alSourcei(s_channels[source].alSource, AL_LOOPING, AL_TRUE);
		alSourcef(s_channels[source].alSource, AL_GAIN, (float)(ch->master_vol) * s_volume->value * fVolume);
		
		if (s_bEALFileLoaded)
			UpdateEAXBuffer(ch);

		alGetError();
		alSourcePlay(s_channels[source].alSource);
		if (alGetError() == AL_NO_ERROR)
		{
			ch->bPlaying = true;
			loop->bPlaying = true;
		}
	}
}

void UpdateRawSamples()
{
	ALuint buffer;
	ALint size;
	ALint processed;
	ALint state;
	int i,j,src;


#ifdef _DEBUG
	char szString[256];
	// Clear Open AL Error
	alGetError();
#endif

	S_UpdateBackgroundTrack();

	// Find out how many buffers have been processed (played) by the Source
	alGetSourcei(s_channels[0].alSource, AL_BUFFERS_PROCESSED, &processed);

	while (processed)
	{
		// Unqueue each buffer, determine the length of the buffer, and then delete it
		alSourceUnqueueBuffers(s_channels[0].alSource, 1, &buffer);
		alGetBufferi(buffer, AL_SIZE, &size);
		alDeleteBuffers(1, &buffer);
		
		// Update sg.soundtime (+= number of samples played (number of bytes / 4))
		s_soundtime += (size >> 2);

		processed--;
	}

//	S_UpdateBackgroundTrack();

	// Add new data to a new Buffer and queue it on the Source
	if (s_rawend > s_paintedtime)
	{
		size = (s_rawend - s_paintedtime)<<2;
		if (size > (MAX_RAW_SAMPLES<<2))
		{
			OutputDebugString("UpdateRawSamples :- Raw Sample buffer has overflowed !!!\n");
//			s_rawend = s_paintedtime + MAX_RAW_SAMPLES;
//			size = MAX_RAW_SAMPLES<<2;
			size = MAX_RAW_SAMPLES<<2;
			s_paintedtime = s_rawend - MAX_RAW_SAMPLES;
		}

		// Copy samples from RawSamples to audio buffer (sg.rawdata)
		for (i = s_paintedtime, j = 0; i < s_rawend; i++, j+=2)
		{
			src = i & (MAX_RAW_SAMPLES - 1);
			s_rawdata[j] = (short)(s_rawsamples[src].left>>8);
			s_rawdata[j+1] = (short)(s_rawsamples[src].right>>8);
		}

		// Need to generate more than 1 buffer for music playback
		// iterations = 0;
		// largestBufferSize = (MAX_RAW_SAMPLES / 4) * 4
		// while (size)
		//	generate a buffer
		//	if size > largestBufferSize
		//		copy sg.rawdata + ((iterations * largestBufferSize)>>1) to buffer
		//		size -= largestBufferSize
		//	else
		//		copy remainder
		//		size = 0
		//	queue the buffer
		//  iterations++;

		int iterations = 0;
		int largestBufferSize = MAX_RAW_SAMPLES;	// in bytes (== quarter of Raw Samples data)
		while (size)
		{
			alGenBuffers(1, &buffer);

			if (size > largestBufferSize)
			{
				alBufferData(buffer, AL_FORMAT_STEREO16, (char*)(s_rawdata + ((iterations * largestBufferSize)>>1)), largestBufferSize, 22050);
				size -= largestBufferSize;
			}
			else
			{
				alBufferData(buffer, AL_FORMAT_STEREO16, (char*)(s_rawdata + ((iterations * largestBufferSize)>>1)), size, 22050);
				size = 0;
			}

			alSourceQueueBuffers(s_channels[0].alSource, 1, &buffer);
			iterations++;
		}

		// Update paintedtime
		s_paintedtime = s_rawend;
		
		// Check that the Source is actually playing
		alGetSourcei(s_channels[0].alSource, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING)
		{
			// Stopped playing ... due to buffer underrun
			// Unqueue any buffers still on the Source (they will be PROCESSED), and restart playback
			alGetSourcei(s_channels[0].alSource, AL_BUFFERS_PROCESSED, &processed);
#ifdef _DEBUG
			sprintf(szString, "RawSamples Source stopped with %d buffer processed\n", processed);
			OutputDebugString(szString);
#endif
			while (processed)
			{
				alSourceUnqueueBuffers(s_channels[0].alSource, 1, &buffer);
				processed--;
				alGetBufferi(buffer, AL_SIZE, &size);
				alDeleteBuffers(1, &buffer);
		
				// Update sg.soundtime (+= number of samples played (number of bytes / 4))
				s_soundtime += (size >> 2);
			}

#ifdef _DEBUG
			OutputDebugString("Restarting / Starting playback of Raw Samples\n");
#endif

			alSourcePlay(s_channels[0].alSource);
		}
	}

#ifdef _DEBUG
	if (alGetError() != AL_NO_ERROR)
		OutputDebugString("OAL Error : UpdateRawSamples\n");
#endif
}

/*
===============================================================================

console functions

===============================================================================
*/

void S_Play_f( void ) {
	int 		i;
	sfxHandle_t	h;
	char		name[256];
	
	i = 1;
	while ( i<Cmd_Argc() ) {
		if ( !Q_strrchr(Cmd_Argv(i), '.') ) {
			Com_sprintf( name, sizeof(name), "%s.wav", Cmd_Argv(1) );
		} else {
			Q_strncpyz( name, Cmd_Argv(i), sizeof(name) );
		}
		h = S_RegisterSound( name );
		if( h ) {
			S_StartLocalSound( h, CHAN_LOCAL_SOUND );
		}
		i++;
	}
}

static void S_Music_f( void ) {
	int		c;

	c = Cmd_Argc();

	if ( c == 2 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(1), qfalse );
	} else if ( c == 3 ) {
		S_StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(2), qfalse );		
	} else {
		Com_Printf ("music <musicfile> [loopfile]\n");
		return;
	}
}


// this table needs to be in-sync with the typedef'd enum "SoundCompressionMethod_t"...	-ste
//
static const char *sSoundCompressionMethodStrings[ct_NUMBEROF] = 
{
	"16b",	// ct_16
	"mp3"	// ct_MP3
};
void S_SoundList_f( void ) {
	int		i;
	sfx_t	*sfx;
	int		size, total;

	total = 0;

	Com_Printf("\n");
	Com_Printf("                    InMemory?\n");
	Com_Printf("                    |\n");
	Com_Printf("                    |  LevelLastUsedOn\n");
	Com_Printf("                    |  |\n");
	Com_Printf("                    |  |\n");
	Com_Printf(" Slot   Bytes Type  |  |   Name\n");
//	Com_Printf(" Slot   Bytes Type  InMem?   Name\n");

	for (sfx=s_knownSfx, i=0 ; i<s_numSfx ; i++, sfx++) 
	{
		size = sfx->iSoundLengthInSamples;
		total += sfx->bInMemory ? size : 0;
		Com_Printf("%5d %7i [%s] %s %2d %s\n", i, size, sSoundCompressionMethodStrings[sfx->eSoundCompressionMethod], sfx->bInMemory?"y":"n", sfx->iLastLevelUsedOn, sfx->sSoundName );
	}
	Com_Printf ("Total resident samples: %i  ( not mem usage, see 'meminfo' ).\n", total);
	Com_Printf ("%d out of %d sfx_t slots used\n", s_numSfx, MAX_SFX);
	S_DisplayFreeMemory();
}


/*
===============================================================================

background music functions

===============================================================================
*/

int	FGetLittleLong( fileHandle_t f ) {
	int		v;

	FS_Read( &v, sizeof(v), f );

	return LittleLong( v);
}

int	FGetLittleShort( fileHandle_t f ) {
	short	v;

	FS_Read( &v, sizeof(v), f );

	return LittleShort( v);
}

// returns the length of the data in the chunk, or 0 if not found
int S_FindWavChunk( fileHandle_t f, char *chunk ) {
	char	name[5];
	int		len;
	int		r;

	name[4] = 0;
	len = 0;
	r = FS_Read( name, 4, f );
	if ( r != 4 ) {
		return 0;
	}
	len = FGetLittleLong( f );
	if ( len < 0 || len > 0xfffffff ) {
		len = 0;
		return 0;
	}
	len = (len + 1 ) & ~1;		// pad to word boundary
//	s_nextWavChunk += len + 8;

	if ( strcmp( name, chunk ) ) {
		return 0;
	}

	return len;
}

// fixme: need to move this into qcommon sometime?, but too much stuff altered by other people and I won't be able
//	to compile again for ages if I check that out...
//
// DO NOT replace this with a call to FS_FileExists, that's for checking about writing out, and doesn't work for this.
//
qboolean S_FileExists( const char *psFilename )
{
	fileHandle_t fhTemp;

	FS_FOpenFileRead (psFilename, &fhTemp, qtrue);	// qtrue so I can fclose the handle without closing a PAK
	if (!fhTemp) 
		return qfalse;
	
	FS_FCloseFile(fhTemp);
	return qtrue;
}

// some stuff for streaming MP3 files from disk (not pleasant, but nothing about MP3 is, other than compression ratios...)
//
static void MP3MusicStream_Reset(MusicInfo_t *pMusicInfo)
{
	pMusicInfo->iMP3MusicStream_DiskReadPos		= 0;
	pMusicInfo->iMP3MusicStream_DiskWindowPos	= 0;
}

//
// return is where the decoder should read from...
//
static byte *MP3MusicStream_ReadFromDisk(MusicInfo_t *pMusicInfo, int iReadOffset, int iReadBytesNeeded)
{
	if (iReadOffset < pMusicInfo->iMP3MusicStream_DiskWindowPos)
	{
		assert(0);											// should never happen
		return pMusicInfo->byMP3MusicStream_DiskBuffer;		// ...but return something safe anyway
	}

	while (iReadOffset + iReadBytesNeeded > pMusicInfo->iMP3MusicStream_DiskReadPos)
	{
		int iBytesRead = FS_Read( pMusicInfo->byMP3MusicStream_DiskBuffer + (pMusicInfo->iMP3MusicStream_DiskReadPos - pMusicInfo->iMP3MusicStream_DiskWindowPos), iMP3MusicStream_DiskBytesToRead, pMusicInfo->s_backgroundFile );

		pMusicInfo->iMP3MusicStream_DiskReadPos += iBytesRead;

		if (iBytesRead != iMP3MusicStream_DiskBytesToRead)	// quietly ignore any requests to read past file end
		{
			break;		// we need to do this because the disk read code can't know how much source data we need to
						//	read for a given number of requested output bytes, so we'll always be asking for too many
		}
	}

	// if reached halfway point in buffer (approx 20k), backscroll it...
	//
	if (pMusicInfo->iMP3MusicStream_DiskReadPos - pMusicInfo->iMP3MusicStream_DiskWindowPos > iMP3MusicStream_DiskBufferSize/2)
	{
		int iMoveSrcOffset = iReadOffset - pMusicInfo->iMP3MusicStream_DiskWindowPos;
		int iMoveCount     = (pMusicInfo->iMP3MusicStream_DiskReadPos - pMusicInfo->iMP3MusicStream_DiskWindowPos ) - iMoveSrcOffset;
		memmove( &pMusicInfo->byMP3MusicStream_DiskBuffer, &pMusicInfo->byMP3MusicStream_DiskBuffer[iMoveSrcOffset], iMoveCount);
		pMusicInfo->iMP3MusicStream_DiskWindowPos += iMoveSrcOffset;
	}

	return pMusicInfo->byMP3MusicStream_DiskBuffer + (iReadOffset - pMusicInfo->iMP3MusicStream_DiskWindowPos);
}


// does NOT set s_rawend!...
//
static void S_StopBackgroundTrack_Actual( MusicInfo_t *pMusicInfo ) 
{
	if ( pMusicInfo->s_backgroundFile ) 
	{
		if ( pMusicInfo->s_backgroundFile != -1)
		{
			Sys_EndStreamedFile( pMusicInfo->s_backgroundFile );
			FS_FCloseFile( pMusicInfo->s_backgroundFile );
		}
		pMusicInfo->s_backgroundFile = 0;	
	}
}

static void FreeMusic( MusicInfo_t *pMusicInfo )
{
	if (pMusicInfo->pLoadedData)
	{
		Z_Free(pMusicInfo->pLoadedData);
		pMusicInfo->pLoadedData		= NULL;
		pMusicInfo->iLoadedDataLen	= 0;
		pMusicInfo->sLoadedDataName[0]= '\0';
	}
}

// called only by snd_restart
//
void S_UnCacheDynamicMusic( void )
{
	FreeMusic( &tMusic_Info[eBGRNDTRACK_SLOW] );
}

static void S_StartBackgroundTrack_Actual( MusicInfo_t *pMusicInfo, const char *intro, const char *loop )
{
	int		len;
	char	dump[16];
	char	name[MAX_QPATH];

	Q_strncpyz( sMusic_BackgroundLoop, loop, sizeof( sMusic_BackgroundLoop ));	

	Q_strncpyz( name, intro, sizeof( name ) - 4 );	// this seems to be so that if the filename hasn't got an extension
													//	but doesn't have the room to append on either then you'll just
													//	get the "soft" fopen() error, rather than the ERR_DROP you'd get
													//	if COM_DefaultExtension didn't have room to add it on.
	COM_DefaultExtension( name, sizeof( name ), ".wav" );

	// close the background track, but DON'T reset s_rawend (or remaining music bits that haven't been output yet will be cut off)
	//
#if 0
/*	if ( pMusicInfo->s_backgroundFile ) {
		Sys_EndStreamedFile( pMusicInfo->s_backgroundFile );
		FS_FCloseFile( pMusicInfo->s_backgroundFile );
		pMusicInfo->s_backgroundFile = 0;
	}
*/
#else
	S_StopBackgroundTrack_Actual( pMusicInfo );
#endif

	pMusicInfo->bIsMP3 = qfalse;

	if ( !intro[0] ) {
		return;
	}


	// new bit, if file requested is not same any loaded one (if prev was in-mem), ditch it...
	//
	if (Q_stricmp(name, pMusicInfo->sLoadedDataName))
	{
		FreeMusic( pMusicInfo );
	}

	if (!Q_stricmpn(name+(strlen(name)-4),".mp3",4))
	{
		int iMP3Filelen = FS_FOpenFileRead( name, &pMusicInfo->s_backgroundFile, qtrue );
		if (!pMusicInfo->s_backgroundFile)
		{
			Com_Printf( S_COLOR_RED"Couldn't open music file %s\n", name );
			return;
		}

		MP3MusicStream_Reset( pMusicInfo );

		byte *pbMP3DataSegment	= NULL;
		int iInitialMP3ReadSize = 8192;		// fairly arbitrary, whatever size this is then the decoder is allowed to
											// scan up to halfway of it to find floating headers, so don't make it 
											// too small. 8k works fine.

		pbMP3DataSegment = MP3MusicStream_ReadFromDisk(pMusicInfo, 0, iInitialMP3ReadSize);

		if (MP3_IsValid(name, pbMP3DataSegment, iInitialMP3ReadSize, qtrue /*bStereoDesired*/))
		{
			// init stream struct...
			//
			memset(&pMusicInfo->streamMP3_Bgrnd,0,sizeof(pMusicInfo->streamMP3_Bgrnd));
			char *psError = C_MP3Stream_DecodeInit( &pMusicInfo->streamMP3_Bgrnd, pbMP3DataSegment, iMP3Filelen,
													dma.speed,
													16,		// sfx->width * 8,
													qtrue	// bStereoDesired
													);


			if (psError == NULL)
			{
				// init sfx struct & setup the few fields I actually need...
				//
				memset(	   &pMusicInfo->sfxMP3_Bgrnd,0,sizeof(pMusicInfo->sfxMP3_Bgrnd));
				//			pMusicInfo->sfxMP3_Bgrnd.width					= 2;			// read by MP3_GetSamples()
							pMusicInfo->sfxMP3_Bgrnd.iSoundLengthInSamples	= 0x7FFFFFFF;	// max possible +ve int, since music finishes when decoder stops
							pMusicInfo->sfxMP3_Bgrnd.pMP3StreamHeader		= &pMusicInfo->streamMP3_Bgrnd;
				Q_strncpyz( pMusicInfo->sfxMP3_Bgrnd.sSoundName, name, sizeof(pMusicInfo->sfxMP3_Bgrnd.sSoundName) );

				pMusicInfo->s_backgroundInfo.format		= WAV_FORMAT_MP3;	// not actually used this way, but just ensures we don't match one of the legit formats
				pMusicInfo->s_backgroundInfo.channels	= 2;		// always, for our MP3s when used for music (else 1 for FX)
				pMusicInfo->s_backgroundInfo.rate		= dma.speed;
				pMusicInfo->s_backgroundInfo.width		= 2;		// always, for our MP3s
				pMusicInfo->s_backgroundInfo.samples	= pMusicInfo->sfxMP3_Bgrnd.iSoundLengthInSamples;
				pMusicInfo->s_backgroundSamples			= pMusicInfo->sfxMP3_Bgrnd.iSoundLengthInSamples;

				memset(&pMusicInfo->chMP3_Bgrnd,0,sizeof(pMusicInfo->chMP3_Bgrnd));
						pMusicInfo->chMP3_Bgrnd.thesfx = &pMusicInfo->sfxMP3_Bgrnd;
				memcpy(&pMusicInfo->chMP3_Bgrnd.MP3StreamHeader, pMusicInfo->sfxMP3_Bgrnd.pMP3StreamHeader, sizeof(*pMusicInfo->sfxMP3_Bgrnd.pMP3StreamHeader));				

				pMusicInfo->bIsMP3 = qtrue;
			}
			else
			{
				Com_Printf(S_COLOR_RED"Error streaming file %s: %s\n", name, psError);
				FS_FCloseFile( pMusicInfo->s_backgroundFile );
				pMusicInfo->s_backgroundFile = 0;
			}
		}
		else
		{
			// MP3_IsValid() will already have printed any errors via Com_Printf at this point...
			//			
			FS_FCloseFile( pMusicInfo->s_backgroundFile );
			pMusicInfo->s_backgroundFile = 0;
		}
		
		return;
	}
	else	// not an mp3 file
	{
		//
		// open up a wav file and get all the info
		//
		FS_FOpenFileRead( name, &pMusicInfo->s_backgroundFile, qtrue );
		if ( !pMusicInfo->s_backgroundFile ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: couldn't open music file %s\n", name );
			return;
		}

		// skip the riff wav header

		FS_Read(dump, 12, pMusicInfo->s_backgroundFile);

		if ( !S_FindWavChunk( pMusicInfo->s_backgroundFile, "fmt " ) ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: No fmt chunk in %s\n", name );
			FS_FCloseFile( pMusicInfo->s_backgroundFile );
			pMusicInfo->s_backgroundFile = 0;
			return;
		}

		// save name for soundinfo
		pMusicInfo->s_backgroundInfo.format = FGetLittleShort( pMusicInfo->s_backgroundFile );
		pMusicInfo->s_backgroundInfo.channels = FGetLittleShort( pMusicInfo->s_backgroundFile );
		pMusicInfo->s_backgroundInfo.rate = FGetLittleLong( pMusicInfo->s_backgroundFile );
		FGetLittleLong(  pMusicInfo->s_backgroundFile );
		FGetLittleShort(  pMusicInfo->s_backgroundFile );
		pMusicInfo->s_backgroundInfo.width = FGetLittleShort( pMusicInfo->s_backgroundFile ) / 8;

		if ( pMusicInfo->s_backgroundInfo.format != WAV_FORMAT_PCM ) {
			FS_FCloseFile( pMusicInfo->s_backgroundFile );
			pMusicInfo->s_backgroundFile = 0;
			Com_Printf(S_COLOR_YELLOW "WARNING: Not a microsoft PCM format wav: %s\n", name);
			return;
		}

		if ( pMusicInfo->s_backgroundInfo.channels != 2 || pMusicInfo->s_backgroundInfo.rate != 22050 ) {
			Com_Printf(S_COLOR_YELLOW "WARNING: music file %s is not 22k stereo\n", name );
		}

		if ( ( len = S_FindWavChunk( pMusicInfo->s_backgroundFile, "data" ) ) == 0 ) {
			FS_FCloseFile( pMusicInfo->s_backgroundFile );
			pMusicInfo->s_backgroundFile = 0;
			Com_Printf(S_COLOR_YELLOW "WARNING: No data chunk in %s\n", name);
			return;
		}

		pMusicInfo->s_backgroundInfo.samples = len / (pMusicInfo->s_backgroundInfo.width * pMusicInfo->s_backgroundInfo.channels);

		pMusicInfo->s_backgroundSamples = pMusicInfo->s_backgroundInfo.samples;

		//
		// start the background streaming
		//
		Sys_BeginStreamedFile( pMusicInfo->s_backgroundFile, 0x10000 );
	}
}



static char gsIntroMusic[MAX_QPATH]={0};
static char gsLoopMusic [MAX_QPATH]={0};

void S_RestartMusic( void ) 
{
	if (s_soundStarted && !s_soundMuted )
	{
		if (gsIntroMusic[0] || gsLoopMusic[0])
		{				
			S_StartBackgroundTrack( gsIntroMusic, gsLoopMusic, qfalse );	// ( default music start will set the state to SLOW )			
		}
	}
}

// Basic logic here is to see if the intro file specified actually exists, and if so, then it's not dynamic music,
//	but if it doesn't, and it DOES exist by strcat()ing "_fast" and "_slow" (.mp3) onto 2 copies of it, then it's dynamic.
//
// In either case, open it if it exits, and just set some vars that'll inhibit dynamicness if needed.
//
void S_StartBackgroundTrack( const char *intro, const char *loop, qboolean bReturnWithoutStarting )
{
	if ( !intro ) {
		intro = "";
	}
	if ( !loop || !loop[0] ) {
		loop = intro;
	}

	Q_strncpyz(gsIntroMusic,intro, sizeof(gsIntroMusic));
	Q_strncpyz(gsLoopMusic, loop,  sizeof(gsLoopMusic));

	// do NOT start music from cgame now, the functions Com_TouchMemory() and RegisterMedia_LevelLoadEnd()
	//	can take some time to execute (after music start), and can stutter it. So now, RegisterMedia_LevelLoadEnd()
	//	will call S_RestartMusic now that the strings are set...
	//
	if ( bReturnWithoutStarting )
		return;	

	char sName[MAX_QPATH];
	Q_strncpyz(sName,intro,sizeof(sName));
	COM_DefaultExtension( sName, sizeof( sName ), ".mp3" );

	// conceptually we always play the 'intro'[/sName] track, intro-to-loop transition is handled in UpdateBackGroundTrack().
	//
	if (S_FileExists( sName ))
	{		
		Com_DPrintf("S_StartBackgroundTrack: Found/using music track '%s'\n", sName);
		S_StartBackgroundTrack_Actual( &tMusic_Info[eBGRNDTRACK_SLOW], sName, loop );
	}
	else
	{
		Com_Printf( S_COLOR_RED "ERROR: Unable to find music file:\n( %s )\n", sName );
		S_StopBackgroundTrack();
	}	
}

void S_StopBackgroundTrack( void )
{
	S_StopBackgroundTrack_Actual( &tMusic_Info[eBGRNDTRACK_SLOW] );

	s_rawend = 0;
}



// qboolean return is true only if we're changing from a streamed intro to a dynamic loop...
//
static qboolean S_UpdateBackgroundTrack_Actual( MusicInfo_t *pMusicInfo ) 
{
	int		bufferSamples;
	int		fileSamples;
	byte	raw[30000];		// just enough to fit in a mac stack frame  (note that MP3 doesn't use full size of it)
	int		fileBytes;
	int		r;

	float fMasterVol = (s_musicVolume->value*s_musicMult->value);

	static	float	musicVolume = 0.25f;

// this is to work around an obscure issue to do with sliding decoder windows and amounts being requested, since the
//	original MP3 stream-decoder wrapper was designed to work with audio-paintbuffer sized pieces... Basically 30000
//	is far too big for the window decoder to handle in one request because of the time-travel issue associated with
//	normal sfx buffer painting, and allowing sufficient sliding room, even though the music file never goes back in time.
//
#define SIZEOF_RAW_BUFFER_FOR_MP3 4096
#define RAWSIZE (pMusicInfo->bIsMP3?SIZEOF_RAW_BUFFER_FOR_MP3:sizeof(raw))

	if ( !pMusicInfo->s_backgroundFile ) {
		return qfalse;
	}	

	musicVolume = (musicVolume + fMasterVol)/2.0f;

	// don't bother playing anything if musicvolume is 0
	if ( musicVolume <= 0 ) {
		return qfalse;
	}

	// see how many samples should be copied into the raw buffer
	if ( s_rawend < s_soundtime ) {
		s_rawend = s_soundtime;
	}

	while ( s_rawend < s_soundtime + MAX_RAW_SAMPLES ) 
	{
		bufferSamples = MAX_RAW_SAMPLES - (s_rawend - s_soundtime);

		// decide how much data needs to be read from the file
		fileSamples = bufferSamples * pMusicInfo->s_backgroundInfo.rate / dma.speed;

		// don't try and read past the end of the file
		if ( fileSamples > pMusicInfo->s_backgroundSamples ) {
			fileSamples = pMusicInfo->s_backgroundSamples;
		}

		// our max buffer size
		fileBytes = fileSamples * (pMusicInfo->s_backgroundInfo.width * pMusicInfo->s_backgroundInfo.channels);
		if (fileBytes > RAWSIZE ) {
			fileBytes = RAWSIZE;
			fileSamples = fileBytes / (pMusicInfo->s_backgroundInfo.width * pMusicInfo->s_backgroundInfo.channels);
		}

		qboolean qbForceFinish = qfalse;
		if (pMusicInfo->bIsMP3)
		{
			int iStartingSampleNum = pMusicInfo->chMP3_Bgrnd.thesfx->iSoundLengthInSamples - pMusicInfo->s_backgroundSamples;	// but this IS relevant
			// Com_Printf(S_COLOR_YELLOW "Requesting MP3 samples: sample %d\n",iStartingSampleNum);


			if (pMusicInfo->s_backgroundFile == -1)
			{
				// in-mem...
				//
				qbForceFinish = (MP3Stream_GetSamples( &pMusicInfo->chMP3_Bgrnd, iStartingSampleNum, fileBytes/2, (short*) raw, qtrue ))?qfalse:qtrue;				

				//Com_Printf(S_COLOR_YELLOW "Music time remaining: %f seconds\n", MP3Stream_GetRemainingTimeInSeconds( &pMusicInfo->chMP3_Bgrnd.MP3StreamHeader ));				
			}
			else
			{
				// streaming an MP3 file instead... (note that the 'fileBytes' request size isn't that relevant for MP3s, 
				//										since code here can't know how much the MP3 needs to decompress)
				//
				byte *pbScrolledStreamData = MP3MusicStream_ReadFromDisk(pMusicInfo, pMusicInfo->chMP3_Bgrnd.MP3StreamHeader.iSourceReadIndex, fileBytes);

				pMusicInfo->chMP3_Bgrnd.MP3StreamHeader.pbSourceData = pbScrolledStreamData - pMusicInfo->chMP3_Bgrnd.MP3StreamHeader.iSourceReadIndex;

				qbForceFinish = (MP3Stream_GetSamples( &pMusicInfo->chMP3_Bgrnd, iStartingSampleNum, fileBytes/2, (short*) raw, qtrue ))?qfalse:qtrue;
			}
		}
		else
		{
			// streaming a WAV off disk...
			//
			r = Sys_StreamedRead( raw, 1, fileBytes, pMusicInfo->s_backgroundFile );
			if ( r != fileBytes ) {
				Com_Printf(S_COLOR_RED"StreamedRead failure on music track\n");
				S_StopBackgroundTrack();
				return qfalse;
			}

			// byte swap if needed (do NOT do for MP3 decoder, that has an internal big/little endian handler)
			//
			S_ByteSwapRawSamples( fileSamples, pMusicInfo->s_backgroundInfo.width, pMusicInfo->s_backgroundInfo.channels, raw );
		}

		// add to raw buffer
		S_RawSamples(	fileSamples, pMusicInfo->s_backgroundInfo.rate, 
						pMusicInfo->s_backgroundInfo.width, pMusicInfo->s_backgroundInfo.channels, raw, musicVolume						
					);

		pMusicInfo->s_backgroundSamples -= fileSamples;
		if ( !pMusicInfo->s_backgroundSamples || qbForceFinish ) 
		{
			// loop the music, or play the next piece if we were on the intro...
			//
			//	(but not for dynamic, that can only be used for loop music)
			//
			// for non-dynamic music we need to check if "sMusic_BackgroundLoop" is an actual filename,
			//	or if it's a dynamic music specifier (which can't literally exist), in which case it should set
			//	a return flag then exit...
			//
			char sTestName[MAX_QPATH*2];// *2 so COM_DefaultExtension doesn't do an ERR_DROP if there was no space
										//	for an extension, since this is a "soft" test				
			Q_strncpyz( sTestName, sMusic_BackgroundLoop, sizeof(sTestName));
			COM_DefaultExtension(sTestName, sizeof(sTestName), ".wav");

			if (S_FileExists( sTestName ))
			{
				S_StartBackgroundTrack_Actual( pMusicInfo, sMusic_BackgroundLoop, sMusic_BackgroundLoop );
			}
			else
			{
				// proposed file doesn't exist, but this may be a dynamic track we're wanting to loop, 
				//	so exit with a special flag...
				//
				return qtrue;
			}

			if ( !pMusicInfo->s_backgroundFile ) 
			{
				return qfalse;		// loop failed to restart
			}
		}
	}

#undef SIZEOF_RAW_BUFFER_FOR_MP3
#undef RAWSIZE

	return qfalse;
}


static void S_UpdateBackgroundTrack( void )
{
	qboolean bNewTrackDesired = S_UpdateBackgroundTrack_Actual(&tMusic_Info[eBGRNDTRACK_SLOW]);

	if (bNewTrackDesired)
	{
		S_StartBackgroundTrack( sMusic_BackgroundLoop, sMusic_BackgroundLoop, qfalse );
	}
}


// currently passing in sfx as a param in case I want to do something with it later.
//
byte *SND_malloc(int iSize, sfx_t *sfx) 
{
	byte *pData = (byte *) Z_Malloc(iSize, TAG_SND_RAWDATA);	// don't bother asking for zeroed mem
	return pData;
}

cvar_t *s_soundpoolmegs = NULL;

// called once-only in EXE lifetime...
//
void SND_setup() 
{		 
	s_soundpoolmegs = Cvar_Get("s_soundpoolmegs", "25", CVAR_ARCHIVE);
	if (Sys_LowPhysicalMemory() )
	{
		Cvar_Set("s_soundpoolmegs", "0");
	}

	Com_Printf("Sound memory manager started\n");
}

// ask how much mem an sfx has allocated...
//
static int SND_MemUsed(sfx_t *sfx)
{
	int iSize = 0;
	if (sfx->pSoundData){
		iSize += Z_Size(sfx->pSoundData);
	}

	if (sfx->pMP3StreamHeader) {
		iSize += Z_Size(sfx->pMP3StreamHeader);
	}

	return iSize;
}

// free any allocated sfx mem...
//
// now returns # bytes freed to help with z_malloc()-fail recovery
//
static int SND_FreeSFXMem(sfx_t *sfx)
{
	int iBytesFreed = 0;

	if (						sfx->pSoundData) {
		iBytesFreed +=	Z_Size(	sfx->pSoundData);
						Z_Free(	sfx->pSoundData );
								sfx->pSoundData = NULL;
	}

	sfx->bInMemory = qfalse;	

	if (						sfx->pMP3StreamHeader) {
		iBytesFreed +=	Z_Size(	sfx->pMP3StreamHeader);
						Z_Free(	sfx->pMP3StreamHeader );
								sfx->pMP3StreamHeader = NULL;
	}

	return iBytesFreed;
}



void S_DisplayFreeMemory() 
{
//	Com_Printf("%.2fMB total sound bytes, %.2fMB used this map\n",
//				(float)iSNDBytes_Total		/1024.0f/1024.0f, 
//				(float)iSNDBytes_ThisMap	/1024.0f/1024.0f
//				);
	int iSoundDataSize = Z_MemSize ( TAG_SND_RAWDATA ) + Z_MemSize( TAG_SND_MP3STREAMHDR );
	int iMusicDataSize = Z_MemSize ( TAG_SND_DYNAMICMUSIC );

	if (iSoundDataSize || iMusicDataSize)
	{
		Com_Printf("\n%.2fMB audio data:  ( %.2fMB WAV/MP3 ) + ( %.2fMB Music )\n",
					((float)(iSoundDataSize+iMusicDataSize))/1024.0f/1024.0f,
										((float)(iSoundDataSize))/1024.0f/1024.0f,
																((float)(iMusicDataSize))/1024.0f/1024.0f
					);

		// now count up amount used on this level...
		//
		iSoundDataSize = 0;
		for (int i=1; i<s_numSfx; i++)
		{
			sfx_t *sfx = &s_knownSfx[i];

			if (sfx->iLastLevelUsedOn == RE_RegisterMedia_GetLevel()){
				iSoundDataSize += SND_MemUsed(sfx);
			}
		}

		Com_Printf("%.2fMB in sfx_t alloc data (WAV/MP3) loaded this level\n",(float)iSoundDataSize/1024.0f/1024.0f);
	}
}

void SND_TouchSFX(sfx_t *sfx)
{
	sfx->iLastTimeUsed		= Com_Milliseconds();	// ditch this field sometime?
	sfx->iLastLevelUsedOn	= RE_RegisterMedia_GetLevel();
}


// currently this is only called during snd_shutdown or snd_restart
//
void S_FreeAllSFXMem(void)
{
	for (int i=1 ; i < s_numSfx ; i++)	// start @ 1 to skip freeing default sound
	{
		SND_FreeSFXMem(&s_knownSfx[i]);
	}
}

// returns number of bytes freed up...
//
int SND_FreeOldestSound() 
{	
	int iBytesFreed = 0;
	sfx_t *sfx;

	int	iOldest = Com_Milliseconds();
	int	iUsed	= 0;

	// start on 1 so we never dump the default sound...
	//
	for (int i=1 ; i < s_numSfx ; i++) 
	{
		sfx = &s_knownSfx[i];

		if (!sfx->bDefaultSound && sfx->bInMemory && sfx->iLastTimeUsed < iOldest)
		{
			if (sfx->pSoundData)
			{
				iUsed = i;
				iOldest = sfx->iLastTimeUsed;
			}
			else
			{
				assert(0);	// shouldn't need this, but I'm sure I saw one once then had bInMemory but !pSoundData.... ?!?!?
				sfx->bInMemory = qfalse;
			}
		}
	}

	if (iUsed)
	{
		sfx = &s_knownSfx[ iUsed ];
	
		Com_DPrintf("SND_FreeOldestSound: freeing sound %s\n", sfx->sSoundName);
	
		iBytesFreed = SND_FreeSFXMem(sfx);
	}

	return iBytesFreed;
}


// just before we drop into a level, ensure the audio pool is under whatever the maximum
//	pool size is (but not by dropping out sounds used by the current level)...
//
// returns qtrue if at least one sound was dropped out, so z_malloc-fail recovery code knows if anything changed
//
extern qboolean gbInsideLoadSound;
qboolean SND_RegisterAudio_LevelLoadEnd(qboolean bDeleteEverythingNotUsedThisLevel /* 99% qfalse */)
{
	qboolean bAtLeastOneSoundDropped = qfalse;

	Com_DPrintf( "SND_RegisterAudio_LevelLoadEnd():\n");

	if (gbInsideLoadSound)
	{
		Com_DPrintf( "(Inside S_LoadSound (z_malloc recovery?), exiting...\n");
	}
	else
	{
		int iLoadedAudioBytes	 = Z_MemSize ( TAG_SND_RAWDATA ) + Z_MemSize( TAG_SND_MP3STREAMHDR );
		const int iMaxAudioBytes = s_soundpoolmegs->integer * 1024 * 1024;

		for (int i=1; i<s_numSfx && ( iLoadedAudioBytes > iMaxAudioBytes || bDeleteEverythingNotUsedThisLevel) ; i++) // i=1 so we never page out default sound
		{
			sfx_t *sfx = &s_knownSfx[i];

			if (sfx->bInMemory)
			{
				qboolean bDeleteThis = qfalse;

				if (bDeleteEverythingNotUsedThisLevel)
				{
					bDeleteThis = (sfx->iLastLevelUsedOn != RE_RegisterMedia_GetLevel() ) ? qtrue : qfalse;
				}
				else
				{
					bDeleteThis = (sfx->iLastLevelUsedOn < RE_RegisterMedia_GetLevel() ) ? qtrue : qfalse;
				}

				if (bDeleteThis)
				{
					Com_DPrintf( "Dumping sfx_t \"%s\"\n",sfx->sSoundName);			

					if (SND_FreeSFXMem(sfx))
					{
						bAtLeastOneSoundDropped = qtrue;
					}

					iLoadedAudioBytes = Z_MemSize ( TAG_SND_RAWDATA ) + Z_MemSize( TAG_SND_MP3STREAMHDR );					
				}
			}
		}
	}

	Com_DPrintf( "SND_RegisterAudio_LevelLoadEnd(): Ok\n");	

	return bAtLeastOneSoundDropped;
}

/****************************************************************************************************\
*
*	EAX Related
*
\****************************************************************************************************/

/*
	Initialize the EAX Manager
*/
void InitEAXManager()
{
	LPEAXMANAGERCREATE lpEAXManagerCreateFn;
	HRESULT hr;

	// Check for EAX 3.0 support
	s_bEAX = alIsExtensionPresent((ALubyte*)"EAX3.0");
	if (s_bEAX)
	{
		s_eaxSet = (EAXSet)alGetProcAddress((ALubyte*)"EAXSet");
		if (s_eaxSet == NULL)
			s_bEAX = false;
		s_eaxGet = (EAXGet)alGetProcAddress((ALubyte*)"EAXGet");
		if (s_eaxGet == NULL)
			s_bEAX = false;
	}

	// If we have detected EAX support, then try and load the EAX Manager DLL
	if (s_bEAX)
	{
		s_hEAXManInst = LoadLibrary("EAXMan.dll");
		if (s_hEAXManInst)
		{
			lpEAXManagerCreateFn = (LPEAXMANAGERCREATE)GetProcAddress(s_hEAXManInst, "EaxManagerCreate");
			if (lpEAXManagerCreateFn)
			{
				hr = lpEAXManagerCreateFn(&s_lpEAXManager);
				if (hr == EM_OK)
					return;
			}
		}
	}

	// If the EAXManager library was loaded (and there was a problem), then unload it
	if (s_hEAXManInst)
	{
		FreeLibrary(s_hEAXManInst);
		s_hEAXManInst = NULL;
	}

	s_lpEAXManager = NULL;
	s_bEAX = false;

	return;
}

/*
	Release the EAX Manager
*/
void ReleaseEAXManager()
{
	s_bEAX = false;
	if (s_lpEAXManager)
	{
		s_lpEAXManager->Release();
		s_lpEAXManager = NULL;
	}
	if (s_hEAXManInst)
	{
		FreeLibrary(s_hEAXManInst);
		s_hEAXManInst = NULL;
	}
}


/*
	Try to load the given .eal file
*/
bool LoadEALFile(char *szEALFilename)
{
	char		*ealData = NULL;
	int			result;
	HRESULT		hr;
	char		szFullEALFilename[MAX_QPATH];

	if ((!s_lpEAXManager) || (!s_bEAX))
	{
		return false;
	}

	s_EnvironmentID = 0xFFFFFFFF;

	// Load EAL file from PAK file
	result = FS_ReadFile(szEALFilename, (void **)&ealData);
	if ((ealData) && (result != -1))
	{
		hr = s_lpEAXManager->LoadDataSet(ealData, EMFLAG_LOADFROMMEMORY);
	
		// Unload EAL file
		FS_FreeFile (ealData);
	
		if (hr == EM_OK)
			return true;
	}
	else
	{
		// Failed to load via Quake loader, try manually
		Com_sprintf(szFullEALFilename, MAX_QPATH, "base/%s", szEALFilename);
		hr = s_lpEAXManager->LoadDataSet(szFullEALFilename, 0);
		if (hr == EM_OK)
			return true;
	}

	return false;
}

/*
	Unload current .eal file
*/
void UnloadEALFile()
{
	HRESULT hr;

	if ((!s_lpEAXManager) || (!s_bEAX))
		return;

	hr = s_lpEAXManager->FreeDataSet(0);

	return;
}

/*
	Updates the current EAX Reverb setting, based on the location of the listener
*/
void UpdateEAXListener(bool bUseDefault, bool bUseMorphing)
{
	HRESULT hr;
	EMPOINT EMPoint;
	long lID;

	if ((!s_lpEAXManager) || (!s_bEAX))
		return;

	if (bUseDefault)
	{
		// Get Default EAX Listener attributes
		hr = s_lpEAXManager->GetEnvironmentAttributes(EMFLAG_IDDEFAULT, &s_eaxLPSource);
		if (hr == EM_OK)
		{
			s_eaxLPSource.flAirAbsorptionHF = 0.0f;
			s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
				NULL, &s_eaxLPSource, sizeof(EAXLISTENERPROPERTIES));

			s_eaxLPCur = s_eaxLPSource;
			s_eaxLPDest = s_eaxLPSource;

			s_EnvironmentID = EMFLAG_IDDEFAULT;

			s_eaxMorphStartTime = 0;
			s_eaxMorphCount = 0;

			return;
		}
		return;
	}
	
	// Convert Listener co-ordinate to left-handed system
	EMPoint.fX = listener_pos[0];
	EMPoint.fY = listener_pos[1];
	EMPoint.fZ = -listener_pos[2];

	hr = s_lpEAXManager->GetListenerDynamicAttributes(0, &EMPoint, &lID, EMFLAG_LOCKPOSITION);
	if (hr == EM_OK)
	{
		if (lID != s_EnvironmentID)
		{
			// Get EAX Preset info.
			hr = s_lpEAXManager->GetEnvironmentAttributes(lID, &s_eaxLPDest);
			s_eaxLPDest.flAirAbsorptionHF = 0.0f;
			if (hr == EM_OK)
			{
				if (bUseMorphing)
				{
					// Morph to the new Destination from the Current Settings
					s_eaxLPSource = s_eaxLPCur;
					s_eaxMorphCount = 0;
					s_eaxMorphStartTime = Com_Milliseconds();
					s_eaxMorphing = true;
				}
				else
				{
					// Set Environment
					s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
						NULL, &s_eaxLPDest, sizeof(EAXLISTENERPROPERTIES));
					
					s_eaxLPSource = s_eaxLPCur = s_eaxLPDest;
					s_eaxMorphing = false;
				}

				s_EnvironmentID = lID;
			}
		}
	}

	return;
}


/*
	Updates the EAX Buffer related effects on the given Source
*/
void UpdateEAXBuffer(channel_t *ch)
{
	HRESULT hr;
	EMPOINT EMSourcePoint;
	EMPOINT EMVirtualSourcePoint;
	EAXOBSTRUCTIONPROPERTIES eaxOBProp;
	EAXOCCLUSIONPROPERTIES eaxOCProp;

	// If EAX Manager is not initialized, or there is no EAX support, or the listener
	// is underwater, return
	if ((!s_lpEAXManager) || (!s_bEAX) || (s_bInWater))
		return;
	
	// Set Occlusion Direct Ratio to the default value (it won't get set by the current version of
	// EAX Manager)
	eaxOCProp.flOcclusionDirectRatio = EAXBUFFER_DEFAULTOCCLUSIONDIRECTRATIO;

	// Convert Source co-ordinate to left-handed system
	if (ch->fixed_origin)
	{
		// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
		EMSourcePoint.fX = ch->origin[0];
		EMSourcePoint.fY = ch->origin[2];
		EMSourcePoint.fZ = ch->origin[1];
	}
	else
	{
		if (ch->entnum == listener_number)
		{
			// Source at same position as listener
			// Probably won't be any Occlusion / Obstruction effect -- unless the listener is underwater
			// Converting from Open AL -> DS3D (for EAGLE) ... invert Z
			EMSourcePoint.fX = listener_pos[0];
			EMSourcePoint.fY = listener_pos[1];
			EMSourcePoint.fZ = -listener_pos[2];
		}
		else
		{
			// Get position of Entity
			// Converting from Quake -> DS3D (for EAGLE) ... swap Y and Z
			EMSourcePoint.fX = loopSounds[ ch->entnum ].origin[0];
			EMSourcePoint.fY = loopSounds[ ch->entnum ].origin[2];
			EMSourcePoint.fZ = loopSounds[ ch->entnum ].origin[1];
		}
	}

	hr = s_lpEAXManager->GetSourceDynamicAttributes(0, &EMSourcePoint, &eaxOBProp.lObstruction, &eaxOBProp.flObstructionLFRatio,
		&eaxOCProp.lOcclusion, &eaxOCProp.flOcclusionLFRatio, &eaxOCProp.flOcclusionRoomRatio, &EMVirtualSourcePoint, 0);
	if (hr == EM_OK)
	{	
		// Set EAX effect !
		s_eaxSet(&DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OBSTRUCTIONPARAMETERS,
			ch->alSource, &eaxOBProp, sizeof(EAXOBSTRUCTIONPROPERTIES));

		s_eaxSet(&DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_OCCLUSIONPARAMETERS,
			ch->alSource, &eaxOCProp, sizeof(EAXOCCLUSIONPROPERTIES));
	}

	return;
}


void EAXMorph()
{
	int curPos;
	int curTime;
	float flRatio;

	if ((!s_bEAX) || (!s_eaxMorphing))
		return;

	// Get current time
	curTime = Com_Milliseconds();

	curPos = ((curTime - s_eaxMorphStartTime) / 100);

	if (curPos >= 10)
	{
		// Finished morphing
		s_eaxMorphing = false;
		s_eaxLPSource = s_eaxLPDest;
		s_eaxLPCur = s_eaxLPDest;

		s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
			NULL, &s_eaxLPSource, sizeof(EAXLISTENERPROPERTIES));
	}
	else
	{
		if (curPos > s_eaxMorphCount)
		{
			// Next morph step
			flRatio = (float)curPos / 10.f;

			EAX3ListenerInterpolate(&s_eaxLPSource, &s_eaxLPDest, flRatio, &s_eaxLPCur);

			s_eaxSet(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ALLPARAMETERS,
				NULL, &s_eaxLPCur, sizeof(EAXLISTENERPROPERTIES));

			s_eaxMorphCount = curPos;
		}
	}
}

/***********************************************************************************************\
*
* Definition of the EAXMorph function - EAX3ListenerInterpolate
*
\***********************************************************************************************/

/*
	EAX3ListenerInterpolate
	lpStart			- Initial EAX 3 Listener parameters
	lpFinish		- Final EAX 3 Listener parameters
	flRatio			- Ratio Destination : Source (0.0 == Source, 1.0 == Destination)
	lpResult		- Interpolated EAX 3 Listener parameters
	bCheckValues	- Check EAX 3.0 parameters are in range, default = false (no checking)
*/
bool EAX3ListenerInterpolate(LPEAXLISTENERPROPERTIES lpStart, LPEAXLISTENERPROPERTIES lpFinish,
						float flRatio, LPEAXLISTENERPROPERTIES lpResult, bool bCheckValues)
{
	EAXVECTOR StartVector, FinalVector;

	float flInvRatio;

	if (bCheckValues)
	{
		if (!CheckEAX3LP(lpStart))
			return false;

		if (!CheckEAX3LP(lpFinish))
			return false;
	}

	if (flRatio >= 1.0f)
	{
		memcpy(lpResult, lpFinish, sizeof(EAXLISTENERPROPERTIES));
		return true;
	}
	else if (flRatio <= 0.0f)
	{
		memcpy(lpResult, lpStart, sizeof(EAXLISTENERPROPERTIES));
		return true;
	}

	flInvRatio = (1.0f - flRatio);

	// Environment
	lpResult->ulEnvironment = 26;	// (UNDEFINED environment)

	// Environment Size
	if (lpStart->flEnvironmentSize == lpFinish->flEnvironmentSize)
		lpResult->flEnvironmentSize = lpStart->flEnvironmentSize;
	else
		lpResult->flEnvironmentSize = (float)exp( (log(lpStart->flEnvironmentSize) * flInvRatio) + (log(lpFinish->flEnvironmentSize) * flRatio) );
	
	// Environment Diffusion
	if (lpStart->flEnvironmentDiffusion == lpFinish->flEnvironmentDiffusion)
		lpResult->flEnvironmentDiffusion = lpStart->flEnvironmentDiffusion;
	else
		lpResult->flEnvironmentDiffusion = (lpStart->flEnvironmentDiffusion * flInvRatio) + (lpFinish->flEnvironmentDiffusion * flRatio);
	
	// Room
	if (lpStart->lRoom == lpFinish->lRoom)
		lpResult->lRoom = lpStart->lRoom;
	else
		lpResult->lRoom = (int)( ((float)lpStart->lRoom * flInvRatio) + ((float)lpFinish->lRoom * flRatio) );
	
	// Room HF
	if (lpStart->lRoomHF == lpFinish->lRoomHF)
		lpResult->lRoomHF = lpStart->lRoomHF;
	else
		lpResult->lRoomHF = (int)( ((float)lpStart->lRoomHF * flInvRatio) + ((float)lpFinish->lRoomHF * flRatio) );
	
	// Room LF
	if (lpStart->lRoomLF == lpFinish->lRoomLF)
		lpResult->lRoomLF = lpStart->lRoomLF;
	else
		lpResult->lRoomLF = (int)( ((float)lpStart->lRoomLF * flInvRatio) + ((float)lpFinish->lRoomLF * flRatio) );
	
	// Decay Time
	if (lpStart->flDecayTime == lpFinish->flDecayTime)
		lpResult->flDecayTime = lpStart->flDecayTime;
	else
		lpResult->flDecayTime = (float)exp( (log(lpStart->flDecayTime) * flInvRatio) + (log(lpFinish->flDecayTime) * flRatio) );
	
	// Decay HF Ratio
	if (lpStart->flDecayHFRatio == lpFinish->flDecayHFRatio)
		lpResult->flDecayHFRatio = lpStart->flDecayHFRatio;
	else
		lpResult->flDecayHFRatio = (float)exp( (log(lpStart->flDecayHFRatio) * flInvRatio) + (log(lpFinish->flDecayHFRatio) * flRatio) );
	
	// Decay LF Ratio
	if (lpStart->flDecayLFRatio == lpFinish->flDecayLFRatio)
		lpResult->flDecayLFRatio = lpStart->flDecayLFRatio;
	else
		lpResult->flDecayLFRatio = (float)exp( (log(lpStart->flDecayLFRatio) * flInvRatio) + (log(lpFinish->flDecayLFRatio) * flRatio) );
	
	// Reflections
	if (lpStart->lReflections == lpFinish->lReflections)
		lpResult->lReflections = lpStart->lReflections;
	else
		lpResult->lReflections = (int)( ((float)lpStart->lReflections * flInvRatio) + ((float)lpFinish->lReflections * flRatio) );
	
	// Reflections Delay
	if (lpStart->flReflectionsDelay == lpFinish->flReflectionsDelay)
		lpResult->flReflectionsDelay = lpStart->flReflectionsDelay;
	else
		lpResult->flReflectionsDelay = (float)exp( (log(lpStart->flReflectionsDelay+0.0001) * flInvRatio) + (log(lpFinish->flReflectionsDelay+0.0001) * flRatio) );

	// Reflections Pan

	// To interpolate the vector correctly we need to ensure that both the initial and final vectors vectors are clamped to a length of 1.0f
	StartVector = lpStart->vReflectionsPan;
	FinalVector = lpFinish->vReflectionsPan;

	Clamp(&StartVector);
	Clamp(&FinalVector);

	if (lpStart->vReflectionsPan.x == lpFinish->vReflectionsPan.x)
		lpResult->vReflectionsPan.x = lpStart->vReflectionsPan.x;
	else
		lpResult->vReflectionsPan.x = FinalVector.x + (flInvRatio * (StartVector.x - FinalVector.x));
	
	if (lpStart->vReflectionsPan.y == lpFinish->vReflectionsPan.y)
		lpResult->vReflectionsPan.y = lpStart->vReflectionsPan.y;
	else
		lpResult->vReflectionsPan.y = FinalVector.y + (flInvRatio * (StartVector.y - FinalVector.y));
	
	if (lpStart->vReflectionsPan.z == lpFinish->vReflectionsPan.z)
		lpResult->vReflectionsPan.z = lpStart->vReflectionsPan.z;
	else
		lpResult->vReflectionsPan.z = FinalVector.z + (flInvRatio * (StartVector.z - FinalVector.z));
	
	// Reverb
	if (lpStart->lReverb == lpFinish->lReverb)
		lpResult->lReverb = lpStart->lReverb;
	else
		lpResult->lReverb = (int)( ((float)lpStart->lReverb * flInvRatio) + ((float)lpFinish->lReverb * flRatio) );
	
	// Reverb Delay
	if (lpStart->flReverbDelay == lpFinish->flReverbDelay)
		lpResult->flReverbDelay = lpStart->flReverbDelay;
	else
		lpResult->flReverbDelay = (float)exp( (log(lpStart->flReverbDelay+0.0001) * flInvRatio) + (log(lpFinish->flReverbDelay+0.0001) * flRatio) );
	
	// Reverb Pan

	// To interpolate the vector correctly we need to ensure that both the initial and final vectors are clamped to a length of 1.0f	
	StartVector = lpStart->vReverbPan;
	FinalVector = lpFinish->vReverbPan;

	Clamp(&StartVector);
	Clamp(&FinalVector);

	if (lpStart->vReverbPan.x == lpFinish->vReverbPan.x)
		lpResult->vReverbPan.x = lpStart->vReverbPan.x;
	else
		lpResult->vReverbPan.x = FinalVector.x + (flInvRatio * (StartVector.x - FinalVector.x));
	
	if (lpStart->vReverbPan.y == lpFinish->vReverbPan.y)
		lpResult->vReverbPan.y = lpStart->vReverbPan.y;
	else
		lpResult->vReverbPan.y = FinalVector.y + (flInvRatio * (StartVector.y - FinalVector.y));
	
	if (lpStart->vReverbPan.z == lpFinish->vReverbPan.z)
		lpResult->vReverbPan.z = lpStart->vReverbPan.z;
	else
		lpResult->vReverbPan.z = FinalVector.z + (flInvRatio * (StartVector.z - FinalVector.z));
	
	// Echo Time
	if (lpStart->flEchoTime == lpFinish->flEchoTime)
		lpResult->flEchoTime = lpStart->flEchoTime;
	else
		lpResult->flEchoTime = (float)exp( (log(lpStart->flEchoTime) * flInvRatio) + (log(lpFinish->flEchoTime) * flRatio) );
	
	// Echo Depth
	if (lpStart->flEchoDepth == lpFinish->flEchoDepth)
		lpResult->flEchoDepth = lpStart->flEchoDepth;
	else
		lpResult->flEchoDepth = (lpStart->flEchoDepth * flInvRatio) + (lpFinish->flEchoDepth * flRatio);

	// Modulation Time
	if (lpStart->flModulationTime == lpFinish->flModulationTime)
		lpResult->flModulationTime = lpStart->flModulationTime;
	else
		lpResult->flModulationTime = (float)exp( (log(lpStart->flModulationTime) * flInvRatio) + (log(lpFinish->flModulationTime) * flRatio) );
	
	// Modulation Depth
	if (lpStart->flModulationDepth == lpFinish->flModulationDepth)
		lpResult->flModulationDepth = lpStart->flModulationDepth;
	else
		lpResult->flModulationDepth = (lpStart->flModulationDepth * flInvRatio) + (lpFinish->flModulationDepth * flRatio);
	
	// Air Absorption HF
	if (lpStart->flAirAbsorptionHF == lpFinish->flAirAbsorptionHF)
		lpResult->flAirAbsorptionHF = lpStart->flAirAbsorptionHF;
	else
		lpResult->flAirAbsorptionHF = (lpStart->flAirAbsorptionHF * flInvRatio) + (lpFinish->flAirAbsorptionHF * flRatio);
	
	// HF Reference
	if (lpStart->flHFReference == lpFinish->flHFReference)
		lpResult->flHFReference = lpStart->flHFReference;
	else
		lpResult->flHFReference = (float)exp( (log(lpStart->flHFReference) * flInvRatio) + (log(lpFinish->flHFReference) * flRatio) );
	
	// LF Reference
	if (lpStart->flLFReference == lpFinish->flLFReference)
		lpResult->flLFReference = lpStart->flLFReference;
	else
		lpResult->flLFReference = (float)exp( (log(lpStart->flLFReference) * flInvRatio) + (log(lpFinish->flLFReference) * flRatio) );
	
	// Room Rolloff Factor
	if (lpStart->flRoomRolloffFactor == lpFinish->flRoomRolloffFactor)
		lpResult->flRoomRolloffFactor = lpStart->flRoomRolloffFactor;
	else
		lpResult->flRoomRolloffFactor = (lpStart->flRoomRolloffFactor * flInvRatio) + (lpFinish->flRoomRolloffFactor * flRatio);
	
	// Flags
	lpResult->ulFlags = (lpStart->ulFlags & lpFinish->ulFlags);

	// Clamp Delays
	if (lpResult->flReflectionsDelay > EAXLISTENER_MAXREFLECTIONSDELAY)
		lpResult->flReflectionsDelay = EAXLISTENER_MAXREFLECTIONSDELAY;

	if (lpResult->flReverbDelay > EAXLISTENER_MAXREVERBDELAY)
		lpResult->flReverbDelay = EAXLISTENER_MAXREVERBDELAY;

	return true;
}


/*
	CheckEAX3LP
	Checks that the parameters in the EAX 3 Listener Properties structure are in-range
*/
bool CheckEAX3LP(LPEAXLISTENERPROPERTIES lpEAX3LP)
{
	if ( (lpEAX3LP->lRoom < EAXLISTENER_MINROOM) || (lpEAX3LP->lRoom > EAXLISTENER_MAXROOM) )
		return false;

	if ( (lpEAX3LP->lRoomHF < EAXLISTENER_MINROOMHF) || (lpEAX3LP->lRoomHF > EAXLISTENER_MAXROOMHF) )
		return false;

	if ( (lpEAX3LP->lRoomLF < EAXLISTENER_MINROOMLF) || (lpEAX3LP->lRoomLF > EAXLISTENER_MAXROOMLF) )
		return false;

	if ( (lpEAX3LP->ulEnvironment < EAXLISTENER_MINENVIRONMENT) || (lpEAX3LP->ulEnvironment > EAXLISTENER_MAXENVIRONMENT) )
		return false;

	if ( (lpEAX3LP->flEnvironmentSize < EAXLISTENER_MINENVIRONMENTSIZE) || (lpEAX3LP->flEnvironmentSize > EAXLISTENER_MAXENVIRONMENTSIZE) )
		return false;

	if ( (lpEAX3LP->flEnvironmentDiffusion < EAXLISTENER_MINENVIRONMENTDIFFUSION) || (lpEAX3LP->flEnvironmentDiffusion > EAXLISTENER_MAXENVIRONMENTDIFFUSION) )
		return false;

	if ( (lpEAX3LP->flDecayTime < EAXLISTENER_MINDECAYTIME) || (lpEAX3LP->flDecayTime > EAXLISTENER_MAXDECAYTIME) )
		return false;

	if ( (lpEAX3LP->flDecayHFRatio < EAXLISTENER_MINDECAYHFRATIO) || (lpEAX3LP->flDecayHFRatio > EAXLISTENER_MAXDECAYHFRATIO) )
		return false;

	if ( (lpEAX3LP->flDecayLFRatio < EAXLISTENER_MINDECAYLFRATIO) || (lpEAX3LP->flDecayLFRatio > EAXLISTENER_MAXDECAYLFRATIO) )
		return false;

	if ( (lpEAX3LP->lReflections < EAXLISTENER_MINREFLECTIONS) || (lpEAX3LP->lReflections > EAXLISTENER_MAXREFLECTIONS) )
		return false;

	if ( (lpEAX3LP->flReflectionsDelay < EAXLISTENER_MINREFLECTIONSDELAY) || (lpEAX3LP->flReflectionsDelay > EAXLISTENER_MAXREFLECTIONSDELAY) )
		return false;

	if ( (lpEAX3LP->lReverb < EAXLISTENER_MINREVERB) || (lpEAX3LP->lReverb > EAXLISTENER_MAXREVERB) )
		return false;

	if ( (lpEAX3LP->flReverbDelay < EAXLISTENER_MINREVERBDELAY) || (lpEAX3LP->flReverbDelay > EAXLISTENER_MAXREVERBDELAY) )
		return false;

	if ( (lpEAX3LP->flEchoTime < EAXLISTENER_MINECHOTIME) || (lpEAX3LP->flEchoTime > EAXLISTENER_MAXECHOTIME) )
		return false;

	if ( (lpEAX3LP->flEchoDepth < EAXLISTENER_MINECHODEPTH) || (lpEAX3LP->flEchoDepth > EAXLISTENER_MAXECHODEPTH) )
		return false;

	if ( (lpEAX3LP->flModulationTime < EAXLISTENER_MINMODULATIONTIME) || (lpEAX3LP->flModulationTime > EAXLISTENER_MAXMODULATIONTIME) )
		return false;

	if ( (lpEAX3LP->flModulationDepth < EAXLISTENER_MINMODULATIONDEPTH) || (lpEAX3LP->flModulationDepth > EAXLISTENER_MAXMODULATIONDEPTH) )
		return false;

	if ( (lpEAX3LP->flAirAbsorptionHF < EAXLISTENER_MINAIRABSORPTIONHF) || (lpEAX3LP->flAirAbsorptionHF > EAXLISTENER_MAXAIRABSORPTIONHF) )
		return false;

	if ( (lpEAX3LP->flHFReference < EAXLISTENER_MINHFREFERENCE) || (lpEAX3LP->flHFReference > EAXLISTENER_MAXHFREFERENCE) )
		return false;

	if ( (lpEAX3LP->flLFReference < EAXLISTENER_MINLFREFERENCE) || (lpEAX3LP->flLFReference > EAXLISTENER_MAXLFREFERENCE) )
		return false;

	if ( (lpEAX3LP->flRoomRolloffFactor < EAXLISTENER_MINROOMROLLOFFFACTOR) || (lpEAX3LP->flRoomRolloffFactor > EAXLISTENER_MAXROOMROLLOFFFACTOR) )
		return false;

	if (lpEAX3LP->ulFlags & EAXLISTENERFLAGS_RESERVED)
		return false;

	return true;
}

/*
	Clamp
	Clamps the length of the vector to 1.0f
*/
void Clamp(EAXVECTOR *eaxVector)
{
	float flMagnitude;
	float flInvMagnitude;

	flMagnitude = (float)sqrt((eaxVector->x*eaxVector->x) + (eaxVector->y*eaxVector->y) + (eaxVector->z*eaxVector->z));

	if (flMagnitude <= 1.0f)
		return;

	flInvMagnitude = 1.0f / flMagnitude;

	eaxVector->x *= flInvMagnitude;
	eaxVector->y *= flInvMagnitude;
	eaxVector->z *= flInvMagnitude;
}