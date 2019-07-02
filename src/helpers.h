#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <chrono>
#include <windows.h>
#include "logging.h"

#define GLM_FORCE_RADIANS
#include <glm-0.9.9.3\glm\glm\gtc\type_ptr.hpp>
#include <glm-0.9.9.3\glm\glm\gtc\matrix_transform.hpp>

														// All capturable logging output organized tersely/succinctly for easy log grepping
static std::fstream devMetrics;				// Device metrics external hardware 
														// machine lanaguage detection / polling algorithms. 
														// NOTE: need relational storage structures for integration with other DBS of metric data
static std::ifstream devMapCatalog;			// Development catalog file for mapping files
														// check ios and ostream classes for the any customizable features of the ofstream 
														// class, want to make this a template decorator access wrapper

// Cycle counter structure
struct cycleCounter_struct // information populated from <ctime> objects
{
	double cycleCounterID;		// Unique ID to link cycle counter structure to specific device to monitor counts on
	double startTimeCycleCount;// Processor RDSTC cycle performance counter
	double startDateTime;		// Start time/date of development branch epoch? date/time - OS supplied conversion to and from this value	

	double lastTimeCycleCount;
	double deltaTimeCycle;		// Cycles between last cycle count and most recently updated cycle count
	
	double cps_ctime;		
	double resMultiplier;		// Actual multiplier value to use to multiply from default=nanoseconds=1
	char* unitAbbrev = "ns";	//units abbreviation to use, default: 'ns'/nanoseconds when resMultiplier=1
};

// Helper
// Helper class
// Jump start initial cohesiveness among different areas of development of the ORIGAMI framework
class Helper
{
public:
	Helper()
	{
		logString_to_devLog("ctor invoked: Helper", true);
		logString_to_devLog("Populated Helper class configuration data", true);
		cyclestruc.cycleCounterID = -1;			// Unique ID to link cycle counter structure to specific device to monitor counts on
		cyclestruc.startTimeCycleCount = -1;	// Processor RDSTC cycle performance counter
		cyclestruc.startDateTime = -1;			// Start time/date of development branch epoch? date/time - OS supplied conversion to and from this value
		cyclestruc.lastTimeCycleCount = -1;
		cyclestruc.deltaTimeCycle = -1;			// Cycles between last cycle count and most recently updated cycle count
		cyclestruc.cps_ctime= CLOCKS_PER_SEC;
		cyclestruc.resMultiplier = 1;		// Actual multiplier value to use to multiply from default=nanoseconds=1
	}
	~Helper() 
	{
		std::stringstream s;
		logString_to_devLog("dtor invoked: Helper", true);
	}
  /* 
	*	1.		accurately detect platform kernel OS,OS-specific-information (windows,linux,android,etc....) several different ways
	*	1a.		environment variables, device metrics (memory, cache, etc) either deduced or if possible actual queries from device hardware when
	*				possible
	*	2.		detect processor type and query/populate desired processor metrics (anually as dev machine is Ryzen 2 2700X 16gb ram (bandwidth?)
	*	2a.		setup log / debug / buffers/streams/files (performance logs / activity logs)
	*	3.		XFX RX 580 8gb
	*	4.		detect GPU architecture metrics
	*/
	int detect_contextualize_map_device_metrics()	{
		char* initOutputString = "\u001b[33mDetecting Hardware...\n"//
										 "Contextualizing Maps...\n"			//
										 "Calculating Device Metrics...\n"	//
										 "\n\tContextualizing...\u001b[0m";		// ANSI 33 nice sunburnt yellow
		
		std::cout << std::endl << initOutputString << std::endl;
		
		return 0;																	// replace with EXITCODE_OK 'use enumerators
	}	
	void getCycleCounterStruct(cycleCounter_struct &ccS)
	{
		ccS = cyclestruc;
	}
	void mainExecution() // main loop for now
	{}
private:
	cycleCounter_struct cyclestruc;
};// CALLBACKS? NO ONE HOME