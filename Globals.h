#pragma once

#include <Windows.h>
#include <string>
#include "Memory.h"
#include "Overlay.h"


//Global variables to be used throughout the project
namespace Global {
	//Handle to the game
	extern HANDLE GameHandle;

	//Base address of the game
	extern LPVOID BaseAddress;

	//Local player name, used for finding local entity by comparing names
	extern std::string LocalName;

	

	//Instance of memory class
	extern Memoryyy Memory;

	//Instance of Overlay class
	extern Overlayyy Over;

	extern BOOL Aimbot;

	extern BOOL Spread;

	extern BOOL Recoil;

	extern BOOL ThirdPerson;

	extern BOOL ForceFire;

	extern BOOL Legit;

	extern BOOL Rage;
}

