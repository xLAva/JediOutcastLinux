/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDINPUTOCULUSSDK_1_H
#define HMDINPUTOCULUSSDK_1_H

#include "../HmdInput/HmdInputBase.h"


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

class HmdInputOculusSdk : public HmdInputBase
{
public:
    HmdInputOculusSdk(HmdDeviceOculusSdk* pHmdDeviceOculusSdk);
    virtual ~HmdInputOculusSdk();

    void Update() override;

    size_t GetButtonCount() override;
    bool IsButtonPressed(size_t buttonId) override;

    size_t GetAxisCount() override;
    float GetAxisValue(size_t axisId) override;

protected:


private:
    HmdDeviceOculusSdk* mpDevice;
    ovrInputState mInputState;
    std::vector<ovrButton> mButtonIds;

};
}
#endif
