#include "stdafx.h"
#include "Overlay.h"
#include <iostream>
#include "Globals.h"
#include "Utility.h"
#include <thread>        
#include <chrono>       
#include <stdlib.h> 
#include <time.h>
#include <sstream> // for using stringstream
#include <vector>
#include <iomanip>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"

#define AIMBOTON 1
ID3DXLine* pLine;
int width = 1920;
int Height = 1080;
int CrosshairX = (width / 2);
int CrosshairY = (Height / 2); 
bool wasButtonPressed = false;
int Aimkey = 6; //XBUTTON2 = 6 //LEFT CLICK = 1
int AimSpeed = 10;
int MaximumFov = 100;
float mouseX = 0;
float mouseY = 0;

bool InRound = false;

namespace globals
{
	DWORD_PTR tempTarget;
	DWORD_PTR tempCloseTarget;
	float tempDist;
}

Overlayyy::Overlayyy() {};
struct Items_ {
	bool selected = false;
	bool on_off = false;
};
extern Items_ Items[200];

float BOG_TO_GRD(float BOG) {
	return (180 / M_PI) * BOG;
}

float GRD_TO_BOG(float GRD) {
	return (M_PI / 180) * GRD;
}


WPARAM Overlayyy::Loop() {
	MSG message;
	RECT rect;

	while (TRUE) {
		ZeroMemory(&message, sizeof(MSG));

		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (message.message == WM_QUIT) {
			exit(0);
		}

		tWindow = NULL;
		tWindow = FindWindow(NULL, L"Rainbow Six");
		if (!tWindow) {
			std::cout << "Application exiting, failed to find the specified window!" << std::endl;
			ExitProcess(0);
		}

		// Start ImGui frame
		//ImGui_ImplDX9_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
		//std::cout << "BALLSACK" << std::endl;
		// Render your ImGui GUI here
		//ImGui::Begin("Overlay Window");
	//	ImGui::Text("Hello, world!");
	//ImGui::End();

		// Render ImGui frame
	//	ImGui::Render();
	//	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		
		
		// Present the frame
		ZeroMemory(&rect, sizeof(RECT));

		GetWindowRect(tWindow, &rect);
		ScreenWidth = rect.right - rect.left;
		ScreenHeight = rect.bottom - rect.top;

		MoveWindow(hWindow, rect.left, rect.top, ScreenWidth, ScreenHeight, true);

		HWND hWindow2 = GetForegroundWindow();
		HWND hWindow3 = GetWindow(hWindow2, GW_HWNDPREV);

		SetWindowPos(hWindow, hWindow3, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		UpdateWindow(hWindow);

		

		Render(); 
		// ... (your rendering code goes here)

		// Sleep to reduce CPU usage (optional)
		Sleep(1);
		
	}
	
	ImGui_ImplDX9_Shutdown();
	
	ImGui_ImplWin32_Shutdown();
	
	ImGui::DestroyContext();
	return message.wParam;
}

inline Vector3 RainbowESP()
{
	static uint32_t cnt = 0;
	float freq = 0.0005f;

	if (cnt++ >= (uint32_t)-1)
		cnt = 0;

	Vector3 ret = { std::sin(freq * cnt + 0) * 0.5f + 0.5f, std::sin(freq * cnt + 2) * 0.5f + 0.5f , std::sin(freq * cnt + 4) * 0.5f + 0.5f };
	return ret;
}

void FlyThread() {
	while (true) {
		Global::Memory.Fly();
		// Do something with the fly value
	}
}


void Overlayyy::Render() {
	d3dev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	d3dev->BeginScene();

	MainLoop();

	d3dev->EndScene();
	d3dev->Present(NULL, NULL, NULL, NULL);
}

bool downPressed = false;

void leftclick() {
	// VK_XBUTTON2 corresponds to the Mouse5 button
	if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) { // Check if Mouse5 is pressed
		if (Global::Memory.TB() == 1) {
			downPressed = true;
			mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		}
		else if (Global::Memory.TB() == 0 && downPressed) {
			mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
			downPressed = false;
		}
	}
	else if (Global::Memory.TB() == 0 && downPressed) {
		// Ensure the left mouse button is released if Mouse5 is no longer pressed
		mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
		downPressed = false;
	}
}




void rightclick() {

	mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

bool GetAimKey()
{

	return (GetAsyncKeyState(Aimkey));

}

float DistancefromCrosshair(DWORD_PTR Entity)
{

	PlayerInfo Player = Global::Memory.GetAllEntityInfo(Entity);
	Vector3 spos = Player.w2sHead;

	float x = (spos.x - CrosshairX);
	float y = (spos.y - CrosshairY);
	float EnemyDistFromXhair = (float)sqrt((x * x) + (y * y));
	//Global::Over.DrawLine(1920 / 2, 1080 / 2, spos.x, spos.y, D3DCOLOR_ARGB(255, 255, 255, 255));
	return EnemyDistFromXhair;
}

void GetClosetTargetFromCrosshair(DWORD_PTR Entity)
{
	float DistfromCrosshair = DistancefromCrosshair(Entity);

	if (DistfromCrosshair <= globals::tempDist) {
		globals::tempDist = DistfromCrosshair;
		globals::tempCloseTarget = Entity;
	}
}

void Lockonplayer(DWORD_PTR Entity)
{
	float hp = Global::Memory.GetEntityHealth(Entity);
	if (GetAimKey() && !wasButtonPressed) {
		wasButtonPressed = TRUE;
		globals::tempTarget = Entity;
	}
	if (GetAimKey() && wasButtonPressed && hp == 0)
	{
		wasButtonPressed = FALSE;
	}
}

void AimPlayer(DWORD_PTR Entity)
{

	float DistXhair = DistancefromCrosshair(Entity);

	PlayerInfo Player = Global::Memory.GetAllEntityInfo(Entity);
	float hp = Player.Health;
	Vector3 headPos = Player.w2sHead;

	float headX = headPos.x - GetSystemMetrics(SM_CXSCREEN) / 2;
	float headY = headPos.y - GetSystemMetrics(SM_CYSCREEN) / 2;


	if (GetAimKey() && !globals::tempTarget == 0 && hp != 0)
	{
		mouse_event(MOUSEEVENTF_MOVE, headX, headY, NULL, NULL);
	}


}

void Overlayyy::AimLoop()
{
	if (!GetAimKey())
	{
		wasButtonPressed = FALSE;
		globals::tempTarget = 0;
	}
	if (!globals::tempTarget)
	{
		globals::tempDist = MaximumFov;
	}



	AimPlayer(globals::tempTarget);
}



void Overlayyy::MainLoop() {
	//Update addresses
	//leftclick();
	


	//Global::Memory.UpdateAddresses();

	//Set our colors for later
	DWORD boxColor = D3DCOLOR_ARGB(255, 255, 0, 0);
	DWORD headCircleColor = D3DCOLOR_ARGB(255, 255, 0, 0);
	DWORD healthBarColor = D3DCOLOR_ARGB(255, 255, 100, 100);
	DWORD nameColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	DWORD blackColor = D3DCOLOR_ARGB(255, 0, 0, 0);
	DWORD whiteColor = D3DCOLOR_ARGB(255, 255, 255, 255);
	DWORD redColor = D3DCOLOR_ARGB(255, 255, 0, 0);
	DWORD greenColor = D3DCOLOR_ARGB(255, 0, 255, 0);
	DWORD PurpleColor = D3DCOLOR_ARGB(255, 160, 32, 240);
	DWORD grayColor = D3DCOLOR_ARGB(155, 28, 28, 28);
	DWORD vaulttogglecolor = redColor;
	
	//Global::Over.AimLoop();
	

	

	//Get our local entity
	DWORD_PTR LocalEntity = Global::Memory.GetLocalEntity();

	int textX = 30;
	int textY = 30;
	int velocityX = 2;
	int velocityY = 2;

	// Variables for screen size
	int screenWidth = 1920;
	int screenHeight = 1080;

	//Global::Memory.HoloToAcog();
		//Global::Memory.HoloToAcogS();
	
	//Get all the info on local player
	PlayerInfo LocalPlayer = Global::Memory.GetAllEntityInfo(LocalEntity);

	//Loop through first 12 entities
	//Feel free to add an actual entity count
	//12 will work fine for multiplayer matches tho

	for (int i = 0; i < 24; i++) {

		
		
		//Global::Memory.GreenGlow();

		// Render the text at the updated position
		//Global::Over.DrawString(textX, textY, whiteColor, Global::Over.d3Font, "Skinwalker\n");

		//Get the current entity
		DWORD_PTR Entity = Global::Memory.GetEntity(i);


		//Get a PlayerInfo struct with all of this entity's info
		PlayerInfo Player = Global::Memory.GetAllEntityInfo(Entity);

		Vector3 rainbowVec = RainbowESP();
		D3DCOLOR rainbowColor = D3DCOLOR_RGBA((int)(rainbowVec.x * 255), (int)(rainbowVec.y * 255), (int)(rainbowVec.z * 255), 255);

		//Some checks to validate that its a real player
		//Only draws players on the other team
		//Won't draw on terrorsit in thunt/situation or the hostage becuase their name is blank
		if (Player.Health > 0 //Health over 0
			&& Player.Health <= 200 //Health less than 200
			//&& Player.w2s.z >= 1.0f //Player is actually on the screen
			//&& Player.w2sHead.z >= 1.0f
			&& Player.Name != "Syralix"// cause you dont deserve it.
			//&& Player.TeamId != LocalPlayer.TeamId //Not on our team
			//&& Player.Name.empty() //Name isn't empty 
			)
		{
			Global::Memory.SetEntityHealth(i);
			//printf("EntityTeams: %p\n", Player.TeamId);

			//Draw health bars
			//Find the current health
			//Health goes to 120, 0-20 is down, 20-120 is regular health
			float CurrentHealth = Player.Health / 120.0f;
			if (CurrentHealth < 0 || CurrentHealth >= 1) CurrentHealth = 1;

			if (Player.Name.empty())
			{
				Player.Name = "Bot";
			}

			leftclick();
		

			//Global::Memory.LegitRecoil();
			
				//Find out the box size to fit the player
				//Scales well at all distances
				//Works like 70% of the time when player is repelling
				//Box is kinda small and looks far away when laying down
				float BoxHeight1 = Player.w2s.y - Player.w2sHead.y;
				float BoxHeight2 = std::abs(Player.w2s.x - Player.w2sHead.x);
				float BoxHeight = (BoxHeight1 > BoxHeight2) ? BoxHeight1 : BoxHeight2;
				float BoxWidth = BoxHeight / 1.5f;

				/*
				Global::Over.DrawLine(Player.w2sHead.x, Player.w2sHead.y, Player.w2sNeck.x, Player.w2sNeck.y, PurpleColor);
				Global::Over.DrawLine(Player.w2sNeck.x, Player.w2sNeck.y, Player.w2sLowerNeck.x, Player.w2sLowerNeck.y, boxColor);
				Global::Over.DrawLine(Player.w2sLowerNeck.x, Player.w2sLowerNeck.y, Player.w2sUpperTorso.x, Player.w2sUpperTorso.y, boxColor);
				Global::Over.DrawLine(Player.w2sUpperTorso.x, Player.w2sUpperTorso.y, Player.w2sTorso.x, Player.w2sTorso.y, boxColor);
				Global::Over.DrawLine(Player.w2sUpperTorso.x, Player.w2sUpperTorso.y, Player.w2sLHand.x, Player.w2sLHand.y, boxColor);
				Global::Over.DrawLine(Player.w2sUpperTorso.x, Player.w2sUpperTorso.y, Player.w2sRHand.x, Player.w2sRHand.y, boxColor);
				Global::Over.DrawLine(Player.w2sTorso.x, Player.w2sTorso.y, Player.w2sPelvis.x, Player.w2sPelvis.y, boxColor);
				*/

				
				if (Global::Legit = TRUE)
				{


					//Global::Over.DrawCircle(Player.w2sHead.x, Player.w2sHead.y, 4, 20, PurpleColor);
					//Global::Over.DrawCircle(Player.w2sHead.x, Player.w2sHead.y, 5.5, 20, PurpleColor);
					//Global::Over.DrawCircle(Player.w2sLHand.x, Player.w2sLHand.y, 5.5, 20, PurpleColor);
					//Global::Over.DrawCircle(Player.w2sRHand.x, Player.w2sRHand.y, 5.5, 20, PurpleColor);
					//Global::Over.DrawCircle(1920 / 2, 1080 / 2, 90, 30, PurpleColor);
					//OVERHEAD ESP STUFF
					//Global::Over.DrawString(Player.w2sHead.x - 21, Player.w2sHead.y + -35, whiteColor, Global::Over.d3Font, "%s", Player.Name);
					//Global::Over.DrawBox(Player.w2sHead.x + 5, Player.w2sHead.y - 29 - 7, 20, 12, greenColor);
					//Global::Over.FillRectangle(Player.w2sHead.x - 25, Player.w2sHead.y - 39 - 1, 53, 18, grayColor);
					//Global::Over.FillArea(Player.w2sHead.x - 25, Player.w2sHead.y - 21, CurrentHealth + 53, (2), greenColor);
					//Global::Over.DrawString(Player.w2sHead.x + 7, Player.w2sHead.y - 29 - 7, greenColor, Global::Over.d3Font, "%s", (std::to_string(Player.Health)));


					//BOX ESP
					//Global::Over.DrawCornerBox(Player.w2sHead.x - (BoxWidth / 2), Player.w2sHead.y + (BoxHeight / 500) - (BoxHeight / 5), BoxWidth, BoxHeight + 15);
					
					//Global::Over.FillRectangle(Player.w2sHead.x - (BoxWidth / 2), Player.w2sHead.y + (BoxHeight / 500) - (BoxHeight / 5), BoxWidth, BoxHeight + 15, grayColor);

					//ODD ASS BOXES
					//DrawBox(Player.w2sHead.x - (BoxWidth / 2) - 1, Player.w2sHead.y - (BoxHeight / 5) - 1, BoxWidth + 2, BoxHeight + 10, rainbowColor);
					//DrawBox(Player.w2sHead.x - (BoxWidth / 2) - 1, Player.w2sHead.y - (BoxHeight / 5) - 1, BoxWidth + 2, BoxHeight + 8, rainbowColor);
					//DrawBox(Player.w2sHead.x - (BoxWidth / 2) - 1, Player.w2sHead.y - (BoxHeight / 5) - 1, BoxWidth + 2, BoxHeight + 6, rainbowColor);
					//DrawBox(Player.w2sHead.x - (BoxWidth / 2) - 1, Player.w2sHead.y - (BoxHeight / 5) - 1, BoxWidth + 2, BoxHeight + 4, rainbowColor);
					//DrawBox(Player.w2sHead.x - (BoxWidth / 2) - 1, Player.w2sHead.y - (BoxHeight / 5) - 1, BoxWidth + 2, BoxHeight + 2, rainbowColor);
				}
				//WATERMARK
				
					//if (Global::Rage = TRUE)
					//{
				

			/*

						//WEAPONSHIT
						Global::Memory.NoWeaponSound();
						Global::Memory.ZeroRecoil();
						//Global::Memory.LegitRecoil();
						Global::Memory.ZeroSpread();
						Global::Memory.FireMode();
						Global::Memory.UnlimitedAmmo();
						Global::Memory.WeaponFOV();
						//
									///KNIFE SHIT
						Global::Memory.LongKnife();

						//EXPLOIT SHIT
						//Global::Memory.SwapTeams();
						//Global::Memory.Shoot();

						  // Might Crash?
						//Global::Memory.Speed();

						//if (Global::ThirdPerson) {
						//	Global::Memory.ThirdPerson();
						//}
						//if (Global::ForceFire) {
							Global::Memory.ForceFire();
							Global::Memory.RapidFire();
							Global::Memory.GreenGlow();
							//Global::Memory.RunAndShoot();
						//}
					//}
				//Global::Memory.ForceFire();
				//Global::Memory.ThirdPerson();
				//Global::Memory.ForceFireNoTrig();
				 //NEEDS IMPROVEMENT
				//Global::Memory.Fly(); //todo; fix crashing
							*/


				/*
				if (AIMBOTON == 1) {

					Vector3 spos = Player.w2sHead;

					float x = (spos.x - CrosshairX);
					float y = (spos.y - CrosshairY);
					float EnemyDistFromXhair = (float)sqrt((x * x) + (y * y));

					if (EnemyDistFromXhair <= MaximumFov) {

						GetClosetTargetFromCrosshair(Entity);
					}
				}*/
			


				//if (Global::Aimbot == TRUE) {
					//uint64_t target = 0;
					//Vector3 headPos = Player.w2sHead;
					//target = GetClosestEntityToCrosshair();
					//float headX = headPos.x - GetSystemMetrics(SM_CXSCREEN) / 2;
					//float headY = headPos.y - GetSystemMetrics(SM_CYSCREEN) / 2;
					//float DistXhair = DistancefromCrosshair(Entity);


					//if (headX >= -150 && headX <= 150 && headY >= -150 && headY <= 150) {
						//Global::Over.DrawLine(1920 / 2, 1080 / 2, headPos.x, headPos.y, whiteColor);
						//GetClosetTargetFromCrosshair(Entity);
						//if (GetAsyncKeyState(VK_XBUTTON2) && headX != 0 && headY != 0) {
							//mouse_event(MOUSEEVENTF_MOVE, headX, headY, NULL, NULL);
						//AimPlayer(tempTarget);
						//printf("HeadY: %F\n",headY);
						//Global::Memory.VaultTp(100);
						//vaulttogglecolor = greenColor;
						//if (Global::Memory.TB() == 1)
						//{
							//mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
							//mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
						//}

					//}
					//vaulttogglecolor = redColor;
					//Global::Over.DrawString(25, 50, vaulttogglecolor, Global::Over.d3Font, "Vault Toggle\n");
			}

		}
		//Lockonplayer(globals::tempCloseTarget);
	}


void Overlayyy::InitD3D(HWND handleWindow) {
	//Create our direct3d object
	d3 = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS direct3DPresetParams;

	ZeroMemory(&direct3DPresetParams, sizeof(direct3DPresetParams));

	direct3DPresetParams.Windowed = true;
	direct3DPresetParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	direct3DPresetParams.hDeviceWindow = handleWindow;
	direct3DPresetParams.BackBufferFormat = D3DFMT_A8R8G8B8;
	direct3DPresetParams.BackBufferWidth = ScreenWidth;
	direct3DPresetParams.BackBufferHeight = ScreenHeight;

	direct3DPresetParams.EnableAutoDepthStencil = TRUE;
	direct3DPresetParams.AutoDepthStencilFormat = D3DFMT_D16;


	d3->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handleWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &direct3DPresetParams, &d3dev);
	D3DXCreateFontA(d3dev, 14, 0, FW_HEAVY, 1, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &d3Font);

	


	D3DXCreateLine(d3dev, &d3Line);
}

void Overlayyy::DrawString(int x, int y, DWORD color, LPD3DXFONT g_pFont, const char* fmt, ...)
{
	char buf[1024] = { 0 };
	va_list va_alist;
	RECT FontPos = { x, y, x + 120, y + 16 };
	va_start(va_alist, fmt);
	vsprintf_s(buf, fmt, va_alist);
	va_end(va_alist);
	g_pFont->DrawTextA(NULL, buf, -1, &FontPos, DT_NOCLIP, color);
}

void Overlayyy::DrawCircle(int X, int Y, int radius, int numSides, DWORD color) {
	D3DXVECTOR2 Line[128];
	float Step = M_PI * 2.0 / numSides;
	int Count = 0;
	for (float a = 0; a < M_PI*2.0; a += Step)
	{
		float X1 = radius * cos(a) + X;
		float Y1 = radius * sin(a) + Y;
		float X2 = radius * cos(a + Step) + X;
		float Y2 = radius * sin(a + Step) + Y;
		Line[Count].x = X1;
		Line[Count].y = Y1;
		Line[Count + 1].x = X2;
		Line[Count + 1].y = Y2;
		Count += 2;
	}
	d3Line->Draw(Line, Count, color);
}

void Overlayyy::FillArea(float x, float y, float width, float height, D3DCOLOR color) {
	D3DXVECTOR2 vectorLine[2];
	d3Line->SetWidth(width);

	vectorLine[0].x = x + width / 2;
	vectorLine[0].y = y;
	vectorLine[1].x = x + width / 2;
	vectorLine[1].y = y + height;

	d3Line->Draw(vectorLine, 2, color);
}

D3DXVECTOR3 GetMidPoint(D3DXVECTOR3 V1, D3DXVECTOR3 V2)
{
	D3DXVECTOR3 Mid;
	Mid.x = (V1.x + V2.x) / 2;
	Mid.y = (V1.y + V2.y) / 2;
	Mid.z = (V1.z + V2.z) / 2;
	return Mid;
}

bool Trans3DTo2D(LPDIRECT3DDEVICE9 pDevice, D3DXVECTOR3* InOut)
{
	D3DXVECTOR3 vScreen;
	D3DXVECTOR3 PlayerPos(InOut->x, InOut->y, InOut->z);
	D3DVIEWPORT9 viewPort = { 0 };
	D3DXMATRIX projection, view, world;
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &projection);
	pDevice->GetTransform(D3DTS_WORLD, &world);
	pDevice->GetViewport(&viewPort);
	D3DXVec3Project(&vScreen, &PlayerPos, &viewPort, &projection, &view, &world);
	if (vScreen.z < 1)
	{
		*InOut = vScreen;
		return true;
	}
	return false;
}

void Draw3DBox(D3DXVECTOR3 Head, D3DXVECTOR3 Feet, DWORD Color, LPDIRECT3DDEVICE9 Device)
{
	int xz = 16;
	int uzunluk = xz * 2;
	D3DXVECTOR3 Pos0, Pos1, Pos2, Pos3, Pos4, Pos5, Pos6, Pos7, Pos8;
	Pos0 = GetMidPoint(Head, Feet);
	Pos1 = Pos0 + D3DXVECTOR3(-xz, uzunluk, -xz);
	Pos2 = Pos0 + D3DXVECTOR3(-xz, -uzunluk, -xz);
	Pos3 = Pos0 + D3DXVECTOR3(xz, -uzunluk, -xz);
	Pos4 = Pos0 + D3DXVECTOR3(xz, uzunluk, -xz);
	Pos5 = Pos0 + D3DXVECTOR3(-xz, uzunluk, xz);
	Pos6 = Pos0 + D3DXVECTOR3(-xz, -uzunluk, xz);
	Pos7 = Pos0 + D3DXVECTOR3(xz, -uzunluk, xz);
	Pos8 = Pos0 + D3DXVECTOR3(xz, uzunluk, xz);

	if (Trans3DTo2D(Device, &Pos1) && Trans3DTo2D(Device, &Pos2) &&
		Trans3DTo2D(Device, &Pos3) && Trans3DTo2D(Device, &Pos4) &&
		Trans3DTo2D(Device, &Pos5) && Trans3DTo2D(Device, &Pos6) &&
		Trans3DTo2D(Device, &Pos7) && Trans3DTo2D(Device, &Pos8))
	{
		
		
		Global::Over.DrawLine(Pos1.x, Pos1.y, Pos2.x, Pos2.y, Color);
		Global::Over.DrawLine(Pos2.x, Pos2.y, Pos3.x, Pos3.y, Color);
		Global::Over.DrawLine(Pos3.x, Pos3.y, Pos4.x, Pos4.y, Color);
		Global::Over.DrawLine(Pos4.x, Pos4.y, Pos1.x, Pos1.y, Color);
		Global::Over.DrawLine(Pos5.x, Pos5.y, Pos6.x, Pos6.y, Color);
		Global::Over.DrawLine(Pos6.x, Pos6.y, Pos7.x, Pos7.y, Color);
		Global::Over.DrawLine(Pos7.x, Pos7.y, Pos8.x, Pos8.y, Color);
		Global::Over.DrawLine(Pos8.x, Pos8.y, Pos5.x, Pos5.y, Color);
		Global::Over.DrawLine(Pos1.x, Pos1.y, Pos5.x, Pos5.y, Color);
		Global::Over.DrawLine(Pos2.x, Pos2.y, Pos6.x, Pos6.y, Color);
		Global::Over.DrawLine(Pos3.x, Pos3.y, Pos7.x, Pos7.y, Color);
		Global::Over.DrawLine(Pos4.x, Pos4.y, Pos8.x, Pos8.y, Color);

		
	}
}

void Overlayyy::FillRectangle(double x, double y, double w, double h, D3DCOLOR color)
{
	struct Vertex
	{
		float x, y, z, h;
		D3DCOLOR color;
	};

	Vertex vertices[4] =
	{
		x,  y, 0.0f, 1.0f, color,
		x + w, y, 0.0f, 1.0f, color,

		x + w, y + h, 0.0f, 1.0f, color,
		x,  y + h, 0.0f, 1.0f, color,
	};

	d3dev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	d3dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	d3dev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	d3dev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(Vertex));
}

void Overlayyy::DrawBox(float x, float y, float width, float height, D3DCOLOR color) {
	D3DXVECTOR2 points[5];
	points[0] = D3DXVECTOR2(x, y);
	points[1] = D3DXVECTOR2(x + width, y);
	points[2] = D3DXVECTOR2(x + width, y + height);
	points[3] = D3DXVECTOR2(x, y + height);
	points[4] = D3DXVECTOR2(x, y);
	d3Line->SetWidth(1);
	d3Line->Draw(points, 5, color);
}


void Overlayyy::DrawLine(float x1, float y1, float x2, float y2, D3DCOLOR color) {
	D3DXVECTOR2 points[2];
	points[0] = D3DXVECTOR2(x1, y1);
	points[1] = D3DXVECTOR2(x2, y2);
	d3Line->SetWidth(1);
	d3Line->Draw(points, 2, color);
}

void Overlayyy::DrawCornerBox(int X, int Y, int W, int H) {
	float lineW = (W / 5);
	float lineH = (H / 6);
	float lineT = 1;

	

	//outline

	Global::Over.DrawLine(X - lineT, Y - lineT, X + lineW, Y - lineT, D3DCOLOR_ARGB(255, 0, 0, 0)); //top left
	Global::Over.DrawLine(X - lineT, Y - lineT, X - lineT, Y + lineH, D3DCOLOR_ARGB(255, 0, 0, 0));
	Global::Over.DrawLine(X - lineT, Y + H - lineH, X - lineT, Y + H + lineT, D3DCOLOR_ARGB(255, 0, 0, 0)); //bot left
	Global::Over.DrawLine(X - lineT, Y + H + lineT, X + lineW, Y + H + lineT, D3DCOLOR_ARGB(255, 0, 0, 0));
	Global::Over.DrawLine(X + W - lineW, Y - lineT, X + W + lineT, Y - lineT, D3DCOLOR_ARGB(255, 0, 0, 0)); // top right
	Global::Over.DrawLine(X + W + lineT, Y - lineT, X + W + lineT, Y + lineH, D3DCOLOR_ARGB(255, 0, 0, 0));
	Global::Over.DrawLine(X + W + lineT, Y + H - lineH, X + W + lineT, Y + H + lineT, D3DCOLOR_ARGB(255, 0, 0, 0)); // bot right
	Global::Over.DrawLine(X + W - lineW, Y + H + lineT, X + W + lineT, Y + H + lineT, D3DCOLOR_ARGB(255, 0, 0, 0));

	//inline
	
	Global::Over.DrawLine(X, Y, X, Y + lineH, D3DCOLOR_ARGB(255, 160, 32, 240));//top left
	Global::Over.DrawLine(X, Y, X + lineW, Y, D3DCOLOR_ARGB(255, 160, 32, 240));
	Global::Over.DrawLine(X + W - lineW, Y, X + W, Y, D3DCOLOR_ARGB(255, 160, 32, 240)); //top right
	Global::Over.DrawLine(X + W, Y, X + W, Y + lineH, D3DCOLOR_ARGB(255, 160, 32, 240));
	Global::Over.DrawLine(X, Y + H - lineH, X, Y + H, D3DCOLOR_ARGB(255, 160, 32, 240)); //bot left
	Global::Over.DrawLine(X, Y + H, X + lineW, Y + H, D3DCOLOR_ARGB(255, 160, 32, 240));
	Global::Over.DrawLine(X + W - lineW, Y + H, X + W, Y + H, D3DCOLOR_ARGB(255, 160, 32, 240));//bot right
	Global::Over.DrawLine(X + W, Y + H - lineH, X + W, Y + H, D3DCOLOR_ARGB(255, 160, 32, 240));

}


void Overlayyy::DrawCheck(DWORD Color, float x, float y)
{
	Global::Over.FillRectangle(x, y, 1, 3, Color);
	Global::Over.FillRectangle(x + 1, y + 1, 1, 3, Color);
	Global::Over.FillRectangle(x + 2, y + 2, 1, 3, Color);
	Global::Over.FillRectangle(x + 3, y + 1, 1, 3, Color);
	Global::Over.FillRectangle(x + 4, y, 1, 3, Color);
	Global::Over.FillRectangle(x + 5, y - 1, 1, 3, Color);
	Global::Over.FillRectangle(x + 6, y - 2, 1, 3, Color);
	Global::Over.FillRectangle(x + 7, y - 3, 1, 3, Color);
}

void Overlayyy::DrawBorderBox(int x, int y, int w, int h, int thickness, DWORD color)
{
	//Top horiz line
	Global::Over.FillRectangle(x, y, w, thickness, color);
	//Left vertical line
	Global::Over.FillRectangle(x, y, thickness, h, color);
	//right vertical line
	Global::Over.FillRectangle((x + w), y, thickness, h, color);
	//bottom horiz line
	Global::Over.FillRectangle(x, y + h, w + thickness, thickness, color);
}

bool Overlayyy::MouseHover(int x, int y, int w, int h)
{
	POINT globalMouse;
	GetCursorPos(&globalMouse);
	ScreenToClient(GetForegroundWindow(), &globalMouse);
	if (globalMouse.x > x && globalMouse.y > y && globalMouse.x <= x + w && globalMouse.y <= y + h)
	{
		return true;
	}

	return false;
}

bool Overlayyy::MouseClick(int x, int y, int w, int h)
{

	POINT globalMouse;
	GetCursorPos(&globalMouse);
	ScreenToClient(GetForegroundWindow(), &globalMouse);
	if (Overlayyy::MouseHover(x, y, w, h))
	{
		if (GetAsyncKeyState(VK_LBUTTON) & 1)
			return true;
	}
	return false;
}

void Overlayyy::DrawMouse(DWORD color)
{
	DWORD colorv = 0x008FFF;
	POINT globalMouse;
	GetCursorPos(&globalMouse);
	ScreenToClient(GetForegroundWindow(), &globalMouse);

	int aList[13] = { 14, 12, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	int cList[8] = { 10, 8, 6, 5, 4, 3, 2, 1 };

	for (int i = 0; i < 10; i++)
	{
		Global::Over.DrawBox((globalMouse.x + i) + 1, (globalMouse.y + i), 3, aList[i], colorv);
	}
	for (int i = 0; i < 8; i++)
	{
		Global::Over.DrawBox(globalMouse.x + i + 2, globalMouse.y + i + 2, 2, cList[i], color);
	}
	return;
}

//Implementation


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		Global::Over.Render();
		break;
	case WM_CREATE:
		DwmExtendFrameIntoClientArea(hWnd, &Global::Over.margins);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static const char alphanum[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

char genRandom()
{
	return alphanum[rand() % stringLength];
}


	
	


	void Overlayyy::SetupWindow() {

		RECT rect;

		while (!tWindow) {
			tWindow = FindWindow(NULL, L"Rainbow Six");
		}

		if (tWindow != NULL) {
			GetWindowRect(tWindow, &rect);
		}
		else {
			std::cout << "Application exiting, error preapring the window. Error code: " << GetLastError() << std::endl;
			Sleep(3000);
			ExitProcess(0);
		}

		WNDCLASSEX windowClass;

		ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hInstance = GetModuleHandle(0);
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = (HBRUSH)RGB(0, 0, 0);

		srand(time(0));
		std::wstring Str;
		for (unsigned int i = 0; i < 20; ++i)
		{
			Str += genRandom();

		}
		windowClass.lpszClassName = Str.c_str();

		RegisterClassEx(&windowClass);

		//hWindow = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, windowClass.lpszClassName, Str.c_str(), WS_POPUP, rect.left, rect.top, ScreenWidth, ScreenHeight, NULL, NULL, windowClass.hInstance, NULL);

		SetLayeredWindowAttributes(hWindow, RGB(0, 0, 0), 0, ULW_COLORKEY);
		SetLayeredWindowAttributes(hWindow, 0, 255, LWA_ALPHA);

		ShowWindow(hWindow, SW_SHOW);
		InitD3D(hWindow);
	}

