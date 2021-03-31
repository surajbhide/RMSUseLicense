#include <Windows.h>

BOOL CALLBACK SetFont(HWND child, LPARAM font)
{
	SendMessage(child, WM_SETFONT, font, TRUE);
	return true;
}
