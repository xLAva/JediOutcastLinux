#include "ViewParamsHmdUtility.h"

#include "../ClientHmd.h"
#include "../HmdRenderer/IHmdRenderer.h"

#include "../../client/client.h"

void ViewParamsHmdUtility::UpdateRenderParams(trGlobals_t* trRef, bool isSkyBoxPortal, bool &rViewMatrixCreated)
{
    static float mViewYaw = 0.0f;
    rViewMatrixCreated = false;

    IHmdRenderer* pHmdRenderer = ClientHmd::Get()->GetRenderer();
    if (pHmdRenderer)
    {
        vec3_t origin;

        // transform by the camera placement
        VectorCopy( trRef->viewParms.orient.origin, origin );

        // handle keyhole yaw for decoupled aiming, if the signed difference between the view angle 
        // and the body angle exceeds the keyhole width, move the view angle in that direction
        if (hmd_decoupleAim->integer && !ClientHmd::Get()->HasHand(true))
        {
            float keyholeWidth = hmd_moveAimKeyholeWidth->value / 2.0f;
            float angleDiffRad = DEG2RAD(trRef->viewParms.bodyYaw - mViewYaw);
            float angleDiff = RAD2DEG(atan2(sin(angleDiffRad), cos(angleDiffRad)));
            if (angleDiff > keyholeWidth)
            {
                mViewYaw += trRef->viewParms.bodyYaw - (mViewYaw + keyholeWidth);
            }
            else if (angleDiff < -keyholeWidth)
            {
                mViewYaw += trRef->viewParms.bodyYaw - (mViewYaw - keyholeWidth);
            }
        }
        else
        {
            mViewYaw = trRef->viewParms.bodyYaw;
        }

        // check if the renderer handles the view matrix creation
        bool matrixCreated = pHmdRenderer->GetCustomViewMatrix(trRef->orient.modelMatrix,
                origin[0],
                origin[1],
                origin[2],
                mViewYaw, isSkyBoxPortal);

        if (matrixCreated)
        {
            VectorCopy(origin, trRef->viewParms.orient.origin);
            VectorCopy(origin, trRef->viewParms.orient.viewOrigin);
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
            VectorCopy( trRef->viewParms.orient.origin, origin);

            for (int i=0; i<3; i++)
            {
                origin[i] += hmdOffset[i];
            }

            VectorCopy(origin, trRef->viewParms.orient.origin);
        }
    }
}
