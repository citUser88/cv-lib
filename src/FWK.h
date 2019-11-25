#pragma once

#include <atomic>
#include <memory>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <cassert>
#include <type_traits>

typedef unsigned int uint;

#include "helpers.h"
#include "logging.h"
Helper* oHelp;

// Keeping track, having information about all hardware, mappings, 
// etc, at all times updated asynchronously or synchronously
//
// FWK_base_class
// Abstract base class that VoidPrecursor derives from
class FWK_base_class {};
// Not doing this to be silly, future considerations
class VoidPrecursor : public FWK_base_class
{
};
class VoidMain : public VoidPrecursor
{	
public:			
	VoidMain() 
	{	// HEY NICE RED todo: unicode support, string Resources for translators(human, ultimately)
		// write << ANSI formatting overload functions i.e. std::cout << ANSIFX::fg(color) << ANSIFX::fg(reset) etc... 
		std::cout << "\u001b[31m\t\t\t\tCV-lib (Cross platform multi-paradigm graphics library\u001b[0m" << std::endl;
		std::cout << "\u001b[32m\t\t\t\t\tpre-release v0.1.01ac\u001b[0m" << std::endl;
		logString_to_devLog("ctor invoked: VoidMain", true);		
	};
	~VoidMain() 
	{
		logString_to_devLog("dtor invoked: VoidMain", true);
	};
	void exec() 
	{
		//		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << "VoidMain DOING THIS";
		logString_to_devLog("VoidMain thread EXECUTED this...", true);
	};

};