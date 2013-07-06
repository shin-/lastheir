#include "gamesystem.h"


GameSystem::GameSystem(void) {
	input = NULL;
	graphics = NULL;
	applicationName = L"The Last Heir <ALPHA>";
}


GameSystem::~GameSystem(void) {
	if (graphics) {
		delete graphics;
		graphics = NULL;
	}

	if (input) {
		delete input;
		input = NULL;
	}

	KillWindow();
}

bool GameSystem::Initialize() {
	int screenWidth, screenHeight = 0;

	// Initialize the windows api.
	OpenWindow(screenWidth, screenHeight);

	input = new Input();
	if (!input) {
		return false;
	}

	graphics = new Graphx(screenWidth, screenHeight, hwnd);
	if (!graphics) {
		return false;
	}

	return true;
}

void GameSystem::Run() {
	MSG msg;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	while(true) {
		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) {
			break;
		} else if(!Frame()) {
			break;
		}

	}
}

bool GameSystem::Frame() {
	// Check if the user pressed escape and wants to exit the application.
	if (input->IsKeyDown(VK_ESCAPE)) {
		return false;
	}
	graphics->Render();
	return true;
}

LRESULT CALLBACK GameSystem::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	switch (umsg) {
		// Check if a key has been pressed on the keyboard.
		case WM_KEYDOWN:
			input->KeyDown((unsigned int)wparam);
			return 0;
		case WM_KEYUP:
			input->KeyUp((unsigned int)wparam);
			return 0;
		default:
			return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void GameSystem::OpenWindow(int& screenWidth, int& screenHeight) {
	WNDCLASSEX wc;
	int posX, posY, fullW, fullH;

	// Get the instance of this application.
	hinstance = GetModuleHandle(NULL);

	// Need a global pointer on this class so we can send system messages through (i.e. keypresses etc.)
	g_applicationHandle = this;

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	fullW = GetSystemMetrics(SM_CXSCREEN);
	fullH = GetSystemMetrics(SM_CYSCREEN);

	screenWidth  = (int)(fullW * 0.9);
	screenHeight = (int)(fullH * 0.9);

	// Place the window in the middle of the screen.
	posX = (fullW - screenWidth)  / 2;
	posY = (fullH - screenHeight) / 2;


	// Create the window with the screen settings and get the handle to it.
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName, 
				WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
				posX, posY, screenWidth, screenHeight, NULL, NULL, hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(hwnd, SW_SHOW);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}

void GameSystem::KillWindow() {
	// Show the mouse cursor.
	ShowCursor(true);

	// Remove the window.
	DestroyWindow(hwnd);
	hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(applicationName, hinstance);
	hinstance = NULL;
	g_applicationHandle = NULL;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch(umessage) {
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);		
			return 0;
		default:
			return g_applicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
}