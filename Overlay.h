#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Dwmapi.h> 
#include <TlHelp32.h>

class Overlayyy {

public:
	//Constructor
	Overlayyy();

	int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);

	int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	LPDIRECT3D9 d3;
	LPDIRECT3DDEVICE9 d3dev;

	LPD3DXFONT d3Font;
	LPD3DXFONT d3Font2;

	HWND hWindow;
	HWND tWindow;

	MARGINS margins = { 0, 0, ScreenWidth, ScreenHeight };

	ID3DXLine* d3Line;

	//Initializes our direct3Dx9 instance
	void InitD3D(HWND hWindow);

	//Renders our drawings to the screen
	void Render();

	bool MouseHover(int x, int y, int w, int h);

	bool MouseClick(int x, int y, int w, int h);

	void DrawMouse(DWORD color);

	bool DrawCheckBox(bool* Check, int x, int y, char* text, LPD3DXFONT g_pFont, DWORD color);

	
	//Sets up the overlay window
	void SetupWindow();

	void Quad2(LPDIRECT3DDEVICE9 c_pDev, DWORD COLOR, IDirect3DTexture9* ppD3Dtex, float Point1x, float Point1y, float Point2x, float Point2y, float Point3x, float Point3y, float Point4x, float Point4y);

	//Main loop that calls the esp then renders it
	WPARAM Loop();

	void AimLoop();

	//Esp stuff
	void MainLoop();

	void DrawCircle(int X, int Y, int radius, int numSides, DWORD Color);

	void FillArea(float x, float y, float width, float height, D3DCOLOR color);

	void FillRectangle(double x, double y, double w, double h, D3DCOLOR color);

	void DrawBox(float x, float y, float width, float height, D3DCOLOR color);

	void DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char * fmt, ...);

	void DrawLine(float x1, float y1, float x2, float y2, D3DCOLOR color);

	void DrawCornerBox(int X, int Y, int W, int H);

	void DrawCheck(DWORD Color, float x, float y);

	void DrawBorderBox(int x, int y, int w, int h, int thickness, DWORD color);



	

	


};
