#include "HmdDeviceOculusSdk.h"
#include "../SearchForDisplay.h"

#ifdef FORCE_STATIC_OCULUS_SDK
#include "oculus_static.h"
#else
#include "oculus_dynamic.h"
#endif

#ifdef FORCE_STATIC_OCULUS_SDK
//#define OVR_OS_CONSOLE
//#include "Kernel/OVR_Threads.h"
#endif

#include <cstring>
#include <string>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <algorithm>

#ifdef _WINDOWS
#define _USE_MATH_DEFINES
#include <math.h>
#include <windows.h>
#endif

using namespace std;
using namespace OvrSdk_0_8;

HmdDeviceOculusSdk::HmdDeviceOculusSdk()
    :mIsInitialized(false)
    ,mUsingDebugHmd(false)
    ,mPositionTrackingEnabled(false)
    ,mIsRotated(false)
    ,mpHmd(NULL)
{

}

HmdDeviceOculusSdk::~HmdDeviceOculusSdk()
{

}

bool HmdDeviceOculusSdk::Init(bool allowDummyDevice)
{
    if (mIsInitialized)
    {
        return true;
    }

    bool debugPrint = true;

    if (debugPrint)
    {
        printf("ovr init ...\n");
    }

#if !defined(FORCE_STATIC_OCULUS_SDK)
    ovr_dynamic_load_result loadResult = oculus_dynamic_load(NULL);
    if (loadResult != OVR_DYNAMIC_RESULT_SUCCESS)
    {
        printf("ovr: could not load library\n");
        return false;
    }
#endif

#if defined(OVR_OS_WIN32)
    //OVR::Thread::SetCurrentPriority(OVR::Thread::HighestPriority);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    //if(OVR::Thread::GetCPUCount() >= 4) // Don't do this unless there are at least 4 processors, otherwise the process could hog the machine.
    if (GetCpuCount() >= 4)
    {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    }
#endif
    
    ovrResult result = d_ovr_Initialize(nullptr);
    if (OVR_FAILURE(result))
    {
        printf("ovr: could not init sdk\n");
        return false;
    }
    
    if (debugPrint)
    {
        printf("Create device ...\n");
    }


    result = d_ovr_Create(&mpHmd, &mLuid);

    if (OVR_FAILURE(result))
    {
        d_ovr_Shutdown();

        if (debugPrint)
        {
            printf("ovr init ... failed.\n");
            flush(std::cout);
        }

        return false;
    }

    mDesc = d_ovr_GetHmdDesc(mpHmd);

    mPositionTrackingEnabled = (mDesc.AvailableTrackingCaps & ovrTrackingCap_Position) ? true : false;

    // Start the sensor which provides the Rift’s pose and motion.
    d_ovr_ConfigureTracking(mpHmd, mDesc.DefaultTrackingCaps, 0);

    mInfo = "HmdDeviceOculusSdk:";

    if (strlen(mDesc.ProductName) > 0)
    {
        mInfo += " ";
        mInfo += mDesc.ProductName;
    }

    if (strlen(mDesc.Manufacturer) > 0)
    {
        mInfo += " ";
        mInfo += mDesc.Manufacturer;
    }


    mIsInitialized = true;

    if (debugPrint)
    {
        printf("ovr init ... done.\n");
        flush(std::cout);
    }

    return true;
}

void HmdDeviceOculusSdk::Shutdown()
{
    if (!mIsInitialized)
    {
        return;
    }

    mInfo = "";

    d_ovr_Destroy(mpHmd);
    mpHmd = NULL;

    d_ovr_Shutdown();

    mIsInitialized = false;
}

std::string HmdDeviceOculusSdk::GetInfo()
{
    return mInfo;
}

bool HmdDeviceOculusSdk::HasDisplay()
{
    if (!mIsInitialized || mDesc.Resolution.w <= 0)
    {
        return false;
    }

    return true;
}

std::string HmdDeviceOculusSdk::GetDisplayDeviceName()
{
    return "";
}

bool HmdDeviceOculusSdk::GetDisplayPos(int& rX, int& rY)
{
    return false;
}

bool HmdDeviceOculusSdk::GetDeviceResolution(int& rWidth, int& rHeight, bool& rIsRotated, bool& rIsExtendedMode)
{
    if (!mIsInitialized || mDesc.Resolution.w <= 0)
    {
        return false;
    }

    rWidth = mDesc.Resolution.w;
    rHeight = mDesc.Resolution.h;
    rIsRotated = false;
    rIsExtendedMode = false;

    return true;
}

bool HmdDeviceOculusSdk::GetOrientationRad(float& rPitch, float& rYaw, float& rRoll)
{
    if (!mIsInitialized || mpHmd == nullptr)
    {
        return false;
    }

    // Query the HMD for the sensor state at a given time.
    ovrTrackingState ts  = d_ovr_GetTrackingState(mpHmd, d_ovr_GetTimeInSeconds(), false);
    if ((ts.StatusFlags & ovrStatus_OrientationTracked))
    {
        ovrQuatf orientation = ts.HeadPose.ThePose.Orientation;
        
        float quat[4];
        quat[0] = orientation.x;
        quat[1] = orientation.y;
        quat[2] = orientation.z;
        quat[3] = orientation.w;
        
        ConvertQuatToEuler(&quat[0], rYaw, rPitch, rRoll);
        
        //printf("pitch: %.2f yaw: %.2f roll: %.2f\n", rPitch, rYaw, rRoll);

        return true;
    }

    return false;

}


bool HmdDeviceOculusSdk::GetPosition(float &rX, float &rY, float &rZ)
{
    if (!mIsInitialized || mpHmd == nullptr || !mPositionTrackingEnabled)
    {
        return false;
    }

    // Query the HMD for the sensor state at a given time.
    ovrTrackingState ts  = d_ovr_GetTrackingState(mpHmd, d_ovr_GetTimeInSeconds(), false);
    if ((ts.StatusFlags & ovrStatus_PositionTracked))
    {
        ovrVector3f pos = ts.HeadPose.ThePose.Position;
        rX = pos.x;
        rY = pos.y;
        rZ = pos.z;

        //printf("pitch: %.2f yaw: %.2f roll: %.2f\n", rPitch, rYaw, rRoll);

        return true;
    }

    return false;
}

void HmdDeviceOculusSdk::Recenter()
{
    d_ovr_RecenterPose(mpHmd);
}

void HmdDeviceOculusSdk::ConvertQuatToEuler(const float* quat, float& rYaw, float& rPitch, float& rRoll)
{
    //https://svn.code.sf.net/p/irrlicht/code/trunk/include/quaternion.h
    // modified to get yaw before pitch

    float W = quat[3];
    float X = quat[1];
    float Y = quat[0];
    float Z = quat[2];

    float sqw = W*W;
    float sqx = X*X;
    float sqy = Y*Y;
    float sqz = Z*Z;

    float test = 2.0f * (Y*W - X*Z);

    if (test > (1.0f - 0.000001f))
    {
        // heading = rotation about z-axis
        rRoll = (-2.0f*atan2(X, W));
        // bank = rotation about x-axis
        rYaw = 0;
        // attitude = rotation about y-axis
        rPitch = M_PI/2.0f;
    }
    else if (test < (-1.0f + 0.000001f))
    {
        // heading = rotation about z-axis
        rRoll = (2.0f*atan2(X, W));
        // bank = rotation about x-axis
        rYaw = 0;
        // attitude = rotation about y-axis
        rPitch = M_PI/-2.0f;
    }
    else
    {
        // heading = rotation about z-axis
        rRoll = atan2(2.0f * (X*Y +Z*W),(sqx - sqy - sqz + sqw));
        // bank = rotation about x-axis
        rYaw = atan2(2.0f * (Y*Z +X*W),(-sqx - sqy + sqz + sqw));
        // attitude = rotation about y-axis
        test = max(test, -1.0f);
        test = min(test, 1.0f);
        rPitch = asin(test);
    }
}

int HmdDeviceOculusSdk::GetCpuCount()
{
#if defined(OVR_OS_WIN32)
        SYSTEM_INFO sysInfo;

        #if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501)
            GetNativeSystemInfo(&sysInfo);
        #else
            GetSystemInfo(&sysInfo);
        #endif

        return (int) sysInfo.dwNumberOfProcessors;
#else
    return 1;
#endif
}



