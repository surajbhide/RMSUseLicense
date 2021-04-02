#include <windows.h> 
#include "RMSFunctions.h"
#include "lserv.h"

// prototypes for the functions we need
typedef void(__cdecl *VLSinitializeT)();
typedef void(__cdecl *VLScleanupT)();
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
typedef LS_STATUS_CODE(__cdecl *VLSisVirtualMachineT)(VLSVMInfo *vm_info);
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

VLSinitializeT VLSinitializeFP;
VLScleanupT VLScleanupFP;
VLSsetUserTraceFileT VLSsetUserTraceFileFP;
VLSsetTraceLevelT VLSsetTraceLevelFP;
VLSgetLibInfoT VLSgetLibInfoFP;
VLScontrolRemoteSessionT VLScontrolRemoteSessionFP;
VLSgetServInfoT VLSgetServInfoFP;
VLSsetContactServerT VLSsetContactServerFP;
VLSisVirtualMachineT VLSisVirtualMachineFP;
LSRequestT LSRequestFP;
LSReleaseT LSReleaseFP;

HINSTANCE rmsLib = NULL;

BOOL LoadRMSDll()
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
	VLSisVirtualMachineFP = (VLSisVirtualMachineT)GetProcAddress(rmsLib, "VLSisVirtualMachine");
	LSRequestFP = (LSRequestT)GetProcAddress(rmsLib, "LSRequest");
	LSReleaseFP = (LSReleaseT)GetProcAddress(rmsLib, "LSRelease");

	if (!VLSinitializeFP || !VLScleanupFP || !VLSsetUserTraceFileFP ||
		!VLSsetTraceLevelFP || !VLSgetLibInfoFP || !VLScontrolRemoteSessionFP ||
		!VLSgetServInfoFP || !VLSsetContactServerFP || !VLSisVirtualMachineFP ||
		!LSRequestFP || !LSReleaseFP)
	{
		return FALSE;
	}
	return TRUE;
}

void UnloadRMSDll()
{
	if (rmsLib != NULL)
	{
		FreeLibrary(rmsLib);
	}
}

BOOL CallInitMethodsInRMS()
{
	// calls vlsinitialize/set trace/get lib version etc
	VLSinitializeFP();
	return TRUE;
}

BOOL CallRMSCleanupMethods()
{
	// call vlscleanup here
	VLScleanupFP();
	return TRUE;
}