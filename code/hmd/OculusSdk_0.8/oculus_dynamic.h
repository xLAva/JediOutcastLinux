/* OVR_CAPI.h should really define this. There should be no reason to
 * include all of the other C++ crap just to get something this
 * simple. */

#ifndef OCULUS_DYNAMIC_0_8_H
#define OCULUS_DYNAMIC_0_8_H

#ifdef __linux__
#define OVR_OS_LINUX
#elif defined(WIN32)
#define OVR_OS_WIN32
#elif defined(__APPLE__)
#define OVR_OS_MACOS
#else
#error "Unknown O/S"
#endif

#include <OVR_CAPI_0_8_0.h>
#include <OVR_CAPI_GL.h>

namespace OvrSdk_0_8
{

#define OVRFUNC(need, rtype, fn, params)        \
typedef rtype (*pfn_ ## fn) params;             \
extern pfn_##fn d_##fn;
#include "ovr_dynamic_funcs.h"
#undef OVRFUNC

typedef enum {
    OVR_DYNAMIC_RESULT_SUCCESS,
    OVR_DYNAMIC_RESULT_LIBOVR_COULD_NOT_LOAD,
    OVR_DYNAMIC_RESULT_LIBOVR_COULD_NOT_LOAD_FUNCTION
} ovr_dynamic_load_result;

extern void *oculus_library_handle;
extern ovr_dynamic_load_result oculus_dynamic_load(const char** failed_function);
}
#endif
