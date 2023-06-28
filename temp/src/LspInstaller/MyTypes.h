#pragma once


#include "MemoryModule.h"
#include "ProcessUtil.h"
#include "LoaderInterface.h"
#include "ApiHook.h"

using namespace ProcessUtil;


/**  
 * \brief 
 */
struct TModuleHandles
{
	HMODULE hNtDll;
	HMODULE hKernel32;
	HMODULE hUser32;
	HMODULE hShell32;
	HMODULE hMswsock;
	HMODULE hOpenGL32;
	HMODULE hPsapi;
	HMODULE hWs2_32;
	HMEMORYMODULE hMemLib;
};



/**  
 * \brief 
 */
struct TPatchInfo
{
	LdrLoadDllPtr LdrLoadDll;
	LdrGetProcedureAddressPtr LdrGetProcedureAddress;
	LdrInitializeThunkPtr LdrInitializeThunk;

	NtCreateFilePtr NtCreateFile;

	GetProcAddressPtr GetProcAddress;
	CreateProcessInternalWPtr CreateProcessInternalW;
	ExitProcessPtr ExitProcess;
	TerminateProcessPtr TerminateProcess;
	Process32FirstWPtr Process32FirstW;
	Process32NextWPtr Process32NextW;
	EnumProcessModulesPtr EnumProcessModules;
	EnumProcessesPtr EnumProcesses;
	CreateFileWPtr CreateFileW;
	CreateThreadPtr CreateThread;

	Module32FirstWPtr Module32FirstW;
	Module32NextWPtr Module32NextW;

	GetStartupInfoAPtr GetStartupInfoA;
	GetStartupInfoWPtr GetStartupInfoW;


	WSPStartupPtr WSPStartup;
	sendPtr send;
	WSASendPtr WSASend;

	GetAsyncKeyStatePtr GetAsyncKeyState;
	ChangeDisplaySettingsAPtr ChangeDisplaySettingsA;
	GetForegroundWindowPtr GetForegroundWindow;

	FindWindowAPtr FindWindowA;
	FindWindowWPtr FindWindowW;
	FindWindowExAPtr FindWindowExA;
	FindWindowExWPtr FindWindowExW;
	GetWindowPtr GetWindow;
	GetNextWindowPtr GetNextWindow;

	SetWindowsHookExAPtr SetWindowsHookExA;
	SetWindowsHookExWPtr SetWindowsHookExW;

	EnumWindowsPtr EnumWindows;
	EnumChildWindowsPtr EnumChildWindows;
	EnumThreadWindowsPtr EnumThreadWindows;

	RegisterShellHookWindowPtr RegisterShellHookWindow;
	WindowFromPointPtr WindowFromPoint;
	ChildWindowFromPointPtr ChildWindowFromPoint;

	GetTopWindowPtr GetTopWindow;

	ShellExecuteAPtr ShellExecuteA;
	ShellExecuteWPtr ShellExecuteW;

	wglSwapBuffersPtr wglSwapBuffers;

	void (*MainEntryProc)(void);
};



/**  
 * \brief 
 */
struct THookInfo
{
	ExitProcessPtr ExitProcessHook;
	TerminateProcessPtr TerminateProcessHook;

	GetAsyncKeyStatePtr GetAsyncKeyStateHook;
	ChangeDisplaySettingsAPtr ChangeDisplaySettingsAHook;
	GetForegroundWindowPtr GetForegroundWindowHook;

	ShellExecuteAPtr ShellExecuteAHook;

	wglSwapBuffersPtr wglSwapBuffersHook;

	sendPtr sendHook;
	WSASendPtr WSASendHook;
};



/**  
 * \brief 
 */
struct ApiCallLocation
{
	ApiCallLocation* pNext;
	void* callAddress;
};



/**  
 * \brief 
 */
struct ApiPatchInfo
{
	ApiCallLocation* pCallLocations;

//	ApiFantomLocation* pFantoms;

//	FARPROC	pfnHook;
//	FARPROC* ppfnOriginalApi;

	FARPROC* pIatFnAddr;
};



/**  
 * \brief 
 */
struct TGlobals
{
	HMODULE ghInstance;
	char gszRootDir[512];

	DWORD gdwInitialized;
	DWORD gdwMainThreadId;
	HANDLE ghParentProcess;
	DWORD gdwPatchProcessEnum;

	TPatchInfo gOriginalAPIs;
	THookInfo gHookInfo;
	
	TModuleHandles gModHanldes;

	char gszStartAppName[MAX_PATH+1];
	HANDLE ghFileMapping;

	BOOL gfGameStarted;
	HWND ghwndGame;

	ApiPatchInfo gWsSendPatchData;
	ApiPatchInfo gWsRecvPatchData;
};


