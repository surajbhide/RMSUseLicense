#include <windows.h>
#include <stdio.h>
#include "resource.h" 
#include "InitRMSFunctions.h"
#include "MiscUtils.h"
#include "CommonDefs.h"
#include "SaveSettings.h"

//#pragma comment(linker,"\"/manifestdependency:type='win32' \
//name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
//processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
void ProcessLicenseRelease(HWND hwnd);
// global hook procedure
HHOOK hhookCBTProc = 0;

LRESULT CALLBACK pfnCBTMsgBoxHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_ACTIVATE)
	{
		HWND hwnd = (HWND)wParam;

		// At this point you have the hwnd of the newly created 
		// message box that so you can position it at will
		//SetWindowPos(hwnd, ...);
		RECT formRect;
		RECT messageBoxRect;
		int xPos;
		int yPos;

		GetWindowRect(GetParent(hwnd), &formRect);
		GetWindowRect(hwnd, &messageBoxRect);

		xPos = (int)((formRect.left + (formRect.right - formRect.left) / 2) - ((messageBoxRect.right - messageBoxRect.left) / 2));
		yPos = (int)((formRect.top + (formRect.bottom - formRect.top) / 2) - ((messageBoxRect.bottom - messageBoxRect.top) / 2));

		SetWindowPos(hwnd, 0, xPos, yPos, 0, 0, 0x1 | 0x4 | 0x10);
		// remove the hook
		UnhookWindowsHookEx(hhookCBTProc);
	}

	return (CallNextHookEx(hhookCBTProc, nCode, wParam, lParam));
}

int CenteredMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	// set hook to center the message box that follows
	hhookCBTProc = SetWindowsHookEx(WH_CBT, pfnCBTMsgBoxHook, 0, GetCurrentThreadId());
	int sResult = MessageBox(hWnd, lpText, lpCaption, uType);
	return sResult;
}
void UpdateFontForHelp(HWND hwnd)
{
	long lfHeight;
	HDC hdc;

	hdc = GetDC(NULL);
	lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);

	HFONT hf = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Consolas");
	// set the font on all controls
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hf, (LPARAM)TRUE);
}

// about dialog handler
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		HWND editBox = NULL;
		if ((editBox = GetDlgItem(hwnd, IDC_EDIT_HOWTO)) != NULL)
		{
			char* helpText = "This is a utility to test Basic RMS operation of obtaining a license."NL
				"Developed in 2021 by Suraj for general use."NL NL
				"INPUTS:"NL
				"Feature Name: You must enter a Feature Name but other fields are optional."NL
				"Version: Version is optional if your license does not have a version associated with it."NL
				"Server: If you don't provide a server name, then RMS will use the default logic of either broadcasting or using LSHOST or LSFORCEHOST if it was defined in the environment before you started the application"NL
				"Trace File: If trace path is not provided, no trace file is generated. The path (except for the file name) must exist. If not path is provided but just a file name, it is generated in the same folder as the application."NL NL
				"OPERATIONS:"NL
				"Request: You can click the 'Request' button to request a unit of license."
				"As of now, you can't provide number of units to be requested."NL
				"You may request licenses as many times as you need until the server runs out of tokens."NL
				"Release: Click the 'Release' button to release the obtained token."NL
				"You can release licenses as many times as you have requested them before via this utility."NL NL
				"OUTPUT:"NL
				"Actual Server: Shows the server as returned by RMS for this session."NL
				"License Info: Provides those metrics for the requested features."NL NL
				"MISC:"NL
				"Activity Log: Displays some useful information related to the activity taking place within the utility."NL NL
				"DISCLAIMER:"
				"This SOFTWARE PRODUCT is provided by me 'as is' and 'with all faults.' Thales or I make no representations or warranties of any kind concerning the safety, suitability, inaccuracies, typographical errors, or other harmful components of this utility. There are inherent dangers in the use of any software, and you are solely responsible for determining whether this utility is compatible with your equipment and other software installed on your equipment. You are also solely responsible for the protection of your equipment and backup of your data, and Thales or I will not be liable for any damages you may suffer in connection with using, modifying, or distributing this utility."
				;
			// we are on the how to dialog. add help text to the static edit window
			SendMessage(editBox, WM_SETTEXT, (WPARAM)0, (LPARAM)helpText);
			// change font
			UpdateFontForHelp(editBox);
			SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)editBox, TRUE);
		}
	}
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
	RECT rc = { 0, 0, 0, 0 };
	GetClientRect(hwnd, &rc);

	const SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };

	SCROLLINFO si = { 0, 0, 0, 0, 0, 0, 0 };
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
	RECT rect = { 0, 0, 0, 0 };
	if (GetWindowRect(hwnd, &rect) != TRUE)
	{
		MessageBox(hwnd, L"Unable to get Dialog Size!", L"Error", MB_OK | MB_ICONINFORMATION);
		return;
	}

	// if dialog is bigger than screen, resize it to screen size and warn the user
	if (sHeight <= (rect.bottom - rect.top))
	{
		MessageBox(hwnd, "Screen Height is smaller than required."NL"The GUI might not be displayed correctly.", L"Error", MB_OK | MB_ICONINFORMATION);
		// change the dialog height to sHeight
		rect.bottom = sHeight;
	}
	else
	{
		rect.top = (int)((sHeight / 2) - ((rect.bottom - rect.top) / 2));
	}
	if (sWidth <= (rect.right - rect.left))
	{
		MessageBox(hwnd, "Screen Width is smaller than required."NL"The GUI might not be displayed correctly.", L"Error", MB_OK | MB_ICONINFORMATION);
		rect.right = sWidth;
	}
	else
	{
		rect.left = (int)((sWidth / 2) - ((rect.right - rect.left) / 2));
	}
	SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, 0);
}

/// START: RMS Related Functions.
/// END: RMS Related Functions.

///START: Tooltip related
// Description:
//   Creates a tooltip for an item in a dialog box. 
// Parameters:
//   idTool - identifier of an dialog box item.
//   nDlg - window handle of the dialog box.
//   pszText - string to use as the tooltip text.
// Returns:
//   The handle to the tooltip.
//

///END: tooltip related
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

		// hide scroll bars
		ConfigureDialogScrollArea(hwnd);
		// resize dialog if screen is smaller than dialog size...also warn the user.
		ManageDialogSize(hwnd);

		// limit the max in put user can provide in text fields
		SetTextFieldLimit(GetDlgItem(hwnd, IDC_FEATURE_TEXT), MAX_PATH);
		SetTextFieldLimit(GetDlgItem(hwnd, IDC_VERSION_TEXT), MAX_PATH);
		SetTextFieldLimit(GetDlgItem(hwnd, IDC_SERVER_NAME), MAX_PATH);
		SetTextFieldLimit(GetDlgItem(hwnd, IDC_TRACE_PATH), MAX_PATH);

		SetDialogHandle(hwnd);
		UpdateFontForHelp(GetDlgItem(hwnd, IDC_STATUS_TEXT));
		// set title
		SetAppTitle("");

		// Check if RMS library can be loaded. If not, error out
		if (RMSPersistenceInitLoadDll() != TRUE)
		{
			char msg[BUFSIZE] = { 0 };
			sprintf_s(msg, BUFSIZE, "Unable to load RMS Persistency Init dll. Make sure it is present at [%s].", GetRMSDllLocation());
			MessageBox(hwnd, msg, L"Error", MB_OK | MB_ICONERROR);
			EndDialog(hwnd, -1);
		}
		// restore user settings if any
		RestoreUserSettings(ReadSettingsCB);
	}
	break;
	case WM_CLOSE:
		//CleanUpProcessing(hwnd);
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
			//CleanUpProcessing(hwnd);
		}
		break;
		case ID_HELP_ABOUT:
		{
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hwnd, AboutDlgProc);
			if (ret == -1) {
				CenteredMessageBox(hwnd, L"Dialog failed!", L"Error", MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		case ID_HELP_USAGEINSTRUCTIONS:
		{
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HELPDIALOG), hwnd, AboutDlgProc);
			if (ret == -1) {
				CenteredMessageBox(hwnd, L"Dialog failed!", L"Error", MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		}
		break;
	case WM_DESTROY:
		//CleanUpProcessing(hwnd);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);
}
