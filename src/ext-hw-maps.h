#pragma once

#define DIRECTINPUT_VERSION 0x0800
#define USING_DINPUT8
#ifdef USING_DINPUT8
#include "dinput.h"
#endif

#include <memory>
#include <string>
#include <windows.h>
#include "console-scribe.h"
#include "FWK.h"

struct x86_HARDWARE_metrics
{
	std::string _CPUTYPE = "";
	std::string _GPUTYPE = "";
};
struct x86_GPU_metrics
{
	std::string _GPULIBRARY = "";						  // GPU Library, currently ['OpenGL' or 'Vulkan'(LunarG)GL Next]
	std::string _GPULIBRARY_VERSION = "";			  // Version of GPU Library detected in use 
																  // will be manually set for development purposes or by commandline options
	std::string _GPULIBRARY_INTERFACE_LIBRARY = "";// Library utilized to interface with GPU from source code (if any)
};
struct x86_OS_metrics 
{
	std::string _OS = "";
	std::string _OS_VERSION = "";
	std::string _OS_BUILD = "";
};

// Input mapper for the keyboard
// Read Keys from keyboard (platform dependent)
// 
#ifdef USING_DINPUT8
class Keyboard 
{
	IDirectInput8* DI8 = nullptr;
	IDirectInputDevice8* didvcKBD = nullptr;
	unsigned char kbdstate[256];
	unsigned char oldkbdstate[256];
public:
	volatile Keyboard()
	{
		logString_to_devLog("ctor invoked: Keyboard", true);		//interface ID_InterfaceDirectInput8
		HRESULT hRes = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DI8, NULL);
		if (FAILED(hRes)) {
			logString_to_devLog("Keyboard: ERROR RETRIEVING DINPUT8 interface pointer", true);
			exit(-1);}
		hRes = DI8->CreateDevice(GUID_SysKeyboard, &didvcKBD, NULL);
		if (FAILED(hRes)) {
			logString_to_devLog("Keyboard: ERROR RETRIEVING DINPUT8 keyboard device.", true);
			logString_to_devLog("IDirectInput8->CreateDevice(), failed...", true);
			exit(-1);}
		hRes = didvcKBD->SetDataFormat(&c_dfDIKeyboard);
		if (FAILED(hRes)) {
			logString_to_devLog("Keyboard: ERROR SETTING DINPUT8 data format.", true);
			exit(-1);}
		hRes = didvcKBD->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
		if (FAILED(hRes)) {
			logString_to_devLog("Keyboard: ERROR SETTING DINPUT8 coop level.", true);
			exit(-1);}
		hRes = didvcKBD->Acquire();
		if (FAILED(hRes)) {
			logString_to_devLog("Keyboard: ERROR ACQUIRING DINPUT8 device.", true);
			exit(-1);}
		logString_to_devLog("Keyboard: SUCCESS!!! Keyboard interface acquired from DINPUT8 interface...", true);
	}
	~Keyboard()
	{
		logString_to_devLog("dtor invoked: Keyboard", true);
		didvcKBD->Unacquire();
		logString_to_devLog("Keyboard: Unacquired keyboard resource", true);
		didvcKBD->Release();
		logString_to_devLog("Keyboard: Released keyboard resource", true);
		didvcKBD = nullptr;
	}
	void start() {//thandler(exec);
	}
	// poll / process keyboard information
	void exec() {
		HRESULT hRes;
		memcpy(oldkbdstate, kbdstate, sizeof(kbdstate));
		hRes = didvcKBD->GetDeviceState(sizeof(kbdstate), (LPVOID)&kbdstate);				
		for (int i = 0; i < 255; ++i) {
			if (kbdstate[i] != oldkbdstate[i]) {
				if (kbdstate[i]) {											
					std::cout << "Yoose Pressed Key: " << i << " value[" << std::to_string(kbdstate[i]) << "]   \n";
					resetConsoleCursorPos(0, 1);	// reset back one for next time displayed
													// data representation structures for console, NEXT
				}
			}
		}		
		if (FAILED(hRes)) { 
			if ((hRes == DIERR_INPUTLOST) || (hRes == DIERR_NOTACQUIRED))
				{didvcKBD->Acquire();} else 
				{logString_to_devLog("Keyboard: GetDeviceState() - ERROR, UNKNOWN/UNRESOLVED at this time!!",true);}
		}
	}
	unsigned char& getkbdStateArray() {
		return *kbdstate;
	}
};
#elif // not win32
#endif
