#ifndef _NEON_MATH_H
#define _NEON_MATH_H
#ifdef NEON
#include <arm_neon.h>

#define to_neon(a, v) float a[4] = { v[0], v[1], v[2], 0 }

inline float32_t DotProductNeon(const float* a, const float* b)
{
        float32_t result[2];
        register float32x2_t v1a = vld1_f32((float32_t*)a);
        register float32x2_t v1b = vld1_f32((float32_t*)(a + 2));
        register float32x2_t v2a = vld1_f32((float32_t*)b);
        register float32x2_t v2b = vld1_f32((float32_t*)(b + 2));
        v1a = vmul_f32(v1a, v2a);
        v1a = vmla_f32(v1a, v1b, v2b);
        v1a = vpadd_f32(v1a, v1a);
        vst1_f32(result, v1a);
        return(result[0]);
}

inline float neon_recip(float y) {
	register float32x2_t a,b;
	float res[2];
	a=vdup_n_f32(y);
	b=a;
	a=vrecpe_f32(b);
	a=vmul_f32(a,vrecps_f32(b, a));
	a=vmul_f32(a,vrecps_f32(b, a));
//	a=vmul_f32(a,vrecps_f32(b, a));

	vst1_f32(res, a);
	return res[0];	
}

inline float DotProduct3Scale( const float v0[3], const float scale, const float v1[3], const float v2[3], const float v3[3], float vout[3] ) {
#if 0
	float temp[2];
	float32x2_t scalex2 = vdup_n_f32(scale);
	float32x2_t v0_1 = vld1_f32(v0);
	float32x2_t v0_2 = vdup_n_f32(v0[2]);
	float32x2_t v1_1 = vld1_f32(v1);
	float32x2_t v1_2 = vdup_n_f32(v1[2]);
	
	float32x2_t res = vmul_f32(v0_1, v1_1);
	vst1_f32(temp, vmul_f32(vmla_f32(vpadd_f32(res, res), v0_2, v1_2),scalex2));
	vout[0]=temp[0];
	

	v1_1 = vld1_f32(v2);
	v1_2 = vdup_n_f32(v2[2]);
	
	res = vmul_f32(v0_1, v1_1);
	vst1_f32(temp, vmul_f32(vmla_f32(vpadd_f32(res, res), v0_2, v1_2),scalex2));
	vout[1]=temp[0];

	v1_1 = vld1_f32(v3);
	v1_2 = vdup_n_f32(v3[2]);
	
	res = vmul_f32(v0_1, v1_1);
	vst1_f32(temp, vmul_f32(vmla_f32(vpadd_f32(res, res), v0_2, v1_2),scalex2));
	vout[2]=temp[0];
#else
        asm volatile (
        "vld1.32                {d2}, [%1]                      \n\t"   //d2={x0,y0}
        "flds                   s6, [%1, #8]            		\n\t"   //d3[0]={z0}
        "vmov.32				s12, %2							\n\t"
        "vdup.f32				d6, d4[0]						\n\t"	//d6=scale
        "vld1.32                {d4}, [%3]                      \n\t"   //d4={x1,y1}
        "flds                   s10, [%3, #8]   				\n\t"   //d5[0]={z1}

        "vmul.f32               d0, d2, d4                      \n\t"   //d0= d2*d4
        "vpadd.f32              d0, d0, d0                      \n\t"   //d0 = d[0] + d[1]
        "vmla.f32               d0, d3, d5                      \n\t"   //d0 = d0 + d3*d5 
        "vmul.f32				d0, d0, d6						\n\t"
        "vmov.64				d7, d0							\n\t"
        
        "vld1.32                {d4}, [%4]                      \n\t"   //d4={x1,y1}
        "flds                   s10, [%4, #8]   				\n\t"   //d5[0]={z1}

        "vmul.f32               d0, d2, d4                      \n\t"   //d0= d2*d4
        "vpadd.f32              d0, d0, d0                      \n\t"   //d0 = d[0] + d[1]
        "vmla.f32               d0, d3, d5                      \n\t"   //d0 = d0 + d3*d5 
        "vmul.f32				d0, d0, d6						\n\t"
        "vmov.32				s15, s1							\n\t"
        "vld1.32				{d0}, [%0]						\n\t"
        "vadd.f32				d7, d0, d7						\n\t"
        "vst1.32				d7, [%0]						\n\t"
        
        "vld1.32                {d4}, [%5]                      \n\t"   //d4={x1,y1}
        "flds                   s10, [%5, #8]   				\n\t"   //d5[0]={z1}

        "vmul.f32               d0, d2, d4                      \n\t"   //d0= d2*d4
        "vpadd.f32              d0, d0, d0                      \n\t"   //d0 = d[0] + d[1]
        "vmla.f32               d0, d3, d5                      \n\t"   //d0 = d0 + d3*d5 
        "vmul.f32				d0, d0, d6						\n\t"
        
        "flds					s3,[%0, #8]						\n\t"
        "vadd.f32				d0, d0, d1						\n\t"

        "fsts					s0, [%0, #8]					\n\t"

        : "+&r"(vout) : "r"(v0), "r" (scale), "r"(v1), "r"(v2), "r"(v3)
		: "d0","d1","d2","d3","d4","d5","d6", "d7", "memory"
        );      
#endif
}

inline void R_TransformBoneWeight(vec3_t tempVert, vec3_t tempNormal, float fBoneWeight,
 vec3_t vertCoords, vec3_t normal, const float boneMatrix[3][4])
{
	float32x2_t dp03;
    float32x2_t dp14;
    float32x2_t dp25;
    float32x2_t vertNorm_0 = { vertCoords[0], normal[0] };
    float32x2_t vertNorm_1 = { vertCoords[1], normal[1] };
    float32x2_t vertNorm_2 = { vertCoords[2], normal[2] };
    float32x2_t boneNorm_0 = { boneMatrix[0][3], tempNormal[0] };
    float32x2_t boneNorm_1 = { boneMatrix[1][3], tempNormal[1] };
    float32x2_t boneNorm_2 = { boneMatrix[2][3], tempNormal[2] };

    dp03 = boneMatrix[0][0] * vertNorm_0;
    dp14 = boneMatrix[1][0] * vertNorm_0;
    dp25 = boneMatrix[2][0] * vertNorm_0;

    dp03 += boneMatrix[0][1] * vertNorm_1;
    dp14 += boneMatrix[1][1] * vertNorm_1;
    dp25 += boneMatrix[2][1] * vertNorm_1;

    dp03 += boneMatrix[0][2] * vertNorm_2;
    dp14 += boneMatrix[1][2] * vertNorm_2;
    dp25 += boneMatrix[2][2] * vertNorm_2;

    dp03 *= fBoneWeight;
    dp14 *= fBoneWeight;
    dp25 *= fBoneWeight;

    dp03 += boneNorm_0;
    dp14 += boneNorm_1;
    dp25 += boneNorm_2;
    
    float32x2_t dp01 = {dp03[0], dp14[0]};
    float32x2_t dp45 = {dp14[1], dp25[1]};
    
	dp01 += vld1_f32(tempVert);
    dp25[0] += tempVert[2];

    vst1_f32(tempVert, dp01);
    tempVert[2] = dp25[0];

    tempNormal[0] = dp03[1];
    vst1_f32(tempNormal+1, dp45);
}

#define neon_sign(a) ((a)<0)?-1:((a)>0)?1:0

#endif
#endif
