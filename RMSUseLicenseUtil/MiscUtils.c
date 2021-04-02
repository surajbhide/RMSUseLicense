#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "MiscUtils.h"

#define TMP_BUF_SIZE 256
#define STATUS_MESSAGE_SIZE 1024*4

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

void InternalLogMessage (HWND controlH, char *lpszOutStr)
{
	HWND hPrevWinFocus;
	int iIndex;

	hPrevWinFocus = GetFocus();
	iIndex = GetWindowTextLength(controlH);
	SetFocus(controlH);
	// Place the caret to the very end of the current text line.
	SendMessage(controlH, EM_SETSEL, (WORD)iIndex, (LONG)iIndex);
	// Append the output string to the end.
	SendMessage(controlH, EM_REPLACESEL, 0, (LPARAM)((LPSTR)lpszOutStr));

	SetFocus(hPrevWinFocus);
}

void LogStatusMessage (HWND msgControl, char *fmt, ...)
{
	const char *p;
	va_list argp;
	int i;
	char *s;
	char fmtbuf[TMP_BUF_SIZE];
	static char statusMsg[STATUS_MESSAGE_SIZE];
	
	memset(statusMsg, 0, STATUS_MESSAGE_SIZE);

	va_start(argp, fmt);

	for (p = fmt; *p != '\0'; p++)
	{
		if (*p != '%')
		{
			//sprintf_s(fmtbuf, TMP_BUF_SIZE, "%c", *p);
			//strcat(statusMsg, fmtbuf);
			if (strlen((const char*)statusMsg) + 1 < STATUS_MESSAGE_SIZE)
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
//			sprintf_s(fmtbuf, TMP_BUF_SIZE, "%c", i);
			if (strlen((const char*)statusMsg) + 1 < STATUS_MESSAGE_SIZE)
			{
				statusMsg[strlen(statusMsg) + 1] = 0;
				statusMsg[strlen(statusMsg)] = *p;
			}
			break;

		case 'd':
			i = va_arg(argp, int);
			_itoa_s(i, fmtbuf, TMP_BUF_SIZE, 10);
			if (strlen((const char*)statusMsg) + strlen(fmtbuf) < STATUS_MESSAGE_SIZE)
				strcat_s(statusMsg, STATUS_MESSAGE_SIZE, fmtbuf);
			break;

		case 's':
			s = va_arg(argp, char *);
			if (strlen((const char*)statusMsg) + strlen(s) < STATUS_MESSAGE_SIZE)
				strcat_s(statusMsg, STATUS_MESSAGE_SIZE, s);
			break;

		case 'x':
			i = va_arg(argp, int);
			_itoa_s(i, fmtbuf, TMP_BUF_SIZE, 16);
			if (strlen((const char*)statusMsg) + strlen(fmtbuf) < STATUS_MESSAGE_SIZE)
				strcat_s(statusMsg, STATUS_MESSAGE_SIZE, fmtbuf);
			break;

		case '%':
			sprintf_s(fmtbuf, TMP_BUF_SIZE, "%c", '%');
			if (strlen((const char*)statusMsg) + 1 < STATUS_MESSAGE_SIZE)
			{
				statusMsg[strlen(statusMsg) + 1] = 0;
				statusMsg[strlen(statusMsg)] = (char)'%c';
			}
			break;
		}
	}

	va_end(argp);
	strcat_s(statusMsg, STATUS_MESSAGE_SIZE, "\r\n");
	InternalLogMessage(msgControl, statusMsg);
}