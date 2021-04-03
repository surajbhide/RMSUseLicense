#pragma once
#include "CommonDefs.h"
#include "lserv.h"

BOOL RMSLoadDll();
void RMSUnloadDll();
BOOL RMSCallInitMethods();
BOOL RMSCallCleanupMethods();
void RMSGetServerInfo(InputDataT inputData);
BOOL RMSRequestLicense(InputDataT input);
BOOL ReleaseRMSLicense();
BOOL RMSAreReleasePending();
BOOL RMSSetTracePath(InputDataT inputData);
BOOL RMSSetContactServer(InputDataT inputData);
BOOL RMSUpdateFeatureInfo(char *featureName, char* versionInfo);
BOOL RMSGetAcrualServerName(LS_HANDLE handle);