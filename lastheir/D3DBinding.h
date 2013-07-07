#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <stdexcept>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>

#include "EngineException.h"

class D3DBinding {
public:
    D3DBinding(int, int, bool, HWND, float, float);
    ~D3DBinding(void);
    void BeginScene(float, float, float, float);
    void EndScene();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();

    void GetProjectionMatrix(D3DXMATRIX&);
    void GetWorldMatrix(D3DXMATRIX&);
    void GetOrthoMatrix(D3DXMATRIX&);

    void GetVideoCardInfo(char*, int&);

private:
    bool vsync_enabled;
    int videoCardMemory;
    char videoCardDescription[128];
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* deviceContext;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11RasterizerState* rasterState;
    D3DXMATRIX projectionMatrix;
    D3DXMATRIX worldMatrix;
    D3DXMATRIX orthoMatrix;
};

