#include "HmdInputBase.h"


bool HmdInputBase::Init()
{
    mButtonCount = GetButtonCount();
    if (mButtonCount > 0)
    {
        mpLastButtonState = new bool[mButtonCount];
        for (size_t i = 0; i < mButtonCount; ++i)
        {
            mpLastButtonState[i] = false;
        }
    }

    mAxisCount = GetAxisCount();
    if (mAxisCount > 0)
    {
        mpLastAxisValue = new float[mAxisCount];
        for (size_t i = 0; i < mAxisCount; ++i)
        {
            mpLastAxisValue[i] = 0.0f;
        }
    }
}

void HmdInputBase::Shutdown()
{
    mButtonCount = 0;
    mAxisCount = 0;

    mChangedButtons.clear();
    mChangedAxis.clear();

    if (mpLastButtonState != nullptr)
    {
        delete[] mpLastButtonState;
        mpLastButtonState = nullptr;
    }

    if (mpLastAxisValue != nullptr)
    {
        delete [] mpLastAxisValue;
        mpLastAxisValue = nullptr;
    }
}

bool HmdInputBase::PollChangedButton(size_t& rButtonId, bool& rPressed)
{
    CheckButtons();

    if (mChangedButtons.size() == 0)
    {
        return false;
    }

    rButtonId = mChangedButtons[0].first;
    rPressed = mChangedButtons[0].second;

    return true;
}

bool HmdInputBase::PollChangedAxis(size_t& rAxisId, float& rAxisValue)
{
    CheckAxis();

    if (mChangedAxis.size() == 0)
    {
        return false;
    }

    rAxisId = mChangedAxis[0].first;
    rAxisValue = mChangedAxis[0].second;

    return true;
}

void HmdInputBase::CheckButtons()
{
    if (mChangedButtons.size() > 0)
    {
        return;
    }

    for (size_t i=0; i<mButtonCount; ++i)
    {
        bool pressed = IsButtonPressed(i);
        if (mpLastButtonState[i] != pressed)
        {
            mpLastButtonState[i] = pressed;
            mChangedButtons.push_back(std::pair<size_t, bool>(i, pressed));
        }
    }
}

void HmdInputBase::CheckAxis()
{
    if (mChangedAxis.size() > 0)
    {
        return;
    }

    for (size_t i=0; i<mAxisCount; ++i)
    {
        float axisValue = GetAxisValue(i);
        if (mpLastAxisValue[i] != axisValue)
        {
            mpLastAxisValue[i] = axisValue;
            mChangedAxis.push_back(std::pair<size_t, float>(i, axisValue));
        }
    }
}
