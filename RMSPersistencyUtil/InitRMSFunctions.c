#include <windows.h> 
#include "InitRMSFunctions.h"
#include "lspmgmtinit.h"
#include "MiscUtils.h"
#include "CommonDefs.h"
#include "resource.h"

// prototypes for the functions we need
typedef sntl_persistence_status_t(__cdecl* sntl_persistence_context_newT)(sntl_persistence_device_t device, sntl_persistence_attr_t* attr, sntl_persistence_context_t** context);
typedef sntl_persistence_status_t(__cdecl* sntl_persistence_createT)(sntl_persistence_context_t* context, char* input);
typedef sntl_persistence_status_t(__cdecl* sntl_persistence_get_infoT)(sntl_persistence_context_t* context,	const char* scope, const char* query, char** info);
typedef sntl_persistence_status_t(__cdecl* sntl_persistence_context_deleteT)(sntl_persistence_context_t* context);
typedef sntl_persistence_status_t(__cdecl* sntl_persistence_cleanupT)(void);
typedef void(__cdecl* sntl_persistence_freeT)(void* buffer);

// globals
sntl_persistence_context_newT sntl_persistence_context_newFP;
sntl_persistence_createT sntl_persistence_createFP;
sntl_persistence_get_infoT sntl_persistence_get_infoFP;
sntl_persistence_context_deleteT sntl_persistence_context_deleteFP;
sntl_persistence_cleanupT sntl_persistence_cleanupFP;
sntl_persistence_freeT sntl_persistence_freeFP;

static HINSTANCE rmsLib = NULL;
static BOOL rmsInitialized = FALSE;
#define DLL_NAME "lspmgmtinit64.dll"

BOOL RMSPersistenceInitLoadDll()
{
	// open the rms dll from the current directory.
	rmsLib = LoadLibrary(TEXT(DLL_NAME));

	// error out on invalid handle
	if (rmsLib == NULL)
	{
		return FALSE;
	}
	sntl_persistence_context_newFP = (sntl_persistence_context_newT)GetProcAddress(rmsLib, "sntl_persistence_context_new");
	sntl_persistence_createFP = (sntl_persistence_createT)GetProcAddress(rmsLib, "sntl_persistence_create");
	sntl_persistence_get_infoFP = (sntl_persistence_get_infoT)GetProcAddress(rmsLib, "sntl_persistence_get_info");
	sntl_persistence_context_deleteFP = (sntl_persistence_context_deleteT)GetProcAddress(rmsLib, "sntl_persistence_context_delete");
	sntl_persistence_cleanupFP = (sntl_persistence_cleanupT)GetProcAddress(rmsLib, "sntl_persistence_cleanup");
	sntl_persistence_freeFP = (sntl_persistence_freeT)GetProcAddress(rmsLib, "sntl_persistence_free");

	if (!sntl_persistence_context_newFP || !sntl_persistence_createFP || !sntl_persistence_get_infoFP ||
		!sntl_persistence_context_deleteFP || !sntl_persistence_cleanupFP || !sntl_persistence_freeFP
	   )
	{
		return FALSE;
	}
	LogStatusMessage("Loaded [%s] from [%s]...", DLL_NAME, GetRMSDllLocation());
	LogStatusMessage("Imported RMS Persistence Init methods successfully.");
	return TRUE;
}

void RMSPersistenceInitUnloadDll()
{
	if (rmsLib != NULL)
	{
		FreeLibrary(rmsLib);
	}
}

/*
BOOL RMSCallInitMethods()
{
	LS_STATUS_CODE status = LS_NO_SUCCESS;

	if (rmsInitialized == TRUE)
		return TRUE;

	LogStatusMessage("Initializing RMS...");

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
	rmsHandleStack = NULL;
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
		LogStatusMessage("Version     = [%d.%d.%d.%d]\n", serverInfo.major_no, serverInfo.minor_no,
			serverInfo.revision_no, serverInfo.build_no);
		LogStatusMessage("Vendor Info = [%s]\n", serverInfo.vendor_info);
		LogStatusMessage("Platform    = [%s]\n", serverInfo.platform);
		LogStatusMessage("locale      = [%s]\n", serverInfo.locale);
	}
	isAlreadyCalled = TRUE;
}
*/