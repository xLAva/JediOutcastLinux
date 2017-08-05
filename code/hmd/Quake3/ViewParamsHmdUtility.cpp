#include "ViewParamsHmdUtility.h"

#include "../ClientHmd.h"
#include "../HmdRenderer/IHmdRenderer.h"

void ViewParamsHmdUtility::UpdateRenderParams(trGlobals_t* trRef, bool isSkyBoxPortal, bool &rViewMatrixCreated)
{
    rViewMatrixCreated = false;

    IHmdRenderer* pHmdRenderer = ClientHmd::Get()->GetRenderer();
    if (pHmdRenderer)
    {
        vec3_t origin;

        // transform by the camera placement
        VectorCopy( trRef->viewParms.or.origin, origin );

        // check if the renderer handles the view matrix creation
        bool matrixCreated = pHmdRenderer->GetCustomViewMatrix(trRef->or.modelMatrix,
                origin[0],
                origin[1],
                origin[2],
                trRef->viewParms.bodyYaw, isSkyBoxPortal);

        if (matrixCreated)
        {
            VectorCopy(origin, trRef->viewParms.or.origin);
            VectorCopy(origin, trRef->viewParms.or.viewOrigin);
            VectorCopy(origin, trRef->viewParms.pvsOrigin);
        }

        rViewMatrixCreated = matrixCreated;
    }
    else
    {
        vec3_t hmdOffset;
        bool worked = ClientHmd::Get()->GetPosition(hmdOffset[0], hmdOffset[1], hmdOffset[2]);
        if (worked)
        {
            vec3_t origin;
            VectorCopy( trRef->viewParms.or.origin, origin);

            for (int i=0; i<3; i++)
            {
                origin[i] += hmdOffset[i];
            }

            VectorCopy(origin, trRef->viewParms.or.origin);
        }
    }
}
