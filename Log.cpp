#include "Log.h"
#include <stdio.h>

int gLogLevel;

void Log(WCHAR* String, int LogLevel)
{
	wprintf(L"%s \n", String);

	//if(LogLevel)
}