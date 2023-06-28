#include "stdafx.h"
#include "MyLoader.h"
#include "StringUtil.h"
#include "MyGlobals.h"
#include "version.h"
#include "MyUtils.h"
#include "MyHooks.h"
#include "MyHardPatch.h"


#define LDRGPA_TEST_PROC(ApiName) \
	if (StringUtil::StrCmpA(pProcName->Buffer, #ApiName) && gGlobals.gOriginalAPIs.##ApiName) \
	{ \
		if (pvProc) \
			*pvProc = (PVOID)&My##ApiName; \
	\
		return 0; \
	}


static BOOL CALLBACK LoaderTopEnumProc(HWND hwnd, LPARAM lParam);

static void LoaderBringProcessWindowToTop(DWORD dwPid);

static HINSTANCE LoaderGetRemoteBase();

static void InjectIntoNewProcess(HANDLE hProcess, HANDLE hMainThread, DWORD dwMainThreadId, bool fResume);

static void InjectIntoExplorerProcess(HANDLE hProcess, const char* pszStartApp);

static BOOL WINAPI MyLoaderCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
												  BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
												  LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2);

static BOOL WINAPI MyCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
									 BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
									 LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2);

static HINSTANCE WINAPI MyLoaderShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);;

static void CallRemoteExec(HANDLE hProcess, void* base, const char* pszStartApp);

static bool Is64BitOS();

static bool IsDllLoaded(PUNICODE_STRING ModuleFileName);

static bool SkipDllLoad(PUNICODE_STRING ModuleFileName);

static bool IsDllLoadedA(const char* pszModName);

static void FixModuleImports(const wchar_t* pszModName, HMODULE hModHandle);

static void LoaderInitModuleHooks(const wchar_t* pszModName, HMODULE hNewModule);

static bool TestForLoadedModules();

static void WINAPI MyExitProcess(UINT uCode);


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
 * \brief 
 */
ULONG __stdcall MyLdrInitializeThunk(PVOID pv1, PVOID pv2, PVOID pv3)
{
	ULONG res = gGlobals.gOriginalAPIs.LdrInitializeThunk(pv1, pv2, pv3);
	return res;
}




/**  
 * \brief Hooked entry point for LdrLoadDll which is used to detect when certain DLLs
 *        are loaded, such as kernel32, mswsock.dll, etc.
 */
ULONG __stdcall MyLdrLoadDll(IN PWCHAR PathToFile OPTIONAL, IN ULONG Flags OPTIONAL, IN PUNICODE_STRING ModuleFileName, OUT PHANDLE ModuleHandle)
{
	bool fFirstLoad = !IsDllLoaded(ModuleFileName);

	if (gGlobals.gdwInitialized == 0)
	{
		gGlobals.gdwInitialized = 1;
		gGlobals.gModHanldes.hNtDll = GetDllHandleW(L"ntdll.dll");

		ApiHook::Init();
		HardPatch_Init();
	}

	if (SkipDllLoad(ModuleFileName))
	{
		UNICODE_STRING strTemp;
		WCHAR buff[128];
		StringUtil::MemSet(buff, 0, 2*128);
		StringUtil::StrCopyW(buff, L"kernel32.dll");
		StringUtil::InitUnicodeString(&strTemp, buff);

		return gGlobals.gOriginalAPIs.LdrLoadDll(PathToFile, Flags, &strTemp, ModuleHandle);
	}

	HardPatch_ModulePreLoad();


	bool fCallInitHooks = TestForLoadedModules();

	ULONG Status = gGlobals.gOriginalAPIs.LdrLoadDll(PathToFile, Flags, ModuleFileName, ModuleHandle);


	if (fFirstLoad && ModuleHandle)
	{
		const wchar_t* pszModName = FindModuleName((HMODULE)*ModuleHandle);

		if (pszModName)
			FixModuleImports(pszModName, (HMODULE)*ModuleHandle);
	}


	if (fCallInitHooks && ModuleHandle)
	{
		const wchar_t* pszModName = FindModuleName((HMODULE)*ModuleHandle);

		if (pszModName)
			LoaderInitModuleHooks(pszModName, (HMODULE)*ModuleHandle);
	}


	HardPatch_ModulePostLoad();

	return Status;
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
			LDRGPA_TEST_PROC(GetNextWindow);

			LDRGPA_TEST_PROC(RegisterShellHookWindow);

			LDRGPA_TEST_PROC(WindowFromPoint);
			LDRGPA_TEST_PROC(ChildWindowFromPoint);

			LDRGPA_TEST_PROC(GetTopWindow);
		}
		else if (StringUtil::StrCmpIW(pszModName, L"mswsock.dll") && pProcName)
		{
			LDRGPA_TEST_PROC(WSPStartup);
		}
		else if (StringUtil::StrCmpIW(pszModName, L"kernel32.dll") && pProcName)
		{
			LDRGPA_TEST_PROC(EnumProcesses);
			LDRGPA_TEST_PROC(EnumProcessModules);
			LDRGPA_TEST_PROC(Process32FirstW);
			LDRGPA_TEST_PROC(Process32NextW);
//			LDRGPA_TEST_PROC(ExitProcess);
		}
	}

	return gGlobals.gOriginalAPIs.LdrGetProcedureAddress(hModule, pProcName, wOrd, pvProc);
}




/**  
 * \brief 
 */
BOOL WINAPI MyCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
											LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2)
{
	CreateProcessInternalWPtr proc = gGlobals.gOriginalAPIs.CreateProcessInternalW;

	DWORD dwMyFlags = dwCreationFlags | CREATE_SUSPENDED;

	if (lpCommandLine && StringUtil::StrFindIW(lpCommandLine, L"http://") >= 0)
		return FALSE;

	if (lpApplicationName && StringUtil::StrFindIW(lpApplicationName, L"http://") >= 0)
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
BOOL WINAPI MyLoaderCreateProcessInternalW(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
											BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, 
											LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2)
{
	CreateProcessInternalWPtr proc = gGlobals.gOriginalAPIs.CreateProcessInternalW;
	
	PROCESS_INFORMATION pi;
	StringUtil::MemSet(&pi, 0, sizeof(pi));

	BOOL ret = proc(unknown1, lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, CREATE_SUSPENDED, lpEnvironment, lpCurrentDirectory, lpStartupInfo, &pi, unknown2);

	if (!ret)
		return FALSE;

	InjectIntoNewProcess(pi.hProcess, pi.hThread, pi.dwThreadId, true);
	LoaderBringProcessWindowToTop(pi.dwProcessId);

	if (lpProcessInformation)
		StringUtil::MemCopy(lpProcessInformation, &pi, sizeof(pi));

	return TRUE;
}




/**  
 * \brief 
 */
HINSTANCE WINAPI MyLoaderShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
{
	HINSTANCE hInstance = LoaderGetRemoteBase();

	HWND hwndShell = FindWindowA("Shell_TrayWnd", 0);
	DWORD dwProcId = 0;

	GetWindowThreadProcessId(hwndShell, &dwProcId);


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcId);

	if (hProcess && hProcess != INVALID_HANDLE_VALUE)
	{
		if (hInstance != 0)
		{
			CallRemoteExec(hProcess, (void*)hInstance, lpFile);
		}
		else
		{
			InjectIntoExplorerProcess(hProcess, lpFile);
		}

		CloseHandle(hProcess);
	}

	return 0;
}




/**  
 * \brief 
 */
HINSTANCE LoaderGetRemoteBase()
{
	HINSTANCE hInstance = 0;
	HANDLE hFileMapping = OpenFileMappingA(FILE_MAP_READ, 0, "__LordOfMU__" __SOFTWARE_VERSION_STR);

	if (hFileMapping)
	{
		void** ppvInstance = (void**)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, sizeof(hInstance));

		if (ppvInstance)
		{
			hInstance = (HINSTANCE)*ppvInstance;
			UnmapViewOfFile((LPCVOID)ppvInstance);
		}

		CloseHandle(hFileMapping);
	}

	return hInstance;
}




/**  
 * \brief 
 */
void LoaderInitRemoteBase()
{
	gGlobals.ghFileMapping = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(gGlobals.ghInstance), "__LordOfMU__" __SOFTWARE_VERSION_STR);

	if (gGlobals.ghFileMapping != INVALID_HANDLE_VALUE)
	{
		void** ppvInstance = (void**)MapViewOfFile(gGlobals.ghFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(gGlobals.ghInstance));

		if (ppvInstance)
		{
			*ppvInstance = (void*)gGlobals.ghInstance;
			UnmapViewOfFile((LPCVOID)ppvInstance);
		}
	}
}



/**  
 * \brief 
 */
BOOL CALLBACK LoaderTopEnumProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwPid = 0;
	GetWindowThreadProcessId(hwnd, &dwPid);

	if (dwPid != *((DWORD*)lParam))
		return TRUE;

	*((DWORD*)lParam) = (DWORD)hwnd;
	return FALSE;
}




/**  
 * \brief 
 */
void LoaderBringProcessWindowToTop(DWORD dwPid)
{
	Sleep(500);

	DWORD dwPidHwnd = dwPid;

	int i = 3;
	for (; EnumWindows(LoaderTopEnumProc, (LPARAM)&dwPidHwnd) && i >= 0; i--)
	{
		Sleep(2000);
	}

	if (i >= 0)
	{
		BringWindowToTop((HWND)dwPidHwnd);
	}
}




/**  
 * \brief 
 */
DWORD __stdcall LoaderBindThreadProc(void* pData)
{
	if (pData)
	{
		ProcessUtil::BindImageImports(gGlobals.ghInstance);
		LoaderInitRemoteBase();
	}


	char szDir[MAX_PATH+1];
	StringUtil::MemSet(szDir, 0, sizeof(szDir));

	StringUtil::StrCopyA(szDir, gGlobals.gszStartAppName);
	for (int i=(int)StringUtil::StrLenA(szDir)-1; i>=0 && szDir[i] != '\\'; szDir[i--] = 0);


	HMODULE hKernel32 = ProcessUtil::FindModuleBase("kernel32.dll");
	FARPROC pfnCreateProcessW = ProcessUtil::FindFnAddressByName(hKernel32, "CreateProcessInternalW");
	ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfnCreateProcessW, (DWORD)&MyLoaderCreateProcessInternalW, (DWORD)&gGlobals.gOriginalAPIs.CreateProcessInternalW);


	HWND hwndShell = FindWindowA("Shell_TrayWnd", 0);
	SetForegroundWindow(hwndShell);

	ShellExecuteAPtr pfnShExec = (ShellExecuteAPtr)FindFnAddressByName(FindModuleBase("shell32.dll"), "ShellExecuteA");

	if (pfnShExec)
		pfnShExec(hwndShell, "open", gGlobals.gszStartAppName, 0, szDir, SW_SHOWDEFAULT);	

	ApiHook::RemoteUnPatch((HANDLE)-1, (DWORD)gGlobals.gOriginalAPIs.CreateProcessInternalW);
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

	ProcessUtil::GetNtApi(NtApi);

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

		CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)((DWORD)&LoaderBindThreadProc + ofs), (void*)1, 0, 0);
	}
}




/**  
 * \brief 
 */
void CallRemoteExec(HANDLE hProcess, void* base, const char* pszStartApp)
{
	void* hInstance = (void*)gGlobals.ghInstance;
	int ofs = (DWORD)base - (DWORD)hInstance;

	ULONG ulImgSize = 0;
	SIZE_T written = 0;


	ProcessUtil::TNtApiTable NtApi = {0};
	NtApi.NtWriteVirtualMemory = (ProcessUtil::NtWriteVirtualMemoryPtr)1;
	NtApi.NtReadVirtualMemory = (ProcessUtil::NtReadVirtualMemoryPtr)1;

	ProcessUtil::GetNtApi(NtApi);

	StringUtil::MemSet(gGlobals.gszStartAppName, 0, sizeof(gGlobals.gszStartAppName));
	StringUtil::StrCopyA(gGlobals.gszStartAppName, pszStartApp);

	NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)gGlobals.gszStartAppName + ofs), gGlobals.gszStartAppName, MAX_PATH, &written);

	CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)((DWORD)&LoaderBindThreadProc + ofs), (void*)0, 0, 0);
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
		DWORD dwApiInit = (DWORD)ProcessUtil::FindFnAddressByName(hNtDll, "LdrInitializeThunk");
		DWORD dwApiGetAddr = (DWORD)ProcessUtil::FindFnAddressByName(hNtDll, "LdrGetProcedureAddress");

		ApiHook::RemoteHotPatch(hProcess, dwApiDllLoad, (DWORD)&MyLdrLoadDll + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrLoadDll + ofs);
		ApiHook::RemoteHotPatch(hProcess, dwApiInit, (DWORD)&MyLdrInitializeThunk + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrInitializeThunk + ofs);
		ApiHook::RemoteHotPatch(hProcess, dwApiGetAddr, (DWORD)&MyLdrGetProcedureAddress + ofs, (DWORD)&gGlobals.gOriginalAPIs.LdrGetProcedureAddress + ofs);


		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.ghInstance + ofs), &new_base, 4, &written);
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)gGlobals.gszRootDir + ofs), gGlobals.gszRootDir, sizeof(gGlobals.gszRootDir), &written);
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.gdwMainThreadId + ofs), &dwMainThreadId, sizeof(dwMainThreadId), &written);


		DWORD dwPid = GetCurrentProcessId();
		HANDLE hThisProcess = OpenProcess(SYNCHRONIZE|PROCESS_ALL_ACCESS, 0, dwPid);
		HANDLE hTargetHandle = 0;
		
		DuplicateHandle(GetCurrentProcess(), hThisProcess, hProcess, &hTargetHandle, 0, 0, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
		
		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.ghParentProcess + ofs), &hTargetHandle, sizeof(dwPid), &written);
		

		char szFName[MAX_PATH+1];
		szFName[MAX_PATH] = 0;
		GetModuleFileNameA(0, szFName, MAX_PATH);

		DWORD dwPatch = 0;
		
		if (StringUtil::StrFindIA(szFName, "start-S2") >= 0)
			dwPatch = 1;

		NtApi.NtWriteVirtualMemory(hProcess, (LPVOID)((DWORD)&gGlobals.gdwPatchProcessEnum + ofs), &dwPatch, 4, &written);
	}

	if (fResume)
		WinApi.ResumeThread(hMainThread);
}




/**  
 * \brief 
 */
bool Is64BitOS()
{
	BOOL bIs64Bit = FALSE;

#if defined(_WIN64)

	bIs64Bit = TRUE;  // 64-bit programs run only on Win64

#elif defined(_WIN32)

	// Note that 32-bit programs run on both 32-bit and 64-bit Windows
	typedef BOOL (WINAPI *LPFNISWOW64PROCESS) (HANDLE, PBOOL);
	LPFNISWOW64PROCESS pfnIsWow64Process = (LPFNISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"), "IsWow64Process");

	if (pfnIsWow64Process)
		pfnIsWow64Process(GetCurrentProcess(), &bIs64Bit);

#endif

	return bIs64Bit == TRUE;
}




/**  
 * \brief 
 */
bool IsDllLoaded(PUNICODE_STRING ModuleFileName)
{
	char szBuffName[MAX_PATH+1];
	StringUtil::MemSet(szBuffName, 0, sizeof(szBuffName));

	bool fIsLoaded = false;

	if (ModuleFileName)
	{
		StringUtil::UnicodeToAnsi(szBuffName, ModuleFileName->Buffer, MAX_PATH, ModuleFileName->Length);

		int i = StringUtil::StrLenA(szBuffName)-1;
		for (i; i >= 0 && szBuffName[i] != '\\'; i--);

		fIsLoaded = (0 != FindModuleBase(szBuffName + i + 1));
	}

	return fIsLoaded;
}



/**  
 * \brief 
 */
bool SkipDllLoad(PUNICODE_STRING ModuleFileName)
{
	char szBuffName[MAX_PATH+1];
	StringUtil::MemSet(szBuffName, 0, sizeof(szBuffName));

	bool fSkipLoad = false;

	if (ModuleFileName)
	{
		StringUtil::UnicodeToAnsi(szBuffName, ModuleFileName->Buffer, MAX_PATH, ModuleFileName->Length);

		fSkipLoad = StringUtil::StrFindIA(szBuffName, "Clicker") >= 0 || StringUtil::StrFindIA(szBuffName, "addict2.dll") >= 0
			 || StringUtil::StrFindIA(szBuffName, "a2.dll") >= 0;
	}

	return fSkipLoad;
}


/**  
 * \brief 
 */
bool IsDllLoadedA(const char* pszModName)
{
	if (!pszModName)
		return false;

	int i = StringUtil::StrLenA(pszModName)-1;
	for (i; i >= 0 && pszModName[i] != '\\'; i--);

	return 0 != FindModuleBase(pszModName + i + 1);
}


/**  
 * \brief 
 */
void FixModuleImports(const wchar_t* pszModName, HMODULE hModHandle)
{
	char szBuffName[MAX_PATH+1];
	StringUtil::MemSet(szBuffName, 0, sizeof(szBuffName));

	if (pszModName)
	{
		StringUtil::UnicodeToAnsi(szBuffName, pszModName, MAX_PATH);
		ProcessUtil::BindImportsForModule(gGlobals.ghInstance, szBuffName, hModHandle);
	}
}


/**  
 * \brief 
 */
static void __declspec(naked) MyFindWindowRet()
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
void WINAPI MyExitProcess(UINT uCode)
{
	if (IsWindow(gGlobals.ghwndGame))
	{
		SuspendThread(GetCurrentThread());
		return;
	}

	gGlobals.gOriginalAPIs.ExitProcess(uCode);
}



/**  
 * \brief 
 */
void LoaderInitModuleHooks(const wchar_t* pszModName, HMODULE hNewModule)
{
	if (!gGlobals.gModHanldes.hKernel32 && 0 != (gGlobals.gModHanldes.hKernel32 = FindModuleBase("kernel32.dll")))
	{
		KernelApiHotPatch(CreateProcessInternalW);
//		KernelApiHotPatch(ExitProcess);

//		KernelApiHotPatch(EnumProcesses);
//		KernelApiHotPatch(EnumProcessModules);
//		KernelApiHotPatch(Process32FirstW);
//		KernelApiHotPatch(Process32NextW);

//		LdrLoadDllA("user32.dll");
	}
	 

	if (!gGlobals.gModHanldes.hUser32 && 0 != (gGlobals.gModHanldes.hUser32 = FindModuleBase("user32.dll")))
	{
		ApiHotPatch2(gGlobals.gModHanldes.hUser32, GetAsyncKeyState);
		ApiHotPatch2(gGlobals.gModHanldes.hUser32, ChangeDisplaySettingsA);
		UserApiHotPatch(GetForegroundWindow);

		UserApiHotPatch(SetWindowsHookExA);
		UserApiHotPatch(SetWindowsHookExW);
		
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

		LdrLoadDllA("mswsock.dll");
		LdrLoadDllA("ws2_32.dll");
	}


	if (!gGlobals.gModHanldes.hShell32 && 0 != (gGlobals.gModHanldes.hShell32 = FindModuleBase("shell32.dll")))
	{

	}
	

	if (!gGlobals.gModHanldes.hMswsock && 0 != (gGlobals.gModHanldes.hMswsock = FindModuleBase("mswsock.dll")))
	{
		WinsockApiHotPatch(WSPStartup);
	}


	if (!gGlobals.gModHanldes.hWs2_32 && 0 != (gGlobals.gModHanldes.hWs2_32 = FindModuleBase("ws2_32.dll")))
	{
		LdrLoadDllA("mswsock.dll");
	}


	if (!gGlobals.gModHanldes.hOpenGL32 && 0 != (gGlobals.gModHanldes.hOpenGL32 = FindModuleBase("opengl32.dll")))
	{
		OpenglApiHotPatch(wglSwapBuffers);
	}	
	

	if (!gGlobals.gModHanldes.hPsapi && 0 != (gGlobals.gModHanldes.hPsapi = FindModuleBase("psapi.dll")))
	{

	}
}



/**  
 * \brief 
 */
bool TestForLoadedModules()
{
	if (!IsDllLoadedA("kernel32.dll"))
		gGlobals.gModHanldes.hKernel32 = 0;

	if (!IsDllLoadedA("shell32.dll"))
		gGlobals.gModHanldes.hShell32 = 0;

	if (!IsDllLoadedA("user32.dll"))
		gGlobals.gModHanldes.hUser32 = 0;

	if (!IsDllLoadedA("mswsock.dll"))
		gGlobals.gModHanldes.hMswsock = 0;

	if (!IsDllLoadedA("ws2_32.dll"))
		gGlobals.gModHanldes.hWs2_32 = 0;

	if (!IsDllLoadedA("opengl32.dll"))
		gGlobals.gModHanldes.hOpenGL32 = 0;

	if (!IsDllLoadedA("psapi.dll"))
		gGlobals.gModHanldes.hPsapi = 0;
	
	return gGlobals.gModHanldes.hPsapi == 0 || gGlobals.gModHanldes.hOpenGL32 == 0 || gGlobals.gModHanldes.hWs2_32 == 0
			|| gGlobals.gModHanldes.hMswsock == 0 || gGlobals.gModHanldes.hUser32 == 0 || gGlobals.gModHanldes.hShell32 == 0 || gGlobals.gModHanldes.hKernel32 == 0;
}