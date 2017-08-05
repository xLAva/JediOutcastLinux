
#include "GameHmd.h"
#include <memory>
#include <memory.h>

GameHmd* GameHmd::sGameHmd = NULL;

GameHmd::GameHmd()
    :mIsInitialized(false)
    ,mUsePositionTracking(false)
    ,mPitch(0)
    ,mYaw(0)
    ,mRoll(0)
    ,mInputYaw(0)
    ,mX(0)
    ,mY(0)
    ,mZ(0)
{

}

GameHmd* GameHmd::Get()
{
    if (sGameHmd == NULL)
    {
        sGameHmd = new GameHmd();
    }

    return sGameHmd;
}

void GameHmd::Destroy()
{
    if (sGameHmd == NULL)
    {
        return;
    }

    delete sGameHmd;
    sGameHmd = NULL;
}

void GameHmd::UpdateHmd(float angles[4])
{
    mIsInitialized = true;

    mPitch = angles[0];
    mYaw = angles[1];
    mRoll = angles[2];
    
    mInputYaw = angles[3];
}


void GameHmd::UpdateHmd(float angles[4], float position[3])
{
    mIsInitialized = true;
    mUsePositionTracking = true;

    mPitch = angles[0];
    mYaw = angles[1];
    mRoll = angles[2];
    mInputYaw = angles[3];
    
    mX = position[0];
    mY = position[1];
    mZ = position[2];
}

bool GameHmd::GetOrientation(float& pitch, float& yaw, float& roll)
{
    if (!mIsInitialized)
    {
        return false;
    }

    pitch = mPitch;
    yaw = mYaw;
    roll = mRoll;
    return true;
}

float GameHmd::GetInputYaw()
{
    return mInputYaw;
}

bool GameHmd::GetPosition(float &rX, float &rY, float &rZ)
{
    if (!mIsInitialized || !mUsePositionTracking)
    {
        return false;
    }

    rX = mX;
    rY = mY;
    rZ = mZ;

    return true;
}
