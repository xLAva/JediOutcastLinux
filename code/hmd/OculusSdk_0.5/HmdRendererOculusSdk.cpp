#include "HmdRendererOculusSdk.h"
#include "../../renderer/tr_local.h"
#include "HmdDeviceOculusSdk.h"
#include "../HmdRenderer/PlatformInfo.h"

#include <OVR_CAPI_0_5_0.h>
// stupid OVR include bug
//#define OVR_OS_CONSOLE
//#include <Kernel/OVR_Types.h>
#include <Extras/OVR_Math.h>
#include <OVR_CAPI_GL.h>

#include <math.h>
#include <iostream>
#include <algorithm>

#ifdef _WINDOWS
#include <stdlib.h>
#include <dwmapi.h>
#endif

using namespace OVR;
using namespace std;
using namespace OvrSdk_0_5;

HmdRendererOculusSdk::HmdRendererOculusSdk(HmdDeviceOculusSdk* pHmdDeviceOculusSdk)
    :mIsInitialized(false)
    ,mStartedFrame(false)
    ,mFrameStartTime(0)
    ,mStartedRendering(false)
    ,mEyeId(-1)
    ,mWindowWidth(0)
    ,mWindowHeight(0)
    ,mRenderWidth(0)
    ,mRenderHeight(0)
    ,mGuiScale(1.0f)
    ,mGuiOffsetFactorX(0)
    ,mDismissHealthSafetyWarning(false)
    ,mAllowZooming(false)
    ,mCurrentHmdMode(GAMEWORLD)
    ,mpDevice(pHmdDeviceOculusSdk)
    ,mpHmd(NULL)
{

}

HmdRendererOculusSdk::~HmdRendererOculusSdk()
{

}

bool HmdRendererOculusSdk::Init(int windowWidth, int windowHeight, PlatformInfo platformInfo)
{
    if (mpDevice == NULL || mpDevice->GetHmd() == NULL)
    {
        return false;
    }

    PreparePlatform();

    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;

    mpHmd = mpDevice->GetHmd();

    
    mRenderWidth = mpHmd->Resolution.w/2;
    mRenderHeight = mpHmd->Resolution.h;
    
    if (mpHmd->Type == ovrHmd_DK1)
    {
        mGuiScale = 0.3f;
        mGuiOffsetFactorX = 5.0f;
    }
    else if (mpHmd->Type == ovrHmd_DK2)
    {
        mGuiScale = 0.50f;
        mGuiOffsetFactorX = 0;
        mAllowZooming = true;
    }
    

    // Configure Stereo settings.
    ovrSizei recommenedTex0Size = d_ovrHmd_GetFovTextureSize(mpHmd, ovrEye_Left, mpHmd->DefaultEyeFov[0], 1.0f);
    ovrSizei recommenedTex1Size = d_ovrHmd_GetFovTextureSize(mpHmd, ovrEye_Right, mpHmd->DefaultEyeFov[1], 1.0f);
    
    mRenderWidth = max(recommenedTex0Size.w, recommenedTex1Size.w);
    mRenderHeight = max(recommenedTex0Size.h, recommenedTex1Size.h);


    printf("HmdRendererOculusSdk: target texture size (%dx%d)\n", mRenderWidth, mRenderHeight);
    flush(std::cout);


    for (int i=0; i<FBO_COUNT; i++)
    {
        bool worked = RenderTool::CreateFrameBuffer(mFboInfos[i], mRenderWidth, mRenderHeight);
        if (!worked)
        {
            return false;
        }
    }

    
    bool isRotated = false;
#ifdef LINUX
    int dummyW = 0;
    int dummyH = 0;
    bool isExtended = false;
    
    mpDevice->GetDeviceResolution(dummyW, dummyH, isRotated, isExtended);
#endif
    

    ovrGLConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
    cfg.OGL.Header.BackBufferSize.w = !isRotated ? mpHmd->Resolution.w : mpHmd->Resolution.h;
    cfg.OGL.Header.BackBufferSize.h = !isRotated ? mpHmd->Resolution.h : mpHmd->Resolution.w;
    cfg.OGL.Header.Multisample = 1;

#ifdef LINUX
    cfg.OGL.Disp = platformInfo.pDisplay;
#endif

#ifdef _WINDOWS
    cfg.OGL.Window = platformInfo.Window;
    cfg.OGL.DC = platformInfo.DC;
#endif

    ovrFovPort eyeFov[2];
    eyeFov[0] = mpHmd->DefaultEyeFov[0];
    eyeFov[1] = mpHmd->DefaultEyeFov[1];

    
    unsigned hmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
#ifdef LINUX
    // improve performance on Linux by setting NoVSync until the SDK handles this better
    hmdCaps |= ovrHmdCap_NoVSync;
#endif

    d_ovrHmd_SetEnabledCaps(mpHmd, hmdCaps);
    
    
    unsigned distortionCaps = ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp;
    
    bool PixelLuminanceOverdrive = (mpHmd->DistortionCaps & ovrDistortionCap_Overdrive) ? true : false;
    bool HqAaDistortion = (mpHmd->DistortionCaps & ovrDistortionCap_HqDistortion) ? true : false;

    if (PixelLuminanceOverdrive)
    {
        distortionCaps |= ovrDistortionCap_Overdrive;
    }
    
    if (HqAaDistortion)
    {
        distortionCaps |= ovrDistortionCap_HqDistortion;
    }

#ifdef LINUX
    if (isRotated)
    {
        distortionCaps |= ovrDistortionCap_LinuxDevFullscreen;
    }
#endif
    

    bool worked = d_ovrHmd_ConfigureRendering(mpHmd, &cfg.Config, distortionCaps, eyeFov, mEyeRenderDesc);
    qglUseProgramObjectARB(0);
    if (!worked)
    {
        return false;
    }


    qglBindBuffer(GL_ARRAY_BUFFER, 0);
    qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


    ovrSizei texSize;
    texSize.w = mRenderWidth;
    texSize.h = mRenderHeight;

    ovrRecti renderViewport;
    renderViewport.Pos.x = 0;
    renderViewport.Pos.y = 0;
    renderViewport.Size.w = mRenderWidth;
    renderViewport.Size.h = mRenderHeight;
    
    
    ovrGLTextureData* texData = (ovrGLTextureData*)&EyeTexture[0];
    texData->Header.API            = ovrRenderAPI_OpenGL;
    texData->Header.TextureSize    = texSize;
    texData->Header.RenderViewport = renderViewport;
    texData->TexId                 = mFboInfos[0].ColorBuffer;

    texData = (ovrGLTextureData*)&EyeTexture[1];
    texData->Header.API            = ovrRenderAPI_OpenGL;
    texData->Header.TextureSize    = texSize;
    texData->Header.RenderViewport = renderViewport;
    texData->TexId                 = mFboInfos[1].ColorBuffer;


    mStartedRendering = false;
    mIsInitialized = true;

    return true;
}


void HmdRendererOculusSdk::Shutdown()
{
    if (!mIsInitialized)
    {
        return;
    }

    mpHmd = NULL;

    mIsInitialized = false;
}

std::string HmdRendererOculusSdk::GetInfo()
{
    return "HmdRendererOculusSdk";
}

bool HmdRendererOculusSdk::HandlesSwap()
{
    return true;
}

bool HmdRendererOculusSdk::GetRenderResolution(int& rWidth, int& rHeight)
{
    if (!mIsInitialized)
    {
        return false;
    }

    rWidth = mRenderWidth;
    rHeight = mRenderHeight;

    return true;
}

void HmdRendererOculusSdk::StartFrame()
{
    mStartedFrame = true;

    mFrameStartTime = d_ovr_GetTimeInSeconds();
    mFrameTiming = d_ovrHmd_BeginFrame(mpHmd, 0);
}


void HmdRendererOculusSdk::BeginRenderingForEye(bool leftEye)
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
        // render begin
        mStartedRendering = true;

        ovrTrackingState hmdState;
        ovrVector3f hmdToEyeViewOffset[2] = { mEyeRenderDesc[0].HmdToEyeViewOffset, mEyeRenderDesc[1].HmdToEyeViewOffset };
        d_ovrHmd_GetEyePoses(mpHmd, 0, hmdToEyeViewOffset, mEyePoses, &hmdState);
        
        for (int i=0; i<FBO_COUNT; i++)
        {
            qglBindFramebuffer(GL_FRAMEBUFFER, mFboInfos[i].Fbo);
            RenderTool::ClearFBO(mFboInfos[i]);

            ovrQuatf orientation = mEyePoses[i].Orientation;
            mCurrentOrientations[i] = glm::quat(orientation.w, orientation.x, orientation.y, orientation.z);
            
            ovrVector3f position = mEyePoses[i].Position;
            mCurrentPosition[i] = glm::vec3(position.x, position.y, position.z);
        }

        qglBindBuffer(GL_ARRAY_BUFFER, 0);
        qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        qglBindVertexArray(0);
        qglUseProgramObjectARB(0);
        qglDisable(GL_FRAMEBUFFER_SRGB);
        qglFrontFace(GL_CCW);
    }

    // bind framebuffer
    qglBindFramebuffer(GL_FRAMEBUFFER, mFboInfos[mEyeId].Fbo);


}

void HmdRendererOculusSdk::EndFrame()
{
    if (!mIsInitialized || !mStartedFrame)
    {
        return;
    }

    HandleSafetyWarning();
    
    if (mStartedFrame)
    {
        GLboolean depth_test = qglIsEnabled(GL_DEPTH_TEST);
        GLboolean blend = qglIsEnabled(GL_BLEND);
        GLboolean texture_2d = qglIsEnabled(GL_TEXTURE_2D);
        GLboolean texture_coord_array = qglIsEnabled(GL_TEXTURE_COORD_ARRAY);
        GLboolean color_array = qglIsEnabled(GL_COLOR_ARRAY);
        GLint viewport[4];
        GLint scissor[4];
        GLint texture;
        qglGetIntegerv(GL_VIEWPORT, viewport);
        qglGetIntegerv(GL_SCISSOR_BOX, scissor);
        qglGetIntegerv(GL_TEXTURE_BINDING_2D, &texture);
        
        qglViewport(0, 0, mWindowWidth, mWindowHeight);
        qglScissor(0, 0, mWindowWidth, mWindowHeight);
        
        // set state
        qglBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        qglDisable(GL_DEPTH_TEST);
        qglDisable(GL_BLEND);
        qglEnable(GL_TEXTURE_2D);
        qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
        qglDisableClientState(GL_COLOR_ARRAY);
        qglDisableClientState(GL_VERTEX_ARRAY);


        d_ovrHmd_EndFrame(mpHmd, mEyePoses, EyeTexture);


        // restore the old state
        qglUseProgramObjectARB(0);
        
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
        
        qglEnableClientState(GL_VERTEX_ARRAY);
        
        qglViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        qglScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        qglBindTexture(GL_TEXTURE_2D, texture);


        mStartedFrame = false;
        mStartedRendering = false;

        // keep for debugging
        //RenderTool::DrawFbos(&mFboInfos[0], FBO_COUNT, mWindowWidth, mWindowHeight);
    }
    
    mEyeId = -1;
}


// Determine whether this frame needs rendering based on time-warp timing and flags.
bool HmdRendererOculusSdk::FrameNeedsRendering()
{
    double curtime = mFrameStartTime;
    
    static double   lastUpdate          = 0.0;    
    double          renderInterval      = 0.0;
    double          timeSinceLast       = curtime - lastUpdate;
    bool            updateRenderedView  = true;

    

    if ( (timeSinceLast < 0.0) || ((float)timeSinceLast > renderInterval) )
    {
        // This allows us to do "fractional" speeds, e.g. 45fps rendering on a 60fps display.
        lastUpdate += renderInterval;
        if ( timeSinceLast > 5.0 )
        {
            // renderInterval is probably tiny (i.e. "as fast as possible")
            lastUpdate = curtime;
        }

        updateRenderedView = true;
    }
    else
    {
        updateRenderedView = false;
    }

    return updateRenderedView;
}

void HmdRendererOculusSdk::HandleSafetyWarning()
{
    // Health and Safety Warning display state.
    ovrHSWDisplayState hswDisplayState;
    d_ovrHmd_GetHSWDisplayState(mpHmd, &hswDisplayState);
    if (hswDisplayState.Displayed)
    {
        // Dismiss the warning if the user pressed the appropriate key or if the user
        // is tapping the side of the HMD.
        // If the user has requested to dismiss the warning via keyboard or controller input...
        if (mDismissHealthSafetyWarning)
            d_ovrHmd_DismissHSWDisplay(mpHmd);
        else
        {
            // Detect a moderate tap on the side of the HMD.
            ovrTrackingState ts = d_ovrHmd_GetTrackingState(mpHmd, d_ovr_GetTimeInSeconds());
            if (ts.StatusFlags & ovrStatus_OrientationTracked)
            {
                const glm::vec3 v(ts.RawSensorData.Accelerometer.x,
                ts.RawSensorData.Accelerometer.y,
                ts.RawSensorData.Accelerometer.z);
                
                float lengthSq = glm::length2(v);
                // Arbitrary value and representing moderate tap on the side of the DK2 Rift.
                if (lengthSq > 250.f)
                    d_ovrHmd_DismissHSWDisplay(mpHmd);
            }
        }
    }
}

bool HmdRendererOculusSdk::GetCustomProjectionMatrix(float* rProjectionMatrix, float zNear, float zFar, float fov)
{
    if (!mIsInitialized)
    {
        return false;
    }

    ovrFovPort fovPort = mEyeRenderDesc[mEyeId].Fov;
    bool allowCustomFov = (mCurrentHmdMode == MENU_QUAD_WORLDPOS || mCurrentHmdMode == MENU_QUAD);
    
    // ugly hardcoded default value
    if (mAllowZooming && fov < 124 || allowCustomFov)
    {
        // this calculation only works on DK2 at the moment
        
        // something needs zooming
        float fovRad = DEG2RAD(fov);
        float tanVal = tanf(fovRad * 0.5f);
        fovPort.DownTan = tanVal;
        fovPort.LeftTan = tanVal;
        fovPort.RightTan = tanVal;
        fovPort.UpTan = tanVal;
    }
    

    ovrMatrix4f projMatrix = d_ovrMatrix4f_Projection(fovPort, zNear, zFar, ovrProjection_RightHanded);
    ConvertMatrix(projMatrix, rProjectionMatrix);

    return true;
}

bool HmdRendererOculusSdk::GetCustomViewMatrix(float* rViewMatrix, float& xPos, float& yPos, float& zPos, float bodyYaw, bool noPosition)
{

    if (!mIsInitialized)
    {
        return false;
    }

    
    // get current hmd rotation 
    glm::quat hmdRotation = glm::inverse(mCurrentOrientations[mEyeId]);

    // change hmd orientation to game coordinate system
    glm::quat convertHmdToGame = glm::rotate(glm::quat(1.0f,0.0f,0.0f,0.0f), (float)DEG2RAD(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    convertHmdToGame = glm::rotate(convertHmdToGame, (float)DEG2RAD(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 hmdRotationMat = glm::mat4_cast(hmdRotation) * glm::mat4_cast(convertHmdToGame);


    // convert body transform to matrix
    glm::mat4 bodyPosition = glm::translate(glm::mat4(1.0f), glm::vec3(-xPos, -yPos, -zPos));
    glm::quat bodyYawRotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(DEG2RAD(-bodyYaw)), glm::vec3(0.0f, 0.0f, 1.0f));

    
    float meterToGame = METER_TO_GAME;
    glm::vec3 hmdPos;
    hmdPos.x = mCurrentPosition[mEyeId].z * meterToGame;
    hmdPos.y = mCurrentPosition[mEyeId].x * meterToGame;
    hmdPos.z = mCurrentPosition[mEyeId].y * -meterToGame;
    
    glm::mat4 hmdPosition = glm::translate(glm::mat4(1.0f), hmdPos);    
    
    // create view matrix
    glm::mat4 viewMatrix;
    if (noPosition)
    {
        viewMatrix = hmdRotationMat * glm::mat4_cast(bodyYawRotation) * bodyPosition;
    }
    else
    {
        viewMatrix = hmdRotationMat * hmdPosition* glm::mat4_cast(bodyYawRotation) * bodyPosition;
    }


    
    memcpy(rViewMatrix, &viewMatrix[0][0], sizeof(float) * 16);
    
    
    if (noPosition)
    {
        return true;
    }
    
    // add hmd offset to body pos

    glm::quat bodyYawRotationReverse = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(DEG2RAD(bodyYaw)), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 offsetMat = glm::mat4_cast(bodyYawRotationReverse) * hmdPosition;
    glm::vec3 offsetPos = glm::vec3(offsetMat[3]);
    
    //Vector3f hmdPos2 = Vector3f(hmdPos.x, hmdPos.y, hmdPos.z);
    
    //Matrix4f bodyYawRotationReverse = Matrix4f::RotationZ(DEG2RAD(bodyYaw));
    //Vector3f offsetPos = (bodyYawRotationReverse * Matrix4f::Translation(hmdPos2)).GetTranslation();
    
    /// TODO: do we need this?
    offsetPos *= -1;

    xPos += offsetPos.x;
    yPos += offsetPos.y;
    zPos += offsetPos.z;

    return true;
}

bool HmdRendererOculusSdk::Get2DViewport(int& rX, int& rY, int& rW, int& rH)
{
    // shrink the gui for the HMD display
    float aspect = 1.0f;

    rW = mRenderWidth * mGuiScale;
    rH = mRenderWidth* mGuiScale * aspect;

    rX = (mRenderWidth - rW)/2.0f;
    int xOff = mGuiOffsetFactorX > 0 ? (mRenderWidth / mGuiOffsetFactorX) : 0;
    xOff *= mEyeId == 0 ? 1 : -1;
    rX += xOff;

    rY = (mRenderHeight - rH)/2;

    return true;
}

bool HmdRendererOculusSdk::Get2DOrtho(double &rLeft, double &rRight, double &rBottom, double &rTop, double &rZNear, double &rZFar)
{
    rLeft = 0;
    rRight = 640;
    rBottom = 480;
    rTop = 0;
    rZNear = 0;
    rZFar = 1;

    return true;
}

void HmdRendererOculusSdk::SetCurrentHmdMode(HmdMode mode)
{
    mCurrentHmdMode = mode;
}

bool HmdRendererOculusSdk::AttachToWindow(void* pWindowHandle)
{
#ifdef _WINDOWS
    if (mpDevice == NULL || mpDevice->GetHmd() == NULL)
    {
        return false;
    }
    
    if (!(mpDevice->GetHmd()->HmdCaps & ovrHmdCap_ExtendDesktop))
    {
        d_ovrHmd_AttachToWindow(mpDevice->GetHmd(), pWindowHandle, NULL, NULL);
    }
    
    return true;
#else
    return false;
#endif
}

void HmdRendererOculusSdk::DismissHealthSafetyWarning()
{
    mDismissHealthSafetyWarning = true;
}

void HmdRendererOculusSdk::ConvertMatrix(const ovrMatrix4f& from, float* rTo)
{
    rTo[0] = from.M[0][0];
    rTo[4] = from.M[0][1];
    rTo[8] = from.M[0][2];
    rTo[12] = from.M[0][3];

    rTo[1] = from.M[1][0];
    rTo[5] = from.M[1][1];
    rTo[9] = from.M[1][2];
    rTo[13] = from.M[1][3];

    rTo[2] = from.M[2][0];
    rTo[6] = from.M[2][1];
    rTo[10] = from.M[2][2];
    rTo[14] = from.M[2][3];

    rTo[3] = from.M[3][0];
    rTo[7] = from.M[3][1];
    rTo[11] = from.M[3][2];
    rTo[15] = from.M[3][3];
}

void HmdRendererOculusSdk::PreparePlatform()
{
#ifdef _WINDOWS
    // disable composition on windows (because of some OpenGL issues)
    typedef HRESULT (WINAPI *PFNDWMENABLECOMPOSITIONPROC) (UINT);
    PFNDWMENABLECOMPOSITIONPROC pDwmEnableComposition;

    HINSTANCE HInstDwmapi = LoadLibraryW( L"dwmapi.dll" );

    if (HInstDwmapi)
    {
        pDwmEnableComposition = (PFNDWMENABLECOMPOSITIONPROC)GetProcAddress(HInstDwmapi, "DwmEnableComposition");
        if (pDwmEnableComposition)
        {
            pDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
        }

        FreeLibrary(HInstDwmapi);
        HInstDwmapi = NULL;
    }
#endif
}
