#pragma once
#include "D3DBinding.h"

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphx {
public:
    Graphx(int, int, HWND);
    ~Graphx(void);
    bool Render();

private:
    bool Draw();
    D3DBinding* d3d;
};

