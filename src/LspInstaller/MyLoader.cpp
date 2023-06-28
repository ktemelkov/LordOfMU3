#include "stdafx.h"
#include "MyLoader.h"
#include "StringUtil.h"
#include "MyGlobals.h"
#include "version.h"
#include "MyUtils.h"
#include "MyHooks.h"




#define LDRGPA_TEST_PROC(ApiName) \
	if (StringUtil::StrCmpA(pProcName->Buffer, #ApiName) && gGlobals.gOriginalAPIs.##ApiName) \
	{ \
	if (pvProc) \
	*pvProc = (PVOID)&My##ApiName; \
	\
	return 0; \
	}



static void InjectIntoNewProcess(HANDLE hProcess, HANDLE hMainThread, DWORD dwMainThreadId, bool fResume);

static void InjectIntoExplorerProcess(HANDLE hProcess, const char* pszStartApp);

static BOOL WINAPI MyLoaderCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
												  BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
												  LPPROCESS_INFORMATION lpProcessInformation);

static BOOL WINAPI MyCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
									 BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
									 LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2);

static HINSTANCE WINAPI MyLoaderShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);;

static void LoaderInitModuleHooks();



/**  
 * \brief 
 */
bool InitLoader(HMODULE hInst, const char* pszRootDir)
{
	StringUtil::MemSet(&gGlobals, 0, sizeof(gGlobals));

	gGlobals.ghInstance = hInst;
	StringUtil::StrCopyA(gGlobals.gszRootDir, pszRootDir);


	if (Is64BitOS())
	{
		HMODULE hKernel32 = ProcessUtil::FindModuleBase("kernel32.dll");
		FARPROC pfnCreateProcessW = ProcessUtil::FindFnAddressByName(hKernel32, "CreateProcessInternalW");

		ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfnCreateProcessW, (DWORD)&MyCreateProcessInternalW, (DWORD)&gGlobals.gOriginalAPIs.CreateProcessInternalW);
	}
	else
	{
		HMODULE hShell32 = ProcessUtil::FindModuleBase("shell32.dll");
		FARPROC pfnShellExecA = ProcessUtil::FindFnAddressByName(hShell32, "ShellExecuteA");

		ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfnShellExecA, (DWORD)&MyLoaderShellExecuteA, (DWORD)&gGlobals.gOriginalAPIs.ShellExecuteA);
	}

	return true;
}




/**  
 * \brief Hooked entry point for LdrLoadDll which is used to detect when certain DLLs
 *        are loaded, such as kernel32, mswsock.dll, etc.
 */
ULONG __stdcall MyLdrLoadDll(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle)
{
	if (SkipDllLoad(ModuleFileName))
	{
		UNICODE_STRING strTemp;
		WCHAR buff[128];
		StringUtil::MemSet(buff, 0, 2*128);
		StringUtil::StrCopyW(buff, L"kernel32.dll");
		StringUtil::InitUnicodeString(&strTemp, buff);

		return gGlobals.gOriginalAPIs.LdrLoadDll(PathToFile, Flags, &strTemp, ModuleHandle);
	}

	ULONG Status = gGlobals.gOriginalAPIs.LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);

	LoaderInitModuleHooks();

	return Status;
}



/**  
 * \brief 
 */
bool IsAllowedApplicationName(const wchar_t* lpwstrAppName)
{
	if (!lpwstrAppName)
		return true;

	return StringUtil::StrFindIW(lpwstrAppName, L"http://") < 0 
				&& StringUtil::StrFindIW(lpwstrAppName, L"iexplore.exe") < 0;
}



/**  
 * \brief 
 */
BOOL WINAPI MyCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
											LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2)
{
	CreateProcessInternalWPtr proc = gGlobals.gOriginalAPIs.CreateProcessInternalW;

	DWORD dwMyFlags = dwCreationFlags;

	if (!IsAllowedApplicationName(lpCommandLine) || !IsAllowedApplicationName(lpApplicationName))
		return FALSE;
	
	PROCESS_INFORMATION pi;
	StringUtil::MemSet(&pi, 0, sizeof(pi));

	BOOL ret = proc(unknown1, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwMyFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, &pi, unknown2);

	if (!ret)
		return FALSE;

	InjectIntoNewProcess(pi.hProcess, pi.hThread, pi.dwThreadId, (dwCreationFlags & CREATE_SUSPENDED) == 0);


	if (lpProcessInformation)
		StringUtil::MemCopy(lpProcessInformation, &pi, sizeof(pi));

	return TRUE;
}




/**  
 * \brief 
 */
BOOL WINAPI MyLoaderCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
											LPPROCESS_INFORMATION lpProcessInformation)
{
	CreateProcessWPtr proc = gGlobals.gOriginalAPIs.CreateProcessW;
	
	PROCESS_INFORMATION pi;
	StringUtil::MemSet(&pi, 0, sizeof(pi));

	BOOL ret = proc(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags | CREATE_SUSPENDED, lpEnvironment, lpCurrentDirectory, lpStartupInfo, &pi);

	if (!ret)
		return FALSE;

	InjectIntoNewProcess(pi.hProcess, pi.hThread, pi.dwThreadId, true);

	if (lpProcessInformation)
		StringUtil::MemCopy(lpProcessInformation, &pi, sizeof(pi));

	return TRUE;
}




/**  
 * \brief 
 */
HINSTANCE WINAPI MyLoaderShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	HWND hwndShell = FindWindowA("Shell_TrayWnd", 0);
	DWORD dwProcId = 0;

	GetWindowThreadProcessId(hwndShell, &dwProcId);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcId);

	if (hProcess && hProcess != INVALID_HANDLE_VALUE)
	{
		InjectIntoExplorerProcess(hProcess, lpFile);
		CloseHandle(hProcess);
	}

	return 0;
}




/**  
 * \brief 
 */
DWORD __stdcall LoaderBindThreadProc(void*)
{
	ProcessUtil::BindImageImports(gGlobals.ghInstance);


	HMODULE hKernel32 = ProcessUtil::FindModuleBase("kernel32.dll");
	FARPROC pfnCreateProcessW = ProcessUtil::FindFnAddressByName(hKernel32, "CreateProcessW");
	ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfnCreateProcessW, (DWORD)&MyLoaderCreateProcessW, (DWORD)&gGlobals.gOriginalAPIs.CreateProcessW);

	HWND hwndShell = FindWindowA("Shell_TrayWnd", 0);
	SetForegroundWindow(hwndShell);

	ShellExecuteWPtr pfnShExec = (ShellExecuteWPtr)FindFnAddressByName(FindModuleBase("shell32.dll"), "ShellExecuteW");

	if (pfnShExec)
	{
		WCHAR szAppName[MAX_PATH+1];
		StringUtil::MemSet(szAppName, 0, sizeof(szAppName));
		StringUtil::AnsiToUnicode(szAppName, gGlobals.gszStartAppName, MAX_PATH);

		WCHAR szDir[MAX_PATH+1];
		StringUtil::MemSet(szDir, 0, sizeof(szDir));

		StringUtil::StrCopyW(szDir, szAppName);
		for (int i=(int)StringUtil::StrLenW(szDir)-1; i>=0 && szDir[i] != L'\\'; szDir[i--] = 0);


		pfnShExec(hwndShell, L"open", szAppName, 0, szDir, SW_SHOWDEFAULT);	
	}

	ApiRemoveHotPatch(CreateProcessW);
	return 0;
}




/**  
 * \brief 
 */
void InjectIntoExplorerProcess(HANDLE hProcess, const char* pszStartApp)
{
	SIZE_T written = 0;
	void* hInstance = (void*)gGlobals.ghInstance;
	void* new_base = 0;
	ULONG ulImgSize = 0;
	int ofs = 0;


	ProcessUtil::TNtApiTable NtApi = {0};
	NtApi.NtWriteVirtualMemory = (ProcessUtil::NtWriteVirtualMemoryPtr)1;
	NtApi.NtReadVirtualMemory = (ProcessUtil::NtReadVirtualMemoryPtr)1;
	NtApi.NtFreeVirtualMemory = (ProcessUtil::NtFreeVirtualMemoryPtr)1;

	if (!ProcessUtil::GetNtApi(NtApi))
		return;

	if (ProcessUtil::ReplicateImageIntoRemoteProcess(hProcess, (void*)hInstance, &new_base, &ulImgSize))
	{
		ofs = (DWORD)new_base - (DWORD)hInstance;

		TGlobals newGlobals;
		StringUtil::MemSet(&newGlobals, 0, sizeof(newGlobals));
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals + ofs), &newGlobals, sizeof(newGlobals), &written);


		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.ghInstance + ofs), &new_base, 4, &written);
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)gGlobals.gszRootDir + ofs), gGlobals.gszRootDir, sizeof(gGlobals.gszRootDir), &written);

		StringUtil::MemSet(gGlobals.gszStartAppName, 0, sizeof(gGlobals.gszStartAppName));
		StringUtil::StrCopyA(gGlobals.gszStartAppName, pszStartApp);

		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)gGlobals.gszStartAppName + ofs), gGlobals.gszStartAppName, MAX_PATH, &written);

		HANDLE hRemThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)((DWORD)&LoaderBindThreadProc + ofs), (void*)1, 0, 0);

		if (hRemThread && hRemThread != INVALID_HANDLE_VALUE)
		{
			if (WaitForSingleObject(hRemThread, 15000) != WAIT_TIMEOUT)
			{
				CloseHandle(hRemThread);

				ulImgSize = 0;
				NtApi.NtFreeVirtualMemory(hProcess, &new_base, &ulImgSize, MEM_RELEASE);
			}
		}
	}
}



/**  
 * \brief 
 */
void InjectIntoNewProcess(HANDLE hProcess, HANDLE hMainThread, DWORD dwMainThreadId, bool fResume)
{
	SIZE_T written = 0;
	void* hInstance = (void*)gGlobals.ghInstance;
	void* new_base = 0;
	ULONG ulImgSize = 0;
	int ofs = 0;


	ProcessUtil::TWinApiTable WinApi = {0};
	WinApi.ResumeThread = (ProcessUtil::ResumeThreadPtr)1;

	ProcessUtil::TNtApiTable NtApi = {0};
	NtApi.NtWriteVirtualMemory = (ProcessUtil::NtWriteVirtualMemoryPtr)1;
	NtApi.NtReadVirtualMemory = (ProcessUtil::NtReadVirtualMemoryPtr)1;


	ProcessUtil::GetWinApi(WinApi);
	ProcessUtil::GetNtApi(NtApi);


	if (ProcessUtil::ReplicateImageIntoRemoteProcess(hProcess, (void*)hInstance, &new_base, &ulImgSize))
	{
		ofs = (DWORD)new_base - (DWORD)hInstance;

		TGlobals newGlobals;
		StringUtil::MemSet(&newGlobals, 0, sizeof(newGlobals));
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals + ofs), &newGlobals, sizeof(newGlobals), &written);

		HMODULE hNtDll = ProcessUtil::FindModuleBase("ntdll.dll");

		DWORD dwApiDllLoad = (DWORD)ProcessUtil::FindFnAddressByName(hNtDll, "LdrLoadDll");
		ApiHook::RemoteHotPatch(hProcess, dwApiDllLoad, (DWORD)&MyLdrLoadDll + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrLoadDll + ofs);

		DWORD dwApiDllFree = (DWORD)ProcessUtil::FindFnAddressByName(hNtDll, "LdrUnloadDll");
		ApiHook::RemoteHotPatch(hProcess, dwApiDllFree, (DWORD)&MyLdrUnloadDll + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrUnloadDll + ofs);

//		DWORD dwApiGetAddr = (DWORD)ProcessUtil::FindFnAddressByName(hNtDll, "LdrGetProcedureAddress");
//		ApiHook::RemoteHotPatch(hProcess, dwApiGetAddr, (DWORD)&MyLdrGetProcedureAddress + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrGetProcedureAddress + ofs);

		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.ghInstance + ofs), &new_base, 4, &written);
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)gGlobals.gszRootDir + ofs), gGlobals.gszRootDir, sizeof(gGlobals.gszRootDir), &written);
	}

	if (fResume)
		WinApi.ResumeThread(hMainThread);
}




/**  
 * \brief 
 */
void __declspec(naked) MyFindWindowRet()
{
	__asm
	{
		push eax

		call ShouldHideWindow
		cmp al, 0
		pop eax
		jz MyFindWindowRet_End

		mov eax, 0

MyFindWindowRet_End:
		ret
	}
}


/**  
 * \brief 
 */
void __declspec(naked) MyGetAsyncKeyStateRet()
{
	__asm
	{
		push ebx
		push eax
		mov eax, gGlobals.gHookInfo.GetAsyncKeyStateHook2
		cmp eax, 0
		jz MyGetAsyncKeyStateRet_End1

		mov ebx, [esp + 16]
		push ebx
		call [gGlobals.gHookInfo.GetAsyncKeyStateHook2]
		jmp MyGetAsyncKeyStateRet_End

MyGetAsyncKeyStateRet_End1:
		pop eax

MyGetAsyncKeyStateRet_End:
		pop ebx
		ret
	}
}


/**  
 * \brief 
 */
int __stdcall Myrecv(SOCKET s, char *buf, int len, int flags)
{
	__asm
	{
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	}

	int j=0;
	for (int i=0; i < 1000; i++)
	{
		j++;
	}

//	if (gGlobals.gOriginalAPIs.recv)
//		return gGlobals.gOriginalAPIs.recv(s, buf, len, flags);

	return -1;
}



/**  
 * \brief 
 */
void LoaderInitModuleHooks()
{
	if (!gGlobals.gModHanldes.hKernel32 && 0 != (gGlobals.gModHanldes.hKernel32 = FindModuleBase("kernel32.dll")))
	{
		FixModuleImports(L"kernel32.dll", gGlobals.gModHanldes.hKernel32);

		ApiHook::Init();
		CollectIatHardPatchInfo();

		KernelApiHotPatch(CreateProcessInternalW);
//		KernelApiHotPatch(VirtualProtectEx);
	}
	 

	if (!gGlobals.gModHanldes.hUser32 && 0 != (gGlobals.gModHanldes.hUser32 = FindModuleBase("user32.dll")))
	{
		FixModuleImports(L"user32.dll", gGlobals.gModHanldes.hUser32);

//		UserApiRetnPatch("GetAsyncKeyState", MyGetAsyncKeyStateRet);
		ApiHotPatch2(gGlobals.gModHanldes.hUser32, GetAsyncKeyState);
		ApiHotPatch2(gGlobals.gModHanldes.hUser32, ChangeDisplaySettingsA);
		ApiHotPatch2(gGlobals.gModHanldes.hUser32, GetForegroundWindow);

		UserApiHotPatch(SetWindowsHookExA);
		UserApiHotPatch(SetWindowsHookExW);

/*
		UserApiRetnPatch("FindWindowA", MyFindWindowRet);
		UserApiRetnPatch("FindWindowW", MyFindWindowRet);
		UserApiRetnPatch("FindWindowExA", MyFindWindowRet);
		UserApiRetnPatch("FindWindowExW", MyFindWindowRet);

		UserApiHotPatch(EnumChildWindows);
		UserApiHotPatch(EnumThreadWindows);
		UserApiHotPatch(EnumWindows);

		UserApiHotPatch(RegisterShellHookWindow);

		UserApiHotPatch(WindowFromPoint);
		UserApiHotPatch(ChildWindowFromPoint);

		UserApiHotPatch(GetTopWindow);
*/
	}
	
	if (!gGlobals.gModHanldes.hMswsock && 0 != (gGlobals.gModHanldes.hMswsock = FindModuleBase("mswsock.dll")))
	{
		WinsockApiHotPatch(WSPStartup);
	}

	if (!gGlobals.gModHanldes.hWs2_32 && 0 != (gGlobals.gModHanldes.hWs2_32 = FindModuleBase("ws2_32.dll")))
	{
		CollectWs2HardPatchInfo();
	}

	if (!gGlobals.gModHanldes.hGdi32 && 0 != (gGlobals.gModHanldes.hGdi32 = FindModuleBase("GDI32.dll")))
	{
		GdiApiHotPatch(SetPixelFormat);
	}	
}



/**  
 * \brief 
 */
ULONG __stdcall MyLdrUnloadDll(HANDLE ModuleHandle)
{
	ULONG ret = gGlobals.gOriginalAPIs.LdrUnloadDll(ModuleHandle);

	if (gGlobals.gModHanldes.hMswsock != 0 && 0 == FindModuleBase("mswsock.dll"))
	{
		gGlobals.gModHanldes.hMswsock = 0;

		MemoryFreeLibrary(gGlobals.gModHanldes.hMemLib);
		gGlobals.gModHanldes.hMemLib = 0;

		ApiFreeHotPatchData(WSPStartup);
	}
	else if (gGlobals.gModHanldes.hWs2_32 != 0 && 0 == FindModuleBase("ws2_32.dll"))
	{
		gGlobals.gModHanldes.hWs2_32 = 0;
		gGlobals.gOriginalAPIs.recv = 0;
		gGlobals.gOriginalAPIs.send = 0;
		gGlobals.gHardPatch.ws2_original_send = 0;
		gGlobals.gHardPatch.ws2_last_send = 0;
		gGlobals.gHardPatch.ws2_original_recv = 0;
	}

	return ret;
}



/**  
 * \brief 
 */
ULONG __stdcall MyLdrGetProcedureAddress(HMODULE hModule, PANSI_STRING pProcName, WORD wOrd, PVOID* pvProc)
{
	const wchar_t* pszModName = FindModuleName(hModule);

	if (pszModName)
	{
		if (StringUtil::StrCmpIW(pszModName, L"user32.dll") && pProcName)
		{
			LDRGPA_TEST_PROC(SetWindowsHookExA);
			LDRGPA_TEST_PROC(SetWindowsHookExW);

			LDRGPA_TEST_PROC(EnumChildWindows);
			LDRGPA_TEST_PROC(EnumThreadWindows);
			LDRGPA_TEST_PROC(EnumWindows);

			LDRGPA_TEST_PROC(FindWindowA);
			LDRGPA_TEST_PROC(FindWindowW);
			LDRGPA_TEST_PROC(FindWindowExA);
			LDRGPA_TEST_PROC(FindWindowExW);

			LDRGPA_TEST_PROC(GetWindow);

			LDRGPA_TEST_PROC(RegisterShellHookWindow);

			LDRGPA_TEST_PROC(WindowFromPoint);
			LDRGPA_TEST_PROC(ChildWindowFromPoint);

			LDRGPA_TEST_PROC(GetTopWindow);
		}
		else if (StringUtil::StrCmpIW(pszModName, L"ws2_32.dll") && pProcName)
		{
			/*
			if (StringUtil::StrCmpA(pProcName->Buffer, "recv"))
			{
				gGlobals.gOriginalAPIs.recv = (recvPtr)FindFnAddressByName(gGlobals.gModHanldes.hWs2_32, "recv");

				if (pvProc)
					*pvProc = (PVOID)&Myrecv;

				return 0;
			}
			*/
//			LDRGPA_TEST_PROC(recv);
		}
		else if (StringUtil::StrCmpIW(pszModName, L"mswsock.dll") && pProcName)
		{
			LDRGPA_TEST_PROC(WSPStartup);
		}
	}

	return gGlobals.gOriginalAPIs.LdrGetProcedureAddress(hModule, pProcName, wOrd, pvProc);
}

