#include "FactoryHmdDevice.h"
#include "HmdDevice/HmdDeviceMouse.h"
#ifdef USE_OPENHMD
#include "HmdDevice/HmdDeviceOpenHmd.h"
#include "HmdRenderer/HmdRendererOculusOpenHmd.h"
#endif

#ifdef USE_OVR_1
#include "OculusSdk_1/HmdDeviceOculusSdk.h"
#include "OculusSdk_1/HmdRendererOculusSdk.h"
#include "OculusSdk_1/HmdInputOculusSdk.h"
#endif

#ifdef USE_OVR_0_8
#include "OculusSdk_0.8/HmdDeviceOculusSdk.h"
#include "OculusSdk_0.8/HmdRendererOculusSdk.h"
#endif

#ifdef USE_OVR_0_5
#include "OculusSdk_0.5/HmdDeviceOculusSdk.h"
#include "OculusSdk_0.5/HmdRendererOculusSdk.h"
#endif

#ifdef USE_OPENVR
#include "openvr/HmdDeviceOpenVr.h"
#include "openvr/HmdRendererOpenVr.h"
#include "openvr/HmdInputOpenVr.h"
#endif

#include "HmdRenderer/HmdRendererOculus.h"

#include <vector>
#include <memory.h>

using namespace std;

IHmdDevice* FactoryHmdDevice::CreateHmdDevice(HmdLibrary library, bool allowDummyDevice)
{
    vector<IHmdDevice*> devices;
   
#ifdef USE_OVR_1
    if (library == LIB_OVR || library == LIB_UNDEFINED)
    {
        devices.push_back(new OvrSdk_1::HmdDeviceOculusSdk());
    }
#endif

#ifdef USE_OPENVR
    if (library == LIB_OPENVR || library == LIB_UNDEFINED)
    {
        devices.push_back(new OpenVr::HmdDeviceOpenVr());
    }
#endif    
    
#ifdef USE_OVR_0_8
    if (library == LIB_OVR || library == LIB_UNDEFINED)
    {
        devices.push_back(new OvrSdk_0_8::HmdDeviceOculusSdk());
    }
#endif
    
#ifdef USE_OVR_0_5
    if (library == LIB_OVR || library == LIB_UNDEFINED)
    {
        devices.push_back(new OvrSdk_0_5::HmdDeviceOculusSdk());
    }
#endif
    
#ifdef USE_OPENHMD
    if (library == LIB_OPENHMD || library == LIB_UNDEFINED)
    {
        devices.push_back(new HmdDeviceOpenHmd());
    }
#endif

    if (library == LIB_MOUSE_DUMMY)
    {
        // only use mouse dummy if it is forced
        devices.push_back(new HmdDeviceMouse());
    }

    IHmdDevice* pSelectedDevice = nullptr;

    for (unsigned int i=0; i<devices.size(); i++)
    {
        if (pSelectedDevice == nullptr)
        {
            bool worked = devices[i]->Init(allowDummyDevice);
            if (worked)
            {
                pSelectedDevice = devices[i];
            }
            else
            {
                devices[i]->Shutdown();
            }
        }

        if (pSelectedDevice != devices[i])
        {
            delete devices[i];
            devices[i] = nullptr;
        }
    }

    devices.clear();

    return pSelectedDevice;
}

IHmdRenderer* FactoryHmdDevice::CreateRendererForDevice(IHmdDevice* pDevice)
{
    if (pDevice == nullptr)
    {
        return nullptr;
    }

    int width = 0;
    int height = 0;
    bool isRotated = false;
    bool isExtendedMode = false;

    bool needsRenderer = pDevice->GetDeviceResolution(width, height, isRotated, isExtendedMode);
    if (!needsRenderer)
    {
        return nullptr;
    }

#ifdef USE_OPENVR
    OpenVr::HmdDeviceOpenVr* pOpenVr = dynamic_cast<OpenVr::HmdDeviceOpenVr*>(pDevice);
    if (pOpenVr != nullptr)
    {
        OpenVr::HmdRendererOpenVr* pRenderer = new OpenVr::HmdRendererOpenVr(pOpenVr);
        return pRenderer;
    }
#endif

#ifdef USE_OVR_1
    OvrSdk_1::HmdDeviceOculusSdk* pOculusSdk_1 = dynamic_cast<OvrSdk_1::HmdDeviceOculusSdk*>(pDevice);
    if (pOculusSdk_1 != nullptr)
    {
        //HmdRendererOculus* pRenderer = new HmdRendererOculus();
        OvrSdk_1::HmdRendererOculusSdk* pRenderer = new OvrSdk_1::HmdRendererOculusSdk(pOculusSdk_1);
        return pRenderer;
    }
#endif

#ifdef USE_OVR_0_8
    OvrSdk_0_8::HmdDeviceOculusSdk* pOculusSdk_0_8 = dynamic_cast<OvrSdk_0_8::HmdDeviceOculusSdk*>(pDevice);
    if (pOculusSdk_0_8 != nullptr)
    {
        //HmdRendererOculus* pRenderer = new HmdRendererOculus();
        OvrSdk_0_8::HmdRendererOculusSdk* pRenderer = new OvrSdk_0_8::HmdRendererOculusSdk(pOculusSdk_0_8);
        return pRenderer;
    }
#endif

#ifdef USE_OVR_0_5
    OvrSdk_0_5::HmdDeviceOculusSdk* pOculusSdk_0_5 = dynamic_cast<OvrSdk_0_5::HmdDeviceOculusSdk*>(pDevice);
    if (pOculusSdk_0_5 != nullptr)
    {
        //HmdRendererOculus* pRenderer = new HmdRendererOculus();
        OvrSdk_0_5::HmdRendererOculusSdk* pRenderer = new OvrSdk_0_5::HmdRendererOculusSdk(pOculusSdk_0_5);
        return pRenderer;
    }
#endif

#ifdef USE_OPENHMD
    HmdDeviceOpenHmd* pOpenHmd = dynamic_cast<HmdDeviceOpenHmd*>(pDevice);
    if (pOpenHmd != nullptr)
    {
        //HmdRendererOculus* pRenderer = new HmdRendererOculus();
        HmdRendererOculusOpenHmd* pRenderer = new HmdRendererOculusOpenHmd(pOpenHmd);
        return pRenderer;
    }
#endif
    
    HmdDeviceMouse* pHmdMouse = dynamic_cast<HmdDeviceMouse*>(pDevice);
    if (pHmdMouse != nullptr)
    {
        HmdRendererOculus* pRenderer = new HmdRendererOculus();
        return pRenderer;
    }

    return nullptr;
}

IHmdInput*FactoryHmdDevice::CreateInputForDevice(IHmdDevice* pDevice)
{
    if (pDevice == nullptr)
    {
        return nullptr;
    }

    bool handlesControllerInput = pDevice->HandlesControllerInput();
    if (!handlesControllerInput)
    {
        return nullptr;
    }

#ifdef USE_OPENVR
    OpenVr::HmdDeviceOpenVr* pOpenVr = dynamic_cast<OpenVr::HmdDeviceOpenVr*>(pDevice);
    if (pOpenVr != nullptr)
    {
        OpenVr::HmdInputOpenVr* pInput = new OpenVr::HmdInputOpenVr(pOpenVr);
        return pInput;
    }
#endif

#ifdef USE_OVR_1
    OvrSdk_1::HmdDeviceOculusSdk* pOculusSdk_1 = dynamic_cast<OvrSdk_1::HmdDeviceOculusSdk*>(pDevice);
    if (pOculusSdk_1 != nullptr)
    {
        OvrSdk_1::HmdInputOculusSdk* pInput = new OvrSdk_1::HmdInputOculusSdk(pOculusSdk_1);
        return pInput;
    }
#endif

    return nullptr;
}
