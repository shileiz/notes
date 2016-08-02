//==============================================================================
// SDL Tutorial 1
//==============================================================================
#ifndef _CAPP_H_
    #define _CAPP_H_

#include <SDL.h>

//==============================================================================
class CApp {
    private:
        bool            Running;

        SDL_Surface*    Surf_Display;

    public:
        CApp();

        int OnExecute();

    public:
        bool OnInit();

        void OnEvent(SDL_Event* Event);

        void OnLoop();  // 更新数据

        void OnRender(); //显示图像

        void OnCleanup();
};

//==============================================================================

#endif
