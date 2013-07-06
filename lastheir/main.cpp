#include "gamesystem.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {
    GameSystem* System;

    // Create the system object.
    System = new GameSystem();
    if (!System) {
        return 1;
    }

    // Initialize and run the system object.
    if (System->Initialize()) {
        System->Run();
    }

    delete System;
    System = NULL;

    return 0;
}