#pragma once

#include <tlhelp32.h>


#define ApiHotPatch(DllHandle, ApiName) { FARPROC pfn = FindFnAddressByName(DllHandle, #ApiName); if (pfn) ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfn, (DWORD)&My##ApiName, (DWORD)&gGlobals.gOriginalAPIs.##ApiName); }
#define ApiExportPatch(DllHandle, ApiName) { ApiHook::PatchExport(DllHandle, #ApiName, (PVOID)&My##ApiName, (PVOID*)&gGlobals.gOriginalAPIs.##ApiName); }


#define ApiHotPatch2(DllHandle, ApiName) \
{ \
	FARPROC pfn = FindFnAddressByName(DllHandle, #ApiName); \
	if (pfn)  \
	{ \
		DWORD dwHook = (DWORD)ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.##ApiName##Hook, (PVOID*)&gGlobals.gOriginalAPIs.##ApiName); \
		if (dwHook) \
			ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfn, dwHook, (DWORD)&gGlobals.gOriginalAPIs.##ApiName);  \
	} \
}


#define ApiExportPatch2(DllHandle, ApiName) \
							{ ApiHook::PatchExport(DllHandle, #ApiName,  \
												   (PVOID)ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.##ApiName##Hook, (PVOID*)&gGlobals.gOriginalAPIs.##ApiName),  \
												   (PVOID*)&gGlobals.gOriginalAPIs.##ApiName); }


#define ApiRetnPatch(DllHandle, ApiName, PatchName) { FARPROC pfn = FindFnAddressByName(DllHandle, #ApiName); if (pfn) ApiHook::PatchRetn((DWORD_PTR)pfn, (DWORD_PTR)&##PatchName); }



#define NtApiHotPatch(ApiName)		ApiHotPatch(gGlobals.gModHanldes.hNtDll, ApiName)
#define KernelApiHotPatch(ApiName)	ApiHotPatch(gGlobals.gModHanldes.hKernel32, ApiName)
#define UserApiHotPatch(ApiName)	ApiHotPatch(gGlobals.gModHanldes.hUser32, ApiName)
#define ShellApiHotPatch(ApiName)	ApiHotPatch(gGlobals.gModHanldes.hShell32, ApiName)
#define WinsockApiHotPatch(ApiName) ApiHotPatch(gGlobals.gModHanldes.hMswsock, ApiName)
#define Ws2_32ApiHotPatch(ApiName)  ApiHotPatch(gGlobals.gModHanldes.hWs2_32, ApiName)
#define OpenglApiHotPatch(ApiName)	ApiHotPatch(gGlobals.gModHanldes.hOpenGL32, ApiName)
#define PsapiApiHotPatch(ApiName)	ApiHotPatch(gGlobals.gModHanldes.hPsapi, ApiName)


#define NtApiExportPatch(ApiName)		ApiExportPatch(gGlobals.gModHanldes.hNtDll, ApiName)
#define KernelApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hKernel32, ApiName)
#define UserApiExportPatch(ApiName)		ApiExportPatch(gGlobals.gModHanldes.hUser32, ApiName)
#define ShellApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hShell32, ApiName)
#define WinsockApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hMswsock, ApiName)
#define Ws2_32ApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hWs2_32, ApiName)
#define OpenglApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hOpenGL32, ApiName)
#define PsapiApiExportPatch(ApiName)	ApiExportPatch(gGlobals.gModHanldes.hPsapi, ApiName)


#define UserApiRetnPatch(ApiName, PatchName)	ApiRetnPatch(gGlobals.gModHanldes.hUser32, ApiName, PatchName)


void KillLauncher();
bool ShouldHideWindow(HWND hwnd);
void DoModuleInit();


BOOL FixProcessScan(LPPROCESSENTRY32W lppe);
