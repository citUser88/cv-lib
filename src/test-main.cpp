//creation timestamp 2-7-19 1:34a
//
//@R 
//ORIGAMI engine PRE-PAC VULKAN learning, testing/development purposes

#include <ctime>
#include <memory>

#include "FWK.h"
#include "helpers.h"
#include "visual.h"
#include "windows-interfacing.h"
#include "ext-hw-maps.h"
#include "console-scribe.h"

VoidMain*	VM;								
Visual*		viz;
Keyboard*	kbd;

int main(int argc, char *argv[]) 
{
	//setConsoleWindowPosition(1933,128);
	setConsoleWindowPosition(10,128);
	logOpenLoggingFile();
	logString_to_devLog("State: INITIALIZING...", true);
	enableConsoleVTP();
	
	oHelp		= new Helper();
	VM = new VoidMain();	
	oHelp->detect_contextualize_map_device_metrics();
	viz		= new Visual();
	kbd		= new Keyboard();//execptions in debugging	

	unsigned char* ks = nullptr;
	ks = &kbd->getkbdStateArray();
	kbd->exec(); // poll keyboard once
	logString_to_devLog("State: RUNNING!", true);
	
	MSG msg;
	bool keepLooping = TRUE;
	while (keepLooping)
	{
		if (ks[0x10]) {
			keepLooping = FALSE;
			PostQuitMessage(0);
		}
		
		kbd->exec();
		viz->exec();
	}
	
	logString_to_devLog("State: STOPPING!", true);
	delete(kbd);
	delete(viz);
	delete(VM);
	delete(oHelp);
	logString_to_devLog("State: EXITING!", true);
	logCloseLoggingFile();
	return 0;
}