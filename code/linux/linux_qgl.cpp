/*
** LINUX_QGL.C
**
** This file implements the operating system binding of GL to QGL function
** pointers.  When doing a port of Quake2 you must implement the following
** two functions:
**
** QGL_Init() - loads libraries, assigns function pointers, etc.
** QGL_Shutdown() - unloads libraries, NULLs function pointers
*/
#include <float.h>
#include "../renderer/tr_local.h"
#include "linux_glw.h"
 #include <unistd.h>
#include <sys/types.h>

//#include <GL/fxmesa.h>
#include <GL/glx.h>

#include <dlfcn.h>

//FX Mesa Functions
//fxMesaContext (*qfxMesaCreateContext)(GLuint win, GrScreenResolution_t, GrScreenRefresh_t, const GLint attribList[]);
//fxMesaContext (*qfxMesaCreateBestContext)(GLuint win, GLint width, GLint height, const GLint attribList[]);
//void (*qfxMesaDestroyContext)(fxMesaContext ctx);
//void (*qfxMesaMakeCurrent)(fxMesaContext ctx);
//fxMesaContext (*qfxMesaGetCurrentContext)(void);
//void (*qfxMesaSwapBuffers)(void);

//GLX Functions
XVisualInfo * (*qglXChooseVisual)( Display *dpy, int screen, int *attribList );
GLXContext (*qglXCreateContext)( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct );
void (*qglXDestroyContext)( Display *dpy, GLXContext ctx );
Bool (*qglXMakeCurrent)( Display *dpy, GLXDrawable drawable, GLXContext ctx);
void (*qglXCopyContext)( Display *dpy, GLXContext src, GLXContext dst, GLuint mask );
void (*qglXSwapBuffers)( Display *dpy, GLXDrawable drawable );

void ( APIENTRY * qglAccum )(GLenum op, GLfloat value);
void ( APIENTRY * qglAlphaFunc )(GLenum func, GLclampf ref);
GLboolean ( APIENTRY * qglAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
void ( APIENTRY * qglArrayElement )(GLint i);
void ( APIENTRY * qglBegin )(GLenum mode);
void ( APIENTRY * qglBindTexture )(GLenum target, GLuint texture);
void ( APIENTRY * qglBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
void ( APIENTRY * qglBlendFunc )(GLenum sfactor, GLenum dfactor);
void ( APIENTRY * qglCallList )(GLuint list);
void ( APIENTRY * qglCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
void ( APIENTRY * qglClear )(GLbitfield mask);
void ( APIENTRY * qglClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void ( APIENTRY * qglClearDepth )(GLclampd depth);
void ( APIENTRY * qglClearIndex )(GLfloat c);
void ( APIENTRY * qglClearStencil )(GLint s);
void ( APIENTRY * qglClipPlane )(GLenum plane, const GLdouble *equation);
void ( APIENTRY * qglColor3b )(GLbyte red, GLbyte green, GLbyte blue);
void ( APIENTRY * qglColor3bv )(const GLbyte *v);
void ( APIENTRY * qglColor3d )(GLdouble red, GLdouble green, GLdouble blue);
void ( APIENTRY * qglColor3dv )(const GLdouble *v);
void ( APIENTRY * qglColor3f )(GLfloat red, GLfloat green, GLfloat blue);
void ( APIENTRY * qglColor3fv )(const GLfloat *v);
void ( APIENTRY * qglColor3i )(GLint red, GLint green, GLint blue);
void ( APIENTRY * qglColor3iv )(const GLint *v);
void ( APIENTRY * qglColor3s )(GLshort red, GLshort green, GLshort blue);
void ( APIENTRY * qglColor3sv )(const GLshort *v);
void ( APIENTRY * qglColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
void ( APIENTRY * qglColor3ubv )(const GLubyte *v);
void ( APIENTRY * qglColor3ui )(GLuint red, GLuint green, GLuint blue);
void ( APIENTRY * qglColor3uiv )(const GLuint *v);
void ( APIENTRY * qglColor3us )(GLushort red, GLushort green, GLushort blue);
void ( APIENTRY * qglColor3usv )(const GLushort *v);
void ( APIENTRY * qglColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
void ( APIENTRY * qglColor4bv )(const GLbyte *v);
void ( APIENTRY * qglColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
void ( APIENTRY * qglColor4dv )(const GLdouble *v);
void ( APIENTRY * qglColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void ( APIENTRY * qglColor4fv )(const GLfloat *v);
void ( APIENTRY * qglColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
void ( APIENTRY * qglColor4iv )(const GLint *v);
void ( APIENTRY * qglColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
void ( APIENTRY * qglColor4sv )(const GLshort *v);
void ( APIENTRY * qglColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
void ( APIENTRY * qglColor4ubv )(const GLubyte *v);
void ( APIENTRY * qglColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
void ( APIENTRY * qglColor4uiv )(const GLuint *v);
void ( APIENTRY * qglColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
void ( APIENTRY * qglColor4usv )(const GLushort *v);
void ( APIENTRY * qglColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void ( APIENTRY * qglColorMaterial )(GLenum face, GLenum mode);
void ( APIENTRY * qglColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
void ( APIENTRY * qglCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
void ( APIENTRY * qglCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void ( APIENTRY * qglCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
void ( APIENTRY * qglCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglCullFace )(GLenum mode);
void ( APIENTRY * qglDeleteLists )(GLuint list, GLsizei range);
void ( APIENTRY * qglDeleteTextures )(GLsizei n, const GLuint *textures);
void ( APIENTRY * qglDepthFunc )(GLenum func);
void ( APIENTRY * qglDepthMask )(GLboolean flag);
void ( APIENTRY * qglDepthRange )(GLclampd zNear, GLclampd zFar);
void ( APIENTRY * qglDisable )(GLenum cap);
void ( APIENTRY * qglDisableClientState )(GLenum array);
void ( APIENTRY * qglDrawArrays )(GLenum mode, GLint first, GLsizei count);
void ( APIENTRY * qglDrawBuffer )(GLenum mode);
void ( APIENTRY * qglDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void ( APIENTRY * qglDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglEdgeFlag )(GLboolean flag);
void ( APIENTRY * qglEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglEdgeFlagv )(const GLboolean *flag);
void ( APIENTRY * qglEnable )(GLenum cap);
void ( APIENTRY * qglEnableClientState )(GLenum array);
void ( APIENTRY * qglEnd )(void);
void ( APIENTRY * qglEndList )(void);
void ( APIENTRY * qglEvalCoord1d )(GLdouble u);
void ( APIENTRY * qglEvalCoord1dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord1f )(GLfloat u);
void ( APIENTRY * qglEvalCoord1fv )(const GLfloat *u);
void ( APIENTRY * qglEvalCoord2d )(GLdouble u, GLdouble v);
void ( APIENTRY * qglEvalCoord2dv )(const GLdouble *u);
void ( APIENTRY * qglEvalCoord2f )(GLfloat u, GLfloat v);
void ( APIENTRY * qglEvalCoord2fv )(const GLfloat *u);
void ( APIENTRY * qglEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
void ( APIENTRY * qglEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
void ( APIENTRY * qglEvalPoint1 )(GLint i);
void ( APIENTRY * qglEvalPoint2 )(GLint i, GLint j);
void ( APIENTRY * qglFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
void ( APIENTRY * qglFinish )(void);
void ( APIENTRY * qglFlush )(void);
void ( APIENTRY * qglFogf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglFogfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglFogi )(GLenum pname, GLint param);
void ( APIENTRY * qglFogiv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglFrontFace )(GLenum mode);
void ( APIENTRY * qglFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint ( APIENTRY * qglGenLists )(GLsizei range);
void ( APIENTRY * qglGenTextures )(GLsizei n, GLuint *textures);
void ( APIENTRY * qglGetBooleanv )(GLenum pname, GLboolean *params);
void ( APIENTRY * qglGetClipPlane )(GLenum plane, GLdouble *equation);
void ( APIENTRY * qglGetDoublev )(GLenum pname, GLdouble *params);
GLenum ( APIENTRY * qglGetError )(void);
void ( APIENTRY * qglGetFloatv )(GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetIntegerv )(GLenum pname, GLint *params);
void ( APIENTRY * qglGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetLightiv )(GLenum light, GLenum pname, GLint *params);
void ( APIENTRY * qglGetMapdv )(GLenum target, GLenum query, GLdouble *v);
void ( APIENTRY * qglGetMapfv )(GLenum target, GLenum query, GLfloat *v);
void ( APIENTRY * qglGetMapiv )(GLenum target, GLenum query, GLint *v);
void ( APIENTRY * qglGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
void ( APIENTRY * qglGetPixelMapfv )(GLenum map, GLfloat *values);
void ( APIENTRY * qglGetPixelMapuiv )(GLenum map, GLuint *values);
void ( APIENTRY * qglGetPixelMapusv )(GLenum map, GLushort *values);
void ( APIENTRY * qglGetPointerv )(GLenum pname, GLvoid* *params);
void ( APIENTRY * qglGetPolygonStipple )(GLubyte *mask);
const GLubyte * ( APIENTRY * qglGetString )(GLenum name);
void ( APIENTRY * qglGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
void ( APIENTRY * qglGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
void ( APIENTRY * qglGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
void ( APIENTRY * qglGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
void ( APIENTRY * qglHint )(GLenum target, GLenum mode);
void ( APIENTRY * qglIndexMask )(GLuint mask);
void ( APIENTRY * qglIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglIndexd )(GLdouble c);
void ( APIENTRY * qglIndexdv )(const GLdouble *c);
void ( APIENTRY * qglIndexf )(GLfloat c);
void ( APIENTRY * qglIndexfv )(const GLfloat *c);
void ( APIENTRY * qglIndexi )(GLint c);
void ( APIENTRY * qglIndexiv )(const GLint *c);
void ( APIENTRY * qglIndexs )(GLshort c);
void ( APIENTRY * qglIndexsv )(const GLshort *c);
void ( APIENTRY * qglIndexub )(GLubyte c);
void ( APIENTRY * qglIndexubv )(const GLubyte *c);
void ( APIENTRY * qglInitNames )(void);
void ( APIENTRY * qglInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean ( APIENTRY * qglIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * qglIsList )(GLuint list);
GLboolean ( APIENTRY * qglIsTexture )(GLuint texture);
void ( APIENTRY * qglLightModelf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglLightModelfv )(GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLightModeli )(GLenum pname, GLint param);
void ( APIENTRY * qglLightModeliv )(GLenum pname, const GLint *params);
void ( APIENTRY * qglLightf )(GLenum light, GLenum pname, GLfloat param);
void ( APIENTRY * qglLightfv )(GLenum light, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglLighti )(GLenum light, GLenum pname, GLint param);
void ( APIENTRY * qglLightiv )(GLenum light, GLenum pname, const GLint *params);
void ( APIENTRY * qglLineStipple )(GLint factor, GLushort pattern);
void ( APIENTRY * qglLineWidth )(GLfloat width);
void ( APIENTRY * qglListBase )(GLuint base);
void ( APIENTRY * qglLoadIdentity )(void);
void ( APIENTRY * qglLoadMatrixd )(const GLdouble *m);
void ( APIENTRY * qglLoadMatrixf )(const GLfloat *m);
void ( APIENTRY * qglLoadName )(GLuint name);
void ( APIENTRY * qglLogicOp )(GLenum opcode);
void ( APIENTRY * qglMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
void ( APIENTRY * qglMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
void ( APIENTRY * qglMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
void ( APIENTRY * qglMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
void ( APIENTRY * qglMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
void ( APIENTRY * qglMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
void ( APIENTRY * qglMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
void ( APIENTRY * qglMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
void ( APIENTRY * qglMaterialf )(GLenum face, GLenum pname, GLfloat param);
void ( APIENTRY * qglMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglMateriali )(GLenum face, GLenum pname, GLint param);
void ( APIENTRY * qglMaterialiv )(GLenum face, GLenum pname, const GLint *params);
void ( APIENTRY * qglMatrixMode )(GLenum mode);
void ( APIENTRY * qglMultMatrixd )(const GLdouble *m);
void ( APIENTRY * qglMultMatrixf )(const GLfloat *m);
void ( APIENTRY * qglNewList )(GLuint list, GLenum mode);
void ( APIENTRY * qglNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
void ( APIENTRY * qglNormal3bv )(const GLbyte *v);
void ( APIENTRY * qglNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
void ( APIENTRY * qglNormal3dv )(const GLdouble *v);
void ( APIENTRY * qglNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
void ( APIENTRY * qglNormal3fv )(const GLfloat *v);
void ( APIENTRY * qglNormal3i )(GLint nx, GLint ny, GLint nz);
void ( APIENTRY * qglNormal3iv )(const GLint *v);
void ( APIENTRY * qglNormal3s )(GLshort nx, GLshort ny, GLshort nz);
void ( APIENTRY * qglNormal3sv )(const GLshort *v);
void ( APIENTRY * qglNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void ( APIENTRY * qglPassThrough )(GLfloat token);
void ( APIENTRY * qglPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
void ( APIENTRY * qglPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
void ( APIENTRY * qglPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
void ( APIENTRY * qglPixelStoref )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelStorei )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelTransferf )(GLenum pname, GLfloat param);
void ( APIENTRY * qglPixelTransferi )(GLenum pname, GLint param);
void ( APIENTRY * qglPixelZoom )(GLfloat xfactor, GLfloat yfactor);
void ( APIENTRY * qglPointSize )(GLfloat size);
void ( APIENTRY * qglPolygonMode )(GLenum face, GLenum mode);
void ( APIENTRY * qglPolygonOffset )(GLfloat factor, GLfloat units);
void ( APIENTRY * qglPolygonStipple )(const GLubyte *mask);
void ( APIENTRY * qglPopAttrib )(void);
void ( APIENTRY * qglPopClientAttrib )(void);
void ( APIENTRY * qglPopMatrix )(void);
void ( APIENTRY * qglPopName )(void);
void ( APIENTRY * qglPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
void ( APIENTRY * qglPushAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushClientAttrib )(GLbitfield mask);
void ( APIENTRY * qglPushMatrix )(void);
void ( APIENTRY * qglPushName )(GLuint name);
void ( APIENTRY * qglRasterPos2d )(GLdouble x, GLdouble y);
void ( APIENTRY * qglRasterPos2dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos2f )(GLfloat x, GLfloat y);
void ( APIENTRY * qglRasterPos2fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos2i )(GLint x, GLint y);
void ( APIENTRY * qglRasterPos2iv )(const GLint *v);
void ( APIENTRY * qglRasterPos2s )(GLshort x, GLshort y);
void ( APIENTRY * qglRasterPos2sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglRasterPos3dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglRasterPos3fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos3i )(GLint x, GLint y, GLint z);
void ( APIENTRY * qglRasterPos3iv )(const GLint *v);
void ( APIENTRY * qglRasterPos3s )(GLshort x, GLshort y, GLshort z);
void ( APIENTRY * qglRasterPos3sv )(const GLshort *v);
void ( APIENTRY * qglRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ( APIENTRY * qglRasterPos4dv )(const GLdouble *v);
void ( APIENTRY * qglRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglRasterPos4fv )(const GLfloat *v);
void ( APIENTRY * qglRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglRasterPos4iv )(const GLint *v);
void ( APIENTRY * qglRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
void ( APIENTRY * qglRasterPos4sv )(const GLshort *v);
void ( APIENTRY * qglReadBuffer )(GLenum mode);
void ( APIENTRY * qglReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void ( APIENTRY * qglRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
void ( APIENTRY * qglRectdv )(const GLdouble *v1, const GLdouble *v2);
void ( APIENTRY * qglRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
void ( APIENTRY * qglRectfv )(const GLfloat *v1, const GLfloat *v2);
void ( APIENTRY * qglRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
void ( APIENTRY * qglRectiv )(const GLint *v1, const GLint *v2);
void ( APIENTRY * qglRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
void ( APIENTRY * qglRectsv )(const GLshort *v1, const GLshort *v2);
GLint ( APIENTRY * qglRenderMode )(GLenum mode);
void ( APIENTRY * qglRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglScaled )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglScalef )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
void ( APIENTRY * qglSelectBuffer )(GLsizei size, GLuint *buffer);
void ( APIENTRY * qglShadeModel )(GLenum mode);
void ( APIENTRY * qglStencilFunc )(GLenum func, GLint ref, GLuint mask);
void ( APIENTRY * qglStencilMask )(GLuint mask);
void ( APIENTRY * qglStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
void ( APIENTRY * qglTexCoord1d )(GLdouble s);
void ( APIENTRY * qglTexCoord1dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord1f )(GLfloat s);
void ( APIENTRY * qglTexCoord1fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord1i )(GLint s);
void ( APIENTRY * qglTexCoord1iv )(const GLint *v);
void ( APIENTRY * qglTexCoord1s )(GLshort s);
void ( APIENTRY * qglTexCoord1sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord2d )(GLdouble s, GLdouble t);
void ( APIENTRY * qglTexCoord2dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord2f )(GLfloat s, GLfloat t);
void ( APIENTRY * qglTexCoord2fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord2i )(GLint s, GLint t);
void ( APIENTRY * qglTexCoord2iv )(const GLint *v);
void ( APIENTRY * qglTexCoord2s )(GLshort s, GLshort t);
void ( APIENTRY * qglTexCoord2sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
void ( APIENTRY * qglTexCoord3dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
void ( APIENTRY * qglTexCoord3fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord3i )(GLint s, GLint t, GLint r);
void ( APIENTRY * qglTexCoord3iv )(const GLint *v);
void ( APIENTRY * qglTexCoord3s )(GLshort s, GLshort t, GLshort r);
void ( APIENTRY * qglTexCoord3sv )(const GLshort *v);
void ( APIENTRY * qglTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
void ( APIENTRY * qglTexCoord4dv )(const GLdouble *v);
void ( APIENTRY * qglTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
void ( APIENTRY * qglTexCoord4fv )(const GLfloat *v);
void ( APIENTRY * qglTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
void ( APIENTRY * qglTexCoord4iv )(const GLint *v);
void ( APIENTRY * qglTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
void ( APIENTRY * qglTexCoord4sv )(const GLshort *v);
void ( APIENTRY * qglTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglTexEnvf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexEnvi )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexEnviv )(GLenum target, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexGend )(GLenum coord, GLenum pname, GLdouble param);
void ( APIENTRY * qglTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
void ( APIENTRY * qglTexGenf )(GLenum coord, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexGeni )(GLenum coord, GLenum pname, GLint param);
void ( APIENTRY * qglTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexParameterf )(GLenum target, GLenum pname, GLfloat param);
void ( APIENTRY * qglTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
void ( APIENTRY * qglTexParameteri )(GLenum target, GLenum pname, GLint param);
void ( APIENTRY * qglTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
void ( APIENTRY * qglTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
void ( APIENTRY * qglTranslated )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglTranslatef )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglVertex2d )(GLdouble x, GLdouble y);
void ( APIENTRY * qglVertex2dv )(const GLdouble *v);
void ( APIENTRY * qglVertex2f )(GLfloat x, GLfloat y);
void ( APIENTRY * qglVertex2fv )(const GLfloat *v);
void ( APIENTRY * qglVertex2i )(GLint x, GLint y);
void ( APIENTRY * qglVertex2iv )(const GLint *v);
void ( APIENTRY * qglVertex2s )(GLshort x, GLshort y);
void ( APIENTRY * qglVertex2sv )(const GLshort *v);
void ( APIENTRY * qglVertex3d )(GLdouble x, GLdouble y, GLdouble z);
void ( APIENTRY * qglVertex3dv )(const GLdouble *v);
void ( APIENTRY * qglVertex3f )(GLfloat x, GLfloat y, GLfloat z);
void ( APIENTRY * qglVertex3fv )(const GLfloat *v);
void ( APIENTRY * qglVertex3i )(GLint x, GLint y, GLint z);
void ( APIENTRY * qglVertex3iv )(const GLint *v);
void ( APIENTRY * qglVertex3s )(GLshort x, GLshort y, GLshort z);
void ( APIENTRY * qglVertex3sv )(const GLshort *v);
void ( APIENTRY * qglVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
void ( APIENTRY * qglVertex4dv )(const GLdouble *v);
void ( APIENTRY * qglVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void ( APIENTRY * qglVertex4fv )(const GLfloat *v);
void ( APIENTRY * qglVertex4i )(GLint x, GLint y, GLint z, GLint w);
void ( APIENTRY * qglVertex4iv )(const GLint *v);
void ( APIENTRY * qglVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
void ( APIENTRY * qglVertex4sv )(const GLshort *v);
void ( APIENTRY * qglVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void ( APIENTRY * qglViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

void ( APIENTRY * qglMultiTexCoord2fARB )( GLenum texture, GLfloat s, GLfloat t );
void ( APIENTRY * qglActiveTextureARB )( GLenum texture );
void ( APIENTRY * qglClientActiveTextureARB )( GLenum texture );

void ( APIENTRY * qglLockArraysEXT)( int, int);
void ( APIENTRY * qglUnlockArraysEXT) ( void );

void ( APIENTRY * qglPointParameterfEXT)( GLenum param, GLfloat value );
void ( APIENTRY * qglPointParameterfvEXT)( GLenum param, GLfloat *value );
void ( APIENTRY * qglColorTableEXT)( int, int, int, int, int, const void * );
void ( APIENTRY * qgl3DfxSetPaletteEXT)( GLuint * );
void ( APIENTRY * qglSelectTextureSGIS)( GLenum );
void ( APIENTRY * qglMTexCoord2fSGIS)( GLenum, GLfloat, GLfloat );

static void ( APIENTRY * dllAccum )(GLenum op, GLfloat value);
static void ( APIENTRY * dllAlphaFunc )(GLenum func, GLclampf ref);
GLboolean ( APIENTRY * dllAreTexturesResident )(GLsizei n, const GLuint *textures, GLboolean *residences);
static void ( APIENTRY * dllArrayElement )(GLint i);
static void ( APIENTRY * dllBegin )(GLenum mode);
static void ( APIENTRY * dllBindTexture )(GLenum target, GLuint texture);
static void ( APIENTRY * dllBitmap )(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
static void ( APIENTRY * dllBlendFunc )(GLenum sfactor, GLenum dfactor);
static void ( APIENTRY * dllCallList )(GLuint list);
static void ( APIENTRY * dllCallLists )(GLsizei n, GLenum type, const GLvoid *lists);
static void ( APIENTRY * dllClear )(GLbitfield mask);
static void ( APIENTRY * dllClearAccum )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static void ( APIENTRY * dllClearColor )(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
static void ( APIENTRY * dllClearDepth )(GLclampd depth);
static void ( APIENTRY * dllClearIndex )(GLfloat c);
static void ( APIENTRY * dllClearStencil )(GLint s);
static void ( APIENTRY * dllClipPlane )(GLenum plane, const GLdouble *equation);
static void ( APIENTRY * dllColor3b )(GLbyte red, GLbyte green, GLbyte blue);
static void ( APIENTRY * dllColor3bv )(const GLbyte *v);
static void ( APIENTRY * dllColor3d )(GLdouble red, GLdouble green, GLdouble blue);
static void ( APIENTRY * dllColor3dv )(const GLdouble *v);
static void ( APIENTRY * dllColor3f )(GLfloat red, GLfloat green, GLfloat blue);
static void ( APIENTRY * dllColor3fv )(const GLfloat *v);
static void ( APIENTRY * dllColor3i )(GLint red, GLint green, GLint blue);
static void ( APIENTRY * dllColor3iv )(const GLint *v);
static void ( APIENTRY * dllColor3s )(GLshort red, GLshort green, GLshort blue);
static void ( APIENTRY * dllColor3sv )(const GLshort *v);
static void ( APIENTRY * dllColor3ub )(GLubyte red, GLubyte green, GLubyte blue);
static void ( APIENTRY * dllColor3ubv )(const GLubyte *v);
static void ( APIENTRY * dllColor3ui )(GLuint red, GLuint green, GLuint blue);
static void ( APIENTRY * dllColor3uiv )(const GLuint *v);
static void ( APIENTRY * dllColor3us )(GLushort red, GLushort green, GLushort blue);
static void ( APIENTRY * dllColor3usv )(const GLushort *v);
static void ( APIENTRY * dllColor4b )(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
static void ( APIENTRY * dllColor4bv )(const GLbyte *v);
static void ( APIENTRY * dllColor4d )(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
static void ( APIENTRY * dllColor4dv )(const GLdouble *v);
static void ( APIENTRY * dllColor4f )(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
static void ( APIENTRY * dllColor4fv )(const GLfloat *v);
static void ( APIENTRY * dllColor4i )(GLint red, GLint green, GLint blue, GLint alpha);
static void ( APIENTRY * dllColor4iv )(const GLint *v);
static void ( APIENTRY * dllColor4s )(GLshort red, GLshort green, GLshort blue, GLshort alpha);
static void ( APIENTRY * dllColor4sv )(const GLshort *v);
static void ( APIENTRY * dllColor4ub )(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
static void ( APIENTRY * dllColor4ubv )(const GLubyte *v);
static void ( APIENTRY * dllColor4ui )(GLuint red, GLuint green, GLuint blue, GLuint alpha);
static void ( APIENTRY * dllColor4uiv )(const GLuint *v);
static void ( APIENTRY * dllColor4us )(GLushort red, GLushort green, GLushort blue, GLushort alpha);
static void ( APIENTRY * dllColor4usv )(const GLushort *v);
static void ( APIENTRY * dllColorMask )(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
static void ( APIENTRY * dllColorMaterial )(GLenum face, GLenum mode);
static void ( APIENTRY * dllColorPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllCopyPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
static void ( APIENTRY * dllCopyTexImage1D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
static void ( APIENTRY * dllCopyTexImage2D )(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
static void ( APIENTRY * dllCopyTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
static void ( APIENTRY * dllCopyTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
static void ( APIENTRY * dllCullFace )(GLenum mode);
static void ( APIENTRY * dllDeleteLists )(GLuint list, GLsizei range);
static void ( APIENTRY * dllDeleteTextures )(GLsizei n, const GLuint *textures);
static void ( APIENTRY * dllDepthFunc )(GLenum func);
static void ( APIENTRY * dllDepthMask )(GLboolean flag);
static void ( APIENTRY * dllDepthRange )(GLclampd zNear, GLclampd zFar);
static void ( APIENTRY * dllDisable )(GLenum cap);
static void ( APIENTRY * dllDisableClientState )(GLenum array);
static void ( APIENTRY * dllDrawArrays )(GLenum mode, GLint first, GLsizei count);
static void ( APIENTRY * dllDrawBuffer )(GLenum mode);
static void ( APIENTRY * dllDrawElements )(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
static void ( APIENTRY * dllDrawPixels )(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllEdgeFlag )(GLboolean flag);
static void ( APIENTRY * dllEdgeFlagPointer )(GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllEdgeFlagv )(const GLboolean *flag);
static void ( APIENTRY * dllEnable )(GLenum cap);
static void ( APIENTRY * dllEnableClientState )(GLenum array);
static void ( APIENTRY * dllEnd )(void);
static void ( APIENTRY * dllEndList )(void);
static void ( APIENTRY * dllEvalCoord1d )(GLdouble u);
static void ( APIENTRY * dllEvalCoord1dv )(const GLdouble *u);
static void ( APIENTRY * dllEvalCoord1f )(GLfloat u);
static void ( APIENTRY * dllEvalCoord1fv )(const GLfloat *u);
static void ( APIENTRY * dllEvalCoord2d )(GLdouble u, GLdouble v);
static void ( APIENTRY * dllEvalCoord2dv )(const GLdouble *u);
static void ( APIENTRY * dllEvalCoord2f )(GLfloat u, GLfloat v);
static void ( APIENTRY * dllEvalCoord2fv )(const GLfloat *u);
static void ( APIENTRY * dllEvalMesh1 )(GLenum mode, GLint i1, GLint i2);
static void ( APIENTRY * dllEvalMesh2 )(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
static void ( APIENTRY * dllEvalPoint1 )(GLint i);
static void ( APIENTRY * dllEvalPoint2 )(GLint i, GLint j);
static void ( APIENTRY * dllFeedbackBuffer )(GLsizei size, GLenum type, GLfloat *buffer);
static void ( APIENTRY * dllFinish )(void);
static void ( APIENTRY * dllFlush )(void);
static void ( APIENTRY * dllFogf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllFogfv )(GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllFogi )(GLenum pname, GLint param);
static void ( APIENTRY * dllFogiv )(GLenum pname, const GLint *params);
static void ( APIENTRY * dllFrontFace )(GLenum mode);
static void ( APIENTRY * dllFrustum )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GLuint ( APIENTRY * dllGenLists )(GLsizei range);
static void ( APIENTRY * dllGenTextures )(GLsizei n, GLuint *textures);
static void ( APIENTRY * dllGetBooleanv )(GLenum pname, GLboolean *params);
static void ( APIENTRY * dllGetClipPlane )(GLenum plane, GLdouble *equation);
static void ( APIENTRY * dllGetDoublev )(GLenum pname, GLdouble *params);
GLenum ( APIENTRY * dllGetError )(void);
static void ( APIENTRY * dllGetFloatv )(GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetIntegerv )(GLenum pname, GLint *params);
static void ( APIENTRY * dllGetLightfv )(GLenum light, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetLightiv )(GLenum light, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetMapdv )(GLenum target, GLenum query, GLdouble *v);
static void ( APIENTRY * dllGetMapfv )(GLenum target, GLenum query, GLfloat *v);
static void ( APIENTRY * dllGetMapiv )(GLenum target, GLenum query, GLint *v);
static void ( APIENTRY * dllGetMaterialfv )(GLenum face, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetMaterialiv )(GLenum face, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetPixelMapfv )(GLenum map, GLfloat *values);
static void ( APIENTRY * dllGetPixelMapuiv )(GLenum map, GLuint *values);
static void ( APIENTRY * dllGetPixelMapusv )(GLenum map, GLushort *values);
static void ( APIENTRY * dllGetPointerv )(GLenum pname, GLvoid* *params);
static void ( APIENTRY * dllGetPolygonStipple )(GLubyte *mask);
const GLubyte * ( APIENTRY * dllGetString )(GLenum name);
static void ( APIENTRY * dllGetTexEnvfv )(GLenum target, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexEnviv )(GLenum target, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexGendv )(GLenum coord, GLenum pname, GLdouble *params);
static void ( APIENTRY * dllGetTexGenfv )(GLenum coord, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexGeniv )(GLenum coord, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexImage )(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
static void ( APIENTRY * dllGetTexLevelParameterfv )(GLenum target, GLint level, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexLevelParameteriv )(GLenum target, GLint level, GLenum pname, GLint *params);
static void ( APIENTRY * dllGetTexParameterfv )(GLenum target, GLenum pname, GLfloat *params);
static void ( APIENTRY * dllGetTexParameteriv )(GLenum target, GLenum pname, GLint *params);
static void ( APIENTRY * dllHint )(GLenum target, GLenum mode);
static void ( APIENTRY * dllIndexMask )(GLuint mask);
static void ( APIENTRY * dllIndexPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllIndexd )(GLdouble c);
static void ( APIENTRY * dllIndexdv )(const GLdouble *c);
static void ( APIENTRY * dllIndexf )(GLfloat c);
static void ( APIENTRY * dllIndexfv )(const GLfloat *c);
static void ( APIENTRY * dllIndexi )(GLint c);
static void ( APIENTRY * dllIndexiv )(const GLint *c);
static void ( APIENTRY * dllIndexs )(GLshort c);
static void ( APIENTRY * dllIndexsv )(const GLshort *c);
static void ( APIENTRY * dllIndexub )(GLubyte c);
static void ( APIENTRY * dllIndexubv )(const GLubyte *c);
static void ( APIENTRY * dllInitNames )(void);
static void ( APIENTRY * dllInterleavedArrays )(GLenum format, GLsizei stride, const GLvoid *pointer);
GLboolean ( APIENTRY * dllIsEnabled )(GLenum cap);
GLboolean ( APIENTRY * dllIsList )(GLuint list);
GLboolean ( APIENTRY * dllIsTexture )(GLuint texture);
static void ( APIENTRY * dllLightModelf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllLightModelfv )(GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllLightModeli )(GLenum pname, GLint param);
static void ( APIENTRY * dllLightModeliv )(GLenum pname, const GLint *params);
static void ( APIENTRY * dllLightf )(GLenum light, GLenum pname, GLfloat param);
static void ( APIENTRY * dllLightfv )(GLenum light, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllLighti )(GLenum light, GLenum pname, GLint param);
static void ( APIENTRY * dllLightiv )(GLenum light, GLenum pname, const GLint *params);
static void ( APIENTRY * dllLineStipple )(GLint factor, GLushort pattern);
static void ( APIENTRY * dllLineWidth )(GLfloat width);
static void ( APIENTRY * dllListBase )(GLuint base);
static void ( APIENTRY * dllLoadIdentity )(void);
static void ( APIENTRY * dllLoadMatrixd )(const GLdouble *m);
static void ( APIENTRY * dllLoadMatrixf )(const GLfloat *m);
static void ( APIENTRY * dllLoadName )(GLuint name);
static void ( APIENTRY * dllLogicOp )(GLenum opcode);
static void ( APIENTRY * dllMap1d )(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
static void ( APIENTRY * dllMap1f )(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
static void ( APIENTRY * dllMap2d )(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
static void ( APIENTRY * dllMap2f )(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
static void ( APIENTRY * dllMapGrid1d )(GLint un, GLdouble u1, GLdouble u2);
static void ( APIENTRY * dllMapGrid1f )(GLint un, GLfloat u1, GLfloat u2);
static void ( APIENTRY * dllMapGrid2d )(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
static void ( APIENTRY * dllMapGrid2f )(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
static void ( APIENTRY * dllMaterialf )(GLenum face, GLenum pname, GLfloat param);
static void ( APIENTRY * dllMaterialfv )(GLenum face, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllMateriali )(GLenum face, GLenum pname, GLint param);
static void ( APIENTRY * dllMaterialiv )(GLenum face, GLenum pname, const GLint *params);
static void ( APIENTRY * dllMatrixMode )(GLenum mode);
static void ( APIENTRY * dllMultMatrixd )(const GLdouble *m);
static void ( APIENTRY * dllMultMatrixf )(const GLfloat *m);
static void ( APIENTRY * dllNewList )(GLuint list, GLenum mode);
static void ( APIENTRY * dllNormal3b )(GLbyte nx, GLbyte ny, GLbyte nz);
static void ( APIENTRY * dllNormal3bv )(const GLbyte *v);
static void ( APIENTRY * dllNormal3d )(GLdouble nx, GLdouble ny, GLdouble nz);
static void ( APIENTRY * dllNormal3dv )(const GLdouble *v);
static void ( APIENTRY * dllNormal3f )(GLfloat nx, GLfloat ny, GLfloat nz);
static void ( APIENTRY * dllNormal3fv )(const GLfloat *v);
static void ( APIENTRY * dllNormal3i )(GLint nx, GLint ny, GLint nz);
static void ( APIENTRY * dllNormal3iv )(const GLint *v);
static void ( APIENTRY * dllNormal3s )(GLshort nx, GLshort ny, GLshort nz);
static void ( APIENTRY * dllNormal3sv )(const GLshort *v);
static void ( APIENTRY * dllNormalPointer )(GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllOrtho )(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
static void ( APIENTRY * dllPassThrough )(GLfloat token);
static void ( APIENTRY * dllPixelMapfv )(GLenum map, GLsizei mapsize, const GLfloat *values);
static void ( APIENTRY * dllPixelMapuiv )(GLenum map, GLsizei mapsize, const GLuint *values);
static void ( APIENTRY * dllPixelMapusv )(GLenum map, GLsizei mapsize, const GLushort *values);
static void ( APIENTRY * dllPixelStoref )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllPixelStorei )(GLenum pname, GLint param);
static void ( APIENTRY * dllPixelTransferf )(GLenum pname, GLfloat param);
static void ( APIENTRY * dllPixelTransferi )(GLenum pname, GLint param);
static void ( APIENTRY * dllPixelZoom )(GLfloat xfactor, GLfloat yfactor);
static void ( APIENTRY * dllPointSize )(GLfloat size);
static void ( APIENTRY * dllPolygonMode )(GLenum face, GLenum mode);
static void ( APIENTRY * dllPolygonOffset )(GLfloat factor, GLfloat units);
static void ( APIENTRY * dllPolygonStipple )(const GLubyte *mask);
static void ( APIENTRY * dllPopAttrib )(void);
static void ( APIENTRY * dllPopClientAttrib )(void);
static void ( APIENTRY * dllPopMatrix )(void);
static void ( APIENTRY * dllPopName )(void);
static void ( APIENTRY * dllPrioritizeTextures )(GLsizei n, const GLuint *textures, const GLclampf *priorities);
static void ( APIENTRY * dllPushAttrib )(GLbitfield mask);
static void ( APIENTRY * dllPushClientAttrib )(GLbitfield mask);
static void ( APIENTRY * dllPushMatrix )(void);
static void ( APIENTRY * dllPushName )(GLuint name);
static void ( APIENTRY * dllRasterPos2d )(GLdouble x, GLdouble y);
static void ( APIENTRY * dllRasterPos2dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos2f )(GLfloat x, GLfloat y);
static void ( APIENTRY * dllRasterPos2fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos2i )(GLint x, GLint y);
static void ( APIENTRY * dllRasterPos2iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos2s )(GLshort x, GLshort y);
static void ( APIENTRY * dllRasterPos2sv )(const GLshort *v);
static void ( APIENTRY * dllRasterPos3d )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllRasterPos3dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos3f )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllRasterPos3fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos3i )(GLint x, GLint y, GLint z);
static void ( APIENTRY * dllRasterPos3iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos3s )(GLshort x, GLshort y, GLshort z);
static void ( APIENTRY * dllRasterPos3sv )(const GLshort *v);
static void ( APIENTRY * dllRasterPos4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
static void ( APIENTRY * dllRasterPos4dv )(const GLdouble *v);
static void ( APIENTRY * dllRasterPos4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
static void ( APIENTRY * dllRasterPos4fv )(const GLfloat *v);
static void ( APIENTRY * dllRasterPos4i )(GLint x, GLint y, GLint z, GLint w);
static void ( APIENTRY * dllRasterPos4iv )(const GLint *v);
static void ( APIENTRY * dllRasterPos4s )(GLshort x, GLshort y, GLshort z, GLshort w);
static void ( APIENTRY * dllRasterPos4sv )(const GLshort *v);
static void ( APIENTRY * dllReadBuffer )(GLenum mode);
static void ( APIENTRY * dllReadPixels )(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
static void ( APIENTRY * dllRectd )(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
static void ( APIENTRY * dllRectdv )(const GLdouble *v1, const GLdouble *v2);
static void ( APIENTRY * dllRectf )(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
static void ( APIENTRY * dllRectfv )(const GLfloat *v1, const GLfloat *v2);
static void ( APIENTRY * dllRecti )(GLint x1, GLint y1, GLint x2, GLint y2);
static void ( APIENTRY * dllRectiv )(const GLint *v1, const GLint *v2);
static void ( APIENTRY * dllRects )(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
static void ( APIENTRY * dllRectsv )(const GLshort *v1, const GLshort *v2);
GLint ( APIENTRY * dllRenderMode )(GLenum mode);
static void ( APIENTRY * dllRotated )(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllRotatef )(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllScaled )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllScalef )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllScissor )(GLint x, GLint y, GLsizei width, GLsizei height);
static void ( APIENTRY * dllSelectBuffer )(GLsizei size, GLuint *buffer);
static void ( APIENTRY * dllShadeModel )(GLenum mode);
static void ( APIENTRY * dllStencilFunc )(GLenum func, GLint ref, GLuint mask);
static void ( APIENTRY * dllStencilMask )(GLuint mask);
static void ( APIENTRY * dllStencilOp )(GLenum fail, GLenum zfail, GLenum zpass);
static void ( APIENTRY * dllTexCoord1d )(GLdouble s);
static void ( APIENTRY * dllTexCoord1dv )(const GLdouble *v);
static void ( APIENTRY * dllTexCoord1f )(GLfloat s);
static void ( APIENTRY * dllTexCoord1fv )(const GLfloat *v);
static void ( APIENTRY * dllTexCoord1i )(GLint s);
static void ( APIENTRY * dllTexCoord1iv )(const GLint *v);
static void ( APIENTRY * dllTexCoord1s )(GLshort s);
static void ( APIENTRY * dllTexCoord1sv )(const GLshort *v);
static void ( APIENTRY * dllTexCoord2d )(GLdouble s, GLdouble t);
static void ( APIENTRY * dllTexCoord2dv )(const GLdouble *v);
static void ( APIENTRY * dllTexCoord2f )(GLfloat s, GLfloat t);
static void ( APIENTRY * dllTexCoord2fv )(const GLfloat *v);
static void ( APIENTRY * dllTexCoord2i )(GLint s, GLint t);
static void ( APIENTRY * dllTexCoord2iv )(const GLint *v);
static void ( APIENTRY * dllTexCoord2s )(GLshort s, GLshort t);
static void ( APIENTRY * dllTexCoord2sv )(const GLshort *v);
static void ( APIENTRY * dllTexCoord3d )(GLdouble s, GLdouble t, GLdouble r);
static void ( APIENTRY * dllTexCoord3dv )(const GLdouble *v);
static void ( APIENTRY * dllTexCoord3f )(GLfloat s, GLfloat t, GLfloat r);
static void ( APIENTRY * dllTexCoord3fv )(const GLfloat *v);
static void ( APIENTRY * dllTexCoord3i )(GLint s, GLint t, GLint r);
static void ( APIENTRY * dllTexCoord3iv )(const GLint *v);
static void ( APIENTRY * dllTexCoord3s )(GLshort s, GLshort t, GLshort r);
static void ( APIENTRY * dllTexCoord3sv )(const GLshort *v);
static void ( APIENTRY * dllTexCoord4d )(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
static void ( APIENTRY * dllTexCoord4dv )(const GLdouble *v);
static void ( APIENTRY * dllTexCoord4f )(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
static void ( APIENTRY * dllTexCoord4fv )(const GLfloat *v);
static void ( APIENTRY * dllTexCoord4i )(GLint s, GLint t, GLint r, GLint q);
static void ( APIENTRY * dllTexCoord4iv )(const GLint *v);
static void ( APIENTRY * dllTexCoord4s )(GLshort s, GLshort t, GLshort r, GLshort q);
static void ( APIENTRY * dllTexCoord4sv )(const GLshort *v);
static void ( APIENTRY * dllTexCoordPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllTexEnvf )(GLenum target, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexEnvfv )(GLenum target, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllTexEnvi )(GLenum target, GLenum pname, GLint param);
static void ( APIENTRY * dllTexEnviv )(GLenum target, GLenum pname, const GLint *params);
static void ( APIENTRY * dllTexGend )(GLenum coord, GLenum pname, GLdouble param);
static void ( APIENTRY * dllTexGendv )(GLenum coord, GLenum pname, const GLdouble *params);
static void ( APIENTRY * dllTexGenf )(GLenum coord, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexGenfv )(GLenum coord, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllTexGeni )(GLenum coord, GLenum pname, GLint param);
static void ( APIENTRY * dllTexGeniv )(GLenum coord, GLenum pname, const GLint *params);
static void ( APIENTRY * dllTexImage1D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTexImage2D )(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTexParameterf )(GLenum target, GLenum pname, GLfloat param);
static void ( APIENTRY * dllTexParameterfv )(GLenum target, GLenum pname, const GLfloat *params);
static void ( APIENTRY * dllTexParameteri )(GLenum target, GLenum pname, GLint param);
static void ( APIENTRY * dllTexParameteriv )(GLenum target, GLenum pname, const GLint *params);
static void ( APIENTRY * dllTexSubImage1D )(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTexSubImage2D )(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
static void ( APIENTRY * dllTranslated )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllTranslatef )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllVertex2d )(GLdouble x, GLdouble y);
static void ( APIENTRY * dllVertex2dv )(const GLdouble *v);
static void ( APIENTRY * dllVertex2f )(GLfloat x, GLfloat y);
static void ( APIENTRY * dllVertex2fv )(const GLfloat *v);
static void ( APIENTRY * dllVertex2i )(GLint x, GLint y);
static void ( APIENTRY * dllVertex2iv )(const GLint *v);
static void ( APIENTRY * dllVertex2s )(GLshort x, GLshort y);
static void ( APIENTRY * dllVertex2sv )(const GLshort *v);
static void ( APIENTRY * dllVertex3d )(GLdouble x, GLdouble y, GLdouble z);
static void ( APIENTRY * dllVertex3dv )(const GLdouble *v);
static void ( APIENTRY * dllVertex3f )(GLfloat x, GLfloat y, GLfloat z);
static void ( APIENTRY * dllVertex3fv )(const GLfloat *v);
static void ( APIENTRY * dllVertex3i )(GLint x, GLint y, GLint z);
static void ( APIENTRY * dllVertex3iv )(const GLint *v);
static void ( APIENTRY * dllVertex3s )(GLshort x, GLshort y, GLshort z);
static void ( APIENTRY * dllVertex3sv )(const GLshort *v);
static void ( APIENTRY * dllVertex4d )(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
static void ( APIENTRY * dllVertex4dv )(const GLdouble *v);
static void ( APIENTRY * dllVertex4f )(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
static void ( APIENTRY * dllVertex4fv )(const GLfloat *v);
static void ( APIENTRY * dllVertex4i )(GLint x, GLint y, GLint z, GLint w);
static void ( APIENTRY * dllVertex4iv )(const GLint *v);
static void ( APIENTRY * dllVertex4s )(GLshort x, GLshort y, GLshort z, GLshort w);
static void ( APIENTRY * dllVertex4sv )(const GLshort *v);
static void ( APIENTRY * dllVertexPointer )(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
static void ( APIENTRY * dllViewport )(GLint x, GLint y, GLsizei width, GLsizei height);

static void APIENTRY logAccum(GLenum op, GLfloat value)
{
	fprintf( glw_state.log_fp, "glAccum\n" );
	dllAccum( op, value );
}

static void APIENTRY logAlphaFunc(GLenum func, GLclampf ref)
{
	fprintf( glw_state.log_fp, "glAlphaFunc( 0x%x, %f )\n", func, ref );
	dllAlphaFunc( func, ref );
}

static GLboolean APIENTRY logAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
	fprintf( glw_state.log_fp, "glAreTexturesResident\n" );
	return dllAreTexturesResident( n, textures, residences );
}

static void APIENTRY logArrayElement(GLint i)
{
	fprintf( glw_state.log_fp, "glArrayElement\n" );
	dllArrayElement( i );
}

static void APIENTRY logBegin(GLenum mode)
{
	fprintf( glw_state.log_fp, "glBegin( 0x%x )\n", mode );
	dllBegin( mode );
}

static void APIENTRY logBindTexture(GLenum target, GLuint texture)
{
	fprintf( glw_state.log_fp, "glBindTexture( 0x%x, %u )\n", target, texture );
	dllBindTexture( target, texture );
}

static void APIENTRY logBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
	fprintf( glw_state.log_fp, "glBitmap\n" );
	dllBitmap( width, height, xorig, yorig, xmove, ymove, bitmap );
}

static void APIENTRY logBlendFunc(GLenum sfactor, GLenum dfactor)
{
	fprintf( glw_state.log_fp, "glBlendFunc( 0x%x, 0x%x )\n", sfactor, dfactor );
	dllBlendFunc( sfactor, dfactor );
}

static void APIENTRY logCallList(GLuint list)
{
	fprintf( glw_state.log_fp, "glCallList( %u )\n", list );
	dllCallList( list );
}

static void APIENTRY logCallLists(GLsizei n, GLenum type, const void *lists)
{
	fprintf( glw_state.log_fp, "glCallLists\n" );
	dllCallLists( n, type, lists );
}

static void APIENTRY logClear(GLbitfield mask)
{
	fprintf( glw_state.log_fp, "glClear\n" );
	dllClear( mask );
}

static void APIENTRY logClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	fprintf( glw_state.log_fp, "glClearAccum\n" );
	dllClearAccum( red, green, blue, alpha );
}

static void APIENTRY logClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
	fprintf( glw_state.log_fp, "glClearColor\n" );
	dllClearColor( red, green, blue, alpha );
}

static void APIENTRY logClearDepth(GLclampd depth)
{
	fprintf( glw_state.log_fp, "glClearDepth\n" );
	dllClearDepth( depth );
}

static void APIENTRY logClearIndex(GLfloat c)
{
	fprintf( glw_state.log_fp, "glClearIndex\n" );
	dllClearIndex( c );
}

static void APIENTRY logClearStencil(GLint s)
{
	fprintf( glw_state.log_fp, "glClearStencil\n" );
	dllClearStencil( s );
}

static void APIENTRY logClipPlane(GLenum plane, const GLdouble *equation)
{
	fprintf( glw_state.log_fp, "glClipPlane\n" );
	dllClipPlane( plane, equation );
}

static void APIENTRY logColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
	fprintf( glw_state.log_fp, "glColor3b\n" );
	dllColor3b( red, green, blue );
}

static void APIENTRY logColor3bv(const GLbyte *v)
{
	fprintf( glw_state.log_fp, "glColor3bv\n" );
	dllColor3bv( v );
}

static void APIENTRY logColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
	fprintf( glw_state.log_fp, "glColor3d\n" );
	dllColor3d( red, green, blue );
}

static void APIENTRY logColor3dv(const GLdouble *v)
{
	fprintf( glw_state.log_fp, "glColor3dv\n" );
	dllColor3dv( v );
}

static void APIENTRY logColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	fprintf( glw_state.log_fp, "glColor3f\n" );
	dllColor3f( red, green, blue );
}

static void APIENTRY logColor3fv(const GLfloat *v)
{
	fprintf( glw_state.log_fp, "glColor3fv\n" );
	dllColor3fv( v );
}

static void APIENTRY logColor3i(GLint red, GLint green, GLint blue)
{
	fprintf( glw_state.log_fp, "glColor3i\n" );
	dllColor3i( red, green, blue );
}

static void APIENTRY logColor3iv(const GLint *v)
{
	fprintf( glw_state.log_fp, "glColor3iv\n" );
	dllColor3iv( v );
}

static void APIENTRY logColor3s(GLshort red, GLshort green, GLshort blue)
{
	fprintf( glw_state.log_fp, "glColor3s\n" );
	dllColor3s( red, green, blue );
}

static void APIENTRY logColor3sv(const GLshort *v)
{
	fprintf( glw_state.log_fp, "glColor3sv\n" );
	dllColor3sv( v );
}

static void APIENTRY logColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	fprintf( glw_state.log_fp, "glColor3ub\n" );
	dllColor3ub( red, green, blue );
}

static void APIENTRY logColor3ubv(const GLubyte *v)
{
	fprintf( glw_state.log_fp, "glColor3ubv\n" );
	dllColor3ubv( v );
}

#define SIG( x ) fprintf( glw_state.log_fp, x "\n" )

static void APIENTRY logColor3ui(GLuint red, GLuint green, GLuint blue)
{
	SIG( "glColor3ui" );
	dllColor3ui( red, green, blue );
}

static void APIENTRY logColor3uiv(const GLuint *v)
{
	SIG( "glColor3uiv" );
	dllColor3uiv( v );
}

static void APIENTRY logColor3us(GLushort red, GLushort green, GLushort blue)
{
	SIG( "glColor3us" );
	dllColor3us( red, green, blue );
}

static void APIENTRY logColor3usv(const GLushort *v)
{
	SIG( "glColor3usv" );
	dllColor3usv( v );
}

static void APIENTRY logColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
	SIG( "glColor4b" );
	dllColor4b( red, green, blue, alpha );
}

static void APIENTRY logColor4bv(const GLbyte *v)
{
	SIG( "glColor4bv" );
	dllColor4bv( v );
}

static void APIENTRY logColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
	SIG( "glColor4d" );
	dllColor4d( red, green, blue, alpha );
}
static void APIENTRY logColor4dv(const GLdouble *v)
{
	SIG( "glColor4dv" );
	dllColor4dv( v );
}
static void APIENTRY logColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	fprintf( glw_state.log_fp, "glColor4f( %f,%f,%f,%f )\n", red, green, blue, alpha );
	dllColor4f( red, green, blue, alpha );
}
static void APIENTRY logColor4fv(const GLfloat *v)
{
	fprintf( glw_state.log_fp, "glColor4fv( %f,%f,%f,%f )\n", v[0], v[1], v[2], v[3] );
	dllColor4fv( v );
}
static void APIENTRY logColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
	SIG( "glColor4i" );
	dllColor4i( red, green, blue, alpha );
}
static void APIENTRY logColor4iv(const GLint *v)
{
	SIG( "glColor4iv" );
	dllColor4iv( v );
}
static void APIENTRY logColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
	SIG( "glColor4s" );
	dllColor4s( red, green, blue, alpha );
}
static void APIENTRY logColor4sv(const GLshort *v)
{
	SIG( "glColor4sv" );
	dllColor4sv( v );
}
static void APIENTRY logColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	SIG( "glColor4b" );
	dllColor4b( red, green, blue, alpha );
}
static void APIENTRY logColor4ubv(const GLubyte *v)
{
	SIG( "glColor4ubv" );
	dllColor4ubv( v );
}
static void APIENTRY logColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	SIG( "glColor4ui" );
	dllColor4ui( red, green, blue, alpha );
}
static void APIENTRY logColor4uiv(const GLuint *v)
{
	SIG( "glColor4uiv" );
	dllColor4uiv( v );
}
static void APIENTRY logColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
	SIG( "glColor4us" );
	dllColor4us( red, green, blue, alpha );
}
static void APIENTRY logColor4usv(const GLushort *v)
{
	SIG( "glColor4usv" );
	dllColor4usv( v );
}
static void APIENTRY logColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	SIG( "glColorMask" );
	dllColorMask( red, green, blue, alpha );
}
static void APIENTRY logColorMaterial(GLenum face, GLenum mode)
{
	SIG( "glColorMaterial" );
	dllColorMaterial( face, mode );
}

static void APIENTRY logColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glColorPointer" );
	dllColorPointer( size, type, stride, pointer );
}

static void APIENTRY logCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
	SIG( "glCopyPixels" );
	dllCopyPixels( x, y, width, height, type );
}

static void APIENTRY logCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
	SIG( "glCopyTexImage1D" );
	dllCopyTexImage1D( target, level, internalFormat, x, y, width, border );
}

static void APIENTRY logCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
	SIG( "glCopyTexImage2D" );
	dllCopyTexImage2D( target, level, internalFormat, x, y, width, height, border );
}

static void APIENTRY logCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
	SIG( "glCopyTexSubImage1D" );
	dllCopyTexSubImage1D( target, level, xoffset, x, y, width );
}

static void APIENTRY logCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glCopyTexSubImage2D" );
	dllCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height );
}

static void APIENTRY logCullFace(GLenum mode)
{
	SIG( "glCullFace" );
	dllCullFace( mode );
}

static void APIENTRY logDeleteLists(GLuint list, GLsizei range)
{
	SIG( "glDeleteLists" );
	dllDeleteLists( list, range );
}

static void APIENTRY logDeleteTextures(GLsizei n, const GLuint *textures)
{
	SIG( "glDeleteTextures" );
	dllDeleteTextures( n, textures );
}

static void APIENTRY logDepthFunc(GLenum func)
{
	SIG( "glDepthFunc" );
	dllDepthFunc( func );
}

static void APIENTRY logDepthMask(GLboolean flag)
{
	SIG( "glDepthMask" );
	dllDepthMask( flag );
}

static void APIENTRY logDepthRange(GLclampd zNear, GLclampd zFar)
{
	SIG( "glDepthRange" );
	dllDepthRange( zNear, zFar );
}

static void APIENTRY logDisable(GLenum cap)
{
	fprintf( glw_state.log_fp, "glDisable( 0x%x )\n", cap );
	dllDisable( cap );
}

static void APIENTRY logDisableClientState(GLenum array)
{
	SIG( "glDisableClientState" );
	dllDisableClientState( array );
}

static void APIENTRY logDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	SIG( "glDrawArrays" );
	dllDrawArrays( mode, first, count );
}

static void APIENTRY logDrawBuffer(GLenum mode)
{
	SIG( "glDrawBuffer" );
	dllDrawBuffer( mode );
}

static void APIENTRY logDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
	SIG( "glDrawElements" );
	dllDrawElements( mode, count, type, indices );
}

static void APIENTRY logDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glDrawPixels" );
	dllDrawPixels( width, height, format, type, pixels );
}

static void APIENTRY logEdgeFlag(GLboolean flag)
{
	SIG( "glEdgeFlag" );
	dllEdgeFlag( flag );
}

static void APIENTRY logEdgeFlagPointer(GLsizei stride, const void *pointer)
{
	SIG( "glEdgeFlagPointer" );
	dllEdgeFlagPointer( stride, pointer );
}

static void APIENTRY logEdgeFlagv(const GLboolean *flag)
{
	SIG( "glEdgeFlagv" );
	dllEdgeFlagv( flag );
}

static void APIENTRY logEnable(GLenum cap)
{
	fprintf( glw_state.log_fp, "glEnable( 0x%x )\n", cap );
	dllEnable( cap );
}

static void APIENTRY logEnableClientState(GLenum array)
{
	SIG( "glEnableClientState" );
	dllEnableClientState( array );
}

static void APIENTRY logEnd(void)
{
	SIG( "glEnd" );
	dllEnd();
}

static void APIENTRY logEndList(void)
{
	SIG( "glEndList" );
	dllEndList();
}

static void APIENTRY logEvalCoord1d(GLdouble u)
{
	SIG( "glEvalCoord1d" );
	dllEvalCoord1d( u );
}

static void APIENTRY logEvalCoord1dv(const GLdouble *u)
{
	SIG( "glEvalCoord1dv" );
	dllEvalCoord1dv( u );
}

static void APIENTRY logEvalCoord1f(GLfloat u)
{
	SIG( "glEvalCoord1f" );
	dllEvalCoord1f( u );
}

static void APIENTRY logEvalCoord1fv(const GLfloat *u)
{
	SIG( "glEvalCoord1fv" );
	dllEvalCoord1fv( u );
}
static void APIENTRY logEvalCoord2d(GLdouble u, GLdouble v)
{
	SIG( "glEvalCoord2d" );
	dllEvalCoord2d( u, v );
}
static void APIENTRY logEvalCoord2dv(const GLdouble *u)
{
	SIG( "glEvalCoord2dv" );
	dllEvalCoord2dv( u );
}
static void APIENTRY logEvalCoord2f(GLfloat u, GLfloat v)
{
	SIG( "glEvalCoord2f" );
	dllEvalCoord2f( u, v );
}
static void APIENTRY logEvalCoord2fv(const GLfloat *u)
{
	SIG( "glEvalCoord2fv" );
	dllEvalCoord2fv( u );
}

static void APIENTRY logEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
	SIG( "glEvalMesh1" );
	dllEvalMesh1( mode, i1, i2 );
}
static void APIENTRY logEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
	SIG( "glEvalMesh2" );
	dllEvalMesh2( mode, i1, i2, j1, j2 );
}
static void APIENTRY logEvalPoint1(GLint i)
{
	SIG( "glEvalPoint1" );
	dllEvalPoint1( i );
}
static void APIENTRY logEvalPoint2(GLint i, GLint j)
{
	SIG( "glEvalPoint2" );
	dllEvalPoint2( i, j );
}

static void APIENTRY logFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
	SIG( "glFeedbackBuffer" );
	dllFeedbackBuffer( size, type, buffer );
}

static void APIENTRY logFinish(void)
{
	SIG( "glFinish" );
	dllFinish();
}

static void APIENTRY logFlush(void)
{
	SIG( "glFlush" );
	dllFlush();
}

static void APIENTRY logFogf(GLenum pname, GLfloat param)
{
	SIG( "glFogf" );
	dllFogf( pname, param );
}

static void APIENTRY logFogfv(GLenum pname, const GLfloat *params)
{
	SIG( "glFogfv" );
	dllFogfv( pname, params );
}

static void APIENTRY logFogi(GLenum pname, GLint param)
{
	SIG( "glFogi" );
	dllFogi( pname, param );
}

static void APIENTRY logFogiv(GLenum pname, const GLint *params)
{
	SIG( "glFogiv" );
	dllFogiv( pname, params );
}

static void APIENTRY logFrontFace(GLenum mode)
{
	SIG( "glFrontFace" );
	dllFrontFace( mode );
}

static void APIENTRY logFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	SIG( "glFrustum" );
	dllFrustum( left, right, bottom, top, zNear, zFar );
}

static GLuint APIENTRY logGenLists(GLsizei range)
{
	SIG( "glGenLists" );
	return dllGenLists( range );
}

static void APIENTRY logGenTextures(GLsizei n, GLuint *textures)
{
	SIG( "glGenTextures" );
	dllGenTextures( n, textures );
}

static void APIENTRY logGetBooleanv(GLenum pname, GLboolean *params)
{
	SIG( "glGetBooleanv" );
	dllGetBooleanv( pname, params );
}

static void APIENTRY logGetClipPlane(GLenum plane, GLdouble *equation)
{
	SIG( "glGetClipPlane" );
	dllGetClipPlane( plane, equation );
}

static void APIENTRY logGetDoublev(GLenum pname, GLdouble *params)
{
	SIG( "glGetDoublev" );
	dllGetDoublev( pname, params );
}

static GLenum APIENTRY logGetError(void)
{
	SIG( "glGetError" );
	return dllGetError();
}

static void APIENTRY logGetFloatv(GLenum pname, GLfloat *params)
{
	SIG( "glGetFloatv" );
	dllGetFloatv( pname, params );
}

static void APIENTRY logGetIntegerv(GLenum pname, GLint *params)
{
	SIG( "glGetIntegerv" );
	dllGetIntegerv( pname, params );
}

static void APIENTRY logGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
	SIG( "glGetLightfv" );
	dllGetLightfv( light, pname, params );
}

static void APIENTRY logGetLightiv(GLenum light, GLenum pname, GLint *params)
{
	SIG( "glGetLightiv" );
	dllGetLightiv( light, pname, params );
}

static void APIENTRY logGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
	SIG( "glGetMapdv" );
	dllGetMapdv( target, query, v );
}

static void APIENTRY logGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
	SIG( "glGetMapfv" );
	dllGetMapfv( target, query, v );
}

static void APIENTRY logGetMapiv(GLenum target, GLenum query, GLint *v)
{
	SIG( "glGetMapiv" );
	dllGetMapiv( target, query, v );
}

static void APIENTRY logGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
	SIG( "glGetMaterialfv" );
	dllGetMaterialfv( face, pname, params );
}

static void APIENTRY logGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
	SIG( "glGetMaterialiv" );
	dllGetMaterialiv( face, pname, params );
}

static void APIENTRY logGetPixelMapfv(GLenum map, GLfloat *values)
{
	SIG( "glGetPixelMapfv" );
	dllGetPixelMapfv( map, values );
}

static void APIENTRY logGetPixelMapuiv(GLenum map, GLuint *values)
{
	SIG( "glGetPixelMapuiv" );
	dllGetPixelMapuiv( map, values );
}

static void APIENTRY logGetPixelMapusv(GLenum map, GLushort *values)
{
	SIG( "glGetPixelMapusv" );
	dllGetPixelMapusv( map, values );
}

static void APIENTRY logGetPointerv(GLenum pname, GLvoid* *params)
{
	SIG( "glGetPointerv" );
	dllGetPointerv( pname, params );
}

static void APIENTRY logGetPolygonStipple(GLubyte *mask)
{
	SIG( "glGetPolygonStipple" );
	dllGetPolygonStipple( mask );
}

static const GLubyte * APIENTRY logGetString(GLenum name)
{
	SIG( "glGetString" );
	return dllGetString( name );
}

static void APIENTRY logGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexEnvfv" );
	dllGetTexEnvfv( target, pname, params );
}

static void APIENTRY logGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
	SIG( "glGetTexEnviv" );
	dllGetTexEnviv( target, pname, params );
}

static void APIENTRY logGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
	SIG( "glGetTexGendv" );
	dllGetTexGendv( coord, pname, params );
}

static void APIENTRY logGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexGenfv" );
	dllGetTexGenfv( coord, pname, params );
}

static void APIENTRY logGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
	SIG( "glGetTexGeniv" );
	dllGetTexGeniv( coord, pname, params );
}

static void APIENTRY logGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
	SIG( "glGetTexImage" );
	dllGetTexImage( target, level, format, type, pixels );
}
static void APIENTRY logGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params )
{
	SIG( "glGetTexLevelParameterfv" );
	dllGetTexLevelParameterfv( target, level, pname, params );
}

static void APIENTRY logGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
	SIG( "glGetTexLevelParameteriv" );
	dllGetTexLevelParameteriv( target, level, pname, params );
}

static void APIENTRY logGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	SIG( "glGetTexParameterfv" );
	dllGetTexParameterfv( target, pname, params );
}

static void APIENTRY logGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	SIG( "glGetTexParameteriv" );
	dllGetTexParameteriv( target, pname, params );
}

static void APIENTRY logHint(GLenum target, GLenum mode)
{
	fprintf( glw_state.log_fp, "glHint( 0x%x, 0x%x )\n", target, mode );
	dllHint( target, mode );
}

static void APIENTRY logIndexMask(GLuint mask)
{
	SIG( "glIndexMask" );
	dllIndexMask( mask );
}

static void APIENTRY logIndexPointer(GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glIndexPointer" );
	dllIndexPointer( type, stride, pointer );
}

static void APIENTRY logIndexd(GLdouble c)
{
	SIG( "glIndexd" );
	dllIndexd( c );
}

static void APIENTRY logIndexdv(const GLdouble *c)
{
	SIG( "glIndexdv" );
	dllIndexdv( c );
}

static void APIENTRY logIndexf(GLfloat c)
{
	SIG( "glIndexf" );
	dllIndexf( c );
}

static void APIENTRY logIndexfv(const GLfloat *c)
{
	SIG( "glIndexfv" );
	dllIndexfv( c );
}

static void APIENTRY logIndexi(GLint c)
{
	SIG( "glIndexi" );
	dllIndexi( c );
}

static void APIENTRY logIndexiv(const GLint *c)
{
	SIG( "glIndexiv" );
	dllIndexiv( c );
}

static void APIENTRY logIndexs(GLshort c)
{
	SIG( "glIndexs" );
	dllIndexs( c );
}

static void APIENTRY logIndexsv(const GLshort *c)
{
	SIG( "glIndexsv" );
	dllIndexsv( c );
}

static void APIENTRY logIndexub(GLubyte c)
{
	SIG( "glIndexub" );
	dllIndexub( c );
}

static void APIENTRY logIndexubv(const GLubyte *c)
{
	SIG( "glIndexubv" );
	dllIndexubv( c );
}

static void APIENTRY logInitNames(void)
{
	SIG( "glInitNames" );
	dllInitNames();
}

static void APIENTRY logInterleavedArrays(GLenum format, GLsizei stride, const void *pointer)
{
	SIG( "glInterleavedArrays" );
	dllInterleavedArrays( format, stride, pointer );
}

static GLboolean APIENTRY logIsEnabled(GLenum cap)
{
	SIG( "glIsEnabled" );
	return dllIsEnabled( cap );
}
static GLboolean APIENTRY logIsList(GLuint list)
{
	SIG( "glIsList" );
	return dllIsList( list );
}
static GLboolean APIENTRY logIsTexture(GLuint texture)
{
	SIG( "glIsTexture" );
	return dllIsTexture( texture );
}

static void APIENTRY logLightModelf(GLenum pname, GLfloat param)
{
	SIG( "glLightModelf" );
	dllLightModelf( pname, param );
}

static void APIENTRY logLightModelfv(GLenum pname, const GLfloat *params)
{
	SIG( "glLightModelfv" );
	dllLightModelfv( pname, params );
}

static void APIENTRY logLightModeli(GLenum pname, GLint param)
{
	SIG( "glLightModeli" );
	dllLightModeli( pname, param );

}

static void APIENTRY logLightModeliv(GLenum pname, const GLint *params)
{
	SIG( "glLightModeliv" );
	dllLightModeliv( pname, params );
}

static void APIENTRY logLightf(GLenum light, GLenum pname, GLfloat param)
{
	SIG( "glLightf" );
	dllLightf( light, pname, param );
}

static void APIENTRY logLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	SIG( "glLightfv" );
	dllLightfv( light, pname, params );
}

static void APIENTRY logLighti(GLenum light, GLenum pname, GLint param)
{
	SIG( "glLighti" );
	dllLighti( light, pname, param );
}

static void APIENTRY logLightiv(GLenum light, GLenum pname, const GLint *params)
{
	SIG( "glLightiv" );
	dllLightiv( light, pname, params );
}

static void APIENTRY logLineStipple(GLint factor, GLushort pattern)
{
	SIG( "glLineStipple" );
	dllLineStipple( factor, pattern );
}

static void APIENTRY logLineWidth(GLfloat width)
{
	SIG( "glLineWidth" );
	dllLineWidth( width );
}

static void APIENTRY logListBase(GLuint base)
{
	SIG( "glListBase" );
	dllListBase( base );
}

static void APIENTRY logLoadIdentity(void)
{
	SIG( "glLoadIdentity" );
	dllLoadIdentity();
}

static void APIENTRY logLoadMatrixd(const GLdouble *m)
{
	SIG( "glLoadMatrixd" );
	dllLoadMatrixd( m );
}

static void APIENTRY logLoadMatrixf(const GLfloat *m)
{
	SIG( "glLoadMatrixf" );
	dllLoadMatrixf( m );
}

static void APIENTRY logLoadName(GLuint name)
{
	SIG( "glLoadName" );
	dllLoadName( name );
}

static void APIENTRY logLogicOp(GLenum opcode)
{
	SIG( "glLogicOp" );
	dllLogicOp( opcode );
}

static void APIENTRY logMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
	SIG( "glMap1d" );
	dllMap1d( target, u1, u2, stride, order, points );
}

static void APIENTRY logMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
	SIG( "glMap1f" );
	dllMap1f( target, u1, u2, stride, order, points );
}

static void APIENTRY logMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
	SIG( "glMap2d" );
	dllMap2d( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

static void APIENTRY logMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
	SIG( "glMap2f" );
	dllMap2f( target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points );
}

static void APIENTRY logMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
	SIG( "glMapGrid1d" );
	dllMapGrid1d( un, u1, u2 );
}

static void APIENTRY logMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
	SIG( "glMapGrid1f" );
	dllMapGrid1f( un, u1, u2 );
}

static void APIENTRY logMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
	SIG( "glMapGrid2d" );
	dllMapGrid2d( un, u1, u2, vn, v1, v2 );
}
static void APIENTRY logMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
	SIG( "glMapGrid2f" );
	dllMapGrid2f( un, u1, u2, vn, v1, v2 );
}
static void APIENTRY logMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	SIG( "glMaterialf" );
	dllMaterialf( face, pname, param );
}
static void APIENTRY logMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	SIG( "glMaterialfv" );
	dllMaterialfv( face, pname, params );
}

static void APIENTRY logMateriali(GLenum face, GLenum pname, GLint param)
{
	SIG( "glMateriali" );
	dllMateriali( face, pname, param );
}

static void APIENTRY logMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
	SIG( "glMaterialiv" );
	dllMaterialiv( face, pname, params );
}

static void APIENTRY logMatrixMode(GLenum mode)
{
	SIG( "glMatrixMode" );
	dllMatrixMode( mode );
}

static void APIENTRY logMultMatrixd(const GLdouble *m)
{
	SIG( "glMultMatrixd" );
	dllMultMatrixd( m );
}

static void APIENTRY logMultMatrixf(const GLfloat *m)
{
	SIG( "glMultMatrixf" );
	dllMultMatrixf( m );
}

static void APIENTRY logNewList(GLuint list, GLenum mode)
{
	SIG( "glNewList" );
	dllNewList( list, mode );
}

static void APIENTRY logNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
	SIG ("glNormal3b" );
	dllNormal3b( nx, ny, nz );
}

static void APIENTRY logNormal3bv(const GLbyte *v)
{
	SIG( "glNormal3bv" );
	dllNormal3bv( v );
}

static void APIENTRY logNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
	SIG( "glNormal3d" );
	dllNormal3d( nx, ny, nz );
}

static void APIENTRY logNormal3dv(const GLdouble *v)
{
	SIG( "glNormal3dv" );
	dllNormal3dv( v );
}

static void APIENTRY logNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	SIG( "glNormal3f" );
	dllNormal3f( nx, ny, nz );
}

static void APIENTRY logNormal3fv(const GLfloat *v)
{
	SIG( "glNormal3fv" );
	dllNormal3fv( v );
}
static void APIENTRY logNormal3i(GLint nx, GLint ny, GLint nz)
{
	SIG( "glNormal3i" );
	dllNormal3i( nx, ny, nz );
}
static void APIENTRY logNormal3iv(const GLint *v)
{
	SIG( "glNormal3iv" );
	dllNormal3iv( v );
}
static void APIENTRY logNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
	SIG( "glNormal3s" );
	dllNormal3s( nx, ny, nz );
}
static void APIENTRY logNormal3sv(const GLshort *v)
{
	SIG( "glNormal3sv" );
	dllNormal3sv( v );
}
static void APIENTRY logNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glNormalPointer" );
	dllNormalPointer( type, stride, pointer );
}
static void APIENTRY logOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
	SIG( "glOrtho" );
	dllOrtho( left, right, bottom, top, zNear, zFar );
}

static void APIENTRY logPassThrough(GLfloat token)
{
	SIG( "glPassThrough" );
	dllPassThrough( token );
}

static void APIENTRY logPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values)
{
	SIG( "glPixelMapfv" );
	dllPixelMapfv( map, mapsize, values );
}

static void APIENTRY logPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values)
{
	SIG( "glPixelMapuiv" );
	dllPixelMapuiv( map, mapsize, values );
}

static void APIENTRY logPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values)
{
	SIG( "glPixelMapusv" );
	dllPixelMapusv( map, mapsize, values );
}
static void APIENTRY logPixelStoref(GLenum pname, GLfloat param)
{
	SIG( "glPixelStoref" );
	dllPixelStoref( pname, param );
}
static void APIENTRY logPixelStorei(GLenum pname, GLint param)
{
	SIG( "glPixelStorei" );
	dllPixelStorei( pname, param );
}
static void APIENTRY logPixelTransferf(GLenum pname, GLfloat param)
{
	SIG( "glPixelTransferf" );
	dllPixelTransferf( pname, param );
}

static void APIENTRY logPixelTransferi(GLenum pname, GLint param)
{
	SIG( "glPixelTransferi" );
	dllPixelTransferi( pname, param );
}

static void APIENTRY logPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
	SIG( "glPixelZoom" );
	dllPixelZoom( xfactor, yfactor );
}

static void APIENTRY logPointSize(GLfloat size)
{
	SIG( "glPointSize" );
	dllPointSize( size );
}

static void APIENTRY logPolygonMode(GLenum face, GLenum mode)
{
	fprintf( glw_state.log_fp, "glPolygonMode( 0x%x, 0x%x )\n", face, mode );
	dllPolygonMode( face, mode );
}

static void APIENTRY logPolygonOffset(GLfloat factor, GLfloat units)
{
	SIG( "glPolygonOffset" );
	dllPolygonOffset( factor, units );
}
static void APIENTRY logPolygonStipple(const GLubyte *mask )
{
	SIG( "glPolygonStipple" );
	dllPolygonStipple( mask );
}
static void APIENTRY logPopAttrib(void)
{
	SIG( "glPopAttrib" );
	dllPopAttrib();
}

static void APIENTRY logPopClientAttrib(void)
{
	SIG( "glPopClientAttrib" );
	dllPopClientAttrib();
}

static void APIENTRY logPopMatrix(void)
{
	SIG( "glPopMatrix" );
	dllPopMatrix();
}

static void APIENTRY logPopName(void)
{
	SIG( "glPopName" );
	dllPopName();
}

static void APIENTRY logPrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
	SIG( "glPrioritizeTextures" );
	dllPrioritizeTextures( n, textures, priorities );
}

static void APIENTRY logPushAttrib(GLbitfield mask)
{
	SIG( "glPushAttrib" );
	dllPushAttrib( mask );
}

static void APIENTRY logPushClientAttrib(GLbitfield mask)
{
	SIG( "glPushClientAttrib" );
	dllPushClientAttrib( mask );
}

static void APIENTRY logPushMatrix(void)
{
	SIG( "glPushMatrix" );
	dllPushMatrix();
}

static void APIENTRY logPushName(GLuint name)
{
	SIG( "glPushName" );
	dllPushName( name );
}

static void APIENTRY logRasterPos2d(GLdouble x, GLdouble y)
{
	SIG ("glRasterPot2d" );
	dllRasterPos2d( x, y );
}

static void APIENTRY logRasterPos2dv(const GLdouble *v)
{
	SIG( "glRasterPos2dv" );
	dllRasterPos2dv( v );
}

static void APIENTRY logRasterPos2f(GLfloat x, GLfloat y)
{
	SIG( "glRasterPos2f" );
	dllRasterPos2f( x, y );
}
static void APIENTRY logRasterPos2fv(const GLfloat *v)
{
	SIG( "glRasterPos2dv" );
	dllRasterPos2fv( v );
}
static void APIENTRY logRasterPos2i(GLint x, GLint y)
{
	SIG( "glRasterPos2if" );
	dllRasterPos2i( x, y );
}
static void APIENTRY logRasterPos2iv(const GLint *v)
{
	SIG( "glRasterPos2iv" );
	dllRasterPos2iv( v );
}
static void APIENTRY logRasterPos2s(GLshort x, GLshort y)
{
	SIG( "glRasterPos2s" );
	dllRasterPos2s( x, y );
}
static void APIENTRY logRasterPos2sv(const GLshort *v)
{
	SIG( "glRasterPos2sv" );
	dllRasterPos2sv( v );
}
static void APIENTRY logRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glRasterPos3d" );
	dllRasterPos3d( x, y, z );
}
static void APIENTRY logRasterPos3dv(const GLdouble *v)
{
	SIG( "glRasterPos3dv" );
	dllRasterPos3dv( v );
}
static void APIENTRY logRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glRasterPos3f" );
	dllRasterPos3f( x, y, z );
}
static void APIENTRY logRasterPos3fv(const GLfloat *v)
{
	SIG( "glRasterPos3fv" );
	dllRasterPos3fv( v );
}
static void APIENTRY logRasterPos3i(GLint x, GLint y, GLint z)
{
	SIG( "glRasterPos3i" );
	dllRasterPos3i( x, y, z );
}
static void APIENTRY logRasterPos3iv(const GLint *v)
{
	SIG( "glRasterPos3iv" );
	dllRasterPos3iv( v );
}
static void APIENTRY logRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	SIG( "glRasterPos3s" );
	dllRasterPos3s( x, y, z );
}
static void APIENTRY logRasterPos3sv(const GLshort *v)
{
	SIG( "glRasterPos3sv" );
	dllRasterPos3sv( v );
}
static void APIENTRY logRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	SIG( "glRasterPos4d" );
	dllRasterPos4d( x, y, z, w );
}
static void APIENTRY logRasterPos4dv(const GLdouble *v)
{
	SIG( "glRasterPos4dv" );
	dllRasterPos4dv( v );
}
static void APIENTRY logRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	SIG( "glRasterPos4f" );
	dllRasterPos4f( x, y, z, w );
}
static void APIENTRY logRasterPos4fv(const GLfloat *v)
{
	SIG( "glRasterPos4fv" );
	dllRasterPos4fv( v );
}
static void APIENTRY logRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	SIG( "glRasterPos4i" );
	dllRasterPos4i( x, y, z, w );
}
static void APIENTRY logRasterPos4iv(const GLint *v)
{
	SIG( "glRasterPos4iv" );
	dllRasterPos4iv( v );
}
static void APIENTRY logRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	SIG( "glRasterPos4s" );
	dllRasterPos4s( x, y, z, w );
}
static void APIENTRY logRasterPos4sv(const GLshort *v)
{
	SIG( "glRasterPos4sv" );
	dllRasterPos4sv( v );
}
static void APIENTRY logReadBuffer(GLenum mode)
{
	SIG( "glReadBuffer" );
	dllReadBuffer( mode );
}
static void APIENTRY logReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
	SIG( "glReadPixels" );
	dllReadPixels( x, y, width, height, format, type, pixels );
}

static void APIENTRY logRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	SIG( "glRectd" );
	dllRectd( x1, y1, x2, y2 );
}

static void APIENTRY logRectdv(const GLdouble *v1, const GLdouble *v2)
{
	SIG( "glRectdv" );
	dllRectdv( v1, v2 );
}

static void APIENTRY logRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	SIG( "glRectf" );
	dllRectf( x1, y1, x2, y2 );
}

static void APIENTRY logRectfv(const GLfloat *v1, const GLfloat *v2)
{
	SIG( "glRectfv" );
	dllRectfv( v1, v2 );
}
static void APIENTRY logRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	SIG( "glRecti" );
	dllRecti( x1, y1, x2, y2 );
}
static void APIENTRY logRectiv(const GLint *v1, const GLint *v2)
{
	SIG( "glRectiv" );
	dllRectiv( v1, v2 );
}
static void APIENTRY logRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	SIG( "glRects" );
	dllRects( x1, y1, x2, y2 );
}
static void APIENTRY logRectsv(const GLshort *v1, const GLshort *v2)
{
	SIG( "glRectsv" );
	dllRectsv( v1, v2 );
}
static GLint APIENTRY logRenderMode(GLenum mode)
{
	SIG( "glRenderMode" );
	return dllRenderMode( mode );
}
static void APIENTRY logRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glRotated" );
	dllRotated( angle, x, y, z );
}

static void APIENTRY logRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glRotatef" );
	dllRotatef( angle, x, y, z );
}

static void APIENTRY logScaled(GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glScaled" );
	dllScaled( x, y, z );
}

static void APIENTRY logScalef(GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glScalef" );
	dllScalef( x, y, z );
}

static void APIENTRY logScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glScissor" );
	dllScissor( x, y, width, height );
}

static void APIENTRY logSelectBuffer(GLsizei size, GLuint *buffer)
{
	SIG( "glSelectBuffer" );
	dllSelectBuffer( size, buffer );
}

static void APIENTRY logShadeModel(GLenum mode)
{
	SIG( "glShadeModel" );
	dllShadeModel( mode );
}

static void APIENTRY logStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	SIG( "glStencilFunc" );
	dllStencilFunc( func, ref, mask );
}

static void APIENTRY logStencilMask(GLuint mask)
{
	SIG( "glStencilMask" );
	dllStencilMask( mask );
}

static void APIENTRY logStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	SIG( "glStencilOp" );
	dllStencilOp( fail, zfail, zpass );
}

static void APIENTRY logTexCoord1d(GLdouble s)
{
	SIG( "glTexCoord1d" );
	dllTexCoord1d( s );
}

static void APIENTRY logTexCoord1dv(const GLdouble *v)
{
	SIG( "glTexCoord1dv" );
	dllTexCoord1dv( v );
}

static void APIENTRY logTexCoord1f(GLfloat s)
{
	SIG( "glTexCoord1f" );
	dllTexCoord1f( s );
}
static void APIENTRY logTexCoord1fv(const GLfloat *v)
{
	SIG( "glTexCoord1fv" );
	dllTexCoord1fv( v );
}
static void APIENTRY logTexCoord1i(GLint s)
{
	SIG( "glTexCoord1i" );
	dllTexCoord1i( s );
}
static void APIENTRY logTexCoord1iv(const GLint *v)
{
	SIG( "glTexCoord1iv" );
	dllTexCoord1iv( v );
}
static void APIENTRY logTexCoord1s(GLshort s)
{
	SIG( "glTexCoord1s" );
	dllTexCoord1s( s );
}
static void APIENTRY logTexCoord1sv(const GLshort *v)
{
	SIG( "glTexCoord1sv" );
	dllTexCoord1sv( v );
}
static void APIENTRY logTexCoord2d(GLdouble s, GLdouble t)
{
	SIG( "glTexCoord2d" );
	dllTexCoord2d( s, t );
}

static void APIENTRY logTexCoord2dv(const GLdouble *v)
{
	SIG( "glTexCoord2dv" );
	dllTexCoord2dv( v );
}
static void APIENTRY logTexCoord2f(GLfloat s, GLfloat t)
{
	SIG( "glTexCoord2f" );
	dllTexCoord2f( s, t );
}
static void APIENTRY logTexCoord2fv(const GLfloat *v)
{
	SIG( "glTexCoord2fv" );
	dllTexCoord2fv( v );
}
static void APIENTRY logTexCoord2i(GLint s, GLint t)
{
	SIG( "glTexCoord2i" );
	dllTexCoord2i( s, t );
}
static void APIENTRY logTexCoord2iv(const GLint *v)
{
	SIG( "glTexCoord2iv" );
	dllTexCoord2iv( v );
}
static void APIENTRY logTexCoord2s(GLshort s, GLshort t)
{
	SIG( "glTexCoord2s" );
	dllTexCoord2s( s, t );
}
static void APIENTRY logTexCoord2sv(const GLshort *v)
{
	SIG( "glTexCoord2sv" );
	dllTexCoord2sv( v );
}
static void APIENTRY logTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	SIG( "glTexCoord3d" );
	dllTexCoord3d( s, t, r );
}
static void APIENTRY logTexCoord3dv(const GLdouble *v)
{
	SIG( "glTexCoord3dv" );
	dllTexCoord3dv( v );
}
static void APIENTRY logTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	SIG( "glTexCoord3f" );
	dllTexCoord3f( s, t, r );
}
static void APIENTRY logTexCoord3fv(const GLfloat *v)
{
	SIG( "glTexCoord3fv" );
	dllTexCoord3fv( v );
}
static void APIENTRY logTexCoord3i(GLint s, GLint t, GLint r)
{
	SIG( "glTexCoord3i" );
	dllTexCoord3i( s, t, r );
}
static void APIENTRY logTexCoord3iv(const GLint *v)
{
	SIG( "glTexCoord3iv" );
	dllTexCoord3iv( v );
}
static void APIENTRY logTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	SIG( "glTexCoord3s" );
	dllTexCoord3s( s, t, r );
}
static void APIENTRY logTexCoord3sv(const GLshort *v)
{
	SIG( "glTexCoord3sv" );
	dllTexCoord3sv( v );
}
static void APIENTRY logTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	SIG( "glTexCoord4d" );
	dllTexCoord4d( s, t, r, q );
}
static void APIENTRY logTexCoord4dv(const GLdouble *v)
{
	SIG( "glTexCoord4dv" );
	dllTexCoord4dv( v );
}
static void APIENTRY logTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	SIG( "glTexCoord4f" );
	dllTexCoord4f( s, t, r, q );
}
static void APIENTRY logTexCoord4fv(const GLfloat *v)
{
	SIG( "glTexCoord4fv" );
	dllTexCoord4fv( v );
}
static void APIENTRY logTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	SIG( "glTexCoord4i" );
	dllTexCoord4i( s, t, r, q );
}
static void APIENTRY logTexCoord4iv(const GLint *v)
{
	SIG( "glTexCoord4iv" );
	dllTexCoord4iv( v );
}
static void APIENTRY logTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	SIG( "glTexCoord4s" );
	dllTexCoord4s( s, t, r, q );
}
static void APIENTRY logTexCoord4sv(const GLshort *v)
{
	SIG( "glTexCoord4sv" );
	dllTexCoord4sv( v );
}
static void APIENTRY logTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glTexCoordPointer" );
	dllTexCoordPointer( size, type, stride, pointer );
}

static void APIENTRY logTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
	fprintf( glw_state.log_fp, "glTexEnvf( 0x%x, 0x%x, %f )\n", target, pname, param );
	dllTexEnvf( target, pname, param );
}

static void APIENTRY logTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
	SIG( "glTexEnvfv" );
	dllTexEnvfv( target, pname, params );
}

static void APIENTRY logTexEnvi(GLenum target, GLenum pname, GLint param)
{
	fprintf( glw_state.log_fp, "glTexEnvi( 0x%x, 0x%x, 0x%x )\n", target, pname, param );
	dllTexEnvi( target, pname, param );
}
static void APIENTRY logTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
	SIG( "glTexEnviv" );
	dllTexEnviv( target, pname, params );
}

static void APIENTRY logTexGend(GLenum coord, GLenum pname, GLdouble param)
{
	SIG( "glTexGend" );
	dllTexGend( coord, pname, param );
}

static void APIENTRY logTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
	SIG( "glTexGendv" );
	dllTexGendv( coord, pname, params );
}

static void APIENTRY logTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
	SIG( "glTexGenf" );
	dllTexGenf( coord, pname, param );
}
static void APIENTRY logTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
	SIG( "glTexGenfv" );
	dllTexGenfv( coord, pname, params );
}
static void APIENTRY logTexGeni(GLenum coord, GLenum pname, GLint param)
{
	SIG( "glTexGeni" );
	dllTexGeni( coord, pname, param );
}
static void APIENTRY logTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
	SIG( "glTexGeniv" );
	dllTexGeniv( coord, pname, params );
}
static void APIENTRY logTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glTexImage1D" );
	dllTexImage1D( target, level, internalformat, width, border, format, type, pixels );
}
static void APIENTRY logTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glTexImage2D" );
	dllTexImage2D( target, level, internalformat, width, height, border, format, type, pixels );
}

static void APIENTRY logTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	fprintf( glw_state.log_fp, "glTexParameterf( 0x%x, 0x%x, %f )\n", target, pname, param );
	dllTexParameterf( target, pname, param );
}

static void APIENTRY logTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
	SIG( "glTexParameterfv" );
	dllTexParameterfv( target, pname, params );
}
static void APIENTRY logTexParameteri(GLenum target, GLenum pname, GLint param)
{
	fprintf( glw_state.log_fp, "glTexParameteri( 0x%x, 0x%x, 0x%x )\n", target, pname, param );
	dllTexParameteri( target, pname, param );
}
static void APIENTRY logTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
	SIG( "glTexParameteriv" );
	dllTexParameteriv( target, pname, params );
}
static void APIENTRY logTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glTexSubImage1D" );
	dllTexSubImage1D( target, level, xoffset, width, format, type, pixels );
}
static void APIENTRY logTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
	SIG( "glTexSubImage2D" );
	dllTexSubImage2D( target, level, xoffset, yoffset, width, height, format, type, pixels );
}
static void APIENTRY logTranslated(GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glTranslated" );
	dllTranslated( x, y, z );
}

static void APIENTRY logTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glTranslatef" );
	dllTranslatef( x, y, z );
}

static void APIENTRY logVertex2d(GLdouble x, GLdouble y)
{
	SIG( "glVertex2d" );
	dllVertex2d( x, y );
}

static void APIENTRY logVertex2dv(const GLdouble *v)
{
	SIG( "glVertex2dv" );
	dllVertex2dv( v );
}
static void APIENTRY logVertex2f(GLfloat x, GLfloat y)
{
	SIG( "glVertex2f" );
	dllVertex2f( x, y );
}
static void APIENTRY logVertex2fv(const GLfloat *v)
{
	SIG( "glVertex2fv" );
	dllVertex2fv( v );
}
static void APIENTRY logVertex2i(GLint x, GLint y)
{
	SIG( "glVertex2i" );
	dllVertex2i( x, y );
}
static void APIENTRY logVertex2iv(const GLint *v)
{
	SIG( "glVertex2iv" );
	dllVertex2iv( v );
}
static void APIENTRY logVertex2s(GLshort x, GLshort y)
{
	SIG( "glVertex2s" );
	dllVertex2s( x, y );
}
static void APIENTRY logVertex2sv(const GLshort *v)
{
	SIG( "glVertex2sv" );
	dllVertex2sv( v );
}
static void APIENTRY logVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	SIG( "glVertex3d" );
	dllVertex3d( x, y, z );
}
static void APIENTRY logVertex3dv(const GLdouble *v)
{
	SIG( "glVertex3dv" );
	dllVertex3dv( v );
}
static void APIENTRY logVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	SIG( "glVertex3f" );
	dllVertex3f( x, y, z );
}
static void APIENTRY logVertex3fv(const GLfloat *v)
{
	SIG( "glVertex3fv" );
	dllVertex3fv( v );
}
static void APIENTRY logVertex3i(GLint x, GLint y, GLint z)
{
	SIG( "glVertex3i" );
	dllVertex3i( x, y, z );
}
static void APIENTRY logVertex3iv(const GLint *v)
{
	SIG( "glVertex3iv" );
	dllVertex3iv( v );
}
static void APIENTRY logVertex3s(GLshort x, GLshort y, GLshort z)
{
	SIG( "glVertex3s" );
	dllVertex3s( x, y, z );
}
static void APIENTRY logVertex3sv(const GLshort *v)
{
	SIG( "glVertex3sv" );
	dllVertex3sv( v );
}
static void APIENTRY logVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	SIG( "glVertex4d" );
	dllVertex4d( x, y, z, w );
}
static void APIENTRY logVertex4dv(const GLdouble *v)
{
	SIG( "glVertex4dv" );
	dllVertex4dv( v );
}
static void APIENTRY logVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	SIG( "glVertex4f" );
	dllVertex4f( x, y, z, w );
}
static void APIENTRY logVertex4fv(const GLfloat *v)
{
	SIG( "glVertex4fv" );
	dllVertex4fv( v );
}
static void APIENTRY logVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	SIG( "glVertex4i" );
	dllVertex4i( x, y, z, w );
}
static void APIENTRY logVertex4iv(const GLint *v)
{
	SIG( "glVertex4iv" );
	dllVertex4iv( v );
}
static void APIENTRY logVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	SIG( "glVertex4s" );
	dllVertex4s( x, y, z, w );
}
static void APIENTRY logVertex4sv(const GLshort *v)
{
	SIG( "glVertex4sv" );
	dllVertex4sv( v );
}
static void APIENTRY logVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	SIG( "glVertexPointer" );
	dllVertexPointer( size, type, stride, pointer );
}
static void APIENTRY logViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	SIG( "glViewport" );
	dllViewport( x, y, width, height );
}

/*
** QGL_Shutdown
**
** Unloads the specified DLL then nulls out all the proc pointers.
*/
void QGL_Shutdown( void )
{
	if ( glw_state.OpenGLLib )
	{
		dlclose ( glw_state.OpenGLLib );
		glw_state.OpenGLLib = NULL;
	}

	glw_state.OpenGLLib = NULL;

	qglAccum                     = NULL;
	qglAlphaFunc                 = NULL;
	qglAreTexturesResident       = NULL;
	qglArrayElement              = NULL;
	qglBegin                     = NULL;
	qglBindTexture               = NULL;
	qglBitmap                    = NULL;
	qglBlendFunc                 = NULL;
	qglCallList                  = NULL;
	qglCallLists                 = NULL;
	qglClear                     = NULL;
	qglClearAccum                = NULL;
	qglClearColor                = NULL;
	qglClearDepth                = NULL;
	qglClearIndex                = NULL;
	qglClearStencil              = NULL;
	qglClipPlane                 = NULL;
	qglColor3b                   = NULL;
	qglColor3bv                  = NULL;
	qglColor3d                   = NULL;
	qglColor3dv                  = NULL;
	qglColor3f                   = NULL;
	qglColor3fv                  = NULL;
	qglColor3i                   = NULL;
	qglColor3iv                  = NULL;
	qglColor3s                   = NULL;
	qglColor3sv                  = NULL;
	qglColor3ub                  = NULL;
	qglColor3ubv                 = NULL;
	qglColor3ui                  = NULL;
	qglColor3uiv                 = NULL;
	qglColor3us                  = NULL;
	qglColor3usv                 = NULL;
	qglColor4b                   = NULL;
	qglColor4bv                  = NULL;
	qglColor4d                   = NULL;
	qglColor4dv                  = NULL;
	qglColor4f                   = NULL;
	qglColor4fv                  = NULL;
	qglColor4i                   = NULL;
	qglColor4iv                  = NULL;
	qglColor4s                   = NULL;
	qglColor4sv                  = NULL;
	qglColor4ub                  = NULL;
	qglColor4ubv                 = NULL;
	qglColor4ui                  = NULL;
	qglColor4uiv                 = NULL;
	qglColor4us                  = NULL;
	qglColor4usv                 = NULL;
	qglColorMask                 = NULL;
	qglColorMaterial             = NULL;
	qglColorPointer              = NULL;
	qglCopyPixels                = NULL;
	qglCopyTexImage1D            = NULL;
	qglCopyTexImage2D            = NULL;
	qglCopyTexSubImage1D         = NULL;
	qglCopyTexSubImage2D         = NULL;
	qglCullFace                  = NULL;
	qglDeleteLists               = NULL;
	qglDeleteTextures            = NULL;
	qglDepthFunc                 = NULL;
	qglDepthMask                 = NULL;
	qglDepthRange                = NULL;
	qglDisable                   = NULL;
	qglDisableClientState        = NULL;
	qglDrawArrays                = NULL;
	qglDrawBuffer                = NULL;
	qglDrawElements              = NULL;
	qglDrawPixels                = NULL;
	qglEdgeFlag                  = NULL;
	qglEdgeFlagPointer           = NULL;
	qglEdgeFlagv                 = NULL;
	qglEnable                    = NULL;
	qglEnableClientState         = NULL;
	qglEnd                       = NULL;
	qglEndList                   = NULL;
	qglEvalCoord1d               = NULL;
	qglEvalCoord1dv              = NULL;
	qglEvalCoord1f               = NULL;
	qglEvalCoord1fv              = NULL;
	qglEvalCoord2d               = NULL;
	qglEvalCoord2dv              = NULL;
	qglEvalCoord2f               = NULL;
	qglEvalCoord2fv              = NULL;
	qglEvalMesh1                 = NULL;
	qglEvalMesh2                 = NULL;
	qglEvalPoint1                = NULL;
	qglEvalPoint2                = NULL;
	qglFeedbackBuffer            = NULL;
	qglFinish                    = NULL;
	qglFlush                     = NULL;
	qglFogf                      = NULL;
	qglFogfv                     = NULL;
	qglFogi                      = NULL;
	qglFogiv                     = NULL;
	qglFrontFace                 = NULL;
	qglFrustum                   = NULL;
	qglGenLists                  = NULL;
	qglGenTextures               = NULL;
	qglGetBooleanv               = NULL;
	qglGetClipPlane              = NULL;
	qglGetDoublev                = NULL;
	qglGetError                  = NULL;
	qglGetFloatv                 = NULL;
	qglGetIntegerv               = NULL;
	qglGetLightfv                = NULL;
	qglGetLightiv                = NULL;
	qglGetMapdv                  = NULL;
	qglGetMapfv                  = NULL;
	qglGetMapiv                  = NULL;
	qglGetMaterialfv             = NULL;
	qglGetMaterialiv             = NULL;
	qglGetPixelMapfv             = NULL;
	qglGetPixelMapuiv            = NULL;
	qglGetPixelMapusv            = NULL;
	qglGetPointerv               = NULL;
	qglGetPolygonStipple         = NULL;
	qglGetString                 = NULL;
	qglGetTexEnvfv               = NULL;
	qglGetTexEnviv               = NULL;
	qglGetTexGendv               = NULL;
	qglGetTexGenfv               = NULL;
	qglGetTexGeniv               = NULL;
	qglGetTexImage               = NULL;
	qglGetTexLevelParameterfv    = NULL;
	qglGetTexLevelParameteriv    = NULL;
	qglGetTexParameterfv         = NULL;
	qglGetTexParameteriv         = NULL;
	qglHint                      = NULL;
	qglIndexMask                 = NULL;
	qglIndexPointer              = NULL;
	qglIndexd                    = NULL;
	qglIndexdv                   = NULL;
	qglIndexf                    = NULL;
	qglIndexfv                   = NULL;
	qglIndexi                    = NULL;
	qglIndexiv                   = NULL;
	qglIndexs                    = NULL;
	qglIndexsv                   = NULL;
	qglIndexub                   = NULL;
	qglIndexubv                  = NULL;
	qglInitNames                 = NULL;
	qglInterleavedArrays         = NULL;
	qglIsEnabled                 = NULL;
	qglIsList                    = NULL;
	qglIsTexture                 = NULL;
	qglLightModelf               = NULL;
	qglLightModelfv              = NULL;
	qglLightModeli               = NULL;
	qglLightModeliv              = NULL;
	qglLightf                    = NULL;
	qglLightfv                   = NULL;
	qglLighti                    = NULL;
	qglLightiv                   = NULL;
	qglLineStipple               = NULL;
	qglLineWidth                 = NULL;
	qglListBase                  = NULL;
	qglLoadIdentity              = NULL;
	qglLoadMatrixd               = NULL;
	qglLoadMatrixf               = NULL;
	qglLoadName                  = NULL;
	qglLogicOp                   = NULL;
	qglMap1d                     = NULL;
	qglMap1f                     = NULL;
	qglMap2d                     = NULL;
	qglMap2f                     = NULL;
	qglMapGrid1d                 = NULL;
	qglMapGrid1f                 = NULL;
	qglMapGrid2d                 = NULL;
	qglMapGrid2f                 = NULL;
	qglMaterialf                 = NULL;
	qglMaterialfv                = NULL;
	qglMateriali                 = NULL;
	qglMaterialiv                = NULL;
	qglMatrixMode                = NULL;
	qglMultMatrixd               = NULL;
	qglMultMatrixf               = NULL;
	qglNewList                   = NULL;
	qglNormal3b                  = NULL;
	qglNormal3bv                 = NULL;
	qglNormal3d                  = NULL;
	qglNormal3dv                 = NULL;
	qglNormal3f                  = NULL;
	qglNormal3fv                 = NULL;
	qglNormal3i                  = NULL;
	qglNormal3iv                 = NULL;
	qglNormal3s                  = NULL;
	qglNormal3sv                 = NULL;
	qglNormalPointer             = NULL;
	qglOrtho                     = NULL;
	qglPassThrough               = NULL;
	qglPixelMapfv                = NULL;
	qglPixelMapuiv               = NULL;
	qglPixelMapusv               = NULL;
	qglPixelStoref               = NULL;
	qglPixelStorei               = NULL;
	qglPixelTransferf            = NULL;
	qglPixelTransferi            = NULL;
	qglPixelZoom                 = NULL;
	qglPointSize                 = NULL;
	qglPolygonMode               = NULL;
	qglPolygonOffset             = NULL;
	qglPolygonStipple            = NULL;
	qglPopAttrib                 = NULL;
	qglPopClientAttrib           = NULL;
	qglPopMatrix                 = NULL;
	qglPopName                   = NULL;
	qglPrioritizeTextures        = NULL;
	qglPushAttrib                = NULL;
	qglPushClientAttrib          = NULL;
	qglPushMatrix                = NULL;
	qglPushName                  = NULL;
	qglRasterPos2d               = NULL;
	qglRasterPos2dv              = NULL;
	qglRasterPos2f               = NULL;
	qglRasterPos2fv              = NULL;
	qglRasterPos2i               = NULL;
	qglRasterPos2iv              = NULL;
	qglRasterPos2s               = NULL;
	qglRasterPos2sv              = NULL;
	qglRasterPos3d               = NULL;
	qglRasterPos3dv              = NULL;
	qglRasterPos3f               = NULL;
	qglRasterPos3fv              = NULL;
	qglRasterPos3i               = NULL;
	qglRasterPos3iv              = NULL;
	qglRasterPos3s               = NULL;
	qglRasterPos3sv              = NULL;
	qglRasterPos4d               = NULL;
	qglRasterPos4dv              = NULL;
	qglRasterPos4f               = NULL;
	qglRasterPos4fv              = NULL;
	qglRasterPos4i               = NULL;
	qglRasterPos4iv              = NULL;
	qglRasterPos4s               = NULL;
	qglRasterPos4sv              = NULL;
	qglReadBuffer                = NULL;
	qglReadPixels                = NULL;
	qglRectd                     = NULL;
	qglRectdv                    = NULL;
	qglRectf                     = NULL;
	qglRectfv                    = NULL;
	qglRecti                     = NULL;
	qglRectiv                    = NULL;
	qglRects                     = NULL;
	qglRectsv                    = NULL;
	qglRenderMode                = NULL;
	qglRotated                   = NULL;
	qglRotatef                   = NULL;
	qglScaled                    = NULL;
	qglScalef                    = NULL;
	qglScissor                   = NULL;
	qglSelectBuffer              = NULL;
	qglShadeModel                = NULL;
	qglStencilFunc               = NULL;
	qglStencilMask               = NULL;
	qglStencilOp                 = NULL;
	qglTexCoord1d                = NULL;
	qglTexCoord1dv               = NULL;
	qglTexCoord1f                = NULL;
	qglTexCoord1fv               = NULL;
	qglTexCoord1i                = NULL;
	qglTexCoord1iv               = NULL;
	qglTexCoord1s                = NULL;
	qglTexCoord1sv               = NULL;
	qglTexCoord2d                = NULL;
	qglTexCoord2dv               = NULL;
	qglTexCoord2f                = NULL;
	qglTexCoord2fv               = NULL;
	qglTexCoord2i                = NULL;
	qglTexCoord2iv               = NULL;
	qglTexCoord2s                = NULL;
	qglTexCoord2sv               = NULL;
	qglTexCoord3d                = NULL;
	qglTexCoord3dv               = NULL;
	qglTexCoord3f                = NULL;
	qglTexCoord3fv               = NULL;
	qglTexCoord3i                = NULL;
	qglTexCoord3iv               = NULL;
	qglTexCoord3s                = NULL;
	qglTexCoord3sv               = NULL;
	qglTexCoord4d                = NULL;
	qglTexCoord4dv               = NULL;
	qglTexCoord4f                = NULL;
	qglTexCoord4fv               = NULL;
	qglTexCoord4i                = NULL;
	qglTexCoord4iv               = NULL;
	qglTexCoord4s                = NULL;
	qglTexCoord4sv               = NULL;
	qglTexCoordPointer           = NULL;
	qglTexEnvf                   = NULL;
	qglTexEnvfv                  = NULL;
	qglTexEnvi                   = NULL;
	qglTexEnviv                  = NULL;
	qglTexGend                   = NULL;
	qglTexGendv                  = NULL;
	qglTexGenf                   = NULL;
	qglTexGenfv                  = NULL;
	qglTexGeni                   = NULL;
	qglTexGeniv                  = NULL;
	qglTexImage1D                = NULL;
	qglTexImage2D                = NULL;
	qglTexParameterf             = NULL;
	qglTexParameterfv            = NULL;
	qglTexParameteri             = NULL;
	qglTexParameteriv            = NULL;
	qglTexSubImage1D             = NULL;
	qglTexSubImage2D             = NULL;
	qglTranslated                = NULL;
	qglTranslatef                = NULL;
	qglVertex2d                  = NULL;
	qglVertex2dv                 = NULL;
	qglVertex2f                  = NULL;
	qglVertex2fv                 = NULL;
	qglVertex2i                  = NULL;
	qglVertex2iv                 = NULL;
	qglVertex2s                  = NULL;
	qglVertex2sv                 = NULL;
	qglVertex3d                  = NULL;
	qglVertex3dv                 = NULL;
	qglVertex3f                  = NULL;
	qglVertex3fv                 = NULL;
	qglVertex3i                  = NULL;
	qglVertex3iv                 = NULL;
	qglVertex3s                  = NULL;
	qglVertex3sv                 = NULL;
	qglVertex4d                  = NULL;
	qglVertex4dv                 = NULL;
	qglVertex4f                  = NULL;
	qglVertex4fv                 = NULL;
	qglVertex4i                  = NULL;
	qglVertex4iv                 = NULL;
	qglVertex4s                  = NULL;
	qglVertex4sv                 = NULL;
	qglVertexPointer             = NULL;
	qglViewport                  = NULL;

//	qfxMesaCreateContext         = NULL;
//	qfxMesaCreateBestContext     = NULL;
//	qfxMesaDestroyContext        = NULL;
//	qfxMesaMakeCurrent           = NULL;
//	qfxMesaGetCurrentContext     = NULL;
//	qfxMesaSwapBuffers           = NULL;

	qglXChooseVisual             = NULL;
	qglXCreateContext            = NULL;
	qglXDestroyContext           = NULL;
	qglXMakeCurrent              = NULL;
	qglXCopyContext              = NULL;
	qglXSwapBuffers              = NULL;
}

#define GPA( a ) dlsym( glw_state.OpenGLLib, a )

void *qwglGetProcAddress(char *symbol)
{
	if (glw_state.OpenGLLib)
		return GPA ( symbol );
	return NULL;
}

/*
** QGL_Init
**
** This is responsible for binding our qgl function pointers to 
** the appropriate GL stuff.  In Windows this means doing a 
** LoadLibrary and a bunch of calls to GetProcAddress.  On other
** operating systems we need to do the right thing, whatever that
** might be.
** 
*/

qboolean QGL_Init( const char *dllname )
{
#if 0 //FIXME
	// update 3Dfx gamma irrespective of underlying DLL
	{
		char envbuffer[1024];
		float g;

		g = 2.00 * ( 0.8 - ( vid_gamma->value - 0.5 ) ) + 1.0F;
		Com_sprintf( envbuffer, sizeof(envbuffer), "SSTV2_GAMMA=%f", g );
		putenv( envbuffer );
		Com_sprintf( envbuffer, sizeof(envbuffer), "SST_GAMMA=%f", g );
		putenv( envbuffer );
	}
#endif

	if ( ( glw_state.OpenGLLib = dlopen( dllname, RTLD_LAZY ) ) == 0 )
	{
		char	fn[1024];
		FILE *fp;
		extern uid_t saved_euid; // unix_main.c

//fprintf(stdout, "uid=%d,euid=%d\n", getuid(), geteuid()); fflush(stdout);

		// if we are not setuid, try current directory
		if (getuid() == saved_euid) {
			getcwd(fn, sizeof(fn));
			Q_strcat(fn, sizeof(fn), "/");
			Q_strcat(fn, sizeof(fn), dllname);

			if ( ( glw_state.OpenGLLib = dlopen( fn, RTLD_LAZY ) ) == 0 ) {
				ri.Printf(PRINT_ALL, "QGL_Init: Can't load %s from /etc/ld.so.conf or current dir: %s\n", dllname, dlerror());
				return qfalse;
			}
		} else {
			ri.Printf(PRINT_ALL, "QGL_Init: Can't load %s from /etc/ld.so.conf: %s\n", dllname, dlerror());
			return qfalse;
		}
	}

	qglAccum                     = dllAccum = (void (*)(GLenum, GLfloat)) GPA( "glAccum" );
	qglAlphaFunc                 = dllAlphaFunc = (void (*)(GLenum, GLclampf)) GPA( "glAlphaFunc" );
	qglAreTexturesResident       = dllAreTexturesResident = (GLboolean (*)(GLsizei, const GLuint*, GLboolean*)) GPA( "glAreTexturesResident" );
	qglArrayElement              = dllArrayElement = (void (*)(GLint)) GPA( "glArrayElement" );
	qglBegin                     = dllBegin = (void (*)(GLenum)) GPA( "glBegin" );
	qglBindTexture               = dllBindTexture = (void (*)(GLenum, GLuint)) GPA( "glBindTexture" );
	qglBitmap                    = dllBitmap = (void (*)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte*)) GPA( "glBitmap" );
	qglBlendFunc                 = dllBlendFunc = (void (*)(GLenum, GLenum)) GPA( "glBlendFunc" );
	qglCallList                  = dllCallList = (void (*)(GLuint)) GPA( "glCallList" );
	qglCallLists                 = dllCallLists = (void (*)(GLsizei, GLenum, const GLvoid*)) GPA( "glCallLists" );
	qglClear                     = dllClear = (void (*)(GLbitfield)) GPA( "glClear" );
	qglClearAccum                = dllClearAccum = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glClearAccum" );
	qglClearColor                = dllClearColor = (void (*)(GLclampf, GLclampf, GLclampf, GLclampf)) GPA( "glClearColor" );
	qglClearDepth                = dllClearDepth = (void (*)(GLclampd)) GPA( "glClearDepth" );
	qglClearIndex                = dllClearIndex = (void (*)(GLfloat)) GPA( "glClearIndex" );
	qglClearStencil              = dllClearStencil = (void (*)(GLint)) GPA( "glClearStencil" );
	qglClipPlane                 = dllClipPlane = (void (*)(GLenum, const GLdouble*)) GPA( "glClipPlane" );
	qglColor3b                   = dllColor3b = (void (*)(GLbyte, GLbyte, GLbyte)) GPA( "glColor3b" );
	qglColor3bv                  = dllColor3bv = (void (*)(const GLbyte*)) GPA( "glColor3bv" );
	qglColor3d                   = dllColor3d = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glColor3d" );
	qglColor3dv                  = dllColor3dv = (void (*)(const GLdouble*)) GPA( "glColor3dv" );
	qglColor3f                   = dllColor3f = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glColor3f" );
	qglColor3fv                  = dllColor3fv = (void (*)(const GLfloat*)) GPA( "glColor3fv" );
	qglColor3i                   = dllColor3i = (void (*)(GLint, GLint, GLint)) GPA( "glColor3i" );
	qglColor3iv                  = dllColor3iv = (void (*)(const GLint*)) GPA( "glColor3iv" );
	qglColor3s                   = dllColor3s = (void (*)(GLshort, GLshort, GLshort)) GPA( "glColor3s" );
	qglColor3sv                  = dllColor3sv = (void (*)(const GLshort*)) GPA( "glColor3sv" );
	qglColor3ub                  = dllColor3ub = (void (*)(GLubyte, GLubyte, GLubyte)) GPA( "glColor3ub" );
	qglColor3ubv                 = dllColor3ubv = (void (*)(const GLubyte*)) GPA( "glColor3ubv" );
	qglColor3ui                  = dllColor3ui = (void (*)(GLuint, GLuint, GLuint)) GPA( "glColor3ui" );
	qglColor3uiv                 = dllColor3uiv = (void (*)(const GLuint*)) GPA( "glColor3uiv" );
	qglColor3us                  = dllColor3us = (void (*)(GLushort, GLushort, GLushort)) GPA( "glColor3us" );
	qglColor3usv                 = dllColor3usv = (void (*)(const GLushort*)) GPA( "glColor3usv" );
	qglColor4b                   = dllColor4b = (void (*)(GLbyte, GLbyte, GLbyte, GLbyte)) GPA( "glColor4b" );
	qglColor4bv                  = dllColor4bv = (void (*)(const GLbyte*)) GPA( "glColor4bv" );
	qglColor4d                   = dllColor4d = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glColor4d" );
	qglColor4dv                  = dllColor4dv = (void (*)(const GLdouble*)) GPA( "glColor4dv" );
	qglColor4f                   = dllColor4f = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glColor4f" );
	qglColor4fv                  = dllColor4fv = (void (*)(const GLfloat*)) GPA( "glColor4fv" );
	qglColor4i                   = dllColor4i = (void (*)(GLint, GLint, GLint, GLint)) GPA( "glColor4i" );
	qglColor4iv                  = dllColor4iv = (void (*)(const GLint*)) GPA( "glColor4iv" );
	qglColor4s                   = dllColor4s = (void (*)(GLshort, GLshort, GLshort, GLshort)) GPA( "glColor4s" );
	qglColor4sv                  = dllColor4sv = (void (*)(const GLshort*)) GPA( "glColor4sv" );
	qglColor4ub                  = dllColor4ub = (void (*)(GLubyte, GLubyte, GLubyte, GLubyte)) GPA( "glColor4ub" );
	qglColor4ubv                 = dllColor4ubv = (void (*)(const GLubyte*)) GPA( "glColor4ubv" );
	qglColor4ui                  = dllColor4ui = (void (*)(GLuint, GLuint, GLuint, GLuint)) GPA( "glColor4ui" );
	qglColor4uiv                 = dllColor4uiv = (void (*)(const GLuint*)) GPA( "glColor4uiv" );
	qglColor4us                  = dllColor4us = (void (*)(GLushort, GLushort, GLushort, GLushort)) GPA( "glColor4us" );
	qglColor4usv                 = dllColor4usv = (void (*)(const GLushort*)) GPA( "glColor4usv" );
	qglColorMask                 = dllColorMask = (void (*)(GLboolean, GLboolean, GLboolean, GLboolean)) GPA( "glColorMask" );
	qglColorMaterial             = dllColorMaterial = (void (*)(GLenum, GLenum)) GPA( "glColorMaterial" );
	qglColorPointer              = dllColorPointer = (void (*)(GLint, GLenum, GLsizei, const GLvoid*)) GPA( "glColorPointer" );
	qglCopyPixels                = dllCopyPixels = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum)) GPA( "glCopyPixels" );
	qglCopyTexImage1D            = dllCopyTexImage1D = (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint)) GPA( "glCopyTexImage1D" );
	qglCopyTexImage2D            = dllCopyTexImage2D = (void (*)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint)) GPA( "glCopyTexImage2D" );
	qglCopyTexSubImage1D         = dllCopyTexSubImage1D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLsizei)) GPA( "glCopyTexSubImage1D" );
	qglCopyTexSubImage2D         = dllCopyTexSubImage2D = (void (*)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei)) GPA( "glCopyTexSubImage2D" );
	qglCullFace                  = dllCullFace = (void (*)(GLenum)) GPA( "glCullFace" );
	qglDeleteLists               = dllDeleteLists = (void (*)(GLuint, GLsizei)) GPA( "glDeleteLists" );
	qglDeleteTextures            = dllDeleteTextures = (void (*)(GLsizei, const GLuint*)) GPA( "glDeleteTextures" );
	qglDepthFunc                 = dllDepthFunc = (void (*)(GLenum)) GPA( "glDepthFunc" );
	qglDepthMask                 = dllDepthMask = (void (*)(GLboolean)) GPA( "glDepthMask" );
	qglDepthRange                = dllDepthRange = (void (*)(GLclampd, GLclampd)) GPA( "glDepthRange" );
	qglDisable                   = dllDisable = (void (*)(GLenum)) GPA( "glDisable" );
	qglDisableClientState        = dllDisableClientState = (void (*)(GLenum)) GPA( "glDisableClientState" );
	qglDrawArrays                = dllDrawArrays = (void (*)(GLenum, GLint, GLsizei)) GPA( "glDrawArrays" );
	qglDrawBuffer                = dllDrawBuffer = (void (*)(GLenum)) GPA( "glDrawBuffer" );
	qglDrawElements              = dllDrawElements = (void (*)(GLenum, GLsizei, GLenum, const GLvoid*)) GPA( "glDrawElements" );
	qglDrawPixels                = dllDrawPixels = (void (*)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid*)) GPA( "glDrawPixels" );
	qglEdgeFlag                  = dllEdgeFlag = (void (*)(GLboolean)) GPA( "glEdgeFlag" );
	qglEdgeFlagPointer           = dllEdgeFlagPointer = (void (*)(GLsizei, const GLvoid*)) GPA( "glEdgeFlagPointer" );
	qglEdgeFlagv                 = dllEdgeFlagv = (void (*)(const GLboolean*)) GPA( "glEdgeFlagv" );
	qglEnable                    = 	dllEnable                    = (void (*)(GLenum)) GPA( "glEnable" );
	qglEnableClientState         = 	dllEnableClientState         = (void (*)(GLenum)) GPA( "glEnableClientState" );
	qglEnd                       = 	dllEnd                       = (void (*)()) GPA( "glEnd" );
	qglEndList                   = 	dllEndList                   = (void (*)()) GPA( "glEndList" );
	qglEvalCoord1d				 = 	dllEvalCoord1d				 = (void (*)(GLdouble)) GPA( "glEvalCoord1d" );
	qglEvalCoord1dv              = 	dllEvalCoord1dv              = (void (*)(const GLdouble*)) GPA( "glEvalCoord1dv" );
	qglEvalCoord1f               = 	dllEvalCoord1f               = (void (*)(GLfloat)) GPA( "glEvalCoord1f" );
	qglEvalCoord1fv              = 	dllEvalCoord1fv              = (void (*)(const GLfloat*)) GPA( "glEvalCoord1fv" );
	qglEvalCoord2d               = 	dllEvalCoord2d               = (void (*)(GLdouble, GLdouble)) GPA( "glEvalCoord2d" );
	qglEvalCoord2dv              = 	dllEvalCoord2dv              = (void (*)(const GLdouble*)) GPA( "glEvalCoord2dv" );
	qglEvalCoord2f               = 	dllEvalCoord2f               = (void (*)(GLfloat, GLfloat)) GPA( "glEvalCoord2f" );
	qglEvalCoord2fv              = 	dllEvalCoord2fv              = (void (*)(const GLfloat*)) GPA( "glEvalCoord2fv" );
	qglEvalMesh1                 = 	dllEvalMesh1                 = (void (*)(GLenum, GLint, GLint)) GPA( "glEvalMesh1" );
	qglEvalMesh2                 = 	dllEvalMesh2                 = (void (*)(GLenum, GLint, GLint, GLint, GLint)) GPA( "glEvalMesh2" );
	qglEvalPoint1                = 	dllEvalPoint1                = (void (*)(GLint)) GPA( "glEvalPoint1" );
	qglEvalPoint2                = 	dllEvalPoint2                = (void (*)(GLint, GLint)) GPA( "glEvalPoint2" );
	qglFeedbackBuffer            = 	dllFeedbackBuffer            = (void (*)(GLsizei, GLenum, GLfloat*)) GPA( "glFeedbackBuffer" );
	qglFinish                    = 	dllFinish                    = (void (*)()) GPA( "glFinish" );
	qglFlush                     = 	dllFlush                     = (void (*)()) GPA( "glFlush" );
	qglFogf                      = 	dllFogf                      = (void (*)(GLenum, GLfloat)) GPA( "glFogf" );
	qglFogfv                     = 	dllFogfv                     = (void (*)(GLenum, const GLfloat*)) GPA( "glFogfv" );
	qglFogi                      = 	dllFogi                      = (void (*)(GLenum, GLint)) GPA( "glFogi" );
	qglFogiv                     = 	dllFogiv                     = (void (*)(GLenum, const GLint*)) GPA( "glFogiv" );
	qglFrontFace                 = 	dllFrontFace                 = (void (*)(GLenum)) GPA( "glFrontFace" );
	qglFrustum                   = 	dllFrustum                   = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glFrustum" );
	qglGenLists                  = 	dllGenLists                  = (GLuint (*)(GLsizei)) GPA( "glGenLists" );
	qglGenTextures               = 	dllGenTextures               = (void (*)(GLsizei, GLuint*)) GPA( "glGenTextures" );
	qglGetBooleanv               = 	dllGetBooleanv               = (void (*)(GLenum, GLboolean*)) GPA( "glGetBooleanv" );
	qglGetClipPlane              = 	dllGetClipPlane              = (void (*)(GLenum, GLdouble*)) GPA( "glGetClipPlane" );
	qglGetDoublev                = 	dllGetDoublev                = (void (*)(GLenum, GLdouble*)) GPA( "glGetDoublev" );
	qglGetError                  = 	dllGetError                  = (GLenum (*)()) GPA( "glGetError" );
	qglGetFloatv                 = 	dllGetFloatv                 = (void (*)(GLenum, GLfloat*)) GPA( "glGetFloatv" );
	qglGetIntegerv               = 	dllGetIntegerv               = (void (*)(GLenum, GLint*)) GPA( "glGetIntegerv" );
	qglGetLightfv                = 	dllGetLightfv                = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetLightfv" );
	qglGetLightiv                = 	dllGetLightiv                = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetLightiv" );
	qglGetMapdv                  = 	dllGetMapdv                  = (void (*)(GLenum, GLenum, GLdouble*)) GPA( "glGetMapdv" );
	qglGetMapfv                  = 	dllGetMapfv                  = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetMapfv" );
	qglGetMapiv                  = 	dllGetMapiv                  = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetMapiv" );
	qglGetMaterialfv             = 	dllGetMaterialfv             = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetMaterialfv" );
	qglGetMaterialiv             = 	dllGetMaterialiv             = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetMaterialiv" );
	qglGetPixelMapfv             = 	dllGetPixelMapfv             = (void (*)(GLenum, GLfloat*)) GPA( "glGetPixelMapfv" );
	qglGetPixelMapuiv            = 	dllGetPixelMapuiv            = (void (*)(GLenum, GLuint*)) GPA( "glGetPixelMapuiv" );
	qglGetPixelMapusv            = 	dllGetPixelMapusv            = (void (*)(GLenum, GLushort*)) GPA( "glGetPixelMapusv" );
	qglGetPointerv               = 	dllGetPointerv               = (void (*)(GLenum, GLvoid**)) GPA( "glGetPointerv" );
	qglGetPolygonStipple         = 	dllGetPolygonStipple         = (void (*)(GLubyte*)) GPA( "glGetPolygonStipple" );
	qglGetString                 = 	dllGetString                 = (const GLubyte* (*)(GLenum)) GPA( "glGetString" );
	qglGetTexEnvfv               = 	dllGetTexEnvfv               = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetTexEnvfv" );
	qglGetTexEnviv               = 	dllGetTexEnviv               = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetTexEnviv" );
	qglGetTexGendv               = 	dllGetTexGendv               = (void (*)(GLenum, GLenum, GLdouble*)) GPA( "glGetTexGendv" );
	qglGetTexGenfv               = 	dllGetTexGenfv               = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetTexGenfv" );
	qglGetTexGeniv               = 	dllGetTexGeniv               = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetTexGeniv" );
	qglGetTexImage               = 	dllGetTexImage               = (void (*)(GLenum, GLint, GLenum, GLenum, GLvoid*)) GPA( "glGetTexImage" );
	qglGetTexLevelParameterfv    = 	dllGetTexLevelParameterfv    = (void (*)(GLenum, GLint, GLenum, GLfloat*)) GPA( "glGetLevelParameterfv" );
	qglGetTexLevelParameteriv    = 	dllGetTexLevelParameteriv    = (void (*)(GLenum, GLint, GLenum, GLint*)) GPA( "glGetLevelParameteriv" );
	qglGetTexParameterfv         = 	dllGetTexParameterfv         = (void (*)(GLenum, GLenum, GLfloat*)) GPA( "glGetTexParameterfv" );
	qglGetTexParameteriv         = 	dllGetTexParameteriv         = (void (*)(GLenum, GLenum, GLint*)) GPA( "glGetTexParameteriv" );
	qglHint                      = 	dllHint                      = (void (*)(GLenum, GLenum)) GPA( "glHint" );
	qglIndexMask                 = 	dllIndexMask                 = (void (*)(GLuint)) GPA( "glIndexMask" );
	qglIndexPointer              = 	dllIndexPointer              = (void (*)(GLenum, GLsizei, const GLvoid*)) GPA( "glIndexPointer" );
	qglIndexd                    = 	dllIndexd                    = (void (*)(GLdouble)) GPA( "glIndexd" );
	qglIndexdv                   = 	dllIndexdv                   = (void (*)(const GLdouble*)) GPA( "glIndexdv" );
	qglIndexf                    = 	dllIndexf                    = (void (*)(GLfloat)) GPA( "glIndexf" );
	qglIndexfv                   = 	dllIndexfv                   = (void (*)(const GLfloat*)) GPA( "glIndexfv" );
	qglIndexi                    = 	dllIndexi                    = (void (*)(GLint)) GPA( "glIndexi" );
	qglIndexiv                   = 	dllIndexiv                   = (void (*)(const GLint*)) GPA( "glIndexiv" );
	qglIndexs                    = 	dllIndexs                    = (void (*)(GLshort)) GPA( "glIndexs" );
	qglIndexsv                   = 	dllIndexsv                   = (void (*)(const GLshort*)) GPA( "glIndexsv" );
	qglIndexub                   = 	dllIndexub                   = (void (*)(GLubyte)) GPA( "glIndexub" );
	qglIndexubv                  = 	dllIndexubv                  = (void (*)(const GLubyte*)) GPA( "glIndexubv" );
	qglInitNames                 = 	dllInitNames                 = (void (*)()) GPA( "glInitNames" );
	qglInterleavedArrays         = 	dllInterleavedArrays         = (void (*)(GLenum, GLsizei, const GLvoid*)) GPA( "glInterleavedArrays" );
	qglIsEnabled                 = 	dllIsEnabled                 = (GLboolean (*)(GLenum)) GPA( "glIsEnabled" );
	qglIsList                    = 	dllIsList                    = (GLboolean (*)(GLuint)) GPA( "glIsList" );
	qglIsTexture                 = 	dllIsTexture                 = (GLboolean (*)(GLuint)) GPA( "glIsTexture" );
	qglLightModelf               = 	dllLightModelf               = (void (*)(GLenum, GLfloat)) GPA( "glLightModelf" );
	qglLightModelfv              = 	dllLightModelfv              = (void (*)(GLenum, const GLfloat*)) GPA( "glLightModelfv" );
	qglLightModeli               = 	dllLightModeli               = (void (*)(GLenum, GLint)) GPA( "glLightModeli" );
	qglLightModeliv              = 	dllLightModeliv              = (void (*)(GLenum, const GLint*)) GPA( "glLightModeliv" );
	qglLightf                    = 	dllLightf                    = (void (*)(GLenum, GLenum, GLfloat)) GPA( "glLightf" );
	qglLightfv                   = 	dllLightfv                   = (void (*)(GLenum, GLenum, const GLfloat*)) GPA( "glLightfv" );
	qglLighti                    = 	dllLighti                    = (void (*)(GLenum, GLenum, GLint)) GPA( "glLighti" );
	qglLightiv                   = 	dllLightiv                   = (void (*)(GLenum, GLenum, const GLint*)) GPA( "glLightiv" );
	qglLineStipple               = 	dllLineStipple               = (void (*)(GLint, GLushort)) GPA( "glLineStipple" );
	qglLineWidth                 = 	dllLineWidth                 = (void (*)(GLfloat)) GPA( "glLineWidth" );
	qglListBase                  = 	dllListBase                  = (void (*)(GLuint)) GPA( "glListBase" );
	qglLoadIdentity              = 	dllLoadIdentity              = (void (*)()) GPA( "glLoadIdentity" );
	qglLoadMatrixd               = 	dllLoadMatrixd               = (void (*)(const GLdouble*)) GPA( "glLoadMatrixd" );
	qglLoadMatrixf               = 	dllLoadMatrixf               = (void (*)(const GLfloat*)) GPA( "glLoadMatrixf" );
	qglLoadName                  = 	dllLoadName                  = (void (*)(GLuint)) GPA( "glLoadName" );
	qglLogicOp                   = 	dllLogicOp                   = (void (*)(GLenum)) GPA( "glLogicOp" );
	qglMap1d                     = 	dllMap1d                     = (void (*)(GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble*)) GPA( "glMap1d" );
	qglMap1f                     = 	dllMap1f                     = (void (*)(GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat*)) GPA( "glMap1f" );
	qglMap2d                     = 	dllMap2d                     = (void (*)(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble*)) GPA( "glMap2d" );
	qglMap2f                     = 	dllMap2f                     = (void (*)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat*)) GPA( "glMap2f" );
	qglMapGrid1d                 = 	dllMapGrid1d                 = (void (*)(GLint, GLdouble, GLdouble)) GPA( "glMapGrid1d" );
	qglMapGrid1f                 = 	dllMapGrid1f                 = (void (*)(GLint, GLfloat, GLfloat)) GPA( "glMapGrid1f" );
	qglMapGrid2d                 = 	dllMapGrid2d                 = (void (*)(GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble)) GPA( "glMapGrid2d" );
	qglMapGrid2f                 = 	dllMapGrid2f                 = (void (*)(GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat)) GPA( "glMapGrid2f" );
	qglMaterialf                 = 	dllMaterialf                 = (void (*)(GLenum, GLenum, GLfloat)) GPA( "glMaterialf" );
	qglMaterialfv                = 	dllMaterialfv                = (void (*)(GLenum, GLenum, const GLfloat*)) GPA( "glMaterialfv" );
	qglMateriali                 = 	dllMateriali                 = (void (*)(GLenum, GLenum, GLint)) GPA( "glMateriali" );
	qglMaterialiv                = 	dllMaterialiv                = (void (*)(GLenum, GLenum, const GLint*)) GPA( "glMaterialiv" );
	qglMatrixMode                = 	dllMatrixMode                = (void (*)(GLenum)) GPA( "glMatrixMode" );
	qglMultMatrixd               = 	dllMultMatrixd               = (void (*)(const GLdouble*)) GPA( "glMultMatrixd" );
	qglMultMatrixf               = 	dllMultMatrixf               = (void (*)(const GLfloat*)) GPA( "glMultMatrixf" );
	qglNewList                   = 	dllNewList                   = (void (*)(GLuint, GLenum)) GPA( "glNewList" );
	qglNormal3b                  = 	dllNormal3b                  = (void (*)(GLbyte, GLbyte, GLbyte)) GPA( "glNormal3b" );
	qglNormal3bv                 = 	dllNormal3bv                 = (void (*)(const GLbyte*)) GPA( "glNormal3bv" );
	qglNormal3d                  = 	dllNormal3d                  = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glNormal3d" );
	qglNormal3dv                 = 	dllNormal3dv                 = (void (*)(const GLdouble*)) GPA( "glNormal3dv" );
	qglNormal3f                  = 	dllNormal3f                  = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glNormal3f" );
	qglNormal3fv                 = 	dllNormal3fv                 = (void (*)(const GLfloat*)) GPA( "glNormal3fv" );
	qglNormal3i                  = 	dllNormal3i                  = (void (*)(GLint, GLint, GLint)) GPA( "glNormal3i" );
	qglNormal3iv                 = 	dllNormal3iv                 = (void (*)(const GLint*)) GPA( "glNormal3iv" );
	qglNormal3s                  = 	dllNormal3s                  = (void (*)(GLshort, GLshort, GLshort)) GPA( "glNormal3s" );
	qglNormal3sv                 = 	dllNormal3sv                 = (void (*)(const GLshort*)) GPA( "glNormal3sv" );
	qglNormalPointer             = 	dllNormalPointer             = (void (*)(GLenum, GLsizei, const GLvoid*)) GPA( "glNormalPointer" );
	qglOrtho                     = 	dllOrtho                     = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glOrtho" );
	qglPassThrough               = 	dllPassThrough               = (void (*)(GLfloat)) GPA( "glPassThrough" );
	qglPixelMapfv                = 	dllPixelMapfv                = (void (*)(GLenum, GLsizei, const GLfloat*)) GPA( "glPixelMapfv" );
	qglPixelMapuiv               = 	dllPixelMapuiv               = (void (*)(GLenum, GLsizei, const GLuint*)) GPA( "glPixelMapuiv" );
	qglPixelMapusv               = 	dllPixelMapusv               = (void (*)(GLenum, GLsizei, const GLushort*)) GPA( "glPixelMapusv" );
	qglPixelStoref               = 	dllPixelStoref               = (void (*)(GLenum, GLfloat)) GPA( "glPixelStoref" );
	qglPixelStorei               = 	dllPixelStorei               = (void (*)(GLenum, GLint)) GPA( "glPixelStorei" );
	qglPixelTransferf            = 	dllPixelTransferf            = (void (*)(GLenum, GLfloat)) GPA( "glPixelTransferf" );
	qglPixelTransferi            = 	dllPixelTransferi            = (void (*)(GLenum, GLint)) GPA( "glPixelTransferi" );
	qglPixelZoom                 = 	dllPixelZoom                 = (void (*)(GLfloat, GLfloat)) GPA( "glPixelZoom" );
	qglPointSize                 = 	dllPointSize                 = (void (*)(GLfloat)) GPA( "glPointSize" );
	qglPolygonMode               = 	dllPolygonMode               = (void (*)(GLenum, GLenum)) GPA( "glPolygonMode" );
	qglPolygonOffset             = 	dllPolygonOffset             = (void (*)(GLfloat, GLfloat)) GPA( "glPolygonOffset" );
	qglPolygonStipple            = 	dllPolygonStipple            = (void (*)(const GLubyte*)) GPA( "glPolygonStipple" );
	qglPopAttrib                 = 	dllPopAttrib                 = (void (*)()) GPA( "glPopAttrib" );
	qglPopClientAttrib           = 	dllPopClientAttrib           = (void (*)()) GPA( "glPopClientAttrib" );
	qglPopMatrix                 = 	dllPopMatrix                 = (void (*)()) GPA( "glPopMatrix" );
	qglPopName                   = 	dllPopName                   = (void (*)()) GPA( "glPopName" );
	qglPrioritizeTextures        = 	dllPrioritizeTextures        = (void (*)(GLsizei, const GLuint*, const GLclampf*)) GPA( "glPrioritizeTextures" );
	qglPushAttrib                = 	dllPushAttrib                = (void (*)(GLbitfield)) GPA( "glPushAttrib" );
	qglPushClientAttrib          = 	dllPushClientAttrib          = (void (*)(GLbitfield)) GPA( "glPushClientAttrib" );
	qglPushMatrix                = 	dllPushMatrix                = (void (*)()) GPA( "glPushMatrix" );
	qglPushName                  = 	dllPushName                  = (void (*)(GLuint)) GPA( "glPushName" );
	qglRasterPos2d               = 	dllRasterPos2d               = (void (*)(GLdouble, GLdouble)) GPA( "glRasterPos2d" );
	qglRasterPos2dv              = 	dllRasterPos2dv              = (void (*)(const GLdouble*)) GPA( "glRasterPos2dv" );
	qglRasterPos2f               = 	dllRasterPos2f               = (void (*)(GLfloat, GLfloat)) GPA( "glRasterPos2f" );
	qglRasterPos2fv              = 	dllRasterPos2fv              = (void (*)(const GLfloat*)) GPA( "glRasterPos2fv" );
	qglRasterPos2i               = 	dllRasterPos2i               = (void (*)(GLint, GLint)) GPA( "glRasterPos2i" );
	qglRasterPos2iv              = 	dllRasterPos2iv              = (void (*)(const GLint*)) GPA( "glRasterPos2iv" );
	qglRasterPos2s               = 	dllRasterPos2s               = (void (*)(GLshort, GLshort)) GPA( "glRasterPos2s" );
	qglRasterPos2sv              = 	dllRasterPos2sv              = (void (*)(const GLshort*)) GPA( "glRasterPos2sv" );
	qglRasterPos3d               = 	dllRasterPos3d               = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glRasterPos3d" );
	qglRasterPos3dv              = 	dllRasterPos3dv              = (void (*)(const GLdouble*)) GPA( "glRasterPos3dv" );
	qglRasterPos3f               = 	dllRasterPos3f               = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glRasterPos3f" );
	qglRasterPos3fv              = 	dllRasterPos3fv              = (void (*)(const GLfloat*)) GPA( "glRasterPos3fv" );
	qglRasterPos3i               = 	dllRasterPos3i               = (void (*)(GLint, GLint, GLint)) GPA( "glRasterPos3i" );
	qglRasterPos3iv              = 	dllRasterPos3iv              = (void (*)(const GLint*)) GPA( "glRasterPos3iv" );
	qglRasterPos3s               = 	dllRasterPos3s               = (void (*)(GLshort, GLshort, GLshort)) GPA( "glRasterPos3s" );
	qglRasterPos3sv              = 	dllRasterPos3sv              = (void (*)(const GLshort*)) GPA( "glRasterPos3sv" );
	qglRasterPos4d               = 	dllRasterPos4d               = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glRasterPos4d" );
	qglRasterPos4dv              = 	dllRasterPos4dv              = (void (*)(const GLdouble*)) GPA( "glRasterPos4dv" );
	qglRasterPos4f               = 	dllRasterPos4f               = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glRasterPos4f" );
	qglRasterPos4fv              = 	dllRasterPos4fv              = (void (*)(const GLfloat*)) GPA( "glRasterPos4fv" );
	qglRasterPos4i               = 	dllRasterPos4i               = (void (*)(GLint, GLint, GLint, GLint)) GPA( "glRasterPos4i" );
	qglRasterPos4iv              = 	dllRasterPos4iv              = (void (*)(const GLint*)) GPA( "glRasterPos4iv" );
	qglRasterPos4s               = 	dllRasterPos4s               = (void (*)(GLshort, GLshort, GLshort, GLshort)) GPA( "glRasterPos4s" );
	qglRasterPos4sv              = 	dllRasterPos4sv              = (void (*)(const GLshort*)) GPA( "glRasterPos4sv" );
	qglReadBuffer                = 	dllReadBuffer                = (void (*)(GLenum)) GPA( "glReadBuffer" );
	qglReadPixels                = 	dllReadPixels                = (void (*)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid*)) GPA( "glReadPixels" );
	qglRectd                     = 	dllRectd                     = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glRectd" );
	qglRectdv                    = 	dllRectdv                    = (void (*)(const GLdouble*, const GLdouble*)) GPA( "glRectdv" );
	qglRectf                     = 	dllRectf                     = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glRectf" );
	qglRectfv                    = 	dllRectfv                    = (void (*)(const GLfloat*, const GLfloat*)) GPA( "glRectfv" );
	qglRecti                     = 	dllRecti                     = (void (*)(GLint, GLint, GLint, GLint)) GPA( "glRecti" );
	qglRectiv                    = 	dllRectiv                    = (void (*)(const GLint*, const GLint*)) GPA( "glRectiv" );
	qglRects                     = 	dllRects                     = (void (*)(GLshort, GLshort, GLshort, GLshort)) GPA( "glRects" );
	qglRectsv                    = 	dllRectsv                    = (void (*)(const GLshort*, const GLshort*)) GPA( "glRectsv" );
	qglRenderMode                = 	dllRenderMode                = (GLint (*)(GLenum)) GPA( "glRenderMode" );
	qglRotated                   = 	dllRotated                   = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glRotated" );
	qglRotatef                   = 	dllRotatef                   = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glRotatef" );
	qglScaled                    = 	dllScaled                    = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glScaled" );
	qglScalef                    = 	dllScalef                    = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glScalef" );
	qglScissor                   = 	dllScissor                   = (void (*)(GLint, GLint, GLsizei, GLsizei)) GPA( "glScissor" );
	qglSelectBuffer              = 	dllSelectBuffer              = (void (*)(GLsizei, GLuint*)) GPA( "glSelectBuffer" );
	qglShadeModel                = 	dllShadeModel                = (void (*)(GLenum)) GPA( "glShadeModel" );
	qglStencilFunc               = 	dllStencilFunc               = (void (*)(GLenum, GLint, GLuint)) GPA( "glStencilFunc" );
	qglStencilMask               = 	dllStencilMask               = (void (*)(GLuint)) GPA( "glStencilMask" );
	qglStencilOp                 = 	dllStencilOp                 = (void (*)(GLenum, GLenum, GLenum)) GPA( "glStencilOp" );
	qglTexCoord1d                = 	dllTexCoord1d                = (void (*)(GLdouble)) GPA( "glTexCoord1d" );
	qglTexCoord1dv               = 	dllTexCoord1dv               = (void (*)(const GLdouble*)) GPA( "glTexCoord1dv" );
	qglTexCoord1f                = 	dllTexCoord1f                = (void (*)(GLfloat)) GPA( "glTexCoord1f" );
	qglTexCoord1fv               = 	dllTexCoord1fv               = (void (*)(const GLfloat*)) GPA( "glTexCoord1fv" );
	qglTexCoord1i                = 	dllTexCoord1i                = (void (*)(GLint)) GPA( "glTexCoord1i" );
	qglTexCoord1iv               = 	dllTexCoord1iv               = (void (*)(const GLint*)) GPA( "glTexCoord1iv" );
	qglTexCoord1s                = 	dllTexCoord1s                = (void (*)(GLshort)) GPA( "glTexCoord1s" );
	qglTexCoord1sv               = 	dllTexCoord1sv               = (void (*)(const GLshort*)) GPA( "glTexCoord1sv" );
	qglTexCoord2d                = 	dllTexCoord2d                = (void (*)(GLdouble, GLdouble)) GPA( "glTexCoord2d" );
	qglTexCoord2dv               = 	dllTexCoord2dv               = (void (*)(const GLdouble*)) GPA( "glTexCoord2dv" );
	qglTexCoord2f                = 	dllTexCoord2f                = (void (*)(GLfloat, GLfloat)) GPA( "glTexCoord2f" );
	qglTexCoord2fv               = 	dllTexCoord2fv               = (void (*)(const GLfloat*)) GPA( "glTexCoord2fv" );
	qglTexCoord2i                = 	dllTexCoord2i                = (void (*)(GLint, GLint)) GPA( "glTexCoord2i" );
	qglTexCoord2iv               = 	dllTexCoord2iv               = (void (*)(const GLint*)) GPA( "glTexCoord2iv" );
	qglTexCoord2s                = 	dllTexCoord2s                = (void (*)(GLshort, GLshort)) GPA( "glTexCoord2s" );
	qglTexCoord2sv               = 	dllTexCoord2sv               = (void (*)(const GLshort*)) GPA( "glTexCoord2sv" );
	qglTexCoord3d                = 	dllTexCoord3d                = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glTexCoord3d" );
	qglTexCoord3dv               = 	dllTexCoord3dv               = (void (*)(const GLdouble*)) GPA( "glTexCoord3dv" );
	qglTexCoord3f                = 	dllTexCoord3f                = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glTexCoord3f" );
	qglTexCoord3fv               = 	dllTexCoord3fv               = (void (*)(const GLfloat*)) GPA( "glTexCoord3fv" );
	qglTexCoord3i                = 	dllTexCoord3i                = (void (*)(GLint, GLint, GLint)) GPA( "glTexCoord3i" );
	qglTexCoord3iv               = 	dllTexCoord3iv               = (void (*)(const GLint*)) GPA( "glTexCoord3iv" );
	qglTexCoord3s                = 	dllTexCoord3s                = (void (*)(GLshort, GLshort, GLshort)) GPA( "glTexCoord3s" );
	qglTexCoord3sv               = 	dllTexCoord3sv               = (void (*)(const GLshort*)) GPA( "glTexCoord3sv" );
	qglTexCoord4d                = 	dllTexCoord4d                = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glTexCoord4d" );
	qglTexCoord4dv               = 	dllTexCoord4dv               = (void (*)(const GLdouble*)) GPA( "glTexCoord4dv" );
	qglTexCoord4f                = 	dllTexCoord4f                = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glTexCoord4f" );
	qglTexCoord4fv               = 	dllTexCoord4fv               = (void (*)(const GLfloat*)) GPA( "glTexCoord4fv" );
	qglTexCoord4i                = 	dllTexCoord4i                = (void (*)(GLint, GLint, GLint, GLint)) GPA( "glTexCoord4i" );
	qglTexCoord4iv               = 	dllTexCoord4iv               = (void (*)(const GLint*)) GPA( "glTexCoord4iv" );
	qglTexCoord4s                = 	dllTexCoord4s                = (void (*)(GLshort, GLshort, GLshort, GLshort)) GPA( "glTexCoord4s" );
	qglTexCoord4sv               = 	dllTexCoord4sv               = (void (*)(const GLshort*)) GPA( "glTexCoord4sv" );
	qglTexCoordPointer           = 	dllTexCoordPointer           = (void (*)(GLint, GLenum, GLsizei, const GLvoid*)) GPA( "glTexCoordPointer" );
	qglTexEnvf                   = 	dllTexEnvf                   = (void (*)(GLenum, GLenum, GLfloat)) GPA( "glTexEnvf" );
	qglTexEnvfv                  = 	dllTexEnvfv                  = (void (*)(GLenum, GLenum, const GLfloat*)) GPA( "glTexEnvfv" );
	qglTexEnvi                   = 	dllTexEnvi                   = (void (*)(GLenum, GLenum, GLint)) GPA( "glTexEnvi" );
	qglTexEnviv                  = 	dllTexEnviv                  = (void (*)(GLenum, GLenum, const GLint*)) GPA( "glTexEnviv" );
	qglTexGend                   = 	dllTexGend                   = (void (*)(GLenum, GLenum, GLdouble)) GPA( "glTexGend" );
	qglTexGendv                  = 	dllTexGendv                  = (void (*)(GLenum, GLenum, const GLdouble*)) GPA( "glTexGendv" );
	qglTexGenf                   = 	dllTexGenf                   = (void (*)(GLenum, GLenum, GLfloat)) GPA( "glTexGenf" );
	qglTexGenfv                  = 	dllTexGenfv                  = (void (*)(GLenum, GLenum, const GLfloat*)) GPA( "glTexGenfv" );
	qglTexGeni                   = 	dllTexGeni                   = (void (*)(GLenum, GLenum, GLint)) GPA( "glTexGeni" );
	qglTexGeniv                  = 	dllTexGeniv                  = (void (*)(GLenum, GLenum, const GLint*)) GPA( "glTexGeniv" );
	qglTexImage1D                = 	dllTexImage1D                = (void (*)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid*)) GPA( "glTexImage1D" );
	qglTexImage2D                = 	dllTexImage2D                = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*)) GPA( "glTexImage2D" );
	qglTexParameterf             = 	dllTexParameterf             = (void (*)(GLenum, GLenum, GLfloat)) GPA( "glTexParameterf" );
	qglTexParameterfv            = 	dllTexParameterfv            = (void (*)(GLenum, GLenum, const GLfloat*)) GPA( "glTexParameterfv" );
	qglTexParameteri             = 	dllTexParameteri             = (void (*)(GLenum, GLenum, GLint)) GPA( "glTexParameteri" );
	qglTexParameteriv            = 	dllTexParameteriv            = (void (*)(GLenum, GLenum, const GLint*)) GPA( "glTexParameteriv" );
	qglTexSubImage1D             = 	dllTexSubImage1D             = (void (*)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid*)) GPA( "glTexSubImage1D" );
	qglTexSubImage2D             = 	dllTexSubImage2D             = (void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid*)) GPA( "glTexSubImage2D" );
	qglTranslated                = 	dllTranslated                = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glTranslated" );
	qglTranslatef                = 	dllTranslatef                = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glTranslatef" );
	qglVertex2d                  = 	dllVertex2d                  = (void (*)(GLdouble, GLdouble)) GPA( "glVertex2d" );
	qglVertex2dv                 = 	dllVertex2dv                 = (void (*)(const GLdouble*)) GPA( "glVertex2dv" );
	qglVertex2f                  = 	dllVertex2f                  = (void (*)(GLfloat, GLfloat)) GPA( "glVertex2f" );
	qglVertex2fv                 = 	dllVertex2fv                 = (void (*)(const GLfloat*)) GPA( "glVertex2fv" );
	qglVertex2i                  = 	dllVertex2i                  = (void (*)(GLint, GLint)) GPA( "glVertex2i" );
	qglVertex2iv                 = 	dllVertex2iv                 = (void (*)(const GLint*)) GPA( "glVertex2iv" );
	qglVertex2s                  = 	dllVertex2s                  = (void (*)(GLshort, GLshort)) GPA( "glVertex2s" );
	qglVertex2sv                 = 	dllVertex2sv                 = (void (*)(const GLshort*)) GPA( "glVertex2sv" );
	qglVertex3d                  = 	dllVertex3d                  = (void (*)(GLdouble, GLdouble, GLdouble)) GPA( "glVertex3d" );
	qglVertex3dv                 = 	dllVertex3dv                 = (void (*)(const GLdouble*)) GPA( "glVertex3dv" );
	qglVertex3f                  = 	dllVertex3f                  = (void (*)(GLfloat, GLfloat, GLfloat)) GPA( "glVertex3f" );
	qglVertex3fv                 = 	dllVertex3fv                 = (void (*)(const GLfloat*)) GPA( "glVertex3fv" );
	qglVertex3i                  = 	dllVertex3i                  = (void (*)(GLint, GLint, GLint)) GPA( "glVertex3i" );
	qglVertex3iv                 = 	dllVertex3iv                 = (void (*)(const GLint*)) GPA( "glVertex3iv" );
	qglVertex3s                  = 	dllVertex3s                  = (void (*)(GLshort, GLshort, GLshort)) GPA( "glVertex3s" );
	qglVertex3sv                 = 	dllVertex3sv                 = (void (*)(const GLshort*)) GPA( "glVertex3sv" );
	qglVertex4d                  = 	dllVertex4d                  = (void (*)(GLdouble, GLdouble, GLdouble, GLdouble)) GPA( "glVertex4d" );
	qglVertex4dv                 = 	dllVertex4dv                 = (void (*)(const GLdouble*)) GPA( "glVertex4dv" );
	qglVertex4f                  = 	dllVertex4f                  = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat)) GPA( "glVertex4f" );
	qglVertex4fv                 = 	dllVertex4fv                 = (void (*)(const GLfloat*)) GPA( "glVertex4fv" );
	qglVertex4i                  = 	dllVertex4i                  = (void (*)(GLint, GLint, GLint, GLint)) GPA( "glVertex4i" );
	qglVertex4iv                 = 	dllVertex4iv                 = (void (*)(const GLint*)) GPA( "glVertex4iv" );
	qglVertex4s                  = 	dllVertex4s                  = (void (*)(GLshort, GLshort, GLshort, GLshort)) GPA( "glVertex4s" );
	qglVertex4sv                 = 	dllVertex4sv                 = (void (*)(const GLshort*)) GPA( "glVertex4sv" );
	qglVertexPointer             = 	dllVertexPointer             = (void (*)(GLint, GLenum, GLsizei, const GLvoid*)) GPA( "glVertexPointer" );
	qglViewport                  = 	dllViewport                  = (void (*)(GLint, GLint, GLsizei, GLsizei)) GPA( "glViewport" );

//	qfxMesaCreateContext         =  GPA("fxMesaCreateContext");
//	qfxMesaCreateBestContext     =  GPA("fxMesaCreateBestContext");
//	qfxMesaDestroyContext        =  GPA("fxMesaDestroyContext");
//	qfxMesaMakeCurrent           =  GPA("fxMesaMakeCurrent");
//	qfxMesaGetCurrentContext     =  GPA("fxMesaGetCurrentContext");
//	qfxMesaSwapBuffers           =  GPA("fxMesaSwapBuffers");

	qglXChooseVisual             =  (XVisualInfo* (*)(Display*, int, int*)) GPA("glXChooseVisual");
	qglXCreateContext            =  (__GLXcontextRec* (*)(Display*, XVisualInfo*, GLXContext, int)) GPA("glXCreateContext");
	qglXDestroyContext           =  (void (*)(Display*, GLXContext)) GPA("glXDestroyContext");
	qglXMakeCurrent              =  (int (*)(Display*, GLXDrawable, GLXContext)) GPA("glXMakeCurrent");
	qglXCopyContext              =  (void (*)(Display*, GLXContext, GLXContext, GLuint)) GPA("glXCopyContext");
	qglXSwapBuffers              =  (void (*)(Display*, GLXDrawable)) GPA("glXSwapBuffers");

	qglLockArraysEXT = 0;
	qglUnlockArraysEXT = 0;
	qglPointParameterfEXT = 0;
	qglPointParameterfvEXT = 0;
	qglColorTableEXT = 0;
	qgl3DfxSetPaletteEXT = 0;
	qglSelectTextureSGIS = 0;
	qglMTexCoord2fSGIS = 0;
	qglActiveTextureARB = 0;
	qglClientActiveTextureARB = 0;
	qglMultiTexCoord2fARB = 0;

	return qtrue;
}

void QGL_EnableLogging( qboolean enable )
{
	if ( enable )
	{
		if ( !glw_state.log_fp )
		{
			struct tm *newtime;
			time_t aclock;
			char buffer[1024];
			cvar_t	*basedir;

			time( &aclock );
			newtime = localtime( &aclock );

			asctime( newtime );

			basedir = ri.Cvar_Get( "basedir", "", 0 );
			Com_sprintf( buffer, sizeof(buffer), "%s/gl.log", basedir->string ); 
			glw_state.log_fp = fopen( buffer, "wt" );

			fprintf( glw_state.log_fp, "%s\n", asctime( newtime ) );
		}

		qglAccum                     = logAccum;
		qglAlphaFunc                 = logAlphaFunc;
		qglAreTexturesResident       = logAreTexturesResident;
		qglArrayElement              = logArrayElement;
		qglBegin                     = logBegin;
		qglBindTexture               = logBindTexture;
		qglBitmap                    = logBitmap;
		qglBlendFunc                 = logBlendFunc;
		qglCallList                  = logCallList;
		qglCallLists                 = logCallLists;
		qglClear                     = logClear;
		qglClearAccum                = logClearAccum;
		qglClearColor                = logClearColor;
		qglClearDepth                = logClearDepth;
		qglClearIndex                = logClearIndex;
		qglClearStencil              = logClearStencil;
		qglClipPlane                 = logClipPlane;
		qglColor3b                   = logColor3b;
		qglColor3bv                  = logColor3bv;
		qglColor3d                   = logColor3d;
		qglColor3dv                  = logColor3dv;
		qglColor3f                   = logColor3f;
		qglColor3fv                  = logColor3fv;
		qglColor3i                   = logColor3i;
		qglColor3iv                  = logColor3iv;
		qglColor3s                   = logColor3s;
		qglColor3sv                  = logColor3sv;
		qglColor3ub                  = logColor3ub;
		qglColor3ubv                 = logColor3ubv;
		qglColor3ui                  = logColor3ui;
		qglColor3uiv                 = logColor3uiv;
		qglColor3us                  = logColor3us;
		qglColor3usv                 = logColor3usv;
		qglColor4b                   = logColor4b;
		qglColor4bv                  = logColor4bv;
		qglColor4d                   = logColor4d;
		qglColor4dv                  = logColor4dv;
		qglColor4f                   = logColor4f;
		qglColor4fv                  = logColor4fv;
		qglColor4i                   = logColor4i;
		qglColor4iv                  = logColor4iv;
		qglColor4s                   = logColor4s;
		qglColor4sv                  = logColor4sv;
		qglColor4ub                  = logColor4ub;
		qglColor4ubv                 = logColor4ubv;
		qglColor4ui                  = logColor4ui;
		qglColor4uiv                 = logColor4uiv;
		qglColor4us                  = logColor4us;
		qglColor4usv                 = logColor4usv;
		qglColorMask                 = logColorMask;
		qglColorMaterial             = logColorMaterial;
		qglColorPointer              = logColorPointer;
		qglCopyPixels                = logCopyPixels;
		qglCopyTexImage1D            = logCopyTexImage1D;
		qglCopyTexImage2D            = logCopyTexImage2D;
		qglCopyTexSubImage1D         = logCopyTexSubImage1D;
		qglCopyTexSubImage2D         = logCopyTexSubImage2D;
		qglCullFace                  = logCullFace;
		qglDeleteLists               = logDeleteLists ;
		qglDeleteTextures            = logDeleteTextures ;
		qglDepthFunc                 = logDepthFunc ;
		qglDepthMask                 = logDepthMask ;
		qglDepthRange                = logDepthRange ;
		qglDisable                   = logDisable ;
		qglDisableClientState        = logDisableClientState ;
		qglDrawArrays                = logDrawArrays ;
		qglDrawBuffer                = logDrawBuffer ;
		qglDrawElements              = logDrawElements ;
		qglDrawPixels                = logDrawPixels ;
		qglEdgeFlag                  = logEdgeFlag ;
		qglEdgeFlagPointer           = logEdgeFlagPointer ;
		qglEdgeFlagv                 = logEdgeFlagv ;
		qglEnable                    = 	logEnable                    ;
		qglEnableClientState         = 	logEnableClientState         ;
		qglEnd                       = 	logEnd                       ;
		qglEndList                   = 	logEndList                   ;
		qglEvalCoord1d				 = 	logEvalCoord1d				 ;
		qglEvalCoord1dv              = 	logEvalCoord1dv              ;
		qglEvalCoord1f               = 	logEvalCoord1f               ;
		qglEvalCoord1fv              = 	logEvalCoord1fv              ;
		qglEvalCoord2d               = 	logEvalCoord2d               ;
		qglEvalCoord2dv              = 	logEvalCoord2dv              ;
		qglEvalCoord2f               = 	logEvalCoord2f               ;
		qglEvalCoord2fv              = 	logEvalCoord2fv              ;
		qglEvalMesh1                 = 	logEvalMesh1                 ;
		qglEvalMesh2                 = 	logEvalMesh2                 ;
		qglEvalPoint1                = 	logEvalPoint1                ;
		qglEvalPoint2                = 	logEvalPoint2                ;
		qglFeedbackBuffer            = 	logFeedbackBuffer            ;
		qglFinish                    = 	logFinish                    ;
		qglFlush                     = 	logFlush                     ;
		qglFogf                      = 	logFogf                      ;
		qglFogfv                     = 	logFogfv                     ;
		qglFogi                      = 	logFogi                      ;
		qglFogiv                     = 	logFogiv                     ;
		qglFrontFace                 = 	logFrontFace                 ;
		qglFrustum                   = 	logFrustum                   ;
		qglGenLists                  = 	logGenLists                  ;
		qglGenTextures               = 	logGenTextures               ;
		qglGetBooleanv               = 	logGetBooleanv               ;
		qglGetClipPlane              = 	logGetClipPlane              ;
		qglGetDoublev                = 	logGetDoublev                ;
		qglGetError                  = 	logGetError                  ;
		qglGetFloatv                 = 	logGetFloatv                 ;
		qglGetIntegerv               = 	logGetIntegerv               ;
		qglGetLightfv                = 	logGetLightfv                ;
		qglGetLightiv                = 	logGetLightiv                ;
		qglGetMapdv                  = 	logGetMapdv                  ;
		qglGetMapfv                  = 	logGetMapfv                  ;
		qglGetMapiv                  = 	logGetMapiv                  ;
		qglGetMaterialfv             = 	logGetMaterialfv             ;
		qglGetMaterialiv             = 	logGetMaterialiv             ;
		qglGetPixelMapfv             = 	logGetPixelMapfv             ;
		qglGetPixelMapuiv            = 	logGetPixelMapuiv            ;
		qglGetPixelMapusv            = 	logGetPixelMapusv            ;
		qglGetPointerv               = 	logGetPointerv               ;
		qglGetPolygonStipple         = 	logGetPolygonStipple         ;
		qglGetString                 = 	logGetString                 ;
		qglGetTexEnvfv               = 	logGetTexEnvfv               ;
		qglGetTexEnviv               = 	logGetTexEnviv               ;
		qglGetTexGendv               = 	logGetTexGendv               ;
		qglGetTexGenfv               = 	logGetTexGenfv               ;
		qglGetTexGeniv               = 	logGetTexGeniv               ;
		qglGetTexImage               = 	logGetTexImage               ;
		qglGetTexLevelParameterfv    = 	logGetTexLevelParameterfv    ;
		qglGetTexLevelParameteriv    = 	logGetTexLevelParameteriv    ;
		qglGetTexParameterfv         = 	logGetTexParameterfv         ;
		qglGetTexParameteriv         = 	logGetTexParameteriv         ;
		qglHint                      = 	logHint                      ;
		qglIndexMask                 = 	logIndexMask                 ;
		qglIndexPointer              = 	logIndexPointer              ;
		qglIndexd                    = 	logIndexd                    ;
		qglIndexdv                   = 	logIndexdv                   ;
		qglIndexf                    = 	logIndexf                    ;
		qglIndexfv                   = 	logIndexfv                   ;
		qglIndexi                    = 	logIndexi                    ;
		qglIndexiv                   = 	logIndexiv                   ;
		qglIndexs                    = 	logIndexs                    ;
		qglIndexsv                   = 	logIndexsv                   ;
		qglIndexub                   = 	logIndexub                   ;
		qglIndexubv                  = 	logIndexubv                  ;
		qglInitNames                 = 	logInitNames                 ;
		qglInterleavedArrays         = 	logInterleavedArrays         ;
		qglIsEnabled                 = 	logIsEnabled                 ;
		qglIsList                    = 	logIsList                    ;
		qglIsTexture                 = 	logIsTexture                 ;
		qglLightModelf               = 	logLightModelf               ;
		qglLightModelfv              = 	logLightModelfv              ;
		qglLightModeli               = 	logLightModeli               ;
		qglLightModeliv              = 	logLightModeliv              ;
		qglLightf                    = 	logLightf                    ;
		qglLightfv                   = 	logLightfv                   ;
		qglLighti                    = 	logLighti                    ;
		qglLightiv                   = 	logLightiv                   ;
		qglLineStipple               = 	logLineStipple               ;
		qglLineWidth                 = 	logLineWidth                 ;
		qglListBase                  = 	logListBase                  ;
		qglLoadIdentity              = 	logLoadIdentity              ;
		qglLoadMatrixd               = 	logLoadMatrixd               ;
		qglLoadMatrixf               = 	logLoadMatrixf               ;
		qglLoadName                  = 	logLoadName                  ;
		qglLogicOp                   = 	logLogicOp                   ;
		qglMap1d                     = 	logMap1d                     ;
		qglMap1f                     = 	logMap1f                     ;
		qglMap2d                     = 	logMap2d                     ;
		qglMap2f                     = 	logMap2f                     ;
		qglMapGrid1d                 = 	logMapGrid1d                 ;
		qglMapGrid1f                 = 	logMapGrid1f                 ;
		qglMapGrid2d                 = 	logMapGrid2d                 ;
		qglMapGrid2f                 = 	logMapGrid2f                 ;
		qglMaterialf                 = 	logMaterialf                 ;
		qglMaterialfv                = 	logMaterialfv                ;
		qglMateriali                 = 	logMateriali                 ;
		qglMaterialiv                = 	logMaterialiv                ;
		qglMatrixMode                = 	logMatrixMode                ;
		qglMultMatrixd               = 	logMultMatrixd               ;
		qglMultMatrixf               = 	logMultMatrixf               ;
		qglNewList                   = 	logNewList                   ;
		qglNormal3b                  = 	logNormal3b                  ;
		qglNormal3bv                 = 	logNormal3bv                 ;
		qglNormal3d                  = 	logNormal3d                  ;
		qglNormal3dv                 = 	logNormal3dv                 ;
		qglNormal3f                  = 	logNormal3f                  ;
		qglNormal3fv                 = 	logNormal3fv                 ;
		qglNormal3i                  = 	logNormal3i                  ;
		qglNormal3iv                 = 	logNormal3iv                 ;
		qglNormal3s                  = 	logNormal3s                  ;
		qglNormal3sv                 = 	logNormal3sv                 ;
		qglNormalPointer             = 	logNormalPointer             ;
		qglOrtho                     = 	logOrtho                     ;
		qglPassThrough               = 	logPassThrough               ;
		qglPixelMapfv                = 	logPixelMapfv                ;
		qglPixelMapuiv               = 	logPixelMapuiv               ;
		qglPixelMapusv               = 	logPixelMapusv               ;
		qglPixelStoref               = 	logPixelStoref               ;
		qglPixelStorei               = 	logPixelStorei               ;
		qglPixelTransferf            = 	logPixelTransferf            ;
		qglPixelTransferi            = 	logPixelTransferi            ;
		qglPixelZoom                 = 	logPixelZoom                 ;
		qglPointSize                 = 	logPointSize                 ;
		qglPolygonMode               = 	logPolygonMode               ;
		qglPolygonOffset             = 	logPolygonOffset             ;
		qglPolygonStipple            = 	logPolygonStipple            ;
		qglPopAttrib                 = 	logPopAttrib                 ;
		qglPopClientAttrib           = 	logPopClientAttrib           ;
		qglPopMatrix                 = 	logPopMatrix                 ;
		qglPopName                   = 	logPopName                   ;
		qglPrioritizeTextures        = 	logPrioritizeTextures        ;
		qglPushAttrib                = 	logPushAttrib                ;
		qglPushClientAttrib          = 	logPushClientAttrib          ;
		qglPushMatrix                = 	logPushMatrix                ;
		qglPushName                  = 	logPushName                  ;
		qglRasterPos2d               = 	logRasterPos2d               ;
		qglRasterPos2dv              = 	logRasterPos2dv              ;
		qglRasterPos2f               = 	logRasterPos2f               ;
		qglRasterPos2fv              = 	logRasterPos2fv              ;
		qglRasterPos2i               = 	logRasterPos2i               ;
		qglRasterPos2iv              = 	logRasterPos2iv              ;
		qglRasterPos2s               = 	logRasterPos2s               ;
		qglRasterPos2sv              = 	logRasterPos2sv              ;
		qglRasterPos3d               = 	logRasterPos3d               ;
		qglRasterPos3dv              = 	logRasterPos3dv              ;
		qglRasterPos3f               = 	logRasterPos3f               ;
		qglRasterPos3fv              = 	logRasterPos3fv              ;
		qglRasterPos3i               = 	logRasterPos3i               ;
		qglRasterPos3iv              = 	logRasterPos3iv              ;
		qglRasterPos3s               = 	logRasterPos3s               ;
		qglRasterPos3sv              = 	logRasterPos3sv              ;
		qglRasterPos4d               = 	logRasterPos4d               ;
		qglRasterPos4dv              = 	logRasterPos4dv              ;
		qglRasterPos4f               = 	logRasterPos4f               ;
		qglRasterPos4fv              = 	logRasterPos4fv              ;
		qglRasterPos4i               = 	logRasterPos4i               ;
		qglRasterPos4iv              = 	logRasterPos4iv              ;
		qglRasterPos4s               = 	logRasterPos4s               ;
		qglRasterPos4sv              = 	logRasterPos4sv              ;
		qglReadBuffer                = 	logReadBuffer                ;
		qglReadPixels                = 	logReadPixels                ;
		qglRectd                     = 	logRectd                     ;
		qglRectdv                    = 	logRectdv                    ;
		qglRectf                     = 	logRectf                     ;
		qglRectfv                    = 	logRectfv                    ;
		qglRecti                     = 	logRecti                     ;
		qglRectiv                    = 	logRectiv                    ;
		qglRects                     = 	logRects                     ;
		qglRectsv                    = 	logRectsv                    ;
		qglRenderMode                = 	logRenderMode                ;
		qglRotated                   = 	logRotated                   ;
		qglRotatef                   = 	logRotatef                   ;
		qglScaled                    = 	logScaled                    ;
		qglScalef                    = 	logScalef                    ;
		qglScissor                   = 	logScissor                   ;
		qglSelectBuffer              = 	logSelectBuffer              ;
		qglShadeModel                = 	logShadeModel                ;
		qglStencilFunc               = 	logStencilFunc               ;
		qglStencilMask               = 	logStencilMask               ;
		qglStencilOp                 = 	logStencilOp                 ;
		qglTexCoord1d                = 	logTexCoord1d                ;
		qglTexCoord1dv               = 	logTexCoord1dv               ;
		qglTexCoord1f                = 	logTexCoord1f                ;
		qglTexCoord1fv               = 	logTexCoord1fv               ;
		qglTexCoord1i                = 	logTexCoord1i                ;
		qglTexCoord1iv               = 	logTexCoord1iv               ;
		qglTexCoord1s                = 	logTexCoord1s                ;
		qglTexCoord1sv               = 	logTexCoord1sv               ;
		qglTexCoord2d                = 	logTexCoord2d                ;
		qglTexCoord2dv               = 	logTexCoord2dv               ;
		qglTexCoord2f                = 	logTexCoord2f                ;
		qglTexCoord2fv               = 	logTexCoord2fv               ;
		qglTexCoord2i                = 	logTexCoord2i                ;
		qglTexCoord2iv               = 	logTexCoord2iv               ;
		qglTexCoord2s                = 	logTexCoord2s                ;
		qglTexCoord2sv               = 	logTexCoord2sv               ;
		qglTexCoord3d                = 	logTexCoord3d                ;
		qglTexCoord3dv               = 	logTexCoord3dv               ;
		qglTexCoord3f                = 	logTexCoord3f                ;
		qglTexCoord3fv               = 	logTexCoord3fv               ;
		qglTexCoord3i                = 	logTexCoord3i                ;
		qglTexCoord3iv               = 	logTexCoord3iv               ;
		qglTexCoord3s                = 	logTexCoord3s                ;
		qglTexCoord3sv               = 	logTexCoord3sv               ;
		qglTexCoord4d                = 	logTexCoord4d                ;
		qglTexCoord4dv               = 	logTexCoord4dv               ;
		qglTexCoord4f                = 	logTexCoord4f                ;
		qglTexCoord4fv               = 	logTexCoord4fv               ;
		qglTexCoord4i                = 	logTexCoord4i                ;
		qglTexCoord4iv               = 	logTexCoord4iv               ;
		qglTexCoord4s                = 	logTexCoord4s                ;
		qglTexCoord4sv               = 	logTexCoord4sv               ;
		qglTexCoordPointer           = 	logTexCoordPointer           ;
		qglTexEnvf                   = 	logTexEnvf                   ;
		qglTexEnvfv                  = 	logTexEnvfv                  ;
		qglTexEnvi                   = 	logTexEnvi                   ;
		qglTexEnviv                  = 	logTexEnviv                  ;
		qglTexGend                   = 	logTexGend                   ;
		qglTexGendv                  = 	logTexGendv                  ;
		qglTexGenf                   = 	logTexGenf                   ;
		qglTexGenfv                  = 	logTexGenfv                  ;
		qglTexGeni                   = 	logTexGeni                   ;
		qglTexGeniv                  = 	logTexGeniv                  ;
		qglTexImage1D                = 	logTexImage1D                ;
		qglTexImage2D                = 	logTexImage2D                ;
		qglTexParameterf             = 	logTexParameterf             ;
		qglTexParameterfv            = 	logTexParameterfv            ;
		qglTexParameteri             = 	logTexParameteri             ;
		qglTexParameteriv            = 	logTexParameteriv            ;
		qglTexSubImage1D             = 	logTexSubImage1D             ;
		qglTexSubImage2D             = 	logTexSubImage2D             ;
		qglTranslated                = 	logTranslated                ;
		qglTranslatef                = 	logTranslatef                ;
		qglVertex2d                  = 	logVertex2d                  ;
		qglVertex2dv                 = 	logVertex2dv                 ;
		qglVertex2f                  = 	logVertex2f                  ;
		qglVertex2fv                 = 	logVertex2fv                 ;
		qglVertex2i                  = 	logVertex2i                  ;
		qglVertex2iv                 = 	logVertex2iv                 ;
		qglVertex2s                  = 	logVertex2s                  ;
		qglVertex2sv                 = 	logVertex2sv                 ;
		qglVertex3d                  = 	logVertex3d                  ;
		qglVertex3dv                 = 	logVertex3dv                 ;
		qglVertex3f                  = 	logVertex3f                  ;
		qglVertex3fv                 = 	logVertex3fv                 ;
		qglVertex3i                  = 	logVertex3i                  ;
		qglVertex3iv                 = 	logVertex3iv                 ;
		qglVertex3s                  = 	logVertex3s                  ;
		qglVertex3sv                 = 	logVertex3sv                 ;
		qglVertex4d                  = 	logVertex4d                  ;
		qglVertex4dv                 = 	logVertex4dv                 ;
		qglVertex4f                  = 	logVertex4f                  ;
		qglVertex4fv                 = 	logVertex4fv                 ;
		qglVertex4i                  = 	logVertex4i                  ;
		qglVertex4iv                 = 	logVertex4iv                 ;
		qglVertex4s                  = 	logVertex4s                  ;
		qglVertex4sv                 = 	logVertex4sv                 ;
		qglVertexPointer             = 	logVertexPointer             ;
		qglViewport                  = 	logViewport                  ;
	}
	else
	{
		qglAccum                     = dllAccum;
		qglAlphaFunc                 = dllAlphaFunc;
		qglAreTexturesResident       = dllAreTexturesResident;
		qglArrayElement              = dllArrayElement;
		qglBegin                     = dllBegin;
		qglBindTexture               = dllBindTexture;
		qglBitmap                    = dllBitmap;
		qglBlendFunc                 = dllBlendFunc;
		qglCallList                  = dllCallList;
		qglCallLists                 = dllCallLists;
		qglClear                     = dllClear;
		qglClearAccum                = dllClearAccum;
		qglClearColor                = dllClearColor;
		qglClearDepth                = dllClearDepth;
		qglClearIndex                = dllClearIndex;
		qglClearStencil              = dllClearStencil;
		qglClipPlane                 = dllClipPlane;
		qglColor3b                   = dllColor3b;
		qglColor3bv                  = dllColor3bv;
		qglColor3d                   = dllColor3d;
		qglColor3dv                  = dllColor3dv;
		qglColor3f                   = dllColor3f;
		qglColor3fv                  = dllColor3fv;
		qglColor3i                   = dllColor3i;
		qglColor3iv                  = dllColor3iv;
		qglColor3s                   = dllColor3s;
		qglColor3sv                  = dllColor3sv;
		qglColor3ub                  = dllColor3ub;
		qglColor3ubv                 = dllColor3ubv;
		qglColor3ui                  = dllColor3ui;
		qglColor3uiv                 = dllColor3uiv;
		qglColor3us                  = dllColor3us;
		qglColor3usv                 = dllColor3usv;
		qglColor4b                   = dllColor4b;
		qglColor4bv                  = dllColor4bv;
		qglColor4d                   = dllColor4d;
		qglColor4dv                  = dllColor4dv;
		qglColor4f                   = dllColor4f;
		qglColor4fv                  = dllColor4fv;
		qglColor4i                   = dllColor4i;
		qglColor4iv                  = dllColor4iv;
		qglColor4s                   = dllColor4s;
		qglColor4sv                  = dllColor4sv;
		qglColor4ub                  = dllColor4ub;
		qglColor4ubv                 = dllColor4ubv;
		qglColor4ui                  = dllColor4ui;
		qglColor4uiv                 = dllColor4uiv;
		qglColor4us                  = dllColor4us;
		qglColor4usv                 = dllColor4usv;
		qglColorMask                 = dllColorMask;
		qglColorMaterial             = dllColorMaterial;
		qglColorPointer              = dllColorPointer;
		qglCopyPixels                = dllCopyPixels;
		qglCopyTexImage1D            = dllCopyTexImage1D;
		qglCopyTexImage2D            = dllCopyTexImage2D;
		qglCopyTexSubImage1D         = dllCopyTexSubImage1D;
		qglCopyTexSubImage2D         = dllCopyTexSubImage2D;
		qglCullFace                  = dllCullFace;
		qglDeleteLists               = dllDeleteLists ;
		qglDeleteTextures            = dllDeleteTextures ;
		qglDepthFunc                 = dllDepthFunc ;
		qglDepthMask                 = dllDepthMask ;
		qglDepthRange                = dllDepthRange ;
		qglDisable                   = dllDisable ;
		qglDisableClientState        = dllDisableClientState ;
		qglDrawArrays                = dllDrawArrays ;
		qglDrawBuffer                = dllDrawBuffer ;
		qglDrawElements              = dllDrawElements ;
		qglDrawPixels                = dllDrawPixels ;
		qglEdgeFlag                  = dllEdgeFlag ;
		qglEdgeFlagPointer           = dllEdgeFlagPointer ;
		qglEdgeFlagv                 = dllEdgeFlagv ;
		qglEnable                    = 	dllEnable                    ;
		qglEnableClientState         = 	dllEnableClientState         ;
		qglEnd                       = 	dllEnd                       ;
		qglEndList                   = 	dllEndList                   ;
		qglEvalCoord1d				 = 	dllEvalCoord1d				 ;
		qglEvalCoord1dv              = 	dllEvalCoord1dv              ;
		qglEvalCoord1f               = 	dllEvalCoord1f               ;
		qglEvalCoord1fv              = 	dllEvalCoord1fv              ;
		qglEvalCoord2d               = 	dllEvalCoord2d               ;
		qglEvalCoord2dv              = 	dllEvalCoord2dv              ;
		qglEvalCoord2f               = 	dllEvalCoord2f               ;
		qglEvalCoord2fv              = 	dllEvalCoord2fv              ;
		qglEvalMesh1                 = 	dllEvalMesh1                 ;
		qglEvalMesh2                 = 	dllEvalMesh2                 ;
		qglEvalPoint1                = 	dllEvalPoint1                ;
		qglEvalPoint2                = 	dllEvalPoint2                ;
		qglFeedbackBuffer            = 	dllFeedbackBuffer            ;
		qglFinish                    = 	dllFinish                    ;
		qglFlush                     = 	dllFlush                     ;
		qglFogf                      = 	dllFogf                      ;
		qglFogfv                     = 	dllFogfv                     ;
		qglFogi                      = 	dllFogi                      ;
		qglFogiv                     = 	dllFogiv                     ;
		qglFrontFace                 = 	dllFrontFace                 ;
		qglFrustum                   = 	dllFrustum                   ;
		qglGenLists                  = 	dllGenLists                  ;
		qglGenTextures               = 	dllGenTextures               ;
		qglGetBooleanv               = 	dllGetBooleanv               ;
		qglGetClipPlane              = 	dllGetClipPlane              ;
		qglGetDoublev                = 	dllGetDoublev                ;
		qglGetError                  = 	dllGetError                  ;
		qglGetFloatv                 = 	dllGetFloatv                 ;
		qglGetIntegerv               = 	dllGetIntegerv               ;
		qglGetLightfv                = 	dllGetLightfv                ;
		qglGetLightiv                = 	dllGetLightiv                ;
		qglGetMapdv                  = 	dllGetMapdv                  ;
		qglGetMapfv                  = 	dllGetMapfv                  ;
		qglGetMapiv                  = 	dllGetMapiv                  ;
		qglGetMaterialfv             = 	dllGetMaterialfv             ;
		qglGetMaterialiv             = 	dllGetMaterialiv             ;
		qglGetPixelMapfv             = 	dllGetPixelMapfv             ;
		qglGetPixelMapuiv            = 	dllGetPixelMapuiv            ;
		qglGetPixelMapusv            = 	dllGetPixelMapusv            ;
		qglGetPointerv               = 	dllGetPointerv               ;
		qglGetPolygonStipple         = 	dllGetPolygonStipple         ;
		qglGetString                 = 	dllGetString                 ;
		qglGetTexEnvfv               = 	dllGetTexEnvfv               ;
		qglGetTexEnviv               = 	dllGetTexEnviv               ;
		qglGetTexGendv               = 	dllGetTexGendv               ;
		qglGetTexGenfv               = 	dllGetTexGenfv               ;
		qglGetTexGeniv               = 	dllGetTexGeniv               ;
		qglGetTexImage               = 	dllGetTexImage               ;
		qglGetTexLevelParameterfv    = 	dllGetTexLevelParameterfv    ;
		qglGetTexLevelParameteriv    = 	dllGetTexLevelParameteriv    ;
		qglGetTexParameterfv         = 	dllGetTexParameterfv         ;
		qglGetTexParameteriv         = 	dllGetTexParameteriv         ;
		qglHint                      = 	dllHint                      ;
		qglIndexMask                 = 	dllIndexMask                 ;
		qglIndexPointer              = 	dllIndexPointer              ;
		qglIndexd                    = 	dllIndexd                    ;
		qglIndexdv                   = 	dllIndexdv                   ;
		qglIndexf                    = 	dllIndexf                    ;
		qglIndexfv                   = 	dllIndexfv                   ;
		qglIndexi                    = 	dllIndexi                    ;
		qglIndexiv                   = 	dllIndexiv                   ;
		qglIndexs                    = 	dllIndexs                    ;
		qglIndexsv                   = 	dllIndexsv                   ;
		qglIndexub                   = 	dllIndexub                   ;
		qglIndexubv                  = 	dllIndexubv                  ;
		qglInitNames                 = 	dllInitNames                 ;
		qglInterleavedArrays         = 	dllInterleavedArrays         ;
		qglIsEnabled                 = 	dllIsEnabled                 ;
		qglIsList                    = 	dllIsList                    ;
		qglIsTexture                 = 	dllIsTexture                 ;
		qglLightModelf               = 	dllLightModelf               ;
		qglLightModelfv              = 	dllLightModelfv              ;
		qglLightModeli               = 	dllLightModeli               ;
		qglLightModeliv              = 	dllLightModeliv              ;
		qglLightf                    = 	dllLightf                    ;
		qglLightfv                   = 	dllLightfv                   ;
		qglLighti                    = 	dllLighti                    ;
		qglLightiv                   = 	dllLightiv                   ;
		qglLineStipple               = 	dllLineStipple               ;
		qglLineWidth                 = 	dllLineWidth                 ;
		qglListBase                  = 	dllListBase                  ;
		qglLoadIdentity              = 	dllLoadIdentity              ;
		qglLoadMatrixd               = 	dllLoadMatrixd               ;
		qglLoadMatrixf               = 	dllLoadMatrixf               ;
		qglLoadName                  = 	dllLoadName                  ;
		qglLogicOp                   = 	dllLogicOp                   ;
		qglMap1d                     = 	dllMap1d                     ;
		qglMap1f                     = 	dllMap1f                     ;
		qglMap2d                     = 	dllMap2d                     ;
		qglMap2f                     = 	dllMap2f                     ;
		qglMapGrid1d                 = 	dllMapGrid1d                 ;
		qglMapGrid1f                 = 	dllMapGrid1f                 ;
		qglMapGrid2d                 = 	dllMapGrid2d                 ;
		qglMapGrid2f                 = 	dllMapGrid2f                 ;
		qglMaterialf                 = 	dllMaterialf                 ;
		qglMaterialfv                = 	dllMaterialfv                ;
		qglMateriali                 = 	dllMateriali                 ;
		qglMaterialiv                = 	dllMaterialiv                ;
		qglMatrixMode                = 	dllMatrixMode                ;
		qglMultMatrixd               = 	dllMultMatrixd               ;
		qglMultMatrixf               = 	dllMultMatrixf               ;
		qglNewList                   = 	dllNewList                   ;
		qglNormal3b                  = 	dllNormal3b                  ;
		qglNormal3bv                 = 	dllNormal3bv                 ;
		qglNormal3d                  = 	dllNormal3d                  ;
		qglNormal3dv                 = 	dllNormal3dv                 ;
		qglNormal3f                  = 	dllNormal3f                  ;
		qglNormal3fv                 = 	dllNormal3fv                 ;
		qglNormal3i                  = 	dllNormal3i                  ;
		qglNormal3iv                 = 	dllNormal3iv                 ;
		qglNormal3s                  = 	dllNormal3s                  ;
		qglNormal3sv                 = 	dllNormal3sv                 ;
		qglNormalPointer             = 	dllNormalPointer             ;
		qglOrtho                     = 	dllOrtho                     ;
		qglPassThrough               = 	dllPassThrough               ;
		qglPixelMapfv                = 	dllPixelMapfv                ;
		qglPixelMapuiv               = 	dllPixelMapuiv               ;
		qglPixelMapusv               = 	dllPixelMapusv               ;
		qglPixelStoref               = 	dllPixelStoref               ;
		qglPixelStorei               = 	dllPixelStorei               ;
		qglPixelTransferf            = 	dllPixelTransferf            ;
		qglPixelTransferi            = 	dllPixelTransferi            ;
		qglPixelZoom                 = 	dllPixelZoom                 ;
		qglPointSize                 = 	dllPointSize                 ;
		qglPolygonMode               = 	dllPolygonMode               ;
		qglPolygonOffset             = 	dllPolygonOffset             ;
		qglPolygonStipple            = 	dllPolygonStipple            ;
		qglPopAttrib                 = 	dllPopAttrib                 ;
		qglPopClientAttrib           = 	dllPopClientAttrib           ;
		qglPopMatrix                 = 	dllPopMatrix                 ;
		qglPopName                   = 	dllPopName                   ;
		qglPrioritizeTextures        = 	dllPrioritizeTextures        ;
		qglPushAttrib                = 	dllPushAttrib                ;
		qglPushClientAttrib          = 	dllPushClientAttrib          ;
		qglPushMatrix                = 	dllPushMatrix                ;
		qglPushName                  = 	dllPushName                  ;
		qglRasterPos2d               = 	dllRasterPos2d               ;
		qglRasterPos2dv              = 	dllRasterPos2dv              ;
		qglRasterPos2f               = 	dllRasterPos2f               ;
		qglRasterPos2fv              = 	dllRasterPos2fv              ;
		qglRasterPos2i               = 	dllRasterPos2i               ;
		qglRasterPos2iv              = 	dllRasterPos2iv              ;
		qglRasterPos2s               = 	dllRasterPos2s               ;
		qglRasterPos2sv              = 	dllRasterPos2sv              ;
		qglRasterPos3d               = 	dllRasterPos3d               ;
		qglRasterPos3dv              = 	dllRasterPos3dv              ;
		qglRasterPos3f               = 	dllRasterPos3f               ;
		qglRasterPos3fv              = 	dllRasterPos3fv              ;
		qglRasterPos3i               = 	dllRasterPos3i               ;
		qglRasterPos3iv              = 	dllRasterPos3iv              ;
		qglRasterPos3s               = 	dllRasterPos3s               ;
		qglRasterPos3sv              = 	dllRasterPos3sv              ;
		qglRasterPos4d               = 	dllRasterPos4d               ;
		qglRasterPos4dv              = 	dllRasterPos4dv              ;
		qglRasterPos4f               = 	dllRasterPos4f               ;
		qglRasterPos4fv              = 	dllRasterPos4fv              ;
		qglRasterPos4i               = 	dllRasterPos4i               ;
		qglRasterPos4iv              = 	dllRasterPos4iv              ;
		qglRasterPos4s               = 	dllRasterPos4s               ;
		qglRasterPos4sv              = 	dllRasterPos4sv              ;
		qglReadBuffer                = 	dllReadBuffer                ;
		qglReadPixels                = 	dllReadPixels                ;
		qglRectd                     = 	dllRectd                     ;
		qglRectdv                    = 	dllRectdv                    ;
		qglRectf                     = 	dllRectf                     ;
		qglRectfv                    = 	dllRectfv                    ;
		qglRecti                     = 	dllRecti                     ;
		qglRectiv                    = 	dllRectiv                    ;
		qglRects                     = 	dllRects                     ;
		qglRectsv                    = 	dllRectsv                    ;
		qglRenderMode                = 	dllRenderMode                ;
		qglRotated                   = 	dllRotated                   ;
		qglRotatef                   = 	dllRotatef                   ;
		qglScaled                    = 	dllScaled                    ;
		qglScalef                    = 	dllScalef                    ;
		qglScissor                   = 	dllScissor                   ;
		qglSelectBuffer              = 	dllSelectBuffer              ;
		qglShadeModel                = 	dllShadeModel                ;
		qglStencilFunc               = 	dllStencilFunc               ;
		qglStencilMask               = 	dllStencilMask               ;
		qglStencilOp                 = 	dllStencilOp                 ;
		qglTexCoord1d                = 	dllTexCoord1d                ;
		qglTexCoord1dv               = 	dllTexCoord1dv               ;
		qglTexCoord1f                = 	dllTexCoord1f                ;
		qglTexCoord1fv               = 	dllTexCoord1fv               ;
		qglTexCoord1i                = 	dllTexCoord1i                ;
		qglTexCoord1iv               = 	dllTexCoord1iv               ;
		qglTexCoord1s                = 	dllTexCoord1s                ;
		qglTexCoord1sv               = 	dllTexCoord1sv               ;
		qglTexCoord2d                = 	dllTexCoord2d                ;
		qglTexCoord2dv               = 	dllTexCoord2dv               ;
		qglTexCoord2f                = 	dllTexCoord2f                ;
		qglTexCoord2fv               = 	dllTexCoord2fv               ;
		qglTexCoord2i                = 	dllTexCoord2i                ;
		qglTexCoord2iv               = 	dllTexCoord2iv               ;
		qglTexCoord2s                = 	dllTexCoord2s                ;
		qglTexCoord2sv               = 	dllTexCoord2sv               ;
		qglTexCoord3d                = 	dllTexCoord3d                ;
		qglTexCoord3dv               = 	dllTexCoord3dv               ;
		qglTexCoord3f                = 	dllTexCoord3f                ;
		qglTexCoord3fv               = 	dllTexCoord3fv               ;
		qglTexCoord3i                = 	dllTexCoord3i                ;
		qglTexCoord3iv               = 	dllTexCoord3iv               ;
		qglTexCoord3s                = 	dllTexCoord3s                ;
		qglTexCoord3sv               = 	dllTexCoord3sv               ;
		qglTexCoord4d                = 	dllTexCoord4d                ;
		qglTexCoord4dv               = 	dllTexCoord4dv               ;
		qglTexCoord4f                = 	dllTexCoord4f                ;
		qglTexCoord4fv               = 	dllTexCoord4fv               ;
		qglTexCoord4i                = 	dllTexCoord4i                ;
		qglTexCoord4iv               = 	dllTexCoord4iv               ;
		qglTexCoord4s                = 	dllTexCoord4s                ;
		qglTexCoord4sv               = 	dllTexCoord4sv               ;
		qglTexCoordPointer           = 	dllTexCoordPointer           ;
		qglTexEnvf                   = 	dllTexEnvf                   ;
		qglTexEnvfv                  = 	dllTexEnvfv                  ;
		qglTexEnvi                   = 	dllTexEnvi                   ;
		qglTexEnviv                  = 	dllTexEnviv                  ;
		qglTexGend                   = 	dllTexGend                   ;
		qglTexGendv                  = 	dllTexGendv                  ;
		qglTexGenf                   = 	dllTexGenf                   ;
		qglTexGenfv                  = 	dllTexGenfv                  ;
		qglTexGeni                   = 	dllTexGeni                   ;
		qglTexGeniv                  = 	dllTexGeniv                  ;
		qglTexImage1D                = 	dllTexImage1D                ;
		qglTexImage2D                = 	dllTexImage2D                ;
		qglTexParameterf             = 	dllTexParameterf             ;
		qglTexParameterfv            = 	dllTexParameterfv            ;
		qglTexParameteri             = 	dllTexParameteri             ;
		qglTexParameteriv            = 	dllTexParameteriv            ;
		qglTexSubImage1D             = 	dllTexSubImage1D             ;
		qglTexSubImage2D             = 	dllTexSubImage2D             ;
		qglTranslated                = 	dllTranslated                ;
		qglTranslatef                = 	dllTranslatef                ;
		qglVertex2d                  = 	dllVertex2d                  ;
		qglVertex2dv                 = 	dllVertex2dv                 ;
		qglVertex2f                  = 	dllVertex2f                  ;
		qglVertex2fv                 = 	dllVertex2fv                 ;
		qglVertex2i                  = 	dllVertex2i                  ;
		qglVertex2iv                 = 	dllVertex2iv                 ;
		qglVertex2s                  = 	dllVertex2s                  ;
		qglVertex2sv                 = 	dllVertex2sv                 ;
		qglVertex3d                  = 	dllVertex3d                  ;
		qglVertex3dv                 = 	dllVertex3dv                 ;
		qglVertex3f                  = 	dllVertex3f                  ;
		qglVertex3fv                 = 	dllVertex3fv                 ;
		qglVertex3i                  = 	dllVertex3i                  ;
		qglVertex3iv                 = 	dllVertex3iv                 ;
		qglVertex3s                  = 	dllVertex3s                  ;
		qglVertex3sv                 = 	dllVertex3sv                 ;
		qglVertex4d                  = 	dllVertex4d                  ;
		qglVertex4dv                 = 	dllVertex4dv                 ;
		qglVertex4f                  = 	dllVertex4f                  ;
		qglVertex4fv                 = 	dllVertex4fv                 ;
		qglVertex4i                  = 	dllVertex4i                  ;
		qglVertex4iv                 = 	dllVertex4iv                 ;
		qglVertex4s                  = 	dllVertex4s                  ;
		qglVertex4sv                 = 	dllVertex4sv                 ;
		qglVertexPointer             = 	dllVertexPointer             ;
		qglViewport                  = 	dllViewport                  ;
	}
}


void GLimp_LogNewFrame( void )
{
	fprintf( glw_state.log_fp, "*** R_BeginFrame ***\n" );
}


