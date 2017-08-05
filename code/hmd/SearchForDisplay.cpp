#include "SearchForDisplay.h"

#ifdef USE_SDL2
#if defined(LINUX) || defined(__APPLE__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#endif


using namespace std;

bool SearchForDisplay::GetDisplayPosition(string displayName, int resolutionW, int resolutionH, DisplayInfo& rInfo)
{

    // only implemented for SDL2
#ifdef USE_SDL2
    
    int displayId = -1;
    std::string foundDisplayName = "";
    
    int displayCount = SDL_GetNumVideoDisplays();
    for (int i=0; i<displayCount; i++)
    {
		const char* name = SDL_GetDisplayName(i);
        if (displayName.length() > 0)
        {
            if (strcmp(name, displayName.c_str()) == 0)
            {
                displayId = i;
                foundDisplayName = name;
                break;
            }
        }

        SDL_Rect r;
        int ret = SDL_GetDisplayBounds(i, &r);
        if (ret == 0)
        {
            if (r.w == resolutionW && r.h == resolutionH)
            {
                // this is a fallback, if the display name is not correct
                displayId = i;
                foundDisplayName = name;
            }
        }
        else if (ret != 0)
        {
            const char* error = SDL_GetError();
            printf("SDL_GetDisplayBounds failed: %s\n", error);

        }

        //printf("display name: %s\n", displayName);
        //flush(std::cout);
    }

    if (displayId >= 0)
    {
        SDL_Rect r;
        int ret = SDL_GetDisplayBounds(displayId, &r);
        if (ret == 0)
        {
            bool isRotated = false;
            
            // this code only works on Linux for now
            #ifdef LINUX
            if (resolutionW == r.h && resolutionH == r.w)
            {
                isRotated = true;
            }
            #endif

            //printf("display x=%d y=%d\n", r.x, r.y);
            //flush(std::cout);
            
            rInfo.id = displayId;
            rInfo.name = foundDisplayName;
            rInfo.posX = r.x;
            rInfo.posY = r.y;
            rInfo.isRotated = isRotated;
            
            return true;
        }
    }
    
#endif    
    
    return false;

    
}

bool SearchForDisplay::GetDisplayPosition(int posX, int posY, int resolutionW, int resolutionH, DisplayInfo& rInfo)
{
    // only implemented for SDL2
#ifdef USE_SDL2

    int displayCount = SDL_GetNumVideoDisplays();
    for (int i = 0; i<displayCount; i++)
    {
        const char* name = SDL_GetDisplayName(i);
        
        SDL_Rect r;
        int ret = SDL_GetDisplayBounds(i, &r);
        if (ret == 0)
        {
            if (r.x == posX && r.y == posY)
            {
                // found display pos
                if (r.w == resolutionW && r.h == resolutionH ||
                    r.w == resolutionH && r.h == resolutionW)
                {
                    // found display with corret resolution on correction position
                    rInfo.id = i;
                    rInfo.name = name;
                    rInfo.posX = posX;
                    rInfo.posY = posY;
                    rInfo.isRotated = r.w == resolutionH;
                    return true;
                }
            }
        }
        else if (ret != 0)
        {
            const char* error = SDL_GetError();
            printf("SDL_GetDisplayBounds failed: %s\n", error);
        }
    
        //printf("display name: %s\n", displayName);
        //flush(std::cout);
    }

#endif

    return false;


}
