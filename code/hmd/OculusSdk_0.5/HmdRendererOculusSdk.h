/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDRENDEREROCULUSSDK_0_5_H
#define HMDRENDEREROCULUSSDK_0_5_H

#include "../HmdRenderer/IHmdRenderer.h"
#include "../../renderer/qgl.h"



#include <OVR_CAPI_0_5_0.h>
#include <Extras/OVR_Math.h>

#include "../HmdRenderer/RenderTool.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace OvrSdk_0_5
{
class HmdDeviceOculusSdk;

class HmdRendererOculusSdk : public IHmdRenderer
{
public:
    HmdRendererOculusSdk(HmdDeviceOculusSdk* pHmdDeviceOculusSdk);
    virtual ~HmdRendererOculusSdk() override;

    virtual bool Init(int windowWidth, int windowHeight, PlatformInfo platformInfo) override;
    virtual void Shutdown() override;

    virtual std::string GetInfo() override;

    virtual bool HandlesSwap() override;
    virtual bool GetRenderResolution(int& rWidth, int& rHeight) override;

    virtual void StartFrame() override;
    virtual void BeginRenderingForEye(bool leftEye) override;
    virtual void EndFrame() override;

    virtual bool GetCustomProjectionMatrix(float* rProjectionMatrix, float zNear, float zFar, float fov) override;
    virtual bool GetCustomViewMatrix(float* rViewMatrix, float& xPos, float& yPos, float& zPos, float bodyYaw, bool noPosition) override;

    virtual bool Get2DViewport(int& rX, int& rY, int& rW, int& rH) override;
    virtual bool Get2DOrtho(double &rLeft, double &rRight, double &rBottom, double &rTop, double &rZNear, double &rZFar) override;

    virtual void SetCurrentHmdMode(HmdMode mode) override;

    bool AttachToWindow(void* pWindowHandle);
    void DismissHealthSafetyWarning();

protected:
    static void ConvertMatrix(const ovrMatrix4f& from, float* rTo);

private:
    bool FrameNeedsRendering();
    void HandleSafetyWarning();
    void PreparePlatform();
    
    static const int FBO_COUNT = 2;
    RenderTool::FrameBufferInfo mFboInfos[FBO_COUNT];

    bool mIsInitialized;
    bool mStartedFrame;
    double mFrameStartTime;
    bool mStartedRendering;
    int mEyeId;

    int mWindowWidth;
    int mWindowHeight;

    int mRenderWidth;
    int mRenderHeight;

    float mGuiScale;
    float mGuiOffsetFactorX;

    bool mDismissHealthSafetyWarning;
    bool mAllowZooming;
    
    HmdMode mCurrentHmdMode;
    
    HmdDeviceOculusSdk* mpDevice;
    ovrHmd mpHmd;
    ovrEyeRenderDesc mEyeRenderDesc[2];
    ovrTexture EyeTexture[2];

    ovrEyeType mEyes[2];
    ovrPosef mEyePoses[2];
    ovrFrameTiming mFrameTiming;

    glm::quat mCurrentOrientations[2];
    glm::vec3 mCurrentPosition[2];


};
}
#endif
