#pragma once
#include <iostream>
#include <assert.h>
#include <string>
#include <sstream>
#include <vector>

#include "logging.h"
#include "FWK.h"
#include "windows-interfacing.h"
#include "vk-interfacing.hpp"

class Visual
{
public:	
	Visual()
	{
		owin = new Window();
		o2vk = new O2VK();
		logString_to_devLog("ctor invoked: ogiVisual", true);
	}
	~Visual()
	{
		logString_to_devLog("dtor invoked: ogiVisual", true);
		delete(o2vk);
		delete(owin);
	}
	void exec() { o2vk->RenderFrame(); }
private:
	Window* owin;
	O2VK* o2vk;
};