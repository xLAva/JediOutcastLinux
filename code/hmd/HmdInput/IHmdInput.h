/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef IHMDINPUT_H
#define IHMDINPUT_H

#include <string>


class IHmdInput
{
public:

    virtual ~IHmdInput() {}

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    virtual void Update() = 0;

    virtual size_t GetButtonCount() = 0;
    virtual bool IsButtonPressed(size_t buttonId) = 0;

    virtual size_t GetAxisCount() = 0;
    virtual float GetAxisValue(size_t axisId) = 0;

    virtual bool PollChangedButton(size_t &rButtonId, bool &pressed) = 0;
    virtual bool PollChangedAxis(size_t &rAxisId, float &axisValue) = 0;

};

#endif
