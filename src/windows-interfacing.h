#pragma once

#include <windows.h>
#include "FWK.h"
#include "logging.h"

#ifdef _WIN32

// public until decide where to stuff these accessibly to what objects need it
// Values like these indicate possible variance attributes to factor into UHE
// Other machines/instances/platforms will surely use different numbering schemas
// This sort of hysterisis is what I'm interested in using when feeding the 
// Conduit Stream

long width = 0;
long height = 0;

HWND			hWnd;
HINSTANCE	hInst;
WNDCLASSEX	wClassEx;

extern "C" WINUSERAPI HWND WINAPI
	CreateWindowExA(
		_In_ DWORD dwExStyle,
		_In_opt_ LPCSTR lpClassName,
		_In_opt_ LPCSTR lpWindowName,
		_In_ DWORD dwStyle,
		_In_ int X,
		_In_ int Y,
		_In_ int nWidth,
		_In_ int nHeight,
		_In_opt_ HWND hWndParent,
		_In_opt_ HMENU hMenu,
		_In_opt_ HINSTANCE hInstance,
		_In_opt_ LPVOID lpParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		case WM_DESTROY:
			logString_to_devLog("JTLYK: WndProc(): WM_CLOSE has posted, window, go byebye...", true);
			PostQuitMessage(0);
			return 0;
		case WM_PAINT: 
			OutputDebugStringA("EVENT TRIGGERED: WM_PAINT");
			return 0;
		default: break;
	}
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

class Window
{
public:	
	Window()
	{
		width = 800;
		height = 600;
		assert(width > 0); assert(height > 0);
		hInst = GetModuleHandle(NULL);
		char* name = "FWKWndClass";
		// fill the windowClassEx structure
		wClassEx.cbSize = sizeof(WNDCLASSEX);
		wClassEx.style = CS_HREDRAW | CS_VREDRAW;
		wClassEx.lpfnWndProc = WndProc;
		wClassEx.cbClsExtra = 0;
		wClassEx.cbWndExtra = 0;
		wClassEx.hInstance = hInst;
		wClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
		wClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wClassEx.lpszMenuName = NULL;
		wClassEx.lpszClassName = name; //info_VULKAN.name;
		wClassEx.hIconSm = LoadIcon(NULL, IDI_WINLOGO);		
		// attempt to register the class
		if (!RegisterClassEx(&wClassEx))
		{
			logString_to_devLog("ERROR: RegisterClassEx() fail....", true);
			exit(-1);
		}
		// create/adjust winRect with passed width and height 
		RECT winRect = { 0, 0, width, height };
		AdjustWindowRect(&winRect, WS_OVERLAPPEDWINDOW, false);
		// call windows CreateWindowExA and attempt to create the window
		hWnd = CreateWindowExA(0,
			name,						//	LPCSTR    lpClassName,
			name,						//	LPCSTR    lpWindowName,			
			WS_POPUPWINDOW |
			WS_VISIBLE,				// DWORD     dwStyle,
			3000, 11,																		 
			winRect.right - winRect.left,		// width
			winRect.bottom - winRect.top,		// height,
			NULL,						// HWND      hWndParent,
			NULL,						// HMENU     hMenu,
			hInst,					// HINSTANCE hInstance,
			NULL);					// LPVOID    lpParam		
		if (!hWnd) // null = failed
		{
			logString_to_devLog("ERROR: CreateWindowEx() fail...", true);
			exit(-1); // maybe there is a way to recover and just go full console, since this framework isn't designed to care about windows/platform specifics ultimately
		}
		hW = hWnd; hI = hInst; wCx = wClassEx;
		width = winRect.right - winRect.left;
		height = winRect.bottom - winRect.top;
		// SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)info_VULKAN); // why this? dynamic data exchange/sharing? backwards/forwards comaptibility
		logString_to_devLog("ctor invoked: Window", true);
		logString_to_devLog("Window: Window Created Successfully", true);
	};
	~Window()
	{
		PostQuitMessage(0);
		DestroyWindow(hWnd);
		logString_to_devLog("dtor invoked: Window", true);
		logString_to_devLog("Window: Window Destroyed", true);
	};
private:
	HWND			hW;
	HINSTANCE	hI;
	WNDCLASSEX	wCx;

};
#endif

//
//// Return true if this window can present the given queue type
//bool Window_win32::CanPresent(VkPhysicalDevice gpu, uint32_t queue_family) {
//	return vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu, queue_family) == VK_TRUE;
//}
