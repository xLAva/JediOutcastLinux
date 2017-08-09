
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
	,lPitch(0)
	,lYaw(0)
	,lRoll(0)
	,lX(0)
	,lY(0)
	,lZ(0)
	,rPitch(0)
	,rYaw(0)
	,rRoll(0)
	,rX(0)
	,rY(0)
	,rZ(0)
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

void GameHmd::UpdateHands(float angles_l[3], float position_l[3], float angles_r[3], float position_r[3])
{
	mUseHands = true;

	lPitch = angles_l[0];
	lYaw = angles_l[1];
	lRoll = angles_l[2];

	lX = position_l[0];
	lY = position_l[1];
	lZ = position_l[2];

	rPitch = angles_r[0];
	rYaw = angles_r[1];
	rRoll = angles_r[2];

	rX = position_r[0];
	rY = position_r[1];
	rZ = position_r[2];
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

bool GameHmd::GetPosition(float &outX, float &outY, float &outZ)
{
    if (!mIsInitialized || !mUsePositionTracking)
    {
        return false;
    }

    outX = mX;
    outY = mY;
    outZ = mZ;

    return true;
}

bool GameHmd::HasHands()
{
	return mUseHands;
}

bool GameHmd::GetLeftHandOrientation(float& pitch, float& yaw, float& roll)
{
	if (!mIsInitialized || !mUseHands)
	{
		return false;
	}

	pitch = lPitch;
	yaw = lYaw;
	roll = lRoll;
	return true;
}

bool GameHmd::GetLeftHandPosition(float &outX, float &outY, float &outZ)
{
	if (!mIsInitialized || !mUseHands)
	{
		return false;
	}

	outX = lX;
	outY = lY;
	outZ = lZ;

	return true;
}

bool GameHmd::GetRightHandOrientation(float& pitch, float& yaw, float& roll)
{
	if (!mIsInitialized || !mUseHands)
	{
		return false;
	}

	pitch = rPitch;
	yaw = rYaw;
	roll = rRoll;
	return true;
}

bool GameHmd::GetRightHandPosition(float &outX, float &outY, float &outZ)
{
	if (!mIsInitialized || !mUseHands)
	{
		return false;
	}

	outX = rX;
	outY = rY;
	outZ = rZ;

	return true;
}