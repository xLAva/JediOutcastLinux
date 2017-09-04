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
    mInputState = {};

    mButtonIds.push_back(ovrButton_A);
    mButtonIds.push_back(ovrButton_B);
    mButtonIds.push_back(ovrButton_X);
    mButtonIds.push_back(ovrButton_Y);
    mButtonIds.push_back(ovrButton_Back);
    mButtonIds.push_back(ovrButton_EnumSize);
    mButtonIds.push_back(ovrButton_Enter);
    mButtonIds.push_back(ovrButton_LThumb);
    mButtonIds.push_back(ovrButton_RThumb);
    mButtonIds.push_back(ovrButton_LShoulder);
    mButtonIds.push_back(ovrButton_RShoulder);
    mButtonIds.push_back(ovrButton_Up);
    mButtonIds.push_back(ovrButton_Down);
    mButtonIds.push_back(ovrButton_Left);
    mButtonIds.push_back(ovrButton_Right);

}

HmdInputOculusSdk::~HmdInputOculusSdk()
{

}

void HmdInputOculusSdk::Update()
{
    if (mpDevice == nullptr)
    {
        return;
    }

    ovrSession session = mpDevice->GetHmd();
    ovrControllerType controllerType = ovrControllerType_Touch;
    d_ovr_GetInputState(session, controllerType, &mInputState);

    HmdInputBase::Update();
}

size_t HmdInputOculusSdk::GetButtonCount()
{
    return mButtonIds.size() + 4;
}

bool HmdInputOculusSdk::IsButtonPressed(size_t buttonId)
{
    size_t buttonCount = GetButtonCount();
    if (buttonId >= buttonCount)
    {
        return false;
    }

    size_t buttonIdCount = mButtonIds.size();

    if (buttonId < buttonIdCount)
    {
        ovrButton button = mButtonIds[buttonId];
        if (button != ovrButton_EnumSize)
        {
            return (mInputState.Buttons & button) != 0;
        }

        return false;
    }

    size_t virtualButtonId = (buttonId - buttonIdCount);

    ovrHandType hand = (virtualButtonId % 2) == 0 ? ovrHand_Left : ovrHand_Right;

    const float pressedValue = 0.7f;

    if (virtualButtonId < 2)
    {
        return (mInputState.IndexTrigger[hand] >= pressedValue);
    }

    return (mInputState.HandTrigger[hand] >= pressedValue);
}

size_t HmdInputOculusSdk::GetAxisCount()
{
    return 4;
}

float HmdInputOculusSdk::GetAxisValue(size_t axisId)
{
    ovrHandType hand = ovrHand_Left;
    if (axisId >= 2)
    {
        hand = ovrHand_Right;
    }

    bool xAxis = (axisId % 2 == 0);

    ovrVector2f thumbstick = mInputState.Thumbstick[hand];
    return xAxis ? thumbstick.x : -thumbstick.y;
}

