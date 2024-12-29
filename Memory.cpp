#include "stdafx.h"
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include "Memory.h"
#include "Globals.h"
#include <thread>

namespace ThirdPerson {
	uint64_t ADSLEANL6;
	uint64_t MAINCHAIN6;
	uint64_t MAINCHAIN7;
	uint64_t MAINCHAIN8;
	uint64_t MAINCHAIN9;
	uint64_t MAINCHAIN10;
	uint64_t Idle6;

	bool InGame = true;
	bool TPEnable = true;
}




bool prevKeyState = false;

bool IsKeyPressed(int keyCode) {
	// Check if the specified key is currently pressed
	return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
}
//All offset variables hardcoded
namespace Offset {
    DWORD_PTR GameManager = 0x47F00D0;
    DWORD_PTR EntityList = 0xC0;
    
    DWORD_PTR Entity = 0x0008;
    DWORD_PTR EntityRef = 0x20;
    
    DWORD_PTR EntityInfo = 0x18;
    DWORD_PTR MainComponent = 0xB8;
    DWORD_PTR ChildComponent = 0x8;
    DWORD_PTR Health = 0x108;
    
    DWORD_PTR PlayerInfo = 0x2A0;
    DWORD_PTR PlayerInfoDeref = 0x0;
    DWORD_PTR PlayerTeamId = 0x140;
    DWORD_PTR PlayerName = 0x158;
    
    DWORD_PTR FeetPosition = 0x1C0;
	DWORD_PTR NeckPosition = 0x170;
	DWORD_PTR TorsoPosition = 0x840;
	DWORD_PTR UpperTorsoPosition = 0x860;
	DWORD_PTR PelvisPosition = 0x880;
	DWORD_PTR LowerNeckPosition = 0x8A0;
    DWORD_PTR HeadPosition = 0x160;
	DWORD_PTR TestPos = 0x0;
	DWORD_PTR LeftHandPosition = 0x1E0;
	DWORD_PTR RightHandPosition = 0x1A0;
	
    
    DWORD_PTR WeaponComp = 0x38;
    DWORD_PTR WeaponProcessor = 0xF0;
    DWORD_PTR Weapon = 0x0;
    DWORD_PTR WeaponInfo = 0x110;
    DWORD_PTR Spread = 0x2A0;
    DWORD_PTR Recoil = 0x2D8;
    DWORD_PTR Recoil2 = 0x354;
    DWORD_PTR Recoil3 = 0x304;
    DWORD_PTR AdsRecoil = 0x330;
    
    DWORD_PTR Renderer = 0x47A4930;
    DWORD_PTR GameRenderer = 0x0;
    DWORD_PTR EngineLink = 0xd8;
    DWORD_PTR Engine = 0x218;
    DWORD_PTR Camera = 0x38;

	DWORD_PTR NetworkManager = 0x4798310;

    
    DWORD_PTR ViewTranslastion = 0x1A0;
    DWORD_PTR ViewRight = 0x170;
    DWORD_PTR ViewUp = 0x180;
    DWORD_PTR ViewForward = 0x190;
    DWORD_PTR FOVX = 0x1B0;
    DWORD_PTR FOVY = 0x1C4;

	DWORD_PTR ESPCHAIN = 0x30;
	DWORD_PTR ESPCHAIN2 = 0x78; // ESPCHAIN1 ->
	DWORD_PTR ESPCHAIN3 = 0x2E4; // ESPCHAIN2 ->
}



Memoryyy::Memoryyy() {};

BOOL Memoryyy::SetBaseAddress() {
	HMODULE hMods[1024];
	DWORD cbNeeded;
	unsigned int i;

	if (EnumProcessModules(Global::GameHandle, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];
			if (GetModuleFileNameEx(Global::GameHandle, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				std::wstring wstrModName = szModName;
				std::wstring wstrModContain = L"RainbowSix.exe";
				if (wstrModName.find(wstrModContain) != std::string::npos)
				{
					Global::BaseAddress = hMods[i];
					return true;
				}
			}
		}
	}
	return false;
}

DWORD_PTR Memoryyy::GetBaseAddress() {
	return (DWORD_PTR)Global::BaseAddress;
}

template<typename T> T Memoryyy::RPM(SIZE_T address) {
	//The buffer for data that is going to be read from memory
	T buffer;

	//The actual RPM
	ReadProcessMemory(Global::GameHandle, (LPCVOID)address, &buffer, sizeof(T), NULL);

	//Return our buffer
	return buffer;
}

std::string Memoryyy::RPMString(SIZE_T address) {
	//Make a char array of 20 bytes
	char name[20];

	//The actual RPM
	ReadProcessMemory(Global::GameHandle, (LPCVOID)address, &name, sizeof(name), NULL);

	//Add each char to our string
	//While also checking for a null terminator to end the string
	std::string nameString;
	for (int i = 0; i < sizeof(name); i++) {
		if (name[i] == 0)
			break;
		else
			nameString += name[i];
	}

	return nameString;
}

template<typename T> void Memoryyy::WPM(SIZE_T address, T buffer) {
	//A couple checks to try and avoid crashing
	//These don't actually make sense, feel free to remove redundent ones
	if (address == 0 || (LPVOID)address == nullptr || address == NULL) {
		return;
	}

	WriteProcessMemory(Global::GameHandle, (LPVOID)address, &buffer, sizeof(buffer), NULL);
}

template <std::size_t Index, typename ReturnType, typename... Args>
__forceinline ReturnType call_virtual(void* instance, Args... args)
{
	using Fn = ReturnType(__thiscall*)(void*, Args...);

	auto function = (*reinterpret_cast<Fn**>(instance))[Index];
	return function(instance, args...);
}

void Memoryyy::UpdateAddresses() {
	//Game manager pointer from games base address + the GameManager offset
	pGameManager = RPM<DWORD_PTR>(GetBaseAddress() + Offset::GameManager);
	//Entity list pointer from the GameManager + EntityList offset
	pEntityList = RPM<DWORD_PTR>(pGameManager + Offset::EntityList);

	//Renderer pointer from games base address + Renderer offset
	pRender = RPM<DWORD_PTR>(GetBaseAddress() + Offset::Renderer);
	//Game Renderer pointer from Renderer + GameRenderer offset
	pGameRender = RPM<DWORD_PTR>(pRender + Offset::GameRenderer);
	//EngineLink pointer from GameRenderer + EngineLink offset
	pEngineLink = RPM<DWORD_PTR>(pGameRender + Offset::EngineLink);
	//Engine pointer from EngineLink + Engine offset
	pEngine = RPM<DWORD_PTR>(pEngineLink + Offset::Engine);
	//Camera pointer from Engine + Camera offset
	pCamera = RPM<DWORD_PTR>(pEngine + Offset::Camera);
}

Vector3 Memoryyy::GetGadgets(int g) {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(GetBaseAddress() + 0x47A41C0);
	DWORD_PTR entityBalls = RPM<DWORD_PTR>(entityBase + 0x60);
	DWORD_PTR entityPenis = RPM<DWORD_PTR>(entityBalls + 0x98);
	return RPM<Vector3>(entityPenis + (g * 0x40));
}

DWORD_PTR Memoryyy::GetEntity(int i) {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(pEntityList + (i * Offset::Entity));
	return RPM<DWORD_PTR>(entityBase + Offset::EntityRef);
}

DWORD_PTR Memoryyy::SetADSSens(int s) {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(GetBaseAddress() + 0x47948D0);
	DWORD_PTR entityBalls = RPM<DWORD_PTR>(entityBase + 0x48);
	WPM<int>(entityBalls + 0x7C , s);
	return entityBalls;
}

DWORD_PTR Memoryyy::GetLocalEntity() {
	//Loop through the first 12
	for (int i = 0; i < 12; i++) {
		//get current entity
		DWORD_PTR entity = GetEntity(i);
		//get that entity's name
		//printf("Players: %p\n", GetEntity(i));
		std::string entityName = GetEntityPlayerName(entity);

		//check it against our local name
		if (strcmp(entityName.c_str(), Global::LocalName.c_str()) == 0) {
			return entity;
		}
	}
	//printf("LocalPlayer: %p\n", GetEntity(0));
	//return the first entity if we didn't find anything
	return GetEntity(0);
}

DWORD Memoryyy::GetEntityHealth(DWORD_PTR entity) {
	//Entity info pointer from the Entity
	DWORD_PTR EntityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	//printf("Entity Info: %p\n", EntityInfo);
	//Main component pointer from the entity info
	DWORD_PTR MainComponent = RPM<DWORD_PTR>(EntityInfo + Offset::MainComponent);
	//Child component pointer form the main component
	DWORD_PTR ChildComponent = RPM<DWORD_PTR>(MainComponent + Offset::ChildComponent);
	//printf("ChildComponent: %p\n", ChildComponent);
	//Finally health from the child component
	return RPM<DWORD>(ChildComponent + Offset::Health);
}

DWORD Memoryyy::FindMainComponent(DWORD_PTR entity) {
	//Entity info pointer from the Entity
	DWORD_PTR EntityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	//Main component pointer from the entity info
	DWORD_PTR MainComponent = RPM<DWORD_PTR>(EntityInfo + Offset::MainComponent);
	//Child component pointer form the main component
	//printf("MainComponent: %p\n", MainComponent);
	return MainComponent;
}

DWORD Memoryyy::SetEntityHealth(DWORD_PTR entity) {
	//Entity info pointer from the Entity
	DWORD_PTR EntityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	//Main component pointer from the entity info
	DWORD_PTR MainComponent = RPM<DWORD_PTR>(EntityInfo + Offset::MainComponent);
	//Child component pointer form the main component
	DWORD_PTR ChildComponent = RPM<DWORD_PTR>(MainComponent + Offset::ChildComponent);

	//Finally health from the child component
	WPM<DWORD>(ChildComponent + Offset::Health, 0);
	return ChildComponent;
}

Vector3 Memoryyy::GetEntityFeetPosition(DWORD_PTR entity) {
	//printf("FeetPosition: % p\n", entity);
	//We get the feet position straight from the entity
	return RPM<Vector3>(entity + Offset::FeetPosition);

}

Vector3 Memoryyy::GetEntityTorsoPosition(DWORD_PTR entity) {
	//bool currentKeyState = IsKeyPressed(VK_UP); // Replace `Key::YourDesiredKey` with the actual key you want to check

	//if (currentKeyState && !prevKeyState) {
	//	Offset::TestPos -= 1; // Increment the TestPos value by 2 when the desired key is pressed
	//}

	//prevKeyState = currentKeyState; // Update the previous key state

	Vector3 torsoPosition = RPM<Vector3>(entity + Offset::TorsoPosition);

	//printf("TestPosition: % p\n", Offset::TestPos);
	

	return torsoPosition;
}

Vector3 Memoryyy::GetEntityUpperTorsoPosition(DWORD_PTR entity) {
	//bool currentKeyState = IsKeyPressed(VK_UP); // Replace `Key::YourDesiredKey` with the actual key you want to check

	//if (currentKeyState && !prevKeyState) {
	//	Offset::TestPos -= 1; // Increment the TestPos value by 2 when the desired key is pressed
	//}

	//prevKeyState = currentKeyState; // Update the previous key state

	Vector3 torsoPosition = RPM<Vector3>(entity + Offset::UpperTorsoPosition);

	//printf("TestPosition: % p\n", Offset::TestPos);


	return torsoPosition;
}

Vector3 Memoryyy::GetEntityLHandPosition(DWORD_PTR entity) {
	//printf("FeetPosition: % p\n", entity);
	//We get the feet position straight from the entity
	return RPM<Vector3>(entity + Offset::LeftHandPosition);



}
Vector3 Memoryyy::GetEntityRHandPosition(DWORD_PTR entity) {
	//printf("FeetPosition: % p\n", entity);
	//We get the feet position straight from the entity
	return RPM<Vector3>(entity + Offset::RightHandPosition);



}

Vector3 Memoryyy::GetEntityNeckPosition(DWORD_PTR entity) {
	//printf("FeetPosition: % p\n", entity);
	//We get the feet position straight from the entity
	return RPM<Vector3>(entity + Offset::NeckPosition);



}

Vector3 Memoryyy::GetEntityHeadPosition(DWORD_PTR entity) {
	//We get the head position straight from the entity
	//printf("HeadPosition: % p\n", entity);
	return  RPM<Vector3>(entity + Offset::HeadPosition);
}


Vector3 Memoryyy::GetEntityPelvisPosition(DWORD_PTR entity) {
	//We get the head position straight from the entity
	//printf("HeadPosition: % p\n", entity);
	return  RPM<Vector3>(entity + Offset::PelvisPosition);
}

Vector3 Memoryyy::GetEntityLowerNeckPosition(DWORD_PTR entity) {
	//We get the head position straight from the entity
	//printf("HeadPosition: % p\n", entity);
	return  RPM<Vector3>(entity + Offset::LowerNeckPosition);
}

std::string Memoryyy::GetEntityPlayerName(DWORD_PTR entity) {
	DWORD_PTR playerInfo = RPM<DWORD_PTR>(entity + Offset::PlayerInfo);
	DWORD_PTR playerInfoD = RPM<DWORD_PTR>(playerInfo + Offset::PlayerInfoDeref);


	return RPMString(RPM<DWORD_PTR>(playerInfoD + Offset::PlayerName) + 0x0);
}


BYTE Memoryyy::GetEntityTeamId(DWORD_PTR entity) {
	//Team id comes from player info
	DWORD_PTR playerInfo = RPM<DWORD_PTR>(entity + Offset::PlayerInfo);
	//We have to derefrnce it as 0x0
	DWORD_PTR playerInfoD = RPM<DWORD_PTR>(playerInfo + Offset::PlayerInfoDeref);
	//printf("Entity Info: %p\n", EntityInfo);
	return RPM<DWORD_PTR>(playerInfoD + Offset::PlayerTeamId);
}

PlayerInfo Memoryyy::GetAllEntityInfo(DWORD_PTR entity) {
	PlayerInfo p;

	p.Health = GetEntityHealth(entity);
	p.Name = GetEntityPlayerName(entity);
	p.Position = GetEntityFeetPosition(entity);
	p.w2s = WorldToScreen(p.Position);
	p.w2sHead = WorldToScreen(GetEntityHeadPosition(entity));
	p.w2sTorso = WorldToScreen(GetEntityTorsoPosition(entity));
	p.w2sNeck = WorldToScreen(GetEntityNeckPosition(entity));
	p.w2sLHand = WorldToScreen(GetEntityLHandPosition(entity));
	p.w2sRHand = WorldToScreen(GetEntityRHandPosition(entity));
	p.w2sPelvis = WorldToScreen(GetEntityPelvisPosition(entity));
	p.w2sUpperTorso = WorldToScreen(GetEntityUpperTorsoPosition(entity));
	p.w2sLowerNeck = WorldToScreen(GetEntityLowerNeckPosition(entity));
	p.TeamId = GetEntityTeamId(entity);

	return p;
}

void Memoryyy::UnlimitedAmmo() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4792150);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x0);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x28);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x0);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0xE8);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x110);

	
	WPM<DWORD_PTR>(weaponInfo + 0x160, 151);
}

void Memoryyy::ZeroSpread() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4794E70);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0xA0);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x0);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x0);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0xD0);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x358);

	float spread = RPM<float>(weaponInfo + 0x2A0);
	WPM<float>(weaponInfo + 0x2A0, 0);
}

void Memoryyy::ZeroRecoil() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4794E70);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0xA0);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x0);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x0);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0xD0);
	//printf("Weapon: %p\n", weapon);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x358);

	float recoil = RPM<float>(weaponInfo + 0x2D8);

	if (recoil != 0) {
		Global::Recoil = FALSE;
	}

	if (recoil == 0) {
		Global::Recoil = TRUE;
	}

	if (GetAsyncKeyState(VK_F2) & 1 || recoil != 0) {
		if (recoil != 0) {
			WPM<float>(weaponInfo + 0x2D8, 0.0f);
			WPM<float>(weaponInfo + 0x330, 0.0f);
			WPM<float>(weaponInfo + 0x334, 0.0f);
			Global::Recoil = TRUE;
		}

	}
}

void Memoryyy::LegitRecoil() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4794E70);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0xA0);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x0);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x0);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0xD0);
	//printf("Weapon: %p\n", weapon);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x358);

	float recoil = RPM<float>(weaponInfo + 0x2D8);
	

	if (recoil != 0) {
		Global::Recoil = FALSE;
	}

	if (recoil == 0) {
		Global::Recoil = TRUE;
	}

		if (recoil != 0) {
			WPM<float>(weaponInfo + 0x2D8, 0.02f);
			WPM<float>(weaponInfo + 0x330, 0.02f);
			WPM<float>(weaponInfo + 0x334, 0.02f);
			Global::Recoil = TRUE;
		}
}

Vector3 Memoryyy::GetViewTranslation() {
	//View translation comes straight from the camera
	return RPM<Vector3>(pCamera + Offset::ViewTranslastion);
}

Vector3 Memoryyy::GetViewRight() {
	//View right comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewRight);
}

Vector3 Memoryyy::GetViewUp() {
	//View up comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewUp);
}

Vector3 Memoryyy::GetViewForward() {
	//View forward comes directly from the camera
	return RPM<Vector3>(pCamera + Offset::ViewForward);
}


float Memoryyy::GetFOVX() {
	//FOV comes directly from the camera
	return RPM<float>(pCamera + Offset::FOVX);
}

float Memoryyy::GetFOVY() {
	//FOV comes directly from the camera
	return RPM<float>(pCamera + Offset::FOVY);
}

Vector3 Memoryyy::WorldToScreen(Vector3 position) {
	Vector3 temp = position - GetViewTranslation();

	float x = temp.Dot(GetViewRight());
	float y = temp.Dot(GetViewUp());
	float z = temp.Dot(GetViewForward() * -1);

	return Vector3((displayWidth / 2) * (1 + x / GetFOVX() / z), (displayHeight / 2) * (1 - y / GetFOVY() / z), z);
}

float Memoryyy::VaultTp(float height) {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x45F7B10);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x8);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x90);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x540);
	//DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0x310);
	//DWORD_PTR steppin = RPM<DWORD_PTR>(weapon + 0x140);
	//DWORD_PTR kevin = RPM<DWORD_PTR>(steppin + 0x4B0);

	WPM<float>(weaponProc + 0x78, height);
	return weaponProc;
}



DWORD_PTR Memoryyy::TB() {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(GetBaseAddress() + 0x47C3498);
	DWORD_PTR entityBalls = RPM<DWORD_PTR>(entityBase + 0xC00);
	DWORD_PTR entityNuts = RPM<DWORD_PTR>(entityBalls + 0x398);
	return RPM<int>(entityNuts + 0x1C8);
}

DWORD_PTR Memoryyy::ThirdPerson() {
	uint64_t ThirdPerson = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t ThirdPerson1 = RPM<uint64_t>(ThirdPerson + 0x38);
	uint64_t ThirdPerson2 = RPM<uint64_t>(ThirdPerson1 + 0x18);
	uint64_t ThirdPerson3 = RPM<uint64_t>(ThirdPerson2 + 0x38);
	uint64_t ThirdPerson4 = RPM<uint64_t>(ThirdPerson3 + 0x80);
	uint64_t ThirdPerson5 = RPM<uint64_t>(ThirdPerson4 + 0xD60);
	uint64_t ThirdPerson6 = RPM<uint64_t>(ThirdPerson5 + 0x28);
	uint64_t ThirdP = RPM<uint64_t>(ThirdPerson6 + 0x420);

	uint64_t ADSLEANL = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t ADSLEANL1 = RPM<uint64_t>(ADSLEANL + 0x38);
	uint64_t ADSLEANL2 = RPM<uint64_t>(ADSLEANL1 + 0x18);
	uint64_t ADSLEANL3 = RPM<uint64_t>(ADSLEANL2 + 0xE0);
	uint64_t ADSLEANL4 = RPM<uint64_t>(ADSLEANL3 + 0x290);
	uint64_t ADSLEANL5 = RPM<uint64_t>(ADSLEANL4 + 0xC8);

	uint64_t MAINCHAIN = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t MAINCHAIN1 = RPM<uint64_t>(MAINCHAIN + 0x38);
	uint64_t MAINCHAIN2 = RPM<uint64_t>(MAINCHAIN1 + 0x18);
	uint64_t MAINCHAIN3 = RPM<uint64_t>(MAINCHAIN2 + 0xE0);
	uint64_t MAINCHAIN4 = RPM<uint64_t>(MAINCHAIN3 + 0x290);
	uint64_t MAINCHAIN5 = RPM<uint64_t>(MAINCHAIN4 + 0xC8);

	uint64_t Run = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t Run1 = RPM<uint64_t>(Run + 0x38);
	uint64_t Run2 = RPM<uint64_t>(Run1 + 0x18);
	uint64_t Run3 = RPM<uint64_t>(Run2 + 0x0);
	uint64_t Run4 = RPM<uint64_t>(Run3 + 0x28);
	uint64_t Run5 = RPM<uint64_t>(Run4 + 0xE0);

	uint64_t Idle = RPM<uint64_t>(GetBaseAddress() + 0x4797948);
	uint64_t Idle1 = RPM<uint64_t>(Idle + 0x10);
	uint64_t Idle2 = RPM<uint64_t>(Idle1 + 0x138);
	uint64_t Idle3 = RPM<uint64_t>(Idle2 + 0x0);
	uint64_t Idle4 = RPM<uint64_t>(Idle3 + 0x28);
	uint64_t Idle5 = RPM<uint64_t>(Idle4 + 0xE0);
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			ThirdPerson::TPEnable = !ThirdPerson::TPEnable;
			
		
		}
		if (ThirdPerson::TPEnable)
		{
			WPM<uint64_t>(Idle5 + 0x30, ThirdP);
			WPM<uint64_t>(ADSLEANL5 + 0x138, ThirdP);
			WPM<uint64_t>(MAINCHAIN5 + 0x150, ThirdP);//idk why no worky...
			WPM<uint64_t>(MAINCHAIN5 + 0x198, ThirdP);
			WPM<uint64_t>(MAINCHAIN5 + 0x2B8, ThirdP);
			WPM<uint64_t>(MAINCHAIN5 + 0x2A0, ThirdP);
			WPM<uint64_t>(MAINCHAIN5 + 0x288, ThirdP);
			//printf("Enabled!");
		}
		if (!ThirdPerson::TPEnable)
	{
	WPM<uint64_t>(Idle5 + 0x30, ThirdPerson::Idle6);
	WPM<uint64_t>(ADSLEANL5 + 0x138, ThirdPerson::ADSLEANL6);
	WPM<uint64_t>(MAINCHAIN5 + 0x150, ThirdPerson::MAINCHAIN6);//idk why no worky...
	WPM<uint64_t>(MAINCHAIN5 + 0x198, ThirdPerson::MAINCHAIN7);
	WPM<uint64_t>(MAINCHAIN5 + 0x2B8, ThirdPerson::MAINCHAIN8);
	WPM<uint64_t>(MAINCHAIN5 + 0x2A0, ThirdPerson::MAINCHAIN9);
	WPM<uint64_t>(MAINCHAIN5 + 0x288, ThirdPerson::MAINCHAIN10);
	//printf("Disabled!");
	}
	

	return ThirdP;
}

DWORD_PTR Memoryyy::FirstPersonCheck() {

	uint64_t ADSLEANL = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t ADSLEANL1 = RPM<uint64_t>(ADSLEANL + 0x38);
	uint64_t ADSLEANL2 = RPM<uint64_t>(ADSLEANL1 + 0x18);
	uint64_t ADSLEANL3 = RPM<uint64_t>(ADSLEANL2 + 0xE0);
	uint64_t ADSLEANL4 = RPM<uint64_t>(ADSLEANL3 + 0x290);
	uint64_t ADSLEANL5 = RPM<uint64_t>(ADSLEANL4 + 0xC8);
	uint64_t ADSLEANL6 = RPM<uint64_t>(ADSLEANL5 + 0x138);

	uint64_t MAINCHAIN = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t MAINCHAIN1 = RPM<uint64_t>(MAINCHAIN + 0x38);
	uint64_t MAINCHAIN2 = RPM<uint64_t>(MAINCHAIN1 + 0x18);
	uint64_t MAINCHAIN3 = RPM<uint64_t>(MAINCHAIN2 + 0xE0);
	uint64_t MAINCHAIN4 = RPM<uint64_t>(MAINCHAIN3 + 0x290);
	uint64_t MAINCHAIN5 = RPM<uint64_t>(MAINCHAIN4 + 0xC8);
	uint64_t MAINCHAIN6 = RPM<uint64_t>(MAINCHAIN5 + 0x150);
	uint64_t MAINCHAIN7 = RPM<uint64_t>(MAINCHAIN5 + 0x198);
	uint64_t MAINCHAIN8 = RPM<uint64_t>(MAINCHAIN5 + 0x2B8);
	uint64_t MAINCHAIN9 = RPM<uint64_t>(MAINCHAIN5 + 0x2A0);
	uint64_t MAINCHAIN10 = RPM<uint64_t>(MAINCHAIN5 + 0x288);
	
	uint64_t Run = RPM<uint64_t>(GetBaseAddress() + 0x4823768);
	uint64_t Run1 = RPM<uint64_t>(Run + 0x38);
	uint64_t Run2 = RPM<uint64_t>(Run1 + 0x18);
	uint64_t Run3 = RPM<uint64_t>(Run2 + 0x0);
	uint64_t Run4 = RPM<uint64_t>(Run3 + 0x28);
	uint64_t Run5 = RPM<uint64_t>(Run4 + 0xE0);

	uint64_t Idle = RPM<uint64_t>(GetBaseAddress() + 0x4797948);
	uint64_t Idle1 = RPM<uint64_t>(Idle + 0x10);
	uint64_t Idle2 = RPM<uint64_t>(Idle1 + 0x138);
	uint64_t Idle3 = RPM<uint64_t>(Idle2 + 0x0);
	uint64_t Idle4 = RPM<uint64_t>(Idle3 + 0x28);
	uint64_t Idle5 = RPM<uint64_t>(Idle4 + 0xE0);
	uint64_t Idle6 = RPM<uint64_t>(Idle5 + 0x30);

	ThirdPerson::ADSLEANL6 =  ADSLEANL6;
	ThirdPerson::MAINCHAIN6 = MAINCHAIN6;
	ThirdPerson::MAINCHAIN7 = MAINCHAIN7;
	ThirdPerson::MAINCHAIN8 = MAINCHAIN8;
	ThirdPerson::MAINCHAIN9 = MAINCHAIN9;
	ThirdPerson::MAINCHAIN10 = MAINCHAIN10;
	ThirdPerson::Idle6 = Idle6;

	//printf("Animation: % p\n", ADSLEANL6);
	//printf("ADSLEANL6: % p\n", MAINCHAIN6);
	//printf("ADSLEANL7: % p\n", MAINCHAIN7);
	//printf("ADSLEANL8: % p\n", MAINCHAIN8);
	//printf("ADSLEANL9: % p\n", MAINCHAIN9);
	//printf("ADSLEANL10: % p\n", MAINCHAIN10);
	//printf("Idle6: % p\n", Idle6);

	return 0;
}




/*DWORD_PTR Memoryyy::ForceShoot() {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(GetBaseAddress() + 0x3587420);
	DWORD_PTR entityBalls = RPM<DWORD_PTR>(entityBase + 0x160);
	DWORD_PTR entityNuts = RPM<DWORD_PTR>(entityBalls + 0x30);
	WPM<DWORD_PTR>(entityNuts + 0xB48, 0);
	return entityNuts;
}*/

inline Vector3 Croccydile()
{
	static uint32_t cnt = 0;
	float freq = 0.0005f;

	if (cnt++ >= (uint32_t)-1)
		cnt = 0;

	Vector3 ret = { std::sin(freq * cnt + 0) * 0.5f + 0.5f, std::sin(freq * cnt + 2) * 0.5f + 0.5f , std::sin(freq * cnt + 4) * 0.5f + 0.5f };
	return ret;
}

DWORD_PTR Memoryyy::GreenGlow() {

	static float x = 0, y = 0;
	static float r = 0, g = 0, b = 0;
	float valueC0 = 255.0f;
	float valueC4 = 0.0f;
	float valueC8 = 150.0f;

	if (y >= 0.0f && y < 255.0f) {
		r = 255.0f;
		g = 0.0f;
		b = x;
	}
	else if (y >= 255.0f && y < 510.0f) {
		r = 255.0f - x;
		g = 0.0f;
		b = 255.0f;
	}
	else if (y >= 510.0f && y < 765.0f) {
		r = 0.0f;
		g = x;
		b = 255.0f;
	}
	else if (y >= 765.0f && y < 1020.0f) {
		r = 0.0f;
		g = 255.0f;
		b = 255.0f - x;
	}
	else if (y >= 1020.0f && y < 1275.0f) {
		r = x;
		g = 255.0f;
		b = 0.0f;
	}
	else if (y >= 1275.0f && y < 1530.0f) {
		r = 255.0f;
		g = 255.0f - x;
		b = 0.0f;
	}

	x += 0.45f; //increase this value to switch colors faster
	if (x >= 255.0f)
		x = 0.0f;

	y += 0.45f; //increase this value to switch colors faster
	if (y > 1530.0f)
		y = 0.0f;
	DWORD_PTR glowmanagrrrrrr = RPM<DWORD_PTR>(GetBaseAddress() + 0x4792038);
	DWORD_PTR glowbayseeeee = RPM<DWORD_PTR>(glowmanagrrrrrr + 0x90);

	uint64_t roundmanagrrrrrr = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t state = RPM<uint64_t>(roundmanagrrrrrr + 0x2F0);

	// Adjust values based on WASD key presses
	if (GetAsyncKeyState(0x87)) {
		valueC0 += 10.0f;
	}
	if (GetAsyncKeyState(0x65)) {
		valueC0 -= 10.0f;
	}
	if (GetAsyncKeyState(0x83)) {
		valueC4 += 10.0f;
	}
	if (GetAsyncKeyState(0x68)) {
		valueC4 -= 10.0f;
	}
	if (GetAsyncKeyState(0x81)) {
		valueC8 += 10.0f;
	}
	if (GetAsyncKeyState(0x69)) {
		valueC8 -= 10.0f;
	}

	if (state == 3)
	{
		WPM<float>(glowbayseeeee + 0x00C0, 255);
		WPM<float>(glowbayseeeee + 0x00C4, 0);
		WPM<float>(glowbayseeeee + 0x00C8, 150);
		WPM<float>(glowbayseeeee + 0x00E0, 0);
		WPM<float>(glowbayseeeee + 0x00DC, 255);
		WPM<float>(glowbayseeeee + 0x00E8, 0);
	}

	return glowbayseeeee;
}

DWORD_PTR Memoryyy::WeaponFOV() {
	DWORD_PTR entityBase = RPM<DWORD_PTR>(GetBaseAddress() + 0x3900E28);
	DWORD_PTR entityBalls = RPM<DWORD_PTR>(entityBase + 0x0);
	DWORD_PTR entitynuts = RPM<DWORD_PTR>(entityBalls + 0x108);
	DWORD_PTR ipv4 = RPM<DWORD_PTR>(entitynuts + 0x50);
	DWORD_PTR ipv6 = RPM<DWORD_PTR>(ipv4 + 0x248);
	DWORD_PTR ipv8 = RPM<DWORD_PTR>(ipv6 + 0x30);
	WPM<float>(ipv8 + 0x3C, 1.5f);

	return ipv8;
}

DWORD_PTR Memoryyy::FireMode() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4794E70);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0xA0);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x0);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x0);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0xD0);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x358);

	float spread = RPM<float>(weaponInfo + 0x138);
	WPM<float>(weaponInfo + 0x138, 0);
	return spread;
} 

DWORD_PTR Memoryyy::LongKnife() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x4797A40);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x20);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0xB8);
	DWORD_PTR weaponProc = RPM<DWORD_PTR>(weaponComp + 0x8);
	DWORD_PTR weapon = RPM<DWORD_PTR>(weaponProc + 0x60);
	DWORD_PTR weaponInfo = RPM<DWORD_PTR>(weapon + 0x30);

	float spread = RPM<float>(weaponInfo + 0x54);
	WPM<float>(weaponInfo + 0x54, 750);
	WPM<float>(weaponInfo + 0x50, 750);
	return spread;
}

DWORD Memoryyy::SpottedESP(DWORD_PTR entity) {
	//Entity info pointer from the Entity
	DWORD_PTR EntityInfo = RPM<DWORD_PTR>(entity + Offset::EntityInfo);
	//Main component pointer from the entity info
	DWORD_PTR MainComponent = RPM<DWORD_PTR>(EntityInfo + Offset::MainComponent);
	//Child component pointer form the main component
	DWORD_PTR ESPCHAIN = RPM<DWORD_PTR>(MainComponent + Offset::ESPCHAIN);
	DWORD_PTR ESPCHAIN2 = RPM<DWORD_PTR>(ESPCHAIN + Offset::ESPCHAIN2);

	
	WPM<DWORD>(ESPCHAIN2 + Offset::ESPCHAIN3, 1);
	return ESPCHAIN2;
}

DWORD_PTR Memoryyy::Fly() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x480C518);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x10);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x148);
	DWORD_PTR gravity = RPM<DWORD_PTR>(weaponComp + 0x778);
	DWORD_PTR Entity = Global::Memory.GetLocalEntity();

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr12 = RPM<uint64_t>(addr2 + 0x2F0);
	

	//Get a PlayerInfo struct with all of this entity's info
	PlayerInfo Player = Global::Memory.GetAllEntityInfo(Entity);
		if (GetAsyncKeyState(VK_SPACE) && ptr12 == 3) {
			WPM<float>(weaponComp + 0x778, -0.6000000238);
		}
		else
		{
			WPM<float>(weaponComp + 0x778, 0.6000000238);
		}



	float fly = RPM<float>(weaponComp + 0x778);
	return fly;
}

DWORD_PTR Memoryyy::UnFly() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x480C518);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x10);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x148);

		WPM<float>(weaponComp + 0x778, 0.6000000238);

	float fly = RPM<float>(weaponComp + 0x778);
	return fly;
}

DWORD_PTR Memoryyy::Speed() {
	DWORD_PTR entityInfo = RPM<DWORD_PTR>(GetBaseAddress() + 0x480C518);
	DWORD_PTR mainComp = RPM<DWORD_PTR>(entityInfo + 0x10);
	DWORD_PTR weaponComp = RPM<DWORD_PTR>(mainComp + 0x148);


	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr12 = RPM<uint64_t>(addr2 + 0x2F0);

	if (ptr12 == 3)
	{
		WPM<float>(weaponComp + 0x774, 0.7000000238);
		WPM<float>(weaponComp + 0x770, 0.7000000238);
	}


	return weaponComp;
}

DWORD_PTR Memoryyy::NoWeaponSound() {
	DWORD_PTR addr = RPM<DWORD_PTR>(GetBaseAddress() + 0x04792150);
	DWORD_PTR ptr1 = RPM<DWORD_PTR>(addr + 0x0);
	DWORD_PTR ptr2 = RPM<DWORD_PTR>(ptr1 + 0x28);
	DWORD_PTR ptr3 = RPM<DWORD_PTR>(ptr2 + 0x0);
	DWORD_PTR ptr4 = RPM<DWORD_PTR>(ptr3 + 0xe8);
	DWORD_PTR ptr5 = RPM<DWORD_PTR>(ptr4 + 0x110);
	DWORD_PTR ptr6 = RPM<DWORD_PTR>(ptr5 + 0x6E3);

	WPM<BYTE>(ptr5 + 0x6E3, 1);

	return ptr5;
}

DWORD_PTR Memoryyy::RapidFire() {
	DWORD_PTR addr = RPM<DWORD_PTR>(GetBaseAddress() + 0x4792150);
	DWORD_PTR ptr1 = RPM<DWORD_PTR>(addr + 0x0);
	DWORD_PTR ptr2 = RPM<DWORD_PTR>(ptr1 + 0x28);
	DWORD_PTR ptr3 = RPM<DWORD_PTR>(ptr2 + 0x0);
	DWORD_PTR ptr4 = RPM<DWORD_PTR>(ptr3 + 0xe8);
	DWORD_PTR ptr5 = RPM<DWORD_PTR>(ptr4 + 0x110);
	DWORD_PTR ptr6 = RPM<DWORD_PTR>(ptr5 + 0x160);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr12 = RPM<uint64_t>(addr2 + 0x2F0);

	if (ptr12 == 3)
	{
		WPM<float>(ptr5 + 0x658, -10.f);
	}
	

	return ptr5;
}

DWORD_PTR Memoryyy::ForceFire() {
	DWORD_PTR addr = RPM<DWORD_PTR>(GetBaseAddress() + 0x4792150);
	DWORD_PTR ptr1 = RPM<DWORD_PTR>(addr + 0x0);
	DWORD_PTR ptr2 = RPM<DWORD_PTR>(ptr1 + 0x28);
	DWORD_PTR ptr3 = RPM<DWORD_PTR>(ptr2 + 0x0);
	DWORD_PTR ptr4 = RPM<DWORD_PTR>(ptr3 + 0xe8);
	DWORD_PTR ptr5 = RPM<DWORD_PTR>(ptr4 + 0x110);
	DWORD_PTR ptr6 = RPM<DWORD_PTR>(ptr5 + 0x160);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr12 = RPM<uint64_t>(addr2 + 0x2F0);
	
	if (TB() == 1)
	{
		WPM<BYTE>(ptr5 + 0x94, 3);
	}
	else if (GetAsyncKeyState(VK_MBUTTON)) {
		WPM<BYTE>(ptr5 + 0x94, 3);
	}
	else {
		WPM<BYTE>(ptr5 + 0x94, 1);
	}
	

	return ptr5;
}

DWORD_PTR Memoryyy::ForceFireNoTrig() {
	DWORD_PTR addr = RPM<DWORD_PTR>(GetBaseAddress() + 0x4792150);
	DWORD_PTR ptr1 = RPM<DWORD_PTR>(addr + 0x0);
	DWORD_PTR ptr2 = RPM<DWORD_PTR>(ptr1 + 0x28);
	DWORD_PTR ptr3 = RPM<DWORD_PTR>(ptr2 + 0x0);
	DWORD_PTR ptr4 = RPM<DWORD_PTR>(ptr3 + 0xe8);
	DWORD_PTR ptr5 = RPM<DWORD_PTR>(ptr4 + 0x110);
	DWORD_PTR ptr6 = RPM<DWORD_PTR>(ptr5 + 0x160);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr12 = RPM<uint64_t>(addr2 + 0x2F0);

	

	if (GetAsyncKeyState(VK_MBUTTON)) {
		WPM<BYTE>(ptr5 + 0x94, 3);
	}

	return ptr5;
}


DWORD_PTR Memoryyy::GetThermBullets() {
	uint64_t addr = RPM<uint64_t>(GetBaseAddress() + 0x3564240);
	uint64_t ptr1 = RPM<uint64_t>(addr + 0x1D8);
	uint64_t ptr2 = RPM<uint64_t>(ptr1 + 0x0);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x4797B08);
	uint64_t ptr3 = RPM<uint64_t>(addr2 + 0x98);
	uint64_t ptr4 = RPM<uint64_t>(ptr1 + 0x3F0);

	WPM<uint64_t>(ptr3 + 0x3F0, ptr2);

	return ptr4;
}

DWORD_PTR Memoryyy::HoloToAcog() {
	uint64_t addr = RPM<uint64_t>(GetBaseAddress() + 0x45F92D0);
	uint64_t ptr1 = RPM<uint64_t>(addr + 0xC0);
	uint64_t ptr2 = RPM<uint64_t>(ptr1 + 0x18);
	uint64_t ptr7 = RPM<uint64_t>(ptr2 + 0x378);
	uint64_t ptr8 = RPM<uint64_t>(ptr7 + 0x690);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x45F92D0);
	uint64_t ptr3 = RPM<uint64_t>(addr2 + 0xC0);
	uint64_t ptr4 = RPM<uint64_t>(ptr3 + 0x18);
	uint64_t ptr5 = RPM<uint64_t>(ptr4 + 0x378);

	WPM<uint64_t>(ptr5 + 0x6C0, ptr8);

	return ptr4;
}

DWORD_PTR Memoryyy::HoloToAcogS() {
	uint64_t addr = RPM<uint64_t>(GetBaseAddress() + 0x45F92D0);
	uint64_t ptr1 = RPM<uint64_t>(addr + 0xC0);
	uint64_t ptr2 = RPM<uint64_t>(ptr1 + 0x18);
	uint64_t ptr7 = RPM<uint64_t>(ptr2 + 0x378);
	uint64_t ptr8 = RPM<uint64_t>(ptr7 + 0x858);

	uint64_t addr2 = RPM<uint64_t>(GetBaseAddress() + 0x45F92D0);
	uint64_t ptr3 = RPM<uint64_t>(addr2 + 0xB8);
	uint64_t ptr4 = RPM<uint64_t>(ptr3 + 0x68);
	uint64_t ptr5 = RPM<uint64_t>(ptr4 + 0x0);
	uint64_t ptr6 = RPM<uint64_t>(ptr5 + 0x18);

	WPM<uint64_t>(ptr6 + 0x7B0, ptr8);

	return ptr4;
}

DWORD_PTR Memoryyy::RoundState() {
	uint64_t addr = RPM<uint64_t>(GetBaseAddress() + 0x4797D70);
	uint64_t ptr1 = RPM<uint64_t>(addr + 0x2F0);

	if (ptr1 == 3)
		
	return ptr1;
}

DWORD_PTR Memoryyy::SwapTeams()
{
	uintptr_t profile_manager = RPM<uintptr_t>(GetBaseAddress() + 0x45F7B60);
	

	uintptr_t chain = RPM<uintptr_t>(profile_manager + 0); 
	chain = RPM<uintptr_t>(chain + 0x38); 
	chain = RPM<uintptr_t>(chain + 0x70); 
	chain = RPM<uintptr_t>(chain + 0x258); 
	chain = RPM<uintptr_t>(chain + 0x220); 
	BYTE original_team = RPM<BYTE>(chain + 0x7A8);

	if (original_team == 3)
	{
		WPM<BYTE>(chain + 0x30, 4);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		WPM<BYTE>(chain + 0x30, 3);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	else if (original_team == 4)
	{
		WPM<BYTE>(chain + 0x30, 3);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		WPM<BYTE>(chain + 0x30, 4);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	return original_team;
}

DWORD_PTR Memoryyy::RunAndShoot()
{
	uintptr_t runandshootmanager = RPM<uint64_t>(GetBaseAddress() + 0x19C28AC);

	if (GetAsyncKeyState(VK_SHIFT)) {
		WPM<uint8_t>(runandshootmanager, 0x1);
	}
	return runandshootmanager;
}

void Memoryyy::Shoot()
{
	//WPM(GetBaseAddress() + 0x10517EC + 0x3, 0x134 : 0xc0);
}