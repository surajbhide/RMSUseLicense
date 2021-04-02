#pragma once

char* GetRMSDllLocation();

void LogStatusMessage(HWND msgControl, char *fmt, ...);

int GetScreenWidth(HWND hwnd);
int GetScreenHeight(HWND hwnd);
void SetTextFieldLimit(HWND hwnd, int limit);