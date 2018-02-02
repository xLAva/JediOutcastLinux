/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDINPUTOPENVR
#define HMDINPUTOPENVR

#include "../HmdInput/HmdInputBase.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <openvr.h>

using namespace vr;

namespace OpenVr
{
class HmdDeviceOpenVr;

class HmdInputOpenVr : public HmdInputBase
{
public:
    HmdInputOpenVr(HmdDeviceOpenVr* pHmdDeviceOpenVr);
    virtual ~HmdInputOpenVr();

    void Update() override;

    size_t GetButtonCount() override;
    bool IsButtonPressed(size_t buttonId) override;

    size_t GetAxisCount() override;
    float GetAxisValue(size_t axisId) override;

protected:


private:
    HmdDeviceOpenVr* mpDevice;
    VRControllerState_t mControllerStates[2];
    std::vector<EVRButtonId> mButtonIds;

};
}
#endif
