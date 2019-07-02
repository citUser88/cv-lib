#pragma once
// Console interfacing utility header
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING // define to allow for color processing of console output
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0X0004
#endif

// todo: animated console cursor moving/ progress(/-\|/-), nice and ANSI RED		
HWND HWndConsole;
HANDLE hStdOut = nullptr;
CONSOLE_SCREEN_BUFFER_INFO csbi; 

// Assumes hStdOut has been acquired from console with correct ACCESS rights
void setConsoleWindowPosition(short nX, short nY)
{
	HWND conWnd;
	conWnd = GetConsoleWindow(); // create/adjust winRect with passed width and height    
	
	RECT R = { nX, nY, 892, 639 };	
	AdjustWindowRect(&R, WS_OVERLAPPED, false);
	if (!SetWindowPos(conWnd, HWND_TOP, R.left,R.top,R.right, R.bottom, SWP_SHOWWINDOW))
	{
		OutputDebugString("ERROR SETTING CONSOLE WINDOW POSITION");
	};
}

void enableConsoleVTP()
{
	if (!hStdOut) hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//	Don't forget to thank this 'dude'(posted solution online) for saving your ANSI colorings --->
	// enable colorizations/'virtual terminal processing' for console window output	
	DWORD handleMode;
	GetConsoleMode(hStdOut, &handleMode);
	handleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hStdOut, handleMode);
}

// carries last accessed or modified version of last accessed cursor position through windows console api
// set console cursor position to nX, Y-dY to reset to last position after return
void resetConsoleCursorPos(short int nX, short int dY)
{
	if (!hStdOut) hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); // check/get the STD_OUTPUT_HANDLE
	GetConsoleScreenBufferInfo(hStdOut, &csbi);
	COORD newCoords{ nX, csbi.dwCursorPosition.Y - dY };
	SetConsoleCursorPosition(hStdOut, newCoords);
}

struct MindedData {
	std::vector<MindedData> children;

	std::string tag;		 // data tag name displayed with data in console
	
	uint tagColor;	 // color to use for tag
	uint dataColor; // color to use for data

	uint x, y; // position of minded data in console screen
	uint updateinterval; // time interval between updates (in milliseconds)
	uint elapsed;	// time elapsed resets every update interval
	bool drawme; // update the information in the console per interval
	std::string databuf; // rendered information to update in console
};

class ConsoleMinder 
{
private:
	std::vector<MindedData> data;

	void renderData() {

	};
};
#elif // else (not win32)
#endif //win32