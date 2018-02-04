/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef GAMEHMD_H
#define GAMEHMD_H

class GameHmd
{
public:

    GameHmd();

    static GameHmd* Get();
    static void Destroy();


    bool IsInitialized() { return mIsInitialized; }
    void UpdateHmd(float angles[3]);
    void UpdateHmd(float angles[3], float position[3]);
    void UpdateHands(float angles_l[3], float position_l[3], float angles_r[3], float position_r[3]);

    bool GetOrientation(float& pitch, float& yaw, float& roll);
    bool GetPosition(float& rX, float& rY, float& rZ);
    float GetInputYaw();

    bool HasHands();
    bool GetLeftHandOrientation(float& pitch, float& yaw, float& roll);
    bool GetLeftHandOrientationGrip(float& pitch, float& yaw, float& roll);
    bool GetLeftHandPosition(float &outX, float &outY, float &outZ);
    bool GetRightHandOrientation(float& pitch, float& yaw, float& roll);
    bool GetRightHandOrientationGrip(float& pitch, float& yaw, float& roll);
    bool GetRightHandPosition(float &outX, float &outY, float &outZ);

private:

    // disable copy constructor
    GameHmd(const GameHmd&);
    GameHmd& operator=(const GameHmd&);

    bool mIsInitialized;
    bool mUsePositionTracking;
    bool mUseHands;

    float mPitch;
    float mYaw;
    float mRoll;
    float mInputYaw;

    float mX;
    float mY;
    float mZ;

    float lPitch;
    float lYaw;
    float lRoll;

    float lPitchGrip;
    float lYawGrip;
    float lRollGrip;

    float lX;
    float lY;
    float lZ;

    float rPitch;
    float rYaw;
    float rRoll;

    float rPitchGrip;
    float rYawGrip;
    float rRollGrip;

    float rX;
    float rY;
    float rZ;

    static GameHmd* sGameHmd;



};

#endif
