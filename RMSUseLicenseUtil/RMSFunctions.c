#include <windows.h> 
#include "RMSFunctions.h"

HINSTANCE rmsLib = NULL;

BOOL InitRMSUtils()
{
	// open the rms dll from the current directory.
	rmsLib = LoadLibrary(TEXT("lsapiw32.dll"));

	// error out on invalid handle
	if (rmsLib == NULL)
	{
		return FALSE;
	}
	return TRUE;
}

void CleanupRMSUtils()
{
	if (rmsLib != NULL)
	{
		FreeLibrary(rmsLib);
	}
}