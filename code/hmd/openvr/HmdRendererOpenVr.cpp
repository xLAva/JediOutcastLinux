#include "HmdRendererOpenVr.h"
#include "../../renderer/tr_local.h"
#include "HmdDeviceOpenVr.h"
#include "../HmdRenderer/PlatformInfo.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#ifdef _WINDOWS
#include <stdlib.h>
#include <dwmapi.h>
#endif

using namespace std;
using namespace OpenVr;

HmdRendererOpenVr::HmdRendererOpenVr(HmdDeviceOpenVr* pHmdDeviceOpenVr)
    :mIsInitialized(false)
    ,mStartedFrame(false)
    ,mFrameStartTime(0)
    ,mStartedRendering(false)
    ,mEyeId(-1)
    ,mWindowWidth(0)
    ,mWindowHeight(0)
    ,mRenderWidth(0)
    ,mRenderHeight(0)
    ,mGuiScale(0.5f)
    ,mGuiOffsetFactorX(0)
    ,mMeterToGameUnits(IHmdDevice::METER_TO_GAME)
    ,mAllowZooming(false)
    ,mUseMirrorTexture(true)
    ,mpDevice(pHmdDeviceOpenVr)
    ,mpHmd(nullptr)
    ,mMenuStencilDepthBuffer(0)
    ,mCurrentHmdMode(GAMEWORLD)
{

}

HmdRendererOpenVr::~HmdRendererOpenVr()
{

}

bool HmdRendererOpenVr::Init(int windowWidth, int windowHeight, PlatformInfo platformInfo)
{
    if (mpDevice == NULL || mpDevice->GetHmd() == NULL)
    {
        return false;
    }

    PreparePlatform();

    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;

    mpHmd = mpDevice->GetHmd();
    
    //TODO: Adjust to headset
    mGuiScale = 0.475f;
    mGuiOffsetFactorX = 50.0f;

    if (!VRCompositor())
    {
        printf("Compositor initialization failed.\n");
        return false;
    }

    VRCompositor()->SetTrackingSpace(TrackingUniverseStanding);

    mpHmd->GetRecommendedRenderTargetSize(&mRenderWidth, &mRenderHeight);
    
    printf("HmdRendererOpenVr: target texture size (%dx%d)\n", mRenderWidth, mRenderHeight);
    flush(std::cout);

    
    for (int i=0; i<FBO_COUNT; i++)
    {
        bool success = RenderTool::CreateFrameBuffer(mFboInfos[i], mRenderWidth, mRenderHeight);
        if (!success)
        {
            return false;
        }
    }
    
    bool success = RenderTool::CreateFrameBuffer(mFboMenuInfo, mRenderWidth, mRenderHeight);
    if (!success)
    {
        return false;
    }

    mStartedRendering = false;
    mIsInitialized = true;

    return true;
}


void HmdRendererOpenVr::Shutdown()
{
    if (!mIsInitialized)
    {
        return;
    }

    for (int i=0; i<FBO_COUNT; i++)
    {
        qglDeleteFramebuffers(1, &mFboInfos[i].Fbo);
    }

    qglDeleteFramebuffers(1, &mFboMenuInfo.Fbo);

    mpHmd = nullptr;

    mIsInitialized = false;
}

std::string HmdRendererOpenVr::GetInfo()
{
    return "HmdRendererOpenVr";
}

bool HmdRendererOpenVr::HandlesSwap()
{
    return false;
}

bool HmdRendererOpenVr::GetRenderResolution(int& rWidth, int& rHeight)
{
    if (!mIsInitialized)
    {
        return false;
    }

    rWidth = mRenderWidth;
    rHeight = mRenderHeight;

    return true;
}

void HmdRendererOpenVr::StartFrame()
{
    mStartedFrame = true;
}


void HmdRendererOpenVr::BeginRenderingForEye(bool leftEye)
{
    if (!mIsInitialized || !mStartedFrame)
    {
        return;
    }

    int fboId = 0;
    if (!leftEye && FBO_COUNT > 1)
    {
        fboId = 1;
    }

    mEyeId = fboId;

    if (!mStartedRendering)
    {
        mStartedRendering = true;
        // render begin
        //qglDisable(GL_SCISSOR_TEST);
        qglBindVertexArray(0);
        qglUseProgramObjectARB(0);
        qglFrontFace(GL_CCW);
        
        qglDisable(GL_FRAMEBUFFER_SRGB);

        for (int i = 0; i<FBO_COUNT; i++)
        {
            qglBindFramebuffer(GL_FRAMEBUFFER, mFboInfos[i].Fbo);
            RenderTool::ClearFBO(mFboInfos[i]);
        }

        qglBindFramebuffer(GL_FRAMEBUFFER, mFboMenuInfo.Fbo);
        RenderTool::ClearFBO(mFboMenuInfo);
    }


    // bind framebuffer
    // this part can be called multiple times before the end of the frame

    //TODO: Fix this back up?
    //if (mCurrentHmdMode == GAMEWORLD)
    {
        qglBindFramebuffer(GL_FRAMEBUFFER, mFboInfos[mEyeId].Fbo);
    }
    /*else
    {
        if (leftEye)
        {
            qglBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        else
        {
            qglBindFramebuffer(GL_FRAMEBUFFER, mFboMenuInfo.Fbo);
        }
    }*/
}

void HmdRendererOpenVr::EndFrame()
{
    if (!mIsInitialized || !mStartedFrame || mpHmd == nullptr)
    {
        return;
    }
    
    if (mStartedFrame)
    {
        GLboolean depth_test = qglIsEnabled(GL_DEPTH_TEST);
        GLboolean blend = qglIsEnabled(GL_BLEND);
        GLboolean texture_2d = qglIsEnabled(GL_TEXTURE_2D);
        GLboolean texture_coord_array = qglIsEnabled(GL_TEXTURE_COORD_ARRAY);
        GLboolean color_array = qglIsEnabled(GL_COLOR_ARRAY);

        qglDisable(GL_SCISSOR_TEST);
        qglDisable(GL_STENCIL_TEST);
        qglBindFramebuffer(GL_FRAMEBUFFER, 0);

        // We pass in depth buffers too, but at the moment chaperone doesn't handle occlusion even with the data
        // so things look a bit off when it shows up...
        VRTextureWithDepth_t leftEyeDepth, rightEyeDepth;
        leftEyeDepth.depth.handle = (void*)(uintptr_t)mFboInfos[0].DepthBuffer;
        leftEyeDepth.depth.mProjection = projMatrixLeft;
        leftEyeDepth.depth.vRange = { lastZNear, lastZFar };
        leftEyeDepth.handle = (void*)(uintptr_t)mFboInfos[0].ColorBuffer;
        leftEyeDepth.eType = TextureType_OpenGL;
        leftEyeDepth.eColorSpace = ColorSpace_Gamma;

        rightEyeDepth.depth.handle = (void*)(uintptr_t)mFboInfos[1].DepthBuffer;
        rightEyeDepth.depth.mProjection = projMatrixRight;
        rightEyeDepth.depth.vRange = { lastZNear, lastZFar };
        rightEyeDepth.handle = (void*)(uintptr_t)mFboInfos[1].ColorBuffer;
        rightEyeDepth.eType = TextureType_OpenGL;
        rightEyeDepth.eColorSpace = ColorSpace_Gamma;


        VRCompositor()->Submit(Eye_Left, &leftEyeDepth, 0, Submit_TextureWithDepth);
        VRCompositor()->Submit(Eye_Right, &rightEyeDepth, 0, Submit_TextureWithDepth);


        qglBindBuffer(GL_ARRAY_BUFFER, 0);
        qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        qglViewport(0, 0, mWindowWidth, mWindowHeight);

        mStartedFrame = false;
        mStartedRendering = false;

        if (depth_test)
        {
            qglEnable(GL_DEPTH_TEST);
        }
        if (blend)
        {
            qglEnable(GL_BLEND);
        }
        if (!texture_2d)
        {
            qglDisable(GL_TEXTURE_2D);
        }
        if (!texture_coord_array)
        {
            qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (color_array)
        {
            qglEnableClientState(GL_COLOR_ARRAY);
        }

        // keep for debugging
        //TODO: Better mirroring
        RenderTool::DrawFbos(&mFboInfos[0], FBO_COUNT, mWindowWidth, mWindowHeight);
    }
    
    mEyeId = -1;

    // Update all poses
    mpDevice->UpdatePoses();
}


bool HmdRendererOpenVr::GetCustomProjectionMatrix(float* rProjectionMatrix, float zNear, float zFar, float fov)
{
    if (!mIsInitialized)
    {
        return false;
    }

    lastZNear = zNear;
    lastZFar = zFar;

    // FOV is forced by OpenVR, and we override zNear to 4 inches from the face
    HmdMatrix44_t projMatrix = mpHmd->GetProjectionMatrix(mEyeId ? Eye_Right : Eye_Left, 4.0f, zFar);
    ConvertMatrix(projMatrix, rProjectionMatrix);

    if (mEyeId)
    {
        projMatrixRight = projMatrix;
    }
    else
    {
        projMatrixLeft = projMatrix;
    }

    return true;
}

bool HmdRendererOpenVr::GetCustomViewMatrix(float* rViewMatrix, float& xPos, float& yPos, float& zPos, float bodyYaw, bool noPosition)
{
    if (!mIsInitialized)
    {
        return false;
    }

    glm::mat4 hmdPose, eyePose;
    mpDevice->GetHMDMatrix4(hmdPose);

    HmdMatrix34_t eyePoseTransform = mpHmd->GetEyeToHeadTransform(mEyeId ? Eye_Right : Eye_Left);
    eyePose = glm::mat4(eyePoseTransform.m[0][0], eyePoseTransform.m[1][0], eyePoseTransform.m[2][0], 0.0,
        eyePoseTransform.m[0][1], eyePoseTransform.m[1][1], eyePoseTransform.m[2][1], 0.0,
        eyePoseTransform.m[0][2], eyePoseTransform.m[1][2], eyePoseTransform.m[2][2], 0.0,
        eyePoseTransform.m[0][3], eyePoseTransform.m[1][3], eyePoseTransform.m[2][3], 1.0f);

    float pX, pY, pZ;
    mpDevice->GetPosition(pX, pY, pZ);
    pX += eyePose[0][3];
    pY += eyePose[1][3];
    pZ += eyePose[2][3];

    hmdPose *= eyePose;

    glm::quat currentOrientation = glm::quat_cast(hmdPose);
    glm::vec3 currentPosition = glm::vec3(pX, pY, pZ);
    
    if (mCurrentHmdMode == MENU_QUAD_WORLDPOS || mCurrentHmdMode == GAMEWORLD_QUAD_WORLDPOS || mCurrentHmdMode == MENU_QUAD)
    {
        currentOrientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        currentPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    // get current hmd rotation 
    glm::quat hmdRotation = glm::inverse(currentOrientation);

    // change hmd orientation to game coordinate system
    glm::quat convertHmdToGame = glm::rotate(glm::quat(1.0f,0.0f,0.0f,0.0f), (float)DEG2RAD(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    convertHmdToGame = glm::rotate(convertHmdToGame, (float)DEG2RAD(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 hmdRotationMat = glm::mat4_cast(hmdRotation) * glm::mat4_cast(convertHmdToGame);


    // convert body transform to matrix
    glm::mat4 bodyPositionMat = glm::translate(glm::mat4(1.0f), glm::vec3(-xPos, -yPos, -zPos));
    glm::quat bodyYawRotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(DEG2RAD(-bodyYaw)), glm::vec3(0.0f, 0.0f, 1.0f));

    

    glm::vec3 hmdPosition;
    hmdPosition.x = currentPosition.z * mMeterToGameUnits;
    hmdPosition.y = currentPosition.x * mMeterToGameUnits;
    hmdPosition.z = currentPosition.y * -mMeterToGameUnits;
    
    glm::mat4 hmdPositionMat = glm::translate(glm::mat4(1.0f), hmdPosition);
    
    // create view matrix
    glm::mat4 viewMatrix;
    if (noPosition)
    {
        viewMatrix = hmdRotationMat * glm::mat4_cast(bodyYawRotation) * bodyPositionMat;
    }
    else
    {
        viewMatrix = hmdRotationMat * hmdPositionMat * glm::mat4_cast(bodyYawRotation) * bodyPositionMat;
    }


    
    memcpy(rViewMatrix, &viewMatrix[0][0], sizeof(float) * 16);
    
    
    if (noPosition)
    {
        return true;
    }
    
    // add hmd offset to body pos

    glm::quat bodyYawRotationReverse = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(DEG2RAD(bodyYaw)), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 offsetMat = glm::mat4_cast(bodyYawRotationReverse) * hmdPositionMat;
    glm::vec3 offsetPos = glm::vec3(offsetMat[3]);
    
    /// TODO: do we need this?
    offsetPos *= -1;

    xPos += offsetPos.x;
    yPos += offsetPos.y;
    zPos += offsetPos.z;

    return true;
}

bool HmdRendererOpenVr::Get2DViewport(int& rX, int& rY, int& rW, int& rH)
{
    //TODO
    /*if (mCurrentHmdMode == MENU_QUAD_WORLDPOS || mCurrentHmdMode == GAMEWORLD_QUAD_WORLDPOS || mCurrentHmdMode == MENU_QUAD)
    {
        return true;
    }*/

    // shrink the gui for the HMD display
    float aspect = 1.0f;

    float guiScale = mGuiScale;

    rW = mRenderWidth *guiScale;
    rH = mRenderWidth *guiScale * aspect;

    rX = (mRenderWidth - rW)/2.0f;
    int xOff = mGuiOffsetFactorX != 0 ? (mRenderWidth / mGuiOffsetFactorX) : 0;
    xOff *= mEyeId == 0 ? 1 : -1;
    rX += xOff;

    rY = (mRenderHeight - rH) / 2;

    return true;
}


bool HmdRendererOpenVr::Get2DOrtho(double &rLeft, double &rRight, double &rBottom, double &rTop, double &rZNear, double &rZFar)
{
    rLeft = 0;
    rRight = 640;
    rBottom = 480;
    rTop = 0;
    rZNear = 0;
    rZFar = 1;

    return true;
}


void HmdRendererOpenVr::SetCurrentHmdMode(HmdMode mode)
{
    mCurrentHmdMode = mode;
}


void HmdRendererOpenVr::ConvertMatrix(const HmdMatrix44_t& from, float* rTo)
{
    rTo[0] = from.m[0][0];
    rTo[4] = from.m[0][1];
    rTo[8] = from.m[0][2];
    rTo[12] = from.m[0][3];

    rTo[1] = from.m[1][0];
    rTo[5] = from.m[1][1];
    rTo[9] = from.m[1][2];
    rTo[13] = from.m[1][3];

    rTo[2] = from.m[2][0];
    rTo[6] = from.m[2][1];
    rTo[10] = from.m[2][2];
    rTo[14] = from.m[2][3];

    rTo[3] = from.m[3][0];
    rTo[7] = from.m[3][1];
    rTo[11] = from.m[3][2];
    rTo[15] = from.m[3][3];
}

void HmdRendererOpenVr::PreparePlatform()
{
    // disable vsync
    // this only affects the mirror display because the oculus sdk handles vsync internally
    // we have to disable it so that the mirror display does not slow down rendering
    RenderTool::SetVSync(false);
}
