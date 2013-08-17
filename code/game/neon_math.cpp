#include "q_shared.h"
#include "neon_math.h"

#ifdef NEON

#include <math.h>
#ifdef M_PI
#undef M_PI
#endif
#define M_PI 3.1141592653592f

/// Computes the sine and cosine of two angles
/// in: angles = Two angles, expressed in radians, in the [-PI,PI] range.
/// out: results = vector containing [sin(angles[0]),cos(angles[0]),sin(angles[1]),cos(angles[1])]
inline void vsincos(const float angles[2], float results[4]) {
    static const float constants[]  = { 
    /* q1 */  0,                M_PI_2,           0,                M_PI_2,
    /* q2 */  M_PI,             M_PI,             M_PI,             M_PI,
    /* q3 */  4.f/M_PI,         4.f/M_PI,         4.f/M_PI,         4.f/M_PI,
    /* q4 */ -4.f/(M_PI*M_PI), -4.f/(M_PI*M_PI), -4.f/(M_PI*M_PI), -4.f/(M_PI*M_PI),
    /* q5 */  2.f,              2.f,              2.f,              2.f,
    /* q6 */  .225f,            .225f,            .225f,            .225f
    };  
    asm volatile(
        // Load q0 with [angle1,angle1,angle2,angle2]
        "vldmia %1, { d3 }\n\t"
        "vdup.f32 d0, d3[0]\n\t"
        "vdup.f32 d1, d3[1]\n\t"
        // Load q1-q6 with constants
        "vldmia %2, { q1-q6 }\n\t"
        // Cos(x) = Sin(x+PI/2), so
        // q0 = [angle1, angle1+PI/2, angle2, angle2+PI/2]
        "vadd.f32 q0,q0,q1\n\t"
        // if angle1+PI/2>PI, substract 2*PI
        // q0-=(q0>PI)?2*PI:0
        "vcge.f32 q1,q0,q2\n\t"
        "vand.f32 q1,q1,q2\n\t"
        "vmls.f32 q0,q1,q5\n\t"
        // q0=(4/PI)*q0 - q0*abs(q0)*4/(PI*PI)
        "vabs.f32 q1,q0\n\t"
        "vmul.f32 q1,q0,q1\n\t"
        "vmul.f32 q0,q0,q3\n\t"
        "vmul.f32 q1,q1,q4\n\t"
        "vadd.f32 q0,q0,q1\n\t"
        // q0+=.225*(q0*abs(q0) - q0)
        "vabs.f32 q1,q0\n\t"
        "vmul.f32 q1,q0,q1\n\t"
        "vsub.f32 q1,q0\n\t"
        "vmla.f32 q0,q1,q6\n\t"
        "vstmia %0, { q0 }\n\t"
        : "+&r"(results), "+&r"(angles): "r"(constants)
        : "memory","cc","q0","q1","q2","q3","q4","q5","q6"
    );  
}

#endif
