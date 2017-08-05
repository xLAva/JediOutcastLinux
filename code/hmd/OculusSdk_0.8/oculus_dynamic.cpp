// Uses SDL for library loading. If you're not using SDL, you can
// replace SDL_LoadObject with dlopen/LoadLibrary and SDL_LoadFunction
// with dlsym/GetProcAddress

#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include "oculus_dynamic.h"
#include <stdlib.h>
#include <stdio.h>


using namespace OvrSdk_0_8;

void *OvrSdk_0_8::oculus_library_handle;

#define OVRFUNC(need, rtype, fn, params)        \
rtype(*OvrSdk_0_8::d_ ## fn) params;
#include "ovr_dynamic_funcs.h"
#undef OVRFUNC

extern ovr_dynamic_load_result OvrSdk_0_8::oculus_dynamic_load(const char** failed_function) {
    const char* liboculus = getenv("LIBOVR");
    if (!liboculus) {
#ifdef OVR_OS_WIN32
        liboculus = "libovr.dll";
#else
        liboculus = "./libovr.so";
#endif
    }

    oculus_library_handle = SDL_LoadObject(liboculus);
    if (!oculus_library_handle) {
        printf("SDL_LoadObject failed: %s\n", SDL_GetError());
        return OVR_DYNAMIC_RESULT_LIBOVR_COULD_NOT_LOAD;
    }

#define OVRFUNC(need, r, f, p)                                          \
    OvrSdk_0_8::d_##f = (pfn_##f)SDL_LoadFunction(oculus_library_handle, #f);       \
    if (need && !OvrSdk_0_8::d_##f) {                                               \
        if (failed_function)                                            \
            *failed_function = #f;                                      \
        return OVR_DYNAMIC_RESULT_LIBOVR_COULD_NOT_LOAD_FUNCTION;       \
    }
#include "ovr_dynamic_funcs.h"
#undef OVRFUNC
    
    return OVR_DYNAMIC_RESULT_SUCCESS;
}
