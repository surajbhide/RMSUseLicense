#include <Windows.h>
#include <ShlObj.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <stdio.h>
#include "MiscUtils.h"
#include "SaveSettings.h"

char* GetSettingsPath()
{
	static TCHAR szPath[MAX_PATH] = TEXT("");

	if (SUCCEEDED(SHGetFolderPath(NULL,
		CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL,
		0,
		szPath)))
	{
		if (strcat_s(szPath, MAX_PATH, "\\"SETTINGS_FILE) != 0)
			szPath[0] = 0;
	}
	else
		szPath[0] = 0;

	LogStatusMessage("Settings path = [%s]", szPath);
	return szPath;
}
BOOL SaveUserSettings(cbSetFuncPtr cb)
{
	// open the settings file. if not present create it
	// save key/value pair
	// close file.
	// get settings file path
	char *path = GetSettingsPath();
	if (strlen(path) <= 0)
		return FALSE;

	// if file doesn't exist return false
	FILE *fileHandle = NULL;
	int ret = fopen_s(&fileHandle, path, "w");
	if (ret != 0 || fileHandle == NULL)
	{
		LogStatusMessage("Unable to open the settings file [%s] for writing. fopen_s error [%d]", path, ret);
		return FALSE;
	}

	// file exists. and is open. read the data.
	char key[BUFSIZE] = "";
	char value[BUFSIZE] = "";
	int counter = 1;
	while (cb(key, value, &counter) == TRUE)
	{
		if (strlen(value) > 0)
		{
			fprintf_s(fileHandle, "%s=%s\n", key, value);
			LogStatusMessage("Stored setting [%s] with value [%s]", key, value);
		}
		counter++;
	}
	fclose(fileHandle);
	return TRUE;
}

BOOL RestoreUserSettings(cbFunctionPtr cb)
{
	// get settings file path
	char *path = GetSettingsPath();
	if (strlen(path) <= 0)
		return FALSE;

	// if file doesn't exist return false
	FILE *fileHandle = NULL;
	int ret = fopen_s (&fileHandle, path, "r");
	if (ret != 0 || fileHandle == NULL)
	{
		LogStatusMessage("Unable to open the settings file [%s]. fopen_s error [%d]", path, ret);
		return FALSE;
	}

	// file exists. and is open. read the data.
	while (!feof(fileHandle))
	{
		char buf[BUFSIZE] = "";
		char buf1[BUFSIZE] = "";
		int fields = fscanf_s (fileHandle, "%[^=]=%[^\n]\n", buf, (unsigned)_countof(buf), buf1, (unsigned)_countof(buf1));
		if (fields == 2)
		{
			LogStatusMessage("Restoring saved value for [%s] to [%s]", buf, buf1);
			cb(buf, buf1);
		}
	}
	fclose(fileHandle);
	return TRUE;
}