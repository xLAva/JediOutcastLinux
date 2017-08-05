/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef IHMDDEVICE_H
#define IHMDDEVICE_H

#include <string>

class IHmdDevice
{
public:
    virtual ~IHmdDevice() {}

    virtual bool Init(bool allowDummyDevice = false) = 0;
    virtual void Shutdown() = 0;

    virtual std::string GetInfo() = 0;

    virtual bool HasDisplay() = 0;
    virtual std::string GetDisplayDeviceName() = 0;
    virtual bool GetDisplayPos(int& rX, int& rY) = 0;

    // return false if no display is used
    virtual bool GetDeviceResolution(int& rWidth, int& rHeight, bool& rIsRotated, bool& rIsExtendedMode) = 0;
    virtual bool GetOrientationRad(float& rPitch, float& rYaw, float& rRoll) = 0;
    virtual bool GetPosition(float& rX, float& rY, float& rZ) = 0;
    
    virtual void Recenter() = 0;

};

#endif
