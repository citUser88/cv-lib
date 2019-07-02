#pragma once

#include <string>
#include <type_traits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>

static std::ofstream devLogFile;
const std::string devLogFileName = "xdlib.log";

// TODO: log function to log vulkan.hpp structures to any output stream object

// logs string to devLog, logTimeDate=true will prepend date and time 
// to string logged returns TRUE success
bool logString_to_devLog(std::string const &s, bool const logDateTime);

void logOpenLoggingFile();
void logCloseLoggingFile();