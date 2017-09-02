/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef HMDINPUTBASE_H
#define HMDINPUTBASE_H

#include "IHmdInput.h"

#include <vector>

class HmdInputBase : public IHmdInput
{
public:


    virtual ~HmdInputBase() {}

    bool Init() override;
    void Shutdown() override;

    bool PollChangedButton(size_t &rButtonId, bool &rPressed) override;
    bool PollChangedAxis(size_t &rAxisId, float &rAxisValue) override;

private:
    void CheckButtons();
    void CheckAxis();

    std::vector<std::pair<size_t, bool> > mChangedButtons;
    std::vector<std::pair<size_t, float> > mChangedAxis;

    size_t mButtonCount;
    size_t mAxisCount;

    bool* mpLastButtonState;
    float* mpLastAxisValue;



};

#endif
