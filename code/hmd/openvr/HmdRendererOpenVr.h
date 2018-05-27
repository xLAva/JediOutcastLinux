/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2015 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDRENDEREROPENVR
#define HMDRENDEREROPENVR

#include "../HmdRenderer/IHmdRenderer.h"
#include "../../renderer/qgl.h"

#include "../HmdRenderer/RenderTool.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
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

class HmdRendererOpenVr : public IHmdRenderer
{
public:
    HmdRendererOpenVr(HmdDeviceOpenVr* pHmdDeviceOpenVr);
    virtual ~HmdRendererOpenVr();

    virtual bool Init(int windowWidth, int windowHeight, PlatformInfo platformInfo) override;
    virtual void Shutdown() override;

    virtual std::string GetInfo() override;

    virtual bool HandlesSwap()override;
    virtual bool GetRenderResolution(int& rWidth, int& rHeight) override;

    virtual void StartFrame() override;
    virtual void BeginRenderingForEye(bool leftEye) override;
    virtual void EndFrame() override;

    virtual bool GetCustomProjectionMatrix(float* rProjectionMatrix, float zNear, float zFar, float fov) override;
    virtual bool GetCustomViewMatrix(float* rViewMatrix, float& xPos, float& yPos, float& zPos, float bodyYaw, bool noPosition) override;

    virtual bool Get2DViewport(int& rX, int& rY, int& rW, int& rH) override;
    virtual bool Get2DOrtho(double &rLeft, double &rRight, double &rBottom, double &rTop, double &rZNear, double &rZFar) override;

    virtual void SetCurrentHmdMode(HmdMode mode) override;
    virtual bool HasQuadWorldPosSupport() override { return true; }

protected:
    static void ConvertMatrix(const HmdMatrix44_t& from, float* rTo);

private:
    void PreparePlatform();
    
    static const int FBO_COUNT = 2;
    RenderTool::FrameBufferInfo mFboInfos[FBO_COUNT];
    RenderTool::FrameBufferInfo mFboMenuInfo;

    bool mIsInitialized;
    bool mStartedFrame;
    double mFrameStartTime;
    bool mStartedRendering;
    int mEyeId;

    int mWindowWidth;
    int mWindowHeight;

    uint32_t mRenderWidth;
    uint32_t mRenderHeight;

    float mGuiScale;
    float mGuiOffsetFactorX;

    float mMeterToGameUnits;

    bool mAllowZooming;

    bool mUseMirrorTexture;

    HmdDeviceOpenVr* mpDevice;
    IVRSystem* mpHmd;

    GLuint mEyeStencilBuffer[2]; 
    GLuint mMenuStencilDepthBuffer;
    
    HmdMatrix44_t projMatrixLeft;
    HmdMatrix44_t projMatrixRight;
    float lastZNear;
    float lastZFar;

    HmdMode mCurrentHmdMode;
};
}
#endif
