#include "GameMenuHmdManager.h"

#include "../../game/q_shared.h"
#include "../../qcommon/qcommon.h"

#include "../../client/client.h"

GameMenuHmdManager::GameMenuHmdManager()
    :mpHmdRenderer(nullptr)
    ,mCurrentGameMode(UNINITIALIZED)
    ,mIsCameraControlled(false)
    ,mIsLoadingActive(false)
    ,mShowCutScenesInVr(true)
{
    mInGameHudNames.insert("mainhud");
}

GameMenuHmdManager::~GameMenuHmdManager()
{

}

void GameMenuHmdManager::SetHmdRenderer(IHmdRenderer* pHmdRenderer)
{
    mpHmdRenderer = pHmdRenderer;
    if (mpHmdRenderer != nullptr)
    {
        SetHmdMode();
    }
}

void GameMenuHmdManager::OnMenuOpen(std::string menuName)
{
    if (mInGameHudNames.find(menuName) != mInGameHudNames.end())
    {
        return;
    }

    if (menuName.compare(LOAD_SCREEN_NAME) == 0)
    {
        mIsLoadingActive = true;
        return;
    }
    
    mCurrentOpenMenu.insert(menuName);
}

void GameMenuHmdManager::OnMenuClose(std::string menuName)
{
    if (menuName.compare(LOAD_SCREEN_NAME) == 0)
    {
        mIsLoadingActive = false;
        return;
    }    
    
    if (mInGameHudNames.find(menuName) != mInGameHudNames.end())
    {
        return;
    }

    auto foundMenu = mCurrentOpenMenu.find(menuName);
    if (foundMenu != mCurrentOpenMenu.end())
    {
        mCurrentOpenMenu.erase(foundMenu);
    }
}

void GameMenuHmdManager::OnCloseAllMenus()
{
    mCurrentOpenMenu.clear();
    mIsLoadingActive = false;
}



void GameMenuHmdManager::SetCameraControlled(bool active)
{
    if (mIsCameraControlled == active)
    {
        return;
    }

    mIsCameraControlled = active;

    if (mShowCutScenesInVr)
    {
        return;
    }

    Update();
}

void GameMenuHmdManager::Update()
{
    GameMode currentGameMode = GAME;
    
    // if no map is loaded we are always in fullscreen menu mode
    //if (Cvar_VariableIntegerValue("sv_running"))
    {
        if (mCurrentOpenMenu.size() > 0)
        {
            currentGameMode = MENU;
        }
    }

    if (mIsCameraControlled)
    {
        currentGameMode = GAME_CUTSCENE;
    }

    if (mIsLoadingActive)
    {
        currentGameMode = LOADING_SCREEN;
    }
    
    if (cls.state == CA_CINEMATIC || CL_IsRunningInGameCinematic())
    {
        currentGameMode = CINEMATIC;
    }

    if (mCurrentGameMode == currentGameMode)
    {
        return;
    }

    mCurrentGameMode = currentGameMode;

    SetHmdMode();
}


void GameMenuHmdManager::SetHmdMode()
{
    if (mpHmdRenderer == nullptr)
    {
        return;
    }

    IHmdRenderer::HmdMode currentMode;
    
    switch (mCurrentGameMode)
    {
        case LOADING_SCREEN:
        {
            currentMode = IHmdRenderer::MENU_QUAD;
            break;
        }
        case MENU:
        case CINEMATIC:
        {
            currentMode = IHmdRenderer::MENU_QUAD_WORLDPOS;
            break;
        }
        case GAME_CUTSCENE:
        {
            if (mShowCutScenesInVr)
            {
                currentMode = IHmdRenderer::GAMEWORLD;
            }
            else
            {
                currentMode = IHmdRenderer::GAMEWORLD_QUAD_WORLDPOS;
            }
            break;
        }        
        case GAME_SECURITY_CAM:
        {
            currentMode = IHmdRenderer::GAMEWORLD_QUAD_WORLDPOS;
            break;
        }
        case GAME:
        default:
        {
            currentMode = IHmdRenderer::GAMEWORLD;
            break;
        }
    }
    
    
    if (!mpHmdRenderer->HasQuadWorldPosSupport())
    {
        if (currentMode == IHmdRenderer::MENU_QUAD_WORLDPOS)
        {
            currentMode = IHmdRenderer::MENU_QUAD;
        }
        else if (currentMode == IHmdRenderer::GAMEWORLD_QUAD_WORLDPOS)
        {
            currentMode = IHmdRenderer::GAMEWORLD;
        }
    }
    
    mpHmdRenderer->SetCurrentHmdMode(currentMode);


    bool useHmd = currentMode == IHmdRenderer::GAMEWORLD;
    if (useHmd)
    {
        Cvar_SetValue("cg_useHmd", 1);
    }
    else
    {
        Cvar_SetValue("cg_useHmd", 0);
    }
}
