#ifndef __GLW_SDL_H__
#define __GLW_SDL_H__

#define GPA( a ) SDL_GL_GetProcAddress( a )

typedef struct
{
	bool OpenGLLib; // instance of OpenGL library

	FILE *log_fp;
	
} glwstate_t;

extern glwstate_t glw_state;

#endif
