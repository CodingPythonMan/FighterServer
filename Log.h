#pragma once
#include <windows.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_SYSTEM 2

void Log(WCHAR* String, int LogLevel);

void Log(WCHAR* String);

#define LOG_MACRO {			\
							\
							\
}							