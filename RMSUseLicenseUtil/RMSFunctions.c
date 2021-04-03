#include <windows.h> 
#include "RMSFunctions.h"
#include "lserv.h"
#include "MiscUtils.h"
#include "Stack.h"
#include "CommonDefs.h"
#include "resource.h"

// prototypes for the functions we need
typedef LS_STATUS_CODE(__cdecl *VLSinitializeT)();
typedef LS_STATUS_CODE(__cdecl *VLScleanupT)();
typedef LS_STATUS_CODE(__cdecl *VLSsetUserTraceFileT)(VLS_ERR_FILE msgFile, char LSFAR * filePath);
typedef LS_STATUS_CODE(__cdecl *VLSsetTraceLevelT)(int trace_level);
typedef LS_STATUS_CODE(__cdecl *VLSgetLibInfoT)(LS_LIBVERSION LSFAR * pInfo);
typedef LS_STATUS_CODE(__cdecl *VLScontrolRemoteSessionT)(int ToCheck);
typedef LS_STATUS_CODE(__cdecl *VLSgetServInfoT)(
	unsigned char LSFAR *server_name, 
	VLSservInfo   LSFAR *srv_info,
	unsigned char LSFAR *unused1,
	unsigned long LSFAR *unused2);
typedef LS_STATUS_CODE(__cdecl *VLSsetContactServerT)(char LSFAR *server_name);
//typedef LS_STATUS_CODE(__cdecl *VLSisVirtualMachineT)(VLSVMInfo *vm_info);
typedef LS_STATUS_CODE(__cdecl *LSRequestT)(
	unsigned char  LSFAR *license_system,
	unsigned char  LSFAR *publisher_name,
	unsigned char  LSFAR *product_name,
	unsigned char  LSFAR *version,
	unsigned long  LSFAR *units_reqd,
	unsigned char  LSFAR *log_comment,
	LS_CHALLENGE   LSFAR *challenge,
	LS_HANDLE      LSFAR *lshandle);
typedef LS_STATUS_CODE(__cdecl *LSReleaseT)(
	LS_HANDLE             lshandle,
	unsigned long         units_consumed,
	unsigned char  LSFAR *log_comment);
typedef LS_STATUS_CODE(__cdecl *VLSgetFeatureInfoT)(
	unsigned char   LSFAR *feature_name,
	unsigned char   LSFAR *version,
	int                    index,
	char            LSFAR *unused1,
	VLSfeatureInfo  LSFAR *feature_info);
typedef LS_STATUS_CODE(__cdecl *VLSgetServerNameFromHandleT)(
	LS_HANDLE        handle_id,
	char   LSFAR     *outBuf,
	int    outBufSz);

// globals
VLSinitializeT VLSinitializeFP;
VLScleanupT VLScleanupFP;
VLSsetUserTraceFileT VLSsetUserTraceFileFP;
VLSsetTraceLevelT VLSsetTraceLevelFP;
VLSgetLibInfoT VLSgetLibInfoFP;
VLScontrolRemoteSessionT VLScontrolRemoteSessionFP;
VLSgetServInfoT VLSgetServInfoFP;
VLSsetContactServerT VLSsetContactServerFP;
//VLSisVirtualMachineT VLSisVirtualMachineFP;
LSRequestT LSRequestFP;
LSReleaseT LSReleaseFP;
VLSgetFeatureInfoT VLSgetFeatureInfoFP;
VLSgetServerNameFromHandleT VLSgetServerNameFromHandleFP;

static HINSTANCE rmsLib = NULL;
static DynamicStack rmsHandleStack;
static BOOL rmsInitialized = FALSE;

BOOL RMSLoadDll()
{
	// open the rms dll from the current directory.
	rmsLib = LoadLibrary(TEXT("lsapiw32.dll"));

	// error out on invalid handle
	if (rmsLib == NULL)
	{
		return FALSE;
	}
	VLSinitializeFP = (VLSinitializeT)GetProcAddress(rmsLib, "VLSinitialize");
	VLScleanupFP = (VLScleanupT)GetProcAddress(rmsLib, "VLScleanup");
	VLSsetUserTraceFileFP = (VLSsetUserTraceFileT)GetProcAddress(rmsLib, "VLSsetUserTraceFile");
	VLSsetTraceLevelFP = (VLSsetTraceLevelT)GetProcAddress(rmsLib, "VLSsetTraceLevel");
	VLSgetLibInfoFP = (VLSgetLibInfoT)GetProcAddress(rmsLib, "VLSgetLibInfo");
	VLScontrolRemoteSessionFP = (VLScontrolRemoteSessionT)GetProcAddress(rmsLib, "VLScontrolRemoteSession");
	VLSgetServInfoFP = (VLSgetServInfoT)GetProcAddress(rmsLib, "VLSgetServInfo");
	VLSsetContactServerFP = (VLSsetContactServerT)GetProcAddress(rmsLib, "VLSsetContactServer");
	//VLSisVirtualMachineFP = (VLSisVirtualMachineT)GetProcAddress(rmsLib, "VLSisVirtualMachine");
	LSRequestFP = (LSRequestT)GetProcAddress(rmsLib, "LSRequest");
	LSReleaseFP = (LSReleaseT)GetProcAddress(rmsLib, "LSRelease");
	VLSgetFeatureInfoFP = (VLSgetFeatureInfoT)GetProcAddress(rmsLib, "VLSgetFeatureInfo");
	VLSgetServerNameFromHandleFP = (VLSgetServerNameFromHandleT)GetProcAddress(rmsLib, "VLSgetServerNameFromHandle");

	if (!VLSinitializeFP || !VLScleanupFP || !VLSsetUserTraceFileFP ||
		!VLSsetTraceLevelFP || !VLSgetLibInfoFP || !VLScontrolRemoteSessionFP ||
		!VLSgetServInfoFP || !VLSsetContactServerFP || 
		//!VLSisVirtualMachineFP ||
		!LSRequestFP || !LSReleaseFP || !VLSgetFeatureInfoFP || !VLSgetServerNameFromHandleFP)
	{
		return FALSE;
	}
	LogStatusMessage("Loaded lsapiw32.dll from [%s]...", GetRMSDllLocation());
	LogStatusMessage("Imported RMS methods successfully.");
	return TRUE;
}

void RMSUnloadDll()
{
	if (rmsLib != NULL)
	{
		FreeLibrary(rmsLib);
	}
}

BOOL RMSCallInitMethods()
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;
	LS_LIBVERSION verinfo;

	if (rmsInitialized == TRUE)
		return TRUE;

	LogStatusMessage("Initializing RMS...");

	if ((status = VLSgetLibInfoFP(&verinfo)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSgetLibInfo returned [%d][0x%X]", status, status);
	}
	else
	{
		LogStatusMessage("Information about the RMS client Library:\n"); 
		LogStatusMessage("szVersion  = [%s]\n", verinfo.szVersion);
		LogStatusMessage("szProtocol = [%s]\n", verinfo.szProtocol);
		LogStatusMessage("szPlatform = [%s]\n", verinfo.szPlatform);
		LogStatusMessage("szLocale   = [%s]\n", verinfo.szLocale);
	}

	// calls vlsinitialize/set trace/get lib version etc
	if ( (status = VLSinitializeFP()) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSinitialize returned [%d][0x%X]", status, status);
		return FALSE;
	}
	else
		LogStatusMessage("VLSinitialize returned success");

	if ((status = VLScontrolRemoteSessionFP(VLS_OFF)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLScontrolRemoteSession returned [%d][0x%X]", status, status);
	}
	else
		LogStatusMessage("VLScontrolRemoteSession returned success");

	// create a dynamic stack to hold handles
	rmsHandleStack = StackNew();
	LogStatusMessage("RMS initialized successfully.");
	rmsInitialized = TRUE;
	return TRUE;
}

BOOL RMSCallCleanupMethods()
{
	if (rmsInitialized == FALSE)
		return TRUE;

	// release any pending handles
	while (RMSAreReleasePending())
	{
		ReleaseRMSLicense();
	}

	StackDestroy(rmsHandleStack);
	// call vlscleanup here
	VLScleanupFP();
	LogStatusMessage("RMS cleanup called successfully.");
	rmsInitialized = FALSE;
	return TRUE;
}

void RMSGetServerInfo(InputDataT inputData)
{
	static BOOL isAlreadyCalled = FALSE;
	VLSservInfo serverInfo;
	//VLSVMInfo vminfo; -> doesn't work with old RMS release so disabled this.
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	if (isAlreadyCalled)
		return;

	// call this after setting the server details.
	//vminfo.structSz = sizeof(VLSVMInfo);
	//if ((status = VLSisVirtualMachineFP(&vminfo)) != LS_SUCCESS)
	//{
	//	LogStatusMessage("FAIL: VLSisVirtualMachine returned [%d][0x%X]", status, status);
	//}
	//else
	//{
	//	if (vminfo.isVirtualMachine == VLS_NO_VIRTUAL_MACHINE_DETECTED)
	//		LogStatusMessage("RMS says the server is NOT running on a virtual machine.");
	//	else if (vminfo.isVirtualMachine == VLS_VIRTUAL_MACHINE_DETECTED)
	//		LogStatusMessage("RMS says the server IS running on a virtual machine.");
	//	else
	//		LogStatusMessage("VLSisVirtualMachine returned success but unkwon isVirtualMachine value.");
	//}
	// get server info
	serverInfo.structSz = sizeof(serverInfo);
	if ((status = VLSgetServInfoFP (inputData.serverName, &serverInfo, NULL, NULL)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSgetServInfo returned [%d][0x%X]", status, status);
	}
	else
	{
		LogStatusMessage("Information about the RMS server:\n");
		LogStatusMessage("Version     = [%d:%d:%d::%d]\n", serverInfo.major_no, serverInfo.minor_no,
			serverInfo.revision_no, serverInfo.build_no);
		LogStatusMessage("Vendor Info = [%s]\n", serverInfo.vendor_info);
		LogStatusMessage("Platform    = [%s]\n", serverInfo.platform);
		LogStatusMessage("locale      = [%s]\n", serverInfo.locale);
	}
	isAlreadyCalled = TRUE;
}

BOOL RMSRequestLicense(InputDataT input)
{
	LS_HANDLE handle;
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	if ((status = LSRequestFP (LS_ANY,
		"Sentinel RMS Utility", input.featureName, input.versionNumber, NULL,
		"calling from RMS utility",
		NULL, &handle)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: LSRequest returned [%d][0x%X]", status, status);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return FALSE;
	}
	else
	{
		LogStatusMessage("LSRequest returned successfully.");
	}

	LicenseInfoT stackData; 
	stackData.handle = handle;
	strcpy_s(stackData.featureName, MAX_PATH, input.featureName);
	strcpy_s(stackData.versionNumber, MAX_PATH, input.versionNumber);
	StackPush(rmsHandleStack, stackData);

	// request server info
	RMSGetServerInfo(input);
	RMSUpdateFeatureInfo(input.featureName, input.versionNumber);
	RMSGetAcrualServerName(handle);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

BOOL ReleaseRMSLicense()
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	if (RMSAreReleasePending () != TRUE)
	{
		LogStatusMessage("No licenses to release...");
		return FALSE;
	}

	StackNode node = StackPop(rmsHandleStack);
	LicenseInfoT stackData = node.data;

	if ((status = LSReleaseFP (stackData.handle, 1, "from RMS Utility")) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: LSRelease returned [%d][0x%X]", status, status);
		return FALSE;
	}
	else
	{
		LogStatusMessage("LSRelease returned successfully.");
	}
	RMSUpdateFeatureInfo(stackData.featureName, stackData.versionNumber);
	return TRUE;
}

BOOL RMSAreReleasePending()
{
	if (StackIsEmpty(rmsHandleStack) == TRUE)
		return FALSE;
	else
		return TRUE;
}

BOOL RMSSetTracePath(InputDataT inputData)
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	if (inputData.tracePath == NULL || strlen(inputData.tracePath) <= 0)
		return FALSE;

	LogStatusMessage("Enabling trace logging...");
	if ((status = VLSsetUserTraceFileFP(VLS_NULL, inputData.tracePath)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSsetUserTraceFile returned [%d][0x%X]", status, status);
		return FALSE;
	}
	else
	{
		LogStatusMessage("VLSsetUserTraceFile returned success");
	}
	if ((status = VLSsetTraceLevelFP(VLS_TRACE_ALL)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSsetTraceLevel returned [%d][0x%X]", status, status);
		return FALSE;
	}
	else
	{
		LogStatusMessage("VLSsetTraceLevel returned success");
	}
	return TRUE;
}

BOOL RMSSetContactServer(InputDataT inputData)
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	if (strlen(inputData.serverName) > 0 &&
		(status = VLSsetContactServerFP(inputData.serverName)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSsetContactServer returned [%d][0x%X] for server name [%s]",
			status, status, inputData.serverName);
		return FALSE;
	}
	LogStatusMessage("VLSsetContactServer returned success. Server set to [%s]", inputData.serverName);
	return TRUE;
}

BOOL RMSUpdateFeatureInfo (char *featureName, char* versionInfo)
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;
	VLSfeatureInfo fInfo;
	char buf[BUFSIZE];

	fInfo.structSz = sizeof(VLSfeatureInfo);

	if ((status = VLSgetFeatureInfoFP (featureName, versionInfo, 0, NULL, &fInfo)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSgetFeatureInfo returned [%d][0x%X] for feature name [%s] and version [%s]",
			status, status, featureName, versionInfo);
		return FALSE;
	}
	LogStatusMessage("VLSgetFeatureInfo returned success.");

	// update hwnd with the feature info
	if (fInfo.num_licenses == VLS_INFINITE_KEYS)
	{ 
		sprintf_s(buf, BUFSIZE, "[%s] / [%d] / [%s]",
			// free available
			"No Limit",
			// in use
			fInfo.lic_from_free_pool + fInfo.lic_from_resv,
			// total
			"No Limit");
	}
	else
	{
		sprintf_s(buf, BUFSIZE, "[%d] / [%d] / [%d]",
			// free available
			fInfo.num_licenses - (fInfo.lic_from_free_pool + fInfo.lic_from_resv),
			// in use
			fInfo.lic_from_free_pool + fInfo.lic_from_resv,
			// total
			fInfo.num_licenses);
	}
	//update the gui
	UpdateLicenseInfo(buf);
	return TRUE;
}

BOOL RMSGetAcrualServerName(LS_HANDLE handle)
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;
	char buf[BUFSIZE];

	if ((status = VLSgetServerNameFromHandleFP (handle, buf, BUFSIZE)) != LS_SUCCESS)
	{
		LogStatusMessage("FAIL: VLSgetServerNameFromHandle returned [%d][0x%X]",
			status, status);
		return FALSE;
	}
	LogStatusMessage("VLSgetServerNameFromHandle returned success.");

	//update the gui
	UpdateServerName(buf);
	return TRUE;
}