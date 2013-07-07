#include "D3DBinding.h"


D3DBinding::D3DBinding(int screenWidth, int screenHeight, bool vsync, HWND hwnd, float screenDepth, float screenNear) {
    HRESULT result;
    IDXGIFactory* factory;
    vsync_enabled = vsync;
    // Create a DirectX graphics interface factory.
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result)) {
        throw EngineException(L"Can't initialize D3D factory");
    }
    // Use the factory to create an adapter for the primary graphics interface (video card).
    IDXGIAdapter* adapter;
    result = factory->EnumAdapters(0, &adapter);
    if (FAILED(result)) {
        throw EngineException(L"Can't find graphic adapters");
    }

    // Enumerate the primary adapter output (monitor).
    IDXGIOutput* adapterOutput;
    result = adapter->EnumOutputs(0, &adapterOutput);
    if (FAILED(result)) {
        throw EngineException(L"Can't find video output");
    }

    // Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
    unsigned int numModes;
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
    if (FAILED(result)) {
        throw EngineException(L"Can't find appropriate display mode");
    }

    // Create a list to hold all the possible display modes for this monitor/video card combination.
    DXGI_MODE_DESC* displayModeList;
    displayModeList = new DXGI_MODE_DESC[numModes];
    if (!displayModeList) {
        throw EngineException(L"Can't create display mode list");
    }

    // Now fill the display mode list structures.
    result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
    if (FAILED(result)) {
        throw EngineException(L"Can't store display modes");
    }

    // Now go through all the display modes and find the one that matches the screen width and height.
    // When a match is found store the numerator and denominator of the refresh rate for that monitor.
    unsigned int i, numerator, denominator;
    for (i = 0; i < numModes; i++) {
        if (displayModeList[i].Width == (unsigned int)screenWidth) {
            if (displayModeList[i].Height == (unsigned int)screenHeight) {
                numerator = displayModeList[i].RefreshRate.Numerator;
                denominator = displayModeList[i].RefreshRate.Denominator;
            }
        }
    }

    // Get the adapter (video card) description.
    DXGI_ADAPTER_DESC adapterDesc;
    result = adapter->GetDesc(&adapterDesc);
    if (FAILED(result)) {
        throw EngineException(L"Failed to retrieve adapter description");
    }

    // Store the dedicated video card memory in megabytes.
    videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

    // Convert the name of the video card to a character array and store it.
    int error;
    unsigned int stringLength;
    error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
    if (error != 0) {
        throw EngineException(L"Can't read video card name");
    }

    // Release the display mode list.
    delete [] displayModeList;
    displayModeList = 0;

    // Release the adapter output.
    adapterOutput->Release();
    adapterOutput = 0;

    // Release the adapter.
    adapter->Release();
    adapter = 0;

    // Release the factory.
    factory->Release();
    factory = 0;

    // Initialize the swap chain description.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    // Set to a single back buffer.
    swapChainDesc.BufferCount = 1;

    // Set the width and height of the back buffer.
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;

    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the refresh rate of the back buffer.
    if (vsync_enabled) {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
    } else {
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    }

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = hwnd;

    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;


    swapChainDesc.Windowed = true;

    // Set the scan line ordering and scaling to unspecified.
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents after presenting.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Don't set the advanced flags.
    swapChainDesc.Flags = 0;
    // Set the feature level to DirectX 11.
    // FIXME: Set to DirectX 10.0 at the moment cause I have an old GTS 240.
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
    // Create the swap chain, Direct3D device, and Direct3D device context.
    result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
                                           D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
    if (FAILED(result)) {
        throw EngineException(L"Failed to create the swap chain and D3D device");
    }

    // Get the pointer to the back buffer.
    ID3D11Texture2D* backBufferPtr;
    result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
    if (FAILED(result)) {
        throw EngineException(L"Could not get backbuffer");
    }

    // Create the render target view with the back buffer pointer.
    result = device->CreateRenderTargetView(backBufferPtr, NULL, &renderTargetView);
    if (FAILED(result)) {
        throw EngineException(L"Could not render target view");
    }

    // Release pointer to the back buffer as we no longer need it.
    backBufferPtr->Release();
    backBufferPtr = 0;

    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

    // Set up the description of the depth buffer.
    depthBufferDesc.Width = screenWidth;
    depthBufferDesc.Height = screenHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    result = device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);
    if (FAILED(result)) {
        throw EngineException(L"Could not create depth buffer texture");
    }

    // Initialize the description of the stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create the depth stencil state.
    result = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
    if (FAILED(result)) {
        throw EngineException(L"Could not create depth stencil state");
    }

    // Set the depth stencil state.
    deviceContext->OMSetDepthStencilState(depthStencilState, 1);

    // Initailze the depth stencil view.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil view description.
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil view.
    result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView);
    if (FAILED(result)) {
        throw EngineException(L"Could not create depth stencil view");
    }

    // Bind the render target view and depth stencil buffer to the output render pipeline.
    deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    // Setup the raster description which will determine how and what polygons will be drawn.
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    result = device->CreateRasterizerState(&rasterDesc, &rasterState);
    if (FAILED(result)) {
        throw EngineException(L"Could not create rasterizer state");
    }

    // Now set the rasterizer state.
    deviceContext->RSSetState(rasterState);

    // Setup the viewport for rendering.
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)screenWidth;
    viewport.Height = (float)screenHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

    // Create the viewport.
    deviceContext->RSSetViewports(1, &viewport);

    // Setup the projection matrix.
    float fieldOfView, screenAspect;
    fieldOfView = (float)D3DX_PI / 4.0f;
    screenAspect = (float)screenWidth / (float)screenHeight;

    // Create the projection matrix for 3D rendering.
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

    // Initialize the world matrix to the identity matrix.
    D3DXMatrixIdentity(&worldMatrix);

    // Create an orthographic projection matrix for 2D rendering.
    D3DXMatrixOrthoLH(&orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);
}


D3DBinding::~D3DBinding(void) {
    // Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
    if (swapChain) {
        swapChain->SetFullscreenState(false, NULL);
    }

    if (rasterState) {
        rasterState->Release();
        rasterState = NULL;
    }

    if (depthStencilView) {
        depthStencilView->Release();
        depthStencilView = NULL;
    }

    if (depthStencilState) {
        depthStencilState->Release();
        depthStencilState = NULL;
    }

    if (depthStencilBuffer) {
        depthStencilBuffer->Release();
        depthStencilBuffer = NULL;
    }

    if (renderTargetView) {
        renderTargetView->Release();
        renderTargetView = NULL;
    }

    if (deviceContext) {
        deviceContext->Release();
        deviceContext = NULL;
    }

    if (device) {
        device->Release();
        device = NULL;
    }

    if (swapChain) {
        swapChain->Release();
        swapChain = NULL;
    }
}

void D3DBinding::BeginScene(float red, float green, float blue, float alpha) {
    float color[4];

    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;

    // Clear the back buffer.
    deviceContext->ClearRenderTargetView(renderTargetView, color);

    // Clear the depth buffer.
    deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    return;
}

void D3DBinding::EndScene() {
    // Present the back buffer to the screen since rendering is complete.
    if (vsync_enabled) {
        // Lock to screen refresh rate.
        swapChain->Present(1, 0);
    } else {
        // Present as fast as possible.
        swapChain->Present(0, 0);
    }

    return;
}

ID3D11Device* D3DBinding::GetDevice() {
    return device;
}


ID3D11DeviceContext* D3DBinding::GetDeviceContext() {
    return deviceContext;
}

void D3DBinding::GetProjectionMatrix(D3DXMATRIX& projectionMatrix) {
    projectionMatrix = this->projectionMatrix;
}


void D3DBinding::GetWorldMatrix(D3DXMATRIX& worldMatrix) {
    worldMatrix = this->worldMatrix;
}


void D3DBinding::GetOrthoMatrix(D3DXMATRIX& orthoMatrix) {
    orthoMatrix = this->orthoMatrix;
}

void D3DBinding::GetVideoCardInfo(char* cardName, int& memory) {
    strcpy_s(cardName, 128, videoCardDescription);
    memory = videoCardMemory;
}