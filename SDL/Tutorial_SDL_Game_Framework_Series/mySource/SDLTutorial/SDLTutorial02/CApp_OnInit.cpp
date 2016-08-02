//==============================================================================
#include "CApp.h"

//==============================================================================
bool CApp::OnInit() {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        return false;
    }

    if((Surf_Display = SDL_SetVideoMode(640, 480, 0, SDL_HWSURFACE )) == NULL) {
        return false;
    }

	if((Surf_Test = CSurface::OnLoad("myimage.bmp")) == NULL) {
		return false;
	}

    return true;
}

//==============================================================================
