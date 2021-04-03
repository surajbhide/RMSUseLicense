#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "CommonDefs.h"
#include "MiscUtils.h"
#include "resource.h"
#include "SaveSettings.h"

static HWND statusLogControl = NULL;
static HWND dialogHandle = NULL;

void SetDialogHandle(HWND ctrl)
{
	dialogHandle = ctrl;
}

HWND GetLogControl()
{
	return GetDlgItem (dialogHandle, IDC_STATUS_TEXT);
}

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

void InternalLogMessage (char *lpszOutStr)
{
	HWND hPrevWinFocus;
	int iIndex;
	HWND controlH = GetLogControl();

	hPrevWinFocus = GetFocus();
	iIndex = GetWindowTextLength(controlH);
	SetFocus(controlH);
	// Place the caret to the very end of the current text line.
	SendMessage(controlH, EM_SETSEL, (WORD)iIndex, (LONG)iIndex);
	// Append the output string to the end.
	SendMessage(controlH, EM_REPLACESEL, 0, (LPARAM)((LPSTR)lpszOutStr));

	SetFocus(hPrevWinFocus);
}

void LogStatusMessage (char *fmt, ...)
{
	const char *p;
	va_list argp;
	int i;
	char *s;
	char fmtbuf[SMALL_BUFFER_SIZE];
	static char statusMsg[BUFSIZE] = TEXT("");
	
	memset(statusMsg, 0, BUFSIZE);

	va_start(argp, fmt);

	for (p = fmt; *p != '\0'; p++)
	{
		if (*p != '%')
		{
			//sprintf_s(fmtbuf, TMP_BUF_SIZE, "%c", *p);
			//strcat(statusMsg, fmtbuf);
			if (strlen((const char*)statusMsg) + 1 < BUFSIZE)
			{
				statusMsg[strlen(statusMsg)+1] = 0; 
				statusMsg[strlen(statusMsg)] = *p;
			}
			continue;
		}

		switch (*++p)
		{
		case 'c':
			i = va_arg(argp, int);
			if (strlen((const char*)statusMsg) + 1 < BUFSIZE)
			{
				statusMsg[strlen(statusMsg) + 1] = 0;
				statusMsg[strlen(statusMsg)] = *p;
			}
			break;

		case 'd':
			i = va_arg(argp, int);
			_itoa_s(i, fmtbuf, SMALL_BUFFER_SIZE, 10);
			if (strlen((const char*)statusMsg) + strlen(fmtbuf) < BUFSIZE)
				strcat_s(statusMsg, BUFSIZE, fmtbuf);
			break;

		case 's':
			s = va_arg(argp, char *);
			if (strlen((const char*)statusMsg) + strlen(s) < BUFSIZE)
				strcat_s(statusMsg, BUFSIZE, s);
			break;

		case 'x':
			i = va_arg(argp, int);
			_itoa_s(i, fmtbuf, SMALL_BUFFER_SIZE, 16);
			if (strlen((const char*)statusMsg) + strlen(fmtbuf) < BUFSIZE)
				strcat_s(statusMsg, BUFSIZE, fmtbuf);
			break;

		case 'X':
			i = va_arg(argp, int);
			_itoa_s(i, fmtbuf, SMALL_BUFFER_SIZE, 16);
			CharUpperBuff(fmtbuf, SMALL_BUFFER_SIZE);
			if (strlen((const char*)statusMsg) + strlen(fmtbuf) < BUFSIZE)
				strcat_s(statusMsg, BUFSIZE, fmtbuf);
			break;

		case '%':
			sprintf_s(fmtbuf, SMALL_BUFFER_SIZE, "%c", '%');
			if (strlen((const char*)statusMsg) + 1 < BUFSIZE)
			{
				statusMsg[strlen(statusMsg) + 1] = 0;
				statusMsg[strlen(statusMsg)] = (char)'%c';
			}
			break;
		}
	}

	va_end(argp);
	strcat_s(statusMsg, BUFSIZE, "\r\n");
	InternalLogMessage(statusMsg);
}

///START: MISC Utility
int GetScreenWidth(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mInfo;
	mInfo.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(monitor, &mInfo) != TRUE)
	{
		MessageBox(hwnd, "Unable to get Monitor Size in GetScreenWidth!", "Error", MB_OK | MB_ICONINFORMATION);
	}
	return (mInfo.rcWork.right - mInfo.rcWork.left);
	//return GetSystemMetrics(SM_CXSCREEN);
}

int GetScreenHeight(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mInfo;
	mInfo.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfo(monitor, &mInfo) != TRUE)
	{
		MessageBox(hwnd, "Unable to get Monitor Size in GetScreenHeight!", "Error", MB_OK | MB_ICONINFORMATION);
	}
	return (mInfo.rcWork.bottom - mInfo.rcWork.top);
	//return GetSystemMetrics(SM_CYSCREEN);
}
///END: MISC Utility

void SetTextFieldLimit(HWND hwnd, int limit)
{
	SendMessage(hwnd, EM_SETLIMITTEXT, limit, 0);
}

BOOL ValidatePath(char *inpath)
{
	static TCHAR path[MAX_PATH] = { 0 };
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	TCHAR  buffer[BUFSIZE] = TEXT("");

	// first get the full path
	if (GetFullPathName(inpath, BUFSIZE, buffer, NULL) == FALSE)
	{
		LogStatusMessage("Unable to get full path for [%s]", inpath);
		return FALSE;
	}
	// get parts that matter
	_splitpath_s(buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	sprintf_s(path, MAX_PATH, "%s%s", drive, dir);
	// check if path exists. if not return false;
	DWORD dwAttrib = GetFileAttributes(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL GetEditControlText(HWND hwnd, char *destination, int destSize)
{
	TCHAR buffer[BUFSIZE] = TEXT("");

	// get window text length
	if (GetWindowTextLength(hwnd) == 0)
	{
		if (GetLastError() != 0)
		{
			// error
			return FALSE;
		}
	}
	else
	{
		// there is text
		if (GetWindowText(hwnd, buffer, BUFSIZE) == 0)
		{
			// error reading text
			return FALSE;
		}
		else
			strcpy_s(destination, destSize, buffer);
	}
	return TRUE;
}

void UpdateServerName(char *servername)
{
	SendMessage(GetDlgItem(dialogHandle, IDC_ACTUAL_SERVER_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)servername);
}
void UpdateLicenseInfo(char *info)
{
	SendMessage(GetDlgItem(dialogHandle, IDC_LICENSE_INFO), WM_SETTEXT, (WPARAM)0, (LPARAM)info);
}

void ReadSettingsCB(char *key, char *value)
{
	if (strcmp(key, FEATURE_SETTING_TOKEN) == 0)
		SendMessage(GetDlgItem(dialogHandle, IDC_FEATURE_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)value);
	if (strcmp(key, VERSION_SETTING_TOKEN) == 0)
		SendMessage(GetDlgItem(dialogHandle, IDC_VERSION_TEXT), WM_SETTEXT, (WPARAM)0, (LPARAM)value);
	if (strcmp(key, SERVER_SETTING_TOKEN) == 0)
		SendMessage(GetDlgItem(dialogHandle, IDC_SERVER_NAME), WM_SETTEXT, (WPARAM)0, (LPARAM)value);
	if (strcmp(key, TRACE_SETTING_TOKEN) == 0)
		SendMessage(GetDlgItem(dialogHandle, IDC_TRACE_PATH), WM_SETTEXT, (WPARAM)0, (LPARAM)value);
}
BOOL SaveSettingsCB(char *key, char *value, int *counter)
{
	char buf[BUFSIZE] = TEXT("");

	switch (*counter)
	{
	case 1:
		strcpy_s(key, BUFSIZE, FEATURE_SETTING_TOKEN);
		if (GetEditControlText(GetDlgItem(dialogHandle, IDC_FEATURE_TEXT), buf, BUFSIZE))
			strcpy_s(value, BUFSIZE, buf);
		return TRUE;
		break;
	case 2:
		strcpy_s(key, BUFSIZE, VERSION_SETTING_TOKEN);
		if (GetEditControlText(GetDlgItem(dialogHandle, IDC_VERSION_TEXT), buf, BUFSIZE))
			strcpy_s(value, BUFSIZE, buf);
		return TRUE;
		break;
	case 3:
		strcpy_s(key, BUFSIZE, SERVER_SETTING_TOKEN);
		if (GetEditControlText(GetDlgItem(dialogHandle, IDC_SERVER_NAME), buf, BUFSIZE))
			strcpy_s(value, BUFSIZE, buf);
		return TRUE;
		break;
	case 4:
		strcpy_s(key, BUFSIZE, TRACE_SETTING_TOKEN);
		if (GetEditControlText(GetDlgItem(dialogHandle, IDC_TRACE_PATH), buf, BUFSIZE))
			strcpy_s(value, BUFSIZE, buf);
		return TRUE;
		break;
	default:
		return FALSE;
		break;
	}
	return FALSE;
}

void SetAppTitle(char *suffix)
{
	char buffer[BUFSIZE] = TEXT("");

	strcpy_s(buffer, BUFSIZE, TITLE);
	if (strlen(suffix) > 0)
	{
		strcat_s(buffer, BUFSIZE, " "); 
		strcat_s(buffer, BUFSIZE, suffix);
	}
	SetWindowText(dialogHandle, buffer);
}