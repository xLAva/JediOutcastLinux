/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDINPUTOCULUSSDK_1_H
#define HMDINPUTOCULUSSDK_1_H

#include "../HmdInput/IHmdInput.h"


#include <OVR_CAPI.h>
#include <Extras/OVR_Math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace OvrSdk_1
{
class HmdDeviceOculusSdk;

class HmdInputOculusSdk : public IHmdInput
{
public:
    HmdInputOculusSdk(HmdDeviceOculusSdk* pHmdDeviceOculusSdk);
    virtual ~HmdInputOculusSdk();


    HmdDeviceOculusSdk* mpDevice;

};
}
#endif
