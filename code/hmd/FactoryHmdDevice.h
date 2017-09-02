/**
 * HMD extension for JediAcademy
 *
 *  Copyright 2014 by Jochen Leopold <jochen.leopold@model-view.com>
 */

#ifndef FACTORYHMDDEVICE_H
#define FACTORYHMDDEVICE_H

#include "HmdDevice/IHmdDevice.h"
#include "HmdRenderer/IHmdRenderer.h"
#include "HmdInput/IHmdInput.h"

class FactoryHmdDevice
{
public:

    enum HmdLibrary {
        LIB_UNDEFINED,
        LIB_OVR,
        LIB_OPENHMD,
        LIB_MOUSE_DUMMY
    };

    static IHmdDevice* CreateHmdDevice(HmdLibrary library, bool allowDummyDevice);
    static IHmdRenderer* CreateRendererForDevice(IHmdDevice* pDevice);
    static IHmdInput* CreateInputForDevice(IHmdDevice* pDevice);

private:
    // disable copy constructor
    FactoryHmdDevice(const FactoryHmdDevice&);
    FactoryHmdDevice& operator=(const FactoryHmdDevice&);
};

#endif
