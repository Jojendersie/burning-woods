#include "stdafx.h"

#include "Renderer.h"
#include "Texturemanager.h"
#include "World.h"
#include "Player.h"

#include <time.h>	// for the "clock()" function - normally I would use timeGetTime, but this needs an additional library

#define PROJECT_NAME "Firerun"

bool Quit = false;
float g_PassedTime = 0.0f;
float g_FrameTime = 0.0f;

// **************************************************************************************************************************************************** //
HRESULT WINAPI WindowProc(HWND Window, unsigned int Message, WPARAM WParam, LPARAM LParam)
{
	switch(Message)
	{
	case WM_QUIT:
	case WM_DESTROY:
		DestroyWindow(Window);
		PostQuitMessage(0);
		Quit = true;
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_MOUSEMOVE:
		Player::Get().MovementMessage(Message, WParam, LParam);
		break;

	default:
		return (HRESULT)DefWindowProc(Window, Message, WParam, LParam);
	}
	
	return true;
};


// **************************************************************************************************************************************************** //
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	// Config - temporary, later this should be configured by bat files
	bool Windowed = true;
	unsigned int Width = 1366;
	unsigned int Height = 768;

	// Window init
	WNDCLASS WindowClass;
	ZeroMemory(&WindowClass, sizeof(WNDCLASS));
	WindowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc		= WindowProc;
	WindowClass.lpszClassName	= PROJECT_NAME;
	if(!RegisterClass(&WindowClass))
		return 0;
	DWORD Style;
	if(!Windowed)	
		Style = WS_POPUP;
    else
		Style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;	
	HWND WindowHandle = CreateWindow(PROJECT_NAME, PROJECT_NAME, Style,
									 GetSystemMetrics(SM_CXSCREEN) / 2 - Width / 2,
									  GetSystemMetrics(SM_CYSCREEN) / 2 - Height / 2,
									 Width, Height, NULL, NULL, WindowClass.hInstance, NULL);
	ShowWindow(WindowHandle, SW_SHOW);
    SetForegroundWindow(WindowHandle);
    SetFocus(WindowHandle);

	// random numbers init
	srand(clock());


	// Renderer
	if(!Renderer::Get().Initialize(Width, Height, WindowHandle, Windowed))
		return 1;

	// Texturemanager
	if(!TextureManager::Get().Initialize())
		return 1;

	// World
	if(!World::Get().Initialize())
		return 1;
	// Player
	Player::Get().m_CameraPosition = D3DXVECTOR3(0, 20, -10);


	// Timer Frequency
	__int64 TimerFrequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)(&TimerFrequency));

	// Game Loop
	__int64	StartTime, EndTime;
	while(!Quit)
	{
		// start
		QueryPerformanceCounter((LARGE_INTEGER*)(&StartTime));
		
			// light TEST
		Renderer::Get().m_LightList[0].Active = true;
		Renderer::Get().m_LightList[0].Position = D3DXVECTOR3(0,15,0);
		Renderer::Get().m_LightList[0].Color = D3DXVECTOR3(2.0f + fabs(sinf(g_PassedTime*4))*0.5,1.0f + cosf(g_PassedTime*5)*0.1f, 0.01f);
		Renderer::Get().m_LightList[0].RangeSq = 10000.0f + sinf(g_PassedTime*5)*3;

		// windows messages
		MSG Message;
		while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		
		// Move
		Player::Get().Move();
		World::Get().Simulate();


		// Draw
		if(!Renderer::Get().Draw(Player::Get().m_ViewMatrix, Player::Get().m_CameraPosition, Player::Get().m_CameraDirection, 1.0f))
			Quit = true;

		// End
		QueryPerformanceCounter((LARGE_INTEGER*)(&EndTime));
		if(EndTime == StartTime) 
			g_FrameTime = 0.000001f;
		else 
			g_FrameTime = static_cast<float>(EndTime - StartTime) / TimerFrequency;
		g_PassedTime += g_FrameTime;
	}

	//TextureManager::Get().Shutdown();

	return 0;
}