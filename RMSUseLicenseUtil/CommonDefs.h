#pragma once
#include <Windows.h>

#define VERSION "0.1"
#define TITLE "RMS Use License Utility - Version " VERSION
#define NL "\r\n"
#define SMALL_BUFFER_SIZE 256
#define BUFSIZE 4096

typedef struct
{
	char featureName[MAX_PATH];
	char versionNumber[MAX_PATH];
	char serverName[MAX_PATH];
	char tracePath[MAX_PATH];
} InputDataT;

typedef struct
{
	unsigned long handle;
	char featureName[MAX_PATH];
	char versionNumber[MAX_PATH];
} LicenseInfoT;