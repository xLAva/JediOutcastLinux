#include "ClientHmd.h"
#include "HmdDevice/IHmdDevice.h"
#include "Quake3/GameMenuHmdManager.h"

#include "../game/q_shared.h"
#include "../client/vmachine.h"

#include <memory>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


ClientHmd* ClientHmd::sClientHmd = NULL;

ClientHmd::ClientHmd()
    :mpDevice(nullptr)
    ,mpRenderer(nullptr)
    ,mpGameMenuHmdManager(nullptr)
    ,mIsInitialized(false)
    ,mLastViewangleYaw(0)
    ,mViewangleDiff(0)
{

}

ClientHmd::~ClientHmd()
{
    if (mpGameMenuHmdManager != nullptr)
    {
        delete(mpGameMenuHmdManager);
        mpGameMenuHmdManager = nullptr;
    }
}

ClientHmd* ClientHmd::Get()
{
    if (sClientHmd == NULL)
    {
        sClientHmd = new ClientHmd();
    }

    return sClientHmd;
}

void ClientHmd::Destroy()
{
    if (sClientHmd == NULL)
    {
        return;
    }

    delete sClientHmd;
    sClientHmd = NULL;
}

void ClientHmd::UpdateInputView(float yawDiff, float& rPitch, float& rYaw, float& rRoll)
{
    if (mpDevice == NULL)
    {
        return;
    }

    if (!mIsInitialized)
    {
        mIsInitialized = true;
        mLastViewangleYaw = rYaw;
    }

    mViewangleDiff += yawDiff;
    mViewangleDiff = fmod(mViewangleDiff, 360.0f);

    mLastViewangleYaw = rYaw;

    float pitch = 0;
    float yaw = 0;
    float roll = 0;

    // ignore failed orientation
    // it will alsow fail during rendering
    // we need to keep render orientation and input orientation the same
    GetOrientation(pitch, yaw, roll);

    rPitch = pitch;

    rPitch = std::max(rPitch, -80.0f);
    rPitch = std::min(rPitch, 80.0f);

    rYaw = yaw + mViewangleDiff;
    mLastViewangleYaw = rYaw;
}

void ClientHmd::UpdateGame()
{
    if (mpDevice == NULL)
    {
        return;
    }

    GameMenuHmdManager* pManager = GetGameMenuHmdManager();
    if (pManager)
    {
        pManager->Update();
    }

    float angles[4];

    bool worked = GetOrientation(angles[0], angles[1], angles[2]);

    if (!worked)
    {
        return;
    }

    //printf("pitch: %.2f yaw: %.2f roll: %.2f\n", pitch, yaw, roll);

    angles[3] = mViewangleDiff;

    float position[3];
    bool usePosition = false; //GetPosition(position[0], position[1], position[2]);
    if (usePosition)
    {
        VM_Call(CG_HMD_UPDATE_ROT_POS, &angles[0], &position[0]);
    }
    else
    {
        VM_Call(CG_HMD_UPDATE_ROT, &angles[0]);
    }
}


bool ClientHmd::GetOrientation(float& rPitch, float& rYaw, float& rRoll)
{
    if (mpDevice == NULL)
    {
        return false;
    }

    bool worked = mpDevice->GetOrientationRad(rPitch, rYaw, rRoll);
    if (!worked)
    {
        return false;
    }

    rPitch = RAD2DEG(-rPitch);
    rYaw = RAD2DEG(rYaw);
    rRoll = RAD2DEG(-rRoll);

    return true;
}

bool ClientHmd::GetPosition(float& rX, float& rY, float& rZ)
{
    if (mpDevice == NULL)
    {
        return false;
    }

    bool worked = mpDevice->GetPosition(rX, rY, rZ);
    if (!worked)
    {
        return false;
    }


    // convert body transform to matrix
    //Matrix4f bodyYawRotation = Matrix4f::RotationZ(DEG2RAD(-bodyYaw));
    
    
    float meterToGame = 26.2464f;// (3.2808f * 8.0f); // meter to feet * game factor 8
    //Vector3f bodyPos = Vector3f(xPos, yPos, zPos);
    //bodyPos *= -1;
    
    //Vector3f hmdPos;
    //hmdPos.x = mCurrentPosition[mEyeId].z * meterToGame;
    //hmdPos.y = mCurrentPosition[mEyeId].x * meterToGame;
    //hmdPos.z = mCurrentPosition[mEyeId].y * -meterToGame;
    
    
    //Matrix4f bodyPosition = Matrix4f::Translation(bodyPos);
    //Matrix4f hmdPosition = Matrix4f::Translation(hmdPos);
    
    //mCurrentView = hmdRotation * hmdPosition * bodyYawRotation * bodyPosition;
    
    
    glm::vec3 hmdPosition = glm::vec3(rZ * meterToGame, rX * meterToGame, -rY * meterToGame);
    glm::quat bodyYawRotation = glm::rotate(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), (float)(DEG2RAD(-mViewangleDiff)), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // create view matrix
    glm::vec3 hmdPositionOffsetInGame = bodyYawRotation * hmdPosition;
    
    rX = hmdPositionOffsetInGame.x;
    rY = hmdPositionOffsetInGame.y;
    rZ = hmdPositionOffsetInGame.z;

    return true;
}

void ClientHmd::SetRenderer(IHmdRenderer* pRenderer) 
{ 
    mpRenderer = pRenderer; 

    GameMenuHmdManager* pGameMenuHmdManager = GetGameMenuHmdManager();
    pGameMenuHmdManager->SetHmdRenderer(pRenderer);
}

GameMenuHmdManager* ClientHmd::GetGameMenuHmdManager()
{
    if (mpGameMenuHmdManager == nullptr)
    {
        mpGameMenuHmdManager = new GameMenuHmdManager();
    }

    return mpGameMenuHmdManager;
}