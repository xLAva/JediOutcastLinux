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

    bool GetOrientation(float& pitch, float& yaw, float& roll);
    bool GetPosition(float& rX, float& rY, float& rZ);
    float GetInputYaw();

private:

    // disable copy constructor
    GameHmd(const GameHmd&);
    GameHmd& operator=(const GameHmd&);

    bool mIsInitialized;
    bool mUsePositionTracking;

    float mPitch;
    float mYaw;
    float mRoll;
    float mInputYaw;

    float mX;
    float mY;
    float mZ;

    static GameHmd* sGameHmd;



};

#endif
