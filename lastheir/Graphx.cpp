#include "Graphx.h"


Graphx::Graphx(int screenWidth, int screenHeight, HWND hwnd) {
    try {
        d3d = new D3DBinding(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, SCREEN_DEPTH, SCREEN_NEAR);
    } catch (EngineException& e) {
        MessageBox(NULL, e.what(), L"Test print handler", MB_OK);
        throw;
    }
}


Graphx::~Graphx(void) {
    if (d3d) {
        delete d3d;
    }
    d3d = NULL;
}

bool Graphx::Draw() {
    d3d->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);
    d3d->EndScene();
    return true;
}


bool Graphx::Render() {
    return Draw();
}