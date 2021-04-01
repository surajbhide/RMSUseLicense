#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "MiscUtils.h"

char* GetRMSDllLocation()
{
	TCHAR buffer[MAX_PATH] = { 0 };
	static TCHAR path[MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	memset(path, 0, MAX_PATH);
	if (GetModuleFileName(NULL, buffer, MAX_PATH) == FALSE)
	{
		return NULL;
	}
	// get parts that matter
	_splitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	sprintf_s(path, MAX_PATH, "%s%s", drive, dir);
	return path;
}