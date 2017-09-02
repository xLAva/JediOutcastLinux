#include "HmdInputOculusSdk.h"
#include "HmdDeviceOculusSdk.h"

#include <OVR_CAPI.h>
#include <Extras/OVR_Math.h>
#include <OVR_CAPI_GL.h>

#ifdef FORCE_STATIC_OCULUS_SDK
#include "oculus_static.h"
#else
#include "oculus_dynamic.h"
#endif

#include <math.h>
#include <iostream>
#include <algorithm>

#ifdef _WINDOWS
#include <stdlib.h>
#include <dwmapi.h>
#endif

using namespace OVR;
using namespace std;
using namespace OvrSdk_1;

HmdInputOculusSdk::HmdInputOculusSdk(HmdDeviceOculusSdk* pHmdDeviceOculusSdk)
    :mpDevice(pHmdDeviceOculusSdk)
{

}

HmdInputOculusSdk::~HmdInputOculusSdk()
{

}

size_t HmdInputOculusSdk::GetButtonCount()
{
    return 0;
}

bool HmdInputOculusSdk::IsButtonPressed(size_t buttonId)
{
    return false;
}

size_t HmdInputOculusSdk::GetAxisCount()
{
    return 0;
}

float HmdInputOculusSdk::GetAxisValue(size_t axisId)
{
    return 0.0f;
}
