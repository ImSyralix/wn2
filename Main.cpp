#include "stdafx.h"
#include "Globals.h"
#include "Utility.h"
#include "Memory.h"
#include "Overlay.h"
#include <iostream>
#include <time.h>
#include <Psapi.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"
#include "d3d9.h"


char type;

char password;

HWND gameWindow = FindWindow(NULL, L"Rainbow Six");

//Defining our globals
namespace Global {
	HANDLE GameHandle = 0x0;

	LPVOID BaseAddress = NULL;

	std::string LocalName = "Byralix";

	Memoryyy Memory = Memoryyy();

	Overlayyy Over = Overlayyy();

	BOOL Aimbot = TRUE;
	
	BOOL Spread = TRUE;

	BOOL Recoil = TRUE;

	BOOL Legit = FALSE;

	BOOL Rage = TRUE;

	BOOL ThirdPerson = TRUE;\
	
	BOOL ForceFire = TRUE;
}



DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}

DWORD processID = FindProcessId(L"RainbowSix.exe");

// Open the game process with necessary access rights
HANDLE gameProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

void OpenHandle() {
	DWORD processID = FindProcessId(L"RainbowSix.exe");
	Global::GameHandle = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processID);
}

static const char consoleNameAlphanum[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int consoleNameLength = sizeof(consoleNameAlphanum) - 1;


char genRandomConsoleName()
{
	return consoleNameAlphanum[rand() % consoleNameLength];
}

int main()
{
	srand(time(0));
	std::wstring ConsoleNameStr;
	std::wstring Passwordt;
	for (unsigned int i = 0; i < 20; ++i)
	{
		ConsoleNameStr += genRandomConsoleName();

	}
	SetConsoleTitle(L"TaskWin");
	//SetConsoleTitle(ConsoleNameStr.c_str());
	//std::cout << "If you ran this you like syralix's cum!" << std::endl;
	if (gameWindow != 0) {
		//std::cout << "Game window was found" << std::endl;

		OpenHandle();

		//std::cout << "Local name: " << std::flush;
		//std::cin >> Global::LocalName;

		Sleep(1000);

		if (Global::GameHandle == INVALID_HANDLE_VALUE || Global::GameHandle == NULL || Global::GameHandle == (HANDLE)0x0) {
			//std::cout << "Invalid handle to R6" << std::endl;
			system("pause");
			return 1;
		}
		
		for (unsigned int i = 0; i < 5; ++i)
		{
			Passwordt += genRandomConsoleName();
		}

		Global::Memory.SetBaseAddress();
		//std::cout << "\nFinding Game (1/2)" << std::endl;

		Global::Over.SetupWindow();
		//std::cout << "\nSetting up Hack (2/2)" << std::endl;
		//std::cout << "\nSucessfully Loaded" << std::endl;

		Global::Memory.UpdateAddresses();
		

		//Global::Memory.FirstPersonCheck();
		//Global::Memory.GetThermBullets();

		::ShowWindow(GetConsoleWindow(), SW_HIDE);

		//Start the main loop
		Global::Over.Loop();
		//Start the aim loop


		system("pause");
		return EXIT_SUCCESS;

		return 0;
	}
}