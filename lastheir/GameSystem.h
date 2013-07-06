#pragma once
#include <windows.h>
#include "input.h"
#include "graphx.h"

class GameSystem {
public:
    GameSystem(void);
    ~GameSystem(void);

    bool Initialize();
    void Shutdown();
    void Run();
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
    bool Frame();
    void OpenWindow(int&, int&);
    void KillWindow();

private:
    LPCWSTR applicationName;
    HINSTANCE hinstance;
    HWND hwnd;

    Input* input;
    Graphx* graphics;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static GameSystem* g_applicationHandle = NULL;