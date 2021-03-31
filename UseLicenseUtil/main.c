#include <windows.h>
#include "resource.h"

#pragma comment(lib, "user32.lib")


#define VERSION "0.1"
#define TITLE "RMS Use License Utility - Version " VERSION
#define LABEL_FEATURE "Feature :"
#define LABEL_VERSION "Version (Optional) :"
#define LABEL_SERVER "Server (if empty, uses defaults) :"
#define LABEL_TRACE "Trace File (if empty no trace is generated): "
// set the below to 0 for normal styling
#define EX_STYLE WS_EX_CLIENTEDGE
#define LABEL_STYLE SS_ENDELLIPSIS | WS_CHILD | WS_VISIBLE | SS_LEFT 
//| SS_ETCHEDFRAME
#define LABEL_HEIGHT 20

HWND hwndFeature;

const char g_szClassName[] = "myWindowClass";

BOOL CALLBACK SetFont(HWND child, LPARAM font)
{
	SendMessage(child, WM_SETFONT, font, TRUE);
	return TRUE;
}

// change the font for all controls on the windows.
void UpdateFont(HWND hwnd)
{
	long lfHeight;
	HDC hdc;

	hdc = GetDC(NULL);
	lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);

	HFONT hf = CreateFont(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Segoe UI");
	// set the font on all controls
	//EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));
	EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)hf);
}

int GetSystemWidth()
{
	return GetSystemMetrics(SM_CXSCREEN);
}

int GetSystemHeight()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

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
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		CreateWindowEx (EX_STYLE, "Static", LABEL_FEATURE, LABEL_STYLE,
			10, 10, 150, LABEL_HEIGHT, hwnd, (HMENU)0, GetModuleHandle(NULL), NULL);
		hwndFeature = CreateWindowEx (0, "Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
			10, 10+LABEL_HEIGHT, 150, 20, hwnd, (HMENU)0, NULL, NULL);

		CreateWindowEx(EX_STYLE, "Static", LABEL_VERSION, LABEL_STYLE,
			200, 10, 150, LABEL_HEIGHT, hwnd, (HMENU)0, GetModuleHandle(NULL), NULL);

		UpdateFont(hwnd);
	}
	break;
	case WM_CLOSE:
		// do clean up and destroy window.
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case ID_FILE_EXIT:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			case ID_HELP_ABOUT:
			{
				int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_ABOUT), hwnd, AboutDlgProc);
				if (ret == -1) {
					MessageBox(hwnd, "Dialog failed!", "Error", MB_OK | MB_ICONINFORMATION);
				}
				break;
			}
		}
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;
	HICON hIcon, hIconSm;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW; // 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE); //(HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
	if (hIcon)
		wc.hIcon = hIcon;
	else
		MessageBox(NULL, "Could not load large icon!", "Error", MB_OK | MB_ICONERROR);
	hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);
	if (hIconSm)
		wc.hIconSm = hIconSm;
	else
		MessageBox(NULL, "Could not load small icon!", "Error", MB_OK | MB_ICONERROR);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx( WS_EX_CLIENTEDGE,
		g_szClassName,
		TITLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, (int) (GetSystemWidth()*.33), (int)(GetSystemHeight()*.60),
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}