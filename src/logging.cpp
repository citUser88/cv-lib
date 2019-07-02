#include "iostream"
#include "fstream"
#include "sstream"
#include "chrono"
#include "logging.h"

#include <windows.h>

// logs string to devLog, logTimeDate=true will prepend date and time to string logged returns TRUE success
bool logString_to_devLog(std::string const & s, bool const logDateTime)
{
	// add in date/time logging support
	char lDtS[80] = { 0x0 };
	if (logDateTime)
	{
		time_t rawTime = NULL;
		struct tm timeinfo;
		time(&rawTime);
		localtime_s(&timeinfo, &rawTime);
		strftime(lDtS, 80, "[%x %X] ", &timeinfo);
	}
	devLogFile << lDtS << s << std::endl; // log char buffer [80] a null-terminated c-string returned by strftime above	
	std::string ds = s + "\n";
	OutputDebugStringA(ds.c_str());
	return true;
}

void logOpenLoggingFile()
{
	devLogFile.open(devLogFileName); // open up log file - maybe append to old file, start and finish logging, ren old/create new(curr) log?????? TBA
}

void logCloseLoggingFile()
{
	logString_to_devLog("-END LOG-", false);
	devLogFile.flush();
	devLogFile.close();
}