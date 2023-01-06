#pragma once
#include "CommonDefs.h"

char* GetRMSDllLocation();
BOOL ValidatePath(char *path);
void LogStatusMessage(char *fmt, ...);
void SetDialogHandle(HWND ctrl);
HWND GetLogControl();
int GetScreenWidth(HWND hwnd);
int GetScreenHeight(HWND hwnd);
void SetTextFieldLimit(HWND hwnd, int limit);
BOOL GetEditControlText(HWND hwnd, char *destination, int destSize);
void UpdateServerName(char *servername);
void UpdateLicenseInfo(char *info);
void ReadSettingsCB(char *key, char *value);
BOOL SaveSettingsCB(char *key, char *value, int *counter);
void SetAppTitle(char *suffix);