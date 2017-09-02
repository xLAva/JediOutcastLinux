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

void HmdInputBase::Update()
{
    CheckButtons();
    CheckAxis();
}

bool HmdInputBase::PollChangedButton(size_t& rButtonId, bool& rPressed)
{
    auto it = mChangedButtons.begin();
    if (it == mChangedButtons.end())
    {
        return false;
    }

    rButtonId = it->first;
    rPressed = it->second;

    mChangedButtons.erase(it);

    return true;
}

bool HmdInputBase::PollChangedAxis(size_t& rAxisId, float& rAxisValue)
{
    auto it = mChangedAxis.begin();
    if (it == mChangedAxis.end())
    {
        false;
    }

    rAxisId = it->first;
    rAxisValue = it->second;

    mChangedAxis.erase(it);

    return true;
}

void HmdInputBase::CheckButtons()
{
    if (mChangedButtons.size() > 0)
    {
        mChangedButtons.clear();
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
        mChangedAxis.clear();
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
