#ifndef DETECTDISPLAYSDL2_H
#define DETECTDISPLAYSDL2_H

#include <string>

class SearchForDisplay
{
public:
    struct DisplayInfo
    {
        std::string name;
        int id;
        int posX;
        int posY;
        bool isRotated;
    };
    
    static bool GetDisplayPosition(std::string displayName, int resolutionW, int resolutionH, DisplayInfo& rInfo);
    static bool GetDisplayPosition(int posX, int posY, int resolutionW, int resolutionH, DisplayInfo& rInfo);
    
    
};


#endif
