/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDDEVICEOCULUSSDK_1_H
#define HMDDEVICEOCULUSSDK_1_H

#include "../HmdDevice/IHmdDevice.h"

#include <OVR_CAPI.h>

namespace OvrSdk_1
{
class HmdDeviceOculusSdk : public IHmdDevice
{
public:

    HmdDeviceOculusSdk();
    virtual ~HmdDeviceOculusSdk();

    virtual bool Init(bool allowDummyDevice = false);
    virtual void Shutdown();

    virtual std::string GetInfo();

    virtual bool HasDisplay();
    virtual std::string GetDisplayDeviceName();
    virtual bool GetDisplayPos(int& rX, int& rY);

    virtual bool GetDeviceResolution(int& rWidth, int& rHeight, bool& rIsRotated, bool& rIsExtendedMode);
    virtual bool GetOrientationRad(float& rPitch, float& rYaw, float& rRoll);
    virtual bool GetPosition(float& rX, float& rY, float& rZ);
    virtual void Recenter();


    ovrSession GetHmd() { return mpHmd; }
    ovrHmdDesc GetHmdDesc() { return mDesc; }
    ovrGraphicsLuid GetGraphicsLuid() { return mLuid; }
    
    bool IsDebugHmd() { return mUsingDebugHmd; }


private:
    // disable copy constructor
    HmdDeviceOculusSdk(const HmdDeviceOculusSdk&);
    HmdDeviceOculusSdk& operator=(const HmdDeviceOculusSdk&);

    void ConvertQuatToEuler(const float* quat, float& rYaw, float& rPitch, float& rRoll);
    int GetCpuCount();
    
    bool mIsInitialized;
    bool mUsingDebugHmd;
    bool mPositionTrackingEnabled;
    bool mIsRotated;
    ovrSession mpHmd;
    ovrHmdDesc mDesc;
    ovrGraphicsLuid mLuid;

    std::string mInfo;
};
}
#endif
