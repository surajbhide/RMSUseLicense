#pragma once
#include "CommonDefs.h"

#define SETTINGS_FILE "RMSUseLicense.settings.txt"
#define FEATURE_SETTING_TOKEN "feature"
#define VERSION_SETTING_TOKEN "version"
#define SERVER_SETTING_TOKEN "server"
#define TRACE_SETTING_TOKEN "trace"

typedef void(*cbFunctionPtr)(char*key, char*value);
typedef BOOL(*cbSetFuncPtr)(char *key, char *value, int *counter);

BOOL RestoreUserSettings(cbFunctionPtr cb);
BOOL SaveUserSettings(cbSetFuncPtr cb);