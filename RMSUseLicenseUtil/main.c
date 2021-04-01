#include <windows.h>
#include "resource.h" 

#define VERSION "0.1"
#define TITLE "RMS Use License Utility - Version " VERSION

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

// about dialog handler
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK); 
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, IDOK);
	default:
		return FALSE;
	}
	return TRUE;
}

///START: Dialog scrolling related
int SD_GetScrollPos(HWND hwnd, int bar, UINT code)
{
	SCROLLINFO si = { 0, 0, 0, 0, 0, 0, 0 };
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(hwnd, bar, &si);

	const int minPos = si.nMin;
	const int maxPos = si.nMax - (si.nPage - 1);

	int result = -1;

	switch (code)
	{
	case SB_LINEUP /*SB_LINELEFT*/:
		result = max(si.nPos - 1, minPos);
		break;

	case SB_LINEDOWN /*SB_LINERIGHT*/:
		result = min(si.nPos + 1, maxPos);
		break;

	case SB_PAGEUP /*SB_PAGELEFT*/:
		result = max(si.nPos - (int)si.nPage, minPos);
		break;

	case SB_PAGEDOWN /*SB_PAGERIGHT*/:
		result = min(si.nPos + (int)si.nPage, maxPos);
		break;

	case SB_THUMBPOSITION:
		// do nothing
		break;

	case SB_THUMBTRACK:
		result = si.nTrackPos;
		break;

	case SB_TOP /*SB_LEFT*/:
		result = minPos;
		break;

	case SB_BOTTOM /*SB_RIGHT*/:
		result = maxPos;
		break;

	case SB_ENDSCROLL:
		// do nothing
		break;
	}

	return result;
}

void ConfigureDialogScrollArea(HWND hwnd)
{
	RECT rc = {0, 0, 0, 0};
	GetClientRect(hwnd, &rc);

	const SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };

	SCROLLINFO si = {0, 0, 0, 0, 0, 0, 0};
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPos = si.nMin = 1;

	si.nMax = sz.cx;
	si.nPage = sz.cx;
	SetScrollInfo(hwnd, SB_HORZ, &si, FALSE);

	si.nMax = sz.cy;
	si.nPage = sz.cy;
	SetScrollInfo(hwnd, SB_VERT, &si, FALSE);
}

void SD_ScrollClient(HWND hwnd, int bar, int pos)
{
	static int s_prevx = 1;
	static int s_prevy = 1;

	int cx = 0;
	int cy = 0;

	int* delta = (bar == SB_HORZ ? &cx : &cy);
	int* prev = (bar == SB_HORZ ? &s_prevx : &s_prevy);

	*delta = *prev - pos;
	*prev = pos;

	if (cx || cy)
	{
		ScrollWindow(hwnd, cx, cy, NULL, NULL);
	}
}

void DialogOnReSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state != SIZE_RESTORED && state != SIZE_MAXIMIZED)
		return;

	SCROLLINFO si = { 0, 0, 0, 0, 0, 0, 0 };
	si.cbSize = sizeof(SCROLLINFO);

	const int bar[] = { SB_HORZ, SB_VERT };
	const int page[] = { cx, cy };

	for (size_t i = 0; i < ARRAYSIZE(bar); ++i)
	{
		si.fMask = SIF_PAGE;
		si.nPage = page[i];
		SetScrollInfo(hwnd, bar[i], &si, TRUE);

		si.fMask = SIF_RANGE | SIF_POS;
		GetScrollInfo(hwnd, bar[i], &si);

		const int maxScrollPos = si.nMax - (page[i] - 1);

		// Scroll client only if scroll bar is visible and window's
		// content is fully scrolled toward right and/or bottom side.
		// Also, update window's content on maximize.
		const BOOL needToScroll =
			(si.nPos != si.nMin && si.nPos == maxScrollPos) ||
			(state == SIZE_MAXIMIZED);

		if (needToScroll)
		{
			SD_ScrollClient(hwnd, bar[i], si.nPos);
		}
	}
}

void SD_OnHVScroll(HWND hwnd, int bar, UINT code)
{
	const int scrollPos = SD_GetScrollPos(hwnd, bar, code);

	if (scrollPos == -1)
		return;

	SetScrollPos(hwnd, bar, scrollPos, TRUE);
	SD_ScrollClient(hwnd, bar, scrollPos);
}

void SD_OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	SD_OnHVScroll(hwnd, SB_HORZ, code);
}

void SD_OnVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
	SD_OnHVScroll(hwnd, SB_VERT, code);
}
///END: Dialog scrolling related

void ManageDialogSize(HWND hwnd)
{
	// get screen height and width
	int sHeight = GetScreenHeight(hwnd);
	int sWidth = GetScreenWidth(hwnd);
	// get dialog height and width
	RECT rect = {0, 0, 0, 0};
	if (GetWindowRect(hwnd, &rect) != TRUE)
	{
		MessageBox(hwnd, "Unable to get Dialog Size!", "Error", MB_OK | MB_ICONINFORMATION);
	}

	// if dialog is bigger than screen, resize it to screen size and warn the user
	if (sHeight <= (rect.bottom - rect.top))
	{
		MessageBox(hwnd, "Screen Height is smaller than required.\r\nThe GUI might not be displayed correctly.", "Error", MB_OK | MB_ICONINFORMATION);
		// change the dialog height to sHeight
		rect.bottom = sHeight;
	}
	if (sWidth <= (rect.right - rect.left))
	{
		MessageBox(hwnd, "Screen Width is smaller than required.\r\nThe GUI might not be displayed correctly.", "Error", MB_OK | MB_ICONINFORMATION);
		rect.right = sWidth;
	}
	SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, 0);
}

/// START: RMS Related Functions.
// used by main dialog handler to clean up anything that needs to be cleaned up.
void CleanUpProcessing(HWND hwnd)
{
	//MessageBox(hwnd, "Are you sure", "Confirm", MB_YESNO);
	EndDialog(hwnd, IDOK);
}
/// END: RMS Related Functions.

///START: Trace file related
char* GetTraceFileName(HWND hwnd)
{
	OPENFILENAME ofn;
	static char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = "txt";

	if (GetSaveFileName(&ofn))
	{
		return szFileName;
	}
	// return empty string on error or user cancels dialog
	return "";
}
///END: Trace File related

// the main dialog handler
BOOL CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		// set menu
		HMENU hm = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU));
		SetMenu(hwnd, hm);
		// set dialog icon
		HICON hicon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hicon);
		HICON smicon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)smicon);

		// set title
		SetWindowText(hwnd, TITLE);
		// hide scroll bars
		ConfigureDialogScrollArea(hwnd);
		// resize dialog if screen is smaller than dialog size...also warn the user.
		ManageDialogSize(hwnd);
	}
		break;
	case WM_CLOSE:
		CleanUpProcessing(hwnd);
		break;
	case WM_SIZE:
		DialogOnReSize(hwnd, wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_HSCROLL:
		SD_OnHScroll(hwnd, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_VSCROLL:
		SD_OnVScroll(hwnd, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
		{
			// close dialog
			CleanUpProcessing(hwnd);
		}
		break;
		case ID_HELP_ABOUT:
		{
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hwnd, AboutDlgProc);
			if (ret == -1) {
				MessageBox(hwnd, "Dialog failed!", "Error", MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		case IDC_BUTTON_TRACE:
		{
			char *fileName = GetTraceFileName(hwnd);
			SendMessage(GetDlgItem (hwnd, IDC_TRACE_PATH), WM_SETTEXT, (WPARAM)0, (LPARAM)fileName);
		}
		break;
		//case IDOK:
		//	EndDialog(hwnd, 0);
		//	break;
		}
		break;
	case WM_DESTROY:
		// called when dialog is being destroyed. you can't do much here
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);
}
