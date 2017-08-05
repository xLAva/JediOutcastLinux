#include "HmdRendererOculus.h"
#include "../../renderer/tr_local.h"
#include "PlatformInfo.h"

#include <math.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


HmdRendererOculus::HmdRendererOculus()
    :mIsInitialized(false)
    ,mOculusProgram(0)
    ,mOculusCenter(0)
    ,mWindowWidth(0)
    ,mWindowHeight(0)
    ,mRenderWidth(0)
    ,mRenderHeight(0)
    ,mCurrentFbo(-1)
{

}

HmdRendererOculus::~HmdRendererOculus()
{

}

bool HmdRendererOculus::Init(int windowWidth, int windowHeight, PlatformInfo platformInfo)
{
    mWindowWidth = windowWidth;
    mWindowHeight = windowHeight;


    mRenderWidth = mWindowWidth / 2;
    mRenderHeight = mWindowHeight;

    for (int i=0; i<FBO_COUNT; i++)
    {
        bool worked = RenderTool::CreateFrameBuffer(mFboInfos[i], mRenderWidth, mRenderHeight);
        if (!worked)
        {
            return false;
        }
    }


    mOculusProgram = RenderTool::CreateShaderProgram(GetVertexShader(), GetPixelShader());
    mOculusCenter = qglGetUniformLocationARB(mOculusProgram, "center");

    mCurrentFbo = -1;

    mIsInitialized = true;

    return true;
}


void HmdRendererOculus::Shutdown()
{
    if (!mIsInitialized)
    {
        return;
    }

    qglBindFramebuffer(GL_FRAMEBUFFER, 0);


    mIsInitialized = false;
}

std::string HmdRendererOculus::GetInfo()
{
    return "HmdRendererOculus";
}

bool HmdRendererOculus::HandlesSwap()
{
    return false;
}

bool HmdRendererOculus::GetRenderResolution(int& rWidth, int& rHeight)
{
    if (!mIsInitialized)
    {
        return false;
    }

    rWidth = mRenderWidth;
    rHeight = mRenderHeight;

    return true;
}


void HmdRendererOculus::BeginRenderingForEye(bool leftEye)
{
    if (!mIsInitialized)
    {
        return;
    }

    int fboId = 0;
    if (!leftEye && FBO_COUNT > 1)
    {
        fboId = 1;
    }

    if (mCurrentFbo == fboId)
    {
        return;
    }

    mCurrentFbo = fboId;

    qglBindFramebuffer(GL_FRAMEBUFFER, mFboInfos[mCurrentFbo].Fbo);
    RenderTool::ClearFBO(mFboInfos[mCurrentFbo]);
}

void HmdRendererOculus::EndFrame()
{
    if (!mIsInitialized)
    {
        return;
    }

    RenderTool::DrawFbos(&mFboInfos[0], FBO_COUNT, mWindowWidth, mWindowHeight, mOculusProgram);
}


bool HmdRendererOculus::GetCustomProjectionMatrix(float* rProjectionMatrix, float zNear, float zFar, float fov)
{
    if (!mIsInitialized)
    {
        return false;
    }

    float vResolution = mWindowHeight;
    float hResolution = mWindowWidth;
    float vScreenSize = 0.0935f;
    float hScreenSize = 0.14976f;
    float eyeToScreenDist = 0.041f;
    float lensSeparationDistance = 0.063500f;

    float aspect = hResolution/(2*vResolution);
    float fovD = DEG2RAD(fov); //DEG2RAD(125.0f);//2*atan(vScreenSize / (2*eyeToScreenDist));

    //	 eye distance: 0.064000, eye to screen: 0.041000, distortionScale: 1.714606, yfov: 125.870984,

    float viewCenter = hScreenSize *0.25f;
    float eyeProjectionShift = viewCenter - lensSeparationDistance*0.5f;
    float projectionCenterOffset = 4.0f * eyeProjectionShift / hScreenSize;

    if (mCurrentFbo == 1)
    {
        projectionCenterOffset *= -1;
    }

    glm::mat4 perspMat = glm::perspective(fovD, aspect, zNear, zFar);
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(projectionCenterOffset, 0, 0));
    perspMat = translate * perspMat;

    memcpy(rProjectionMatrix, &perspMat[0][0], sizeof(float)*16);

    return true;
}

bool HmdRendererOculus::GetCustomViewMatrix(float* rViewMatrix, float& xPos, float& yPos, float& zPos, float bodyYaw, bool noPosition)
{
    return false;
}

bool HmdRendererOculus::Get2DViewport(int& rX, int& rY, int& rW, int& rH)
{
    // shrink the gui for the HMD display
    float scale = 0.3f;
    float aspect = 1.0f;

    rW = mRenderWidth * scale;
    rH = mRenderWidth* scale * aspect;

    rX = (mRenderWidth - rW)/2.0f;
    int xOff = mRenderWidth/10.0f;
    xOff *= mCurrentFbo == 0 ? 1 : -1;
    rX += xOff;

    rY = (mRenderHeight - rH)/2;

    return true;
}


bool HmdRendererOculus::Get2DOrtho(double &rLeft, double &rRight, double &rBottom, double &rTop, double &rZNear, double &rZFar)
{
    rLeft = 0;
    rRight = 640;
    rBottom = 480;
    rTop = 0;
    rZNear = 0;
    rZFar = 1;

    return true;
}



const char* HmdRendererOculus::GetVertexShader()
{
    return
        "void main() {\n"
        "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
        "   gl_Position = ftransform();\n"
        "}";
}

const char* HmdRendererOculus::GetPixelShader()
{
    // only works with DK1

    return

        "#version 120\n"
        "\n"
        "            // Taken from mts3d forums, from user fredrik.\n"
        "           \n"
        "            uniform sampler2D warpTexture;\n"
        "            \n"
        "            const vec2 LeftLensCenter = vec2(0.2863248, 0.5);\n"
        "            const vec2 RightLensCenter = vec2(0.7136753, 0.5);\n"
        "            const vec2 LeftScreenCenter = vec2(0.25, 0.5);\n"
        "            const vec2 RightScreenCenter = vec2(0.75, 0.5);\n"
        "            const vec2 Scale = vec2(0.1469278, 0.2350845);\n"
        "            const vec2 ScaleIn = vec2(4, 2.5);\n"
        "            const vec4 HmdWarpParam   = vec4(1, 0.22, 0.24, 0);\n"
        "            \n"
        "            // Scales input texture coordinates for distortion.\n"
        "            vec2 HmdWarp(vec2 in01, vec2 LensCenter)\n"
        "            {\n"
        "                vec2 theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]\n"
        "                float rSq = theta.x * theta.x + theta.y * theta.y;\n"
        "                vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +\n"
        "                    HmdWarpParam.z * rSq * rSq +\n"
        "                    HmdWarpParam.w * rSq * rSq * rSq);\n"
        "                return LensCenter + Scale * rvector;\n"
        "            }\n"
        "            \n"
        "            void main()\n"
        "            {\n"
        "                // The following two variables need to be set per eye\n"
        "                vec2 LensCenter = gl_FragCoord.x < 640 ? LeftLensCenter : RightLensCenter;\n"
        "                vec2 ScreenCenter = gl_FragCoord.x < 640 ? LeftScreenCenter : RightScreenCenter;\n"
        "            \n"
        "                vec2 oTexCoord = gl_FragCoord.xy / vec2(1280, 800);\n"
        "            \n"
        "                vec2 tc = HmdWarp(oTexCoord, LensCenter);\n"
        "                if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))\n"
        "                {\n"
        "                    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "                    return;\n"
        "                }\nCurrentFbo"
        "            \n"
        "                tc.x = gl_FragCoord.x < 640 ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));\n"
        "                gl_FragColor = texture2D(warpTexture, tc);\n"
        "            }";

#if 0
    "#version 120\n"

    "uniform sampler2D bgl_RenderTexture;\n"
    "//uniform vec2 LensCenter;\n"
    "//const vec2 LeftLensCenter = vec2(0.2863248, 0.5);\n"
    "//const vec2 RightLensCenter = vec2(0.7136753, 0.5);\n"
    "const vec2 LeftLensCenter = vec2(0.348023504, 0.5);\n"
    "const vec2 RightLensCenter = vec2(0.651976496, 0.5);\n"
    "const vec2 LeftScreenCenter = vec2(0.25, 0.5);\n"
    "const vec2 RightScreenCenter = vec2(0.75, 0.5);\n"
    "const vec2 Scale = vec2(1.0, 1.0);\n"
    "const vec2 ScaleIn = vec2(1, 1.0);\n"
    "//const vec2 Scale = vec2(0.1469278, 0.2350845);\n"
    "//const vec2 ScaleIn = vec2(4, 2.5);\n"
    "//const vec2 Scale = vec2(1.714606, 1.714606);\n"
    "//const vec2 ScaleIN = vec2(0.8, 0.8);\n"
    "const vec4 HmdWarpParam = vec4(1, 0.22, 0.24, 0);\n"

    // Scales input texture coordinates for distortion.
    "vec2 HmdWarp(vec2 in01, vec2 LensCenter)\n"
    "{\n"
    "   vec2 theta = (in01- LensCenter) * ScaleIn; // Scales to [-1, 1]\n"
    "   float rSq = theta.x * theta.x + theta.y * theta.y;\n"
    "   vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +\n"
    "      HmdWarpParam.z * rSq * rSq +\n"
    "      HmdWarpParam.w * rSq * rSq * rSq);\n"
    "   //return LensCenter + Scale * theta3;\n"
    "   return LensCenter + Scale * rvector;\n"
    "}\n"

    "void main()\n"
    "{\n"
    // The following two variables need to be set per eye
    "   vec2 LensCenter = gl_FragCoord.x < 640 ? LeftLensCenter : RightLensCenter;\n"
    "   vec2 ScreenCenter = gl_FragCoord.x < 640 ? LeftScreenCenter : RightScreenCenter;\n"

    "   //vec2 oTexCoord = gl_TexCoord[0].st;\n"
    "		//oTexCoord.x = oTexCoord.x* 0.5f;\n"
    "   //oTexCoord.x = oTexCoord.x + gl_TexCoord[0].x < 640 ? 0 : 0.5f;\n"
    "   //vec2 oTexCoord = gl_FragCoord.xy;\n"
    "   vec2 oTexCoord = (gl_FragCoord.xy + vec2(0.5, 0.5)) / vec2(1280, 800);  //Uncomment if using BGE's built-in stereo rendering\n"

    "   vec2 tc = HmdWarp(oTexCoord, LensCenter);\n"
    "   if (any(bvec2(clamp(tc,ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))\n"
    "   {\n"
    "      gl_FragColor = vec4(vec3(0.0), 1.0);\n"
    "      return;\n"
    "   }\n"

    " 	tc.x = gl_FragCoord.x < 640 ? (2.0 * tc.x) : (2.0 * (tc.x - 0.5));  //Uncomment if using BGE's built-in stereo rendering\n"
    "   gl_FragColor = texture2D(bgl_RenderTexture, tc);\n"
    "}\n";

#endif
}
