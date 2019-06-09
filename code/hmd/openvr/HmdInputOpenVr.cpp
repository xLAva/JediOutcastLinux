#include "HmdInputOpenVr.h"
#include "HmdDeviceOpenVr.h"

#include "../../game/q_shared.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#ifdef _WINDOWS
#include <stdlib.h>
#include <dwmapi.h>
#endif

using namespace std;
using namespace OpenVr;

HmdInputOpenVr::HmdInputOpenVr(HmdDeviceOpenVr* pHmdDeviceOpenVr)
    :mpDevice(pHmdDeviceOpenVr)
{
    mButtonIds.push_back(k_EButton_System);
    mButtonIds.push_back(k_EButton_ApplicationMenu);
    mButtonIds.push_back(k_EButton_Grip);
    mButtonIds.push_back(k_EButton_DPad_Left);
    mButtonIds.push_back(k_EButton_DPad_Up);
    mButtonIds.push_back(k_EButton_DPad_Right);
    mButtonIds.push_back(k_EButton_DPad_Down);
    mButtonIds.push_back(k_EButton_A);
    mButtonIds.push_back(k_EButton_ProximitySensor);
    mButtonIds.push_back(k_EButton_Axis0);
    mButtonIds.push_back(k_EButton_Axis1);
    mButtonIds.push_back(k_EButton_Axis2);
    mButtonIds.push_back(k_EButton_Axis3);
    mButtonIds.push_back(k_EButton_Axis4);
}

HmdInputOpenVr::~HmdInputOpenVr()
{

}

void HmdInputOpenVr::Update()
{
    if (mpDevice == nullptr)
    {
        return;
    }

    mpDevice->GetControllerState(false, mControllerStates[0]);
    mpDevice->GetControllerState(true, mControllerStates[1]);

    HmdInputBase::Update();
}

size_t HmdInputOpenVr::GetButtonCount()
{
    return mButtonIds.size() * 2;
}

bool HmdInputOpenVr::IsButtonPressed(size_t buttonId)
{
    size_t buttonCount = GetButtonCount();
    if (buttonId >= buttonCount)
    {
        return false;
    }

    bool right = (buttonId > mButtonIds.size());
    if (right)
    {
        buttonId -= mButtonIds.size();
    }

    return (mControllerStates[right ? 1 : 0].ulButtonPressed & ButtonMaskFromId(mButtonIds[buttonId]));
}

size_t HmdInputOpenVr::GetAxisCount()
{
    return 6;
}

float HmdInputOpenVr::GetAxisValue(size_t axisId)
{
    // Set up axis IDs as leftX, leftY, rightX, rightY, leftTrigger, rightTrigger
    bool right = (axisId == 2 || axisId == 3 || axisId == 5);
    bool trigger = (axisId >= 4);

    if (trigger)
    {
        return mControllerStates[right ? 1 : 0].rAxis[k_EButton_SteamVR_Trigger - k_EButton_Axis0].x;
    }

    if (axisId == 0 || axisId == 2)
    {
        return mControllerStates[right ? 1 : 0].rAxis[k_EButton_SteamVR_Touchpad - k_EButton_Axis0].x;
    }
    else
    {
        return -mControllerStates[right ? 1 : 0].rAxis[k_EButton_SteamVR_Touchpad - k_EButton_Axis0].y;
    }
}

