#ifndef GAMEMENUHMDMANAGER_H
#define GAMEMENUHMDMANAGER_H

#include "../HmdRenderer/IHmdRenderer.h"
#include <string>
#include <unordered_set>

class GameMenuHmdManager
{
public:
    GameMenuHmdManager();
    ~GameMenuHmdManager();

    void SetHmdRenderer(IHmdRenderer* pHmdRenderer);

    void OnMenuOpen(std::string menuName);
    void OnMenuClose(std::string menuName);

    void OnCloseAllMenus();
    void SetCameraControlled(bool active);

    void Update();

private:
    enum GameMode{
        UNINITIALIZED,
        GAME,
        MENU,
        LOADING_SCREEN,
        CINEMATIC,
        GAME_CUTSCENE,
        GAME_SECURITY_CAM
    };
    
    void SetHmdMode();

    IHmdRenderer* mpHmdRenderer;
    GameMode mCurrentGameMode;
    bool mIsCameraControlled;
    bool mIsLoadingActive;
    bool mShowCutScenesInVr;

    std::unordered_set<std::string> mInGameHudNames;
    std::unordered_set<std::string> mCurrentOpenMenu;
    
    const std::string LOAD_SCREEN_NAME = "loadscreen";

};

#endif
