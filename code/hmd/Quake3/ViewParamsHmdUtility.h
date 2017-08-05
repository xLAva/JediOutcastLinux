#include "../../game/q_shared.h"
#include "../../renderer/tr_local.h"

class ViewParamsHmdUtility
{
public:
    static void UpdateRenderParams(trGlobals_t *trRef, bool drawskyboxportal, bool &rViewMatrixCreated);
};
