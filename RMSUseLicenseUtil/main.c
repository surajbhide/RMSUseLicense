#include <windows.h>
#include <stdio.h>
#include "resource.h" 
#include "RMSFunctions.h"
#include "MiscUtils.h"
#include "CommonDefs.h"
#include "SaveSettings.h"

// global hook procedure
HHOOK hhookCBTProc = 0;
static InputDataT inputData = { "", "", "", "" };

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
			char *helpText = "This is a utility to test Basic RMS operation of obtaining a license."NL 
				"Developed in 2021 by Suraj for general use."NL NL
				"INPUTS:"NL
				"Feature Name: You must enter a Feature Name but other fields are optional."NL
				"Version: Version is optional if your license does not have a version associated with it."NL
				"Server: If you don't provide a server name, then RMS will use the default logic of either broadcasting or using LSHOST or LSFORCEHOST if it was defined in the environment before you started the application"NL
				"Trace File: If trace path is not provided, no trace file is generated. The path (except for the file name) must exist. If not path is provided but just a file name, it is generated in the same folder as the application."NL NL
				"OPERATIONS:"NL
				"Request: You can click the 'Request' button to request a unit of license."
				"As of now, you can't provide number of units to be requested."NL
				"Release: Click the 'Release' button to release the obtained token."NL NL
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
		return;
	}

	// if dialog is bigger than screen, resize it to screen size and warn the user
	if (sHeight <= (rect.bottom - rect.top))
	{
		MessageBox(hwnd, "Screen Height is smaller than required."NL"The GUI might not be displayed correctly.", "Error", MB_OK | MB_ICONINFORMATION);
		// change the dialog height to sHeight
		rect.bottom = sHeight;
	}
	else
	{
		rect.top = (int)((sHeight / 2) - ((rect.bottom - rect.top) / 2));
	}
	if (sWidth <= (rect.right - rect.left))
	{
		MessageBox(hwnd, "Screen Width is smaller than required."NL"The GUI might not be displayed correctly.", "Error", MB_OK | MB_ICONINFORMATION);
		rect.right = sWidth;
	}
	else
	{
		rect.left = (int)((sWidth / 2) - ((rect.right - rect.left) / 2));
	}
	SetWindowPos(hwnd, HWND_TOP, rect.left, rect.top, rect.right, rect.bottom, 0);
}

/// START: RMS Related Functions.
void ProcessLicenseRequest(HWND hwnd)
{
	static BOOL shouldReadInput = TRUE;

	if (RMSAreReleasePending() == FALSE)
	{
		if (GetEditControlText(GetDlgItem(hwnd, IDC_FEATURE_TEXT), inputData.featureName, MAX_PATH) != TRUE)
		{
			CenteredMessageBox(hwnd, "Unable to read Feature Name and it can't be empty!", "Error", MB_OK | MB_ICONERROR);
			return;
		}
		else if (strlen(inputData.featureName) <= 0)
		{
			CenteredMessageBox(hwnd, "Feature Name can't be empty!", "Error", MB_OK | MB_ICONERROR);
			SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwnd,IDC_FEATURE_TEXT), TRUE);
			return;
		}

		if (GetEditControlText(GetDlgItem(hwnd, IDC_VERSION_TEXT), inputData.versionNumber, MAX_PATH) != TRUE)
			LogStatusMessage("Unable to read version info...assuming none given...");

		if (GetEditControlText(GetDlgItem(hwnd, IDC_SERVER_NAME), inputData.serverName, MAX_PATH) != TRUE)
			LogStatusMessage("Unable to read server info...assuming none given...");

		if (GetEditControlText(GetDlgItem(hwnd, IDC_TRACE_PATH), inputData.tracePath, MAX_PATH) != TRUE)
			LogStatusMessage("Unable to read trace path ...assuming none given...");

		// make sure the dir in the path is valid
		if (strlen(inputData.tracePath) > 0)
		{
			if (ValidatePath(inputData.tracePath) != TRUE)
			{
				CenteredMessageBox(hwnd, "Unable to validate the directory part of trace path. Ensure it is correct.", "Error", MB_OK | MB_ICONERROR);
				return;
			}
			else
			{
				LogStatusMessage("Trace path [%s] appears to be valid based on validating it's directory path.", inputData.tracePath);
			}
		}

		// pass trace file if given and contact server if given
		if (RMSCallInitMethods() != TRUE)
		{
			CenteredMessageBox(hwnd, "Unable to Initialize RMS. Please check status window for errors and fix them.",
				"Error", MB_OK | MB_ICONERROR);
			return;
		}
		// now set the contact server and trace paths
		RMSSetTracePath(inputData);
		RMSSetContactServer(inputData);
	}

	if (RMSRequestLicense(inputData) != TRUE)
	{
		// failed to get licenses
		return;
	}
	RMSGetServerInfo(inputData);

	// if it comes here then means we got a handle, disable inputs
	if (RMSAreReleasePending() == TRUE)
	{
		EnableWindow(GetDlgItem(hwnd, IDC_TRACE_PATH), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_TRACE), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_TEXT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_VERSION_TEXT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_SERVER_NAME), FALSE);
	}
}

void ProcessLicenseRelease(HWND hwnd)
{
	// call release rms license
	// if there are no more handles then enable controls again
	ReleaseRMSLicense();
	if (RMSAreReleasePending() == FALSE)
	{
		EnableWindow(GetDlgItem(hwnd, IDC_TRACE_PATH), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_BUTTON_TRACE), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_FEATURE_TEXT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_VERSION_TEXT), TRUE);
		EnableWindow(GetDlgItem(hwnd, IDC_SERVER_NAME), TRUE);
	}
}

// used by main dialog handler to clean up anything that needs to be cleaned up.
void CleanUpProcessing(HWND hwnd)
{
	//MessageBox(hwnd, "Are you sure", "Confirm", MB_YESNO);
	SaveUserSettings(SaveSettingsCB);
	LogStatusMessage("Cleaning up and shutting down...");
	// call vls cleanup
	RMSCallCleanupMethods();
	RMSUnloadDll();
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

		// set title
		SetWindowText(hwnd, TITLE);
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

		// Check if RMS library can be loaded. If not, error out
		if (RMSLoadDll() != TRUE)
		{
			char msg[BUFSIZE] = { 0 };
			sprintf_s(msg, BUFSIZE, "Unable to load lsapiw32.dll.Make sure it is present at [%s]. \r\nOr make sure it is newer than or equal to 8.1 release of RMS.", GetRMSDllLocation());
			MessageBox(hwnd, msg, "Error", MB_OK | MB_ICONERROR);
			EndDialog(hwnd, -1);
		}
		// if RMS dll is loaded do the vlsinitialize and few other things now
		if (RMSCallInitMethods() != TRUE)
		{
			MessageBox(hwnd, "Unable to Initialize RMS.", "Error", MB_OK | MB_ICONERROR);
			EndDialog(hwnd, -1);
		}
		// restore user settings if any
		RestoreUserSettings(ReadSettingsCB);
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
				CenteredMessageBox(hwnd, "Dialog failed!", "Error", MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		case ID_HELP_USAGEINSTRUCTIONS:
		{
			int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_HELPDIALOG), hwnd, AboutDlgProc);
			if (ret == -1) {
				CenteredMessageBox(hwnd, "Dialog failed!", "Error", MB_OK | MB_ICONINFORMATION);
			}
		}
		break;
		case IDC_BUTTON_TRACE:
		{
			char *fileName = GetTraceFileName(hwnd);
			SendMessage(GetDlgItem (hwnd, IDC_TRACE_PATH), WM_SETTEXT, (WPARAM)0, (LPARAM)fileName);
			LogStatusMessage("Trace file [%s] selected.", fileName);
		}
		break;
		case IDC_REQUEST_BUTTON:
		{
			ProcessLicenseRequest(hwnd);
		}
		break;
		case IDC_RELEASE_BUTTON:
		{
			ProcessLicenseRelease(hwnd);
		}
		break;
		//case IDOK:
		//	EndDialog(hwnd, 0);
		//	break;
		}
		break;
	case WM_DESTROY:
		CleanUpProcessing(hwnd);
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
