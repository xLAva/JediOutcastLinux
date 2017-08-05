/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef PLATFORMINFO_H
#define PLATFORMINFO_H

struct PlatformInfo
{
public:
    int WindowWidth;
    int WindowHeight;

#ifdef LINUX
    Display* pDisplay;
    Window WindowId;
#endif

#ifdef _WINDOWS
    HWND Window;
    HDC  DC;
#endif
};


#endif
