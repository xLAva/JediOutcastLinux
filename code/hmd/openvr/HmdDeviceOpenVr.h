/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDDEVICEOPENVR
#define HMDDEVICEOPENVR

#include "../HmdDevice/IHmdDevice.h"

#include <glm/glm.hpp>
#include <openvr.h>

using namespace vr;

namespace OpenVr
{
class HmdDeviceOpenVr : public IHmdDevice
{
public:

    HmdDeviceOpenVr();
    virtual ~HmdDeviceOpenVr();

    virtual bool Init(bool allowDummyDevice = false);
    virtual void Shutdown();

    virtual std::string GetInfo();
    virtual std::string GetAudioDeviceName();

    virtual bool HasDisplay();
    virtual bool HandlesControllerInput() { return true; }
    virtual std::string GetDisplayDeviceName();
    virtual bool GetDisplayPos(int& rX, int& rY);

    virtual bool GetDeviceResolution(int& rWidth, int& rHeight, bool& rIsRotated, bool& rIsExtendedMode);
    virtual bool GetOrientationRad(float& rPitch, float& rYaw, float& rRoll);
    virtual bool GetPosition(float& rX, float& rY, float& rZ);
    virtual bool GetHandOrientationRad(bool rightHand, float& rPitch, float& rYaw, float& rRoll);
    virtual bool GetHandOrientationGripRad(bool rightHand, float& rPitch, float& rYaw, float& rRoll);
    virtual bool GetHandPosition(bool rightHand, float& rX, float& rY, float& rZ);
    virtual bool HasHand(bool rightHand);
    virtual void Recenter();

    void UpdatePoses();
    bool GetHMDMatrix4(glm::mat4& mat);
    bool GetHandMatrix4(bool rightHand, glm::mat4& mat);

    void GetControllerState(bool rightHand, VRControllerState_t& state);

    IVRSystem* GetHmd() { return mpHmd; }
    bool IsDebugHmd() { return mUsingDebugHmd; }
	bool IsSeatedPositionInUse() { return mUseSeatedPosition; }


private:
    // disable copy constructor
    HmdDeviceOpenVr(const HmdDeviceOpenVr&);
    HmdDeviceOpenVr& operator=(const HmdDeviceOpenVr&);

	HmdMatrix34_t GetPoseWithOffset(const HmdMatrix34_t& pose);
    void ConvertQuatToEuler(const float* quat, float& rYaw, float& rPitch, float& rRoll);

    
    bool mIsInitialized;
    bool mUsingDebugHmd;
    bool mPositionTrackingEnabled;
	bool mUseSeatedPosition;
    bool mIsRotated;

    IVRSystem* mpHmd;
    uint32_t mTrackerIdHandLeft;
    uint32_t mTrackerIdHandRight;
    size_t mTrackableDeviceCount;
    TrackedDevicePose_t mrTrackedDevicePose[k_unMaxTrackedDeviceCount];

    std::string mInfo;

    float mHeightAdjust;

};
}
#endif
