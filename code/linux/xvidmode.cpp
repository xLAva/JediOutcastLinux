/*
 * DirectDraw XVidMode interface
 *
 * Copyright 2001 TransGaming Technologies, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* 
 * Modified for JKII and JKIII Linux Port
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
#include "linux_local.h"


#include <X11/extensions/xf86vmode.h>
#include <X11/extensions/xf86vmproto.h>

#define SONAME_LIBXXF86VM "libXxf86vm.so.1"

int usexvidmode = 1;

static int xf86vm_event, xf86vm_error, xf86vm_minor;
static int xf86vm_major = 0;

static int xf86vm_gammaramp_size;
static BOOL xf86vm_use_gammaramp;

static struct x11drv_mode_info *dd_modes;
static unsigned int dd_mode_count;
static XF86VidModeModeInfo** real_xf86vm_modes;
static unsigned int real_xf86vm_mode_count;

static Display* gdi_display;




#define MAKE_FUNCPTR(f) static typeof(f) * p##f;
MAKE_FUNCPTR(XF86VidModeQueryExtension)
MAKE_FUNCPTR(XF86VidModeQueryVersion)
MAKE_FUNCPTR(XF86VidModeGetGamma)
MAKE_FUNCPTR(XF86VidModeSetGamma)
MAKE_FUNCPTR(XF86VidModeGetGammaRamp)
MAKE_FUNCPTR(XF86VidModeGetGammaRampSize)
MAKE_FUNCPTR(XF86VidModeSetGammaRamp)
#undef MAKE_FUNCPTR


void X11DRV_XF86VM_Init(Display* dpy)
{
  void *xvidmode_handle;
  Bool ok;
  int nmodes;
  unsigned int i;
  gdi_display = dpy;


  if (xf86vm_major) return; /* already initialized? */

  xvidmode_handle = dlopen(SONAME_LIBXXF86VM, RTLD_NOW);
  if (!xvidmode_handle)
  {
    printf("Unable to open %s, Gamma Control disabled\n", SONAME_LIBXXF86VM);
    usexvidmode = 0;
    return;
  }

#define LOAD_FUNCPTR(f) \
    *(void**)(&p##f) = dlsym(xvidmode_handle, #f);

    LOAD_FUNCPTR(XF86VidModeQueryExtension)
    LOAD_FUNCPTR(XF86VidModeQueryVersion)

    LOAD_FUNCPTR(XF86VidModeGetGamma)
    LOAD_FUNCPTR(XF86VidModeSetGamma)

    LOAD_FUNCPTR(XF86VidModeGetGammaRamp)
    LOAD_FUNCPTR(XF86VidModeGetGammaRampSize)
    LOAD_FUNCPTR(XF86VidModeSetGammaRamp)

#undef LOAD_FUNCPTR

	if (!XF86VidModeQueryExtension || !XF86VidModeQueryVersion || !XF86VidModeGetGammaRamp || !XF86VidModeGetGammaRampSize || !XF86VidModeSetGammaRamp)
	{
    printf("Unable to load function pointers from %s, Gamma Control disabled\n", SONAME_LIBXXF86VM);
    dlclose(xvidmode_handle);
    xvidmode_handle = NULL;
    usexvidmode = 0;
    return;	
	}

  /* see if XVidMode is available */
  if (!(*pXF86VidModeQueryExtension)(gdi_display, &xf86vm_event, &xf86vm_error)) return;

  ok = (*pXF86VidModeQueryVersion)(gdi_display, &xf86vm_major, &xf86vm_minor);
  if (!ok) return;


  if (xf86vm_major > 2 || (xf86vm_major == 2 && xf86vm_minor >= 1))
  {
			xf86vm_gammaramp_size = 0;
      (*pXF86VidModeGetGammaRampSize)(gdi_display, DefaultScreen(gdi_display),
                                   &xf86vm_gammaramp_size);
			if (xf86vm_gammaramp_size == 256)
          xf86vm_use_gammaramp = true;
  }

}


static void GenerateRampFromGamma(unsigned short ramp[256], float gamma)
{
  float r_gamma = 1/gamma;
  unsigned i;
  //printf("gamma is %f\n", r_gamma);
  for (i=0; i<256; i++)
    ramp[i] = pow(i/255.0, r_gamma) * 65535.0;
}

static BOOL ComputeGammaFromRamp(unsigned short ramp[256], float *gamma)
{
  float r_x, r_y, r_lx, r_ly, r_d, r_v, r_e, g_avg, g_min, g_max;
  unsigned i, f, l, g_n, c;
  f = ramp[0];
  l = ramp[255];
  if (f >= l) {
    printf("inverted or flat gamma ramp (%d->%d), rejected\n", f, l);
    return false;
  }
  r_d = l - f;
  g_min = g_max = g_avg = 0.0;
  /* check gamma ramp entries to estimate the gamma */
  //printf("analyzing gamma ramp (%d->%d)\n", f, l);
  for (i=1, g_n=0; i<255; i++) {
    if (ramp[i] < f || ramp[i] > l) {
      printf("strange gamma ramp ([%d]=%d for %d->%d), rejected\n", i, ramp[i], f, l);
      return false;
    }
    c = ramp[i] - f;
    if (!c) continue; /* avoid log(0) */

    /* normalize entry values into 0..1 range */
    r_x = i/255.0; r_y = c / r_d;
    /* compute logarithms of values */
    r_lx = log(r_x); r_ly = log(r_y);
    /* compute gamma for this entry */
    r_v = r_ly / r_lx;
    /* compute differential (error estimate) for this entry */
    /* some games use table-based logarithms that magnifies the error by 128 */
    r_e = -r_lx * 128 / (c * r_lx * r_lx);

    /* compute min & max while compensating for estimated error */
    if (!g_n || g_min > (r_v + r_e)) g_min = r_v + r_e;
    if (!g_n || g_max < (r_v - r_e)) g_max = r_v - r_e;

    /* add to average */
    g_avg += r_v;
    g_n++;
    /* TRACE("[%d]=%d, gamma=%f, error=%f\n", i, ramp[i], r_v, r_e); */
  }
  if (!g_n) {
    printf("no gamma data, shouldn't happen\n");
    return false;
  }
  g_avg /= g_n;
  //printf("low bias is %d, high is %d, gamma is %5.3f\n", f, 65535-l, g_avg);
  /* the bias could be because the app wanted something like a "red shift"
   * like when you're hit in Quake, but XVidMode doesn't support it,
   * so we have to reject a significant bias */
  if (f && f > (pow(1/255.0, g_avg) * 65536.0)) {
    printf("low-biased gamma ramp (%d), rejected\n", f);
    return false;
  }
  /* check that the gamma is reasonably uniform across the ramp */
  if (g_max - g_min > 12.8) {
    printf("ramp not uniform (max=%f, min=%f, avg=%f), rejected\n", g_max, g_min, g_avg);
    return false;
  }
  /* check that the gamma is not too bright */
  if (g_avg < 0.2) {
    printf("too bright gamma ( %5.3f), rejected\n", g_avg);
    return false;
  }
  /* ok, now we're pretty sure we can set the desired gamma ramp,
   * so go for it */
  *gamma = 1/g_avg;
  return true;
}



BOOL X11DRV_XF86VM_GetGammaRamp(struct x11drv_gamma_ramp *ramp)
{

  XF86VidModeGamma gamma;

  if (xf86vm_major < 2) return false; /* no gamma control */

  if (xf86vm_use_gammaramp)
      return (*pXF86VidModeGetGammaRamp)(gdi_display, DefaultScreen(gdi_display), 256,
                                      ramp->red, ramp->green, ramp->blue);

  if ((*pXF86VidModeGetGamma)(gdi_display, DefaultScreen(gdi_display), &gamma))
  {
      GenerateRampFromGamma(ramp->red,   gamma.red);
      GenerateRampFromGamma(ramp->green, gamma.green);
      GenerateRampFromGamma(ramp->blue,  gamma.blue);
      return true;
  }

  return false;
}

BOOL X11DRV_XF86VM_SetGammaRamp(struct x11drv_gamma_ramp *ramp)
{

  XF86VidModeGamma gamma;

  if (xf86vm_major < 2 || !usexvidmode) return false; /* no gamma control */
  if (!ComputeGammaFromRamp(ramp->red,   &gamma.red) || /* ramp validation */
      !ComputeGammaFromRamp(ramp->green, &gamma.green) ||
      !ComputeGammaFromRamp(ramp->blue,  &gamma.blue)) return false;

  if (xf86vm_use_gammaramp)
      return (*pXF86VidModeSetGammaRamp)(gdi_display, DefaultScreen(gdi_display), 256,
                                      ramp->red, ramp->green, ramp->blue);

  return (*pXF86VidModeSetGamma)(gdi_display, DefaultScreen(gdi_display), &gamma);

}




