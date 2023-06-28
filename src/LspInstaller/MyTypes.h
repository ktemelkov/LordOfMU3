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
	HMODULE hKernel32;
	HMODULE hUser32;
	HMODULE hMswsock;
	HMODULE hGdi32;
	HMODULE hWs2_32;
	HMEMORYMODULE hMemLib;
};



/**  
 * \brief 
 */
struct TPatchInfo
{
	// ntdll.dll
	LdrLoadDllPtr LdrLoadDll;
	LdrUnloadDllPtr  LdrUnloadDll;
	LdrGetProcedureAddressPtr LdrGetProcedureAddress;


	// kernel32.dll
	CreateProcessInternalWPtr CreateProcessInternalW;
	CreateProcessWPtr CreateProcessW;
	VirtualProtectExPtr VirtualProtectEx;


	// ws2_32.dll
	WSPStartupPtr WSPStartup;
	sendPtr send;
	recvPtr recv;


	// user32.dll
	GetAsyncKeyStatePtr GetAsyncKeyState;
	ChangeDisplaySettingsAPtr ChangeDisplaySettingsA;
	GetForegroundWindowPtr GetForegroundWindow;

	SetWindowsHookExAPtr SetWindowsHookExA;
	SetWindowsHookExWPtr SetWindowsHookExW;

	FindWindowAPtr FindWindowA;
	FindWindowWPtr FindWindowW;
	FindWindowExAPtr FindWindowExA;
	FindWindowExWPtr FindWindowExW;
	GetWindowPtr GetWindow;

	EnumWindowsPtr EnumWindows;
	EnumChildWindowsPtr EnumChildWindows;
	EnumThreadWindowsPtr EnumThreadWindows;

	RegisterShellHookWindowPtr RegisterShellHookWindow;
	WindowFromPointPtr WindowFromPoint;
	ChildWindowFromPointPtr ChildWindowFromPoint;

	GetTopWindowPtr GetTopWindow;


	// shell32.dll
	ShellExecuteAPtr ShellExecuteA;

	// GDI32.dll
	SwapBuffersPtr SwapBuffers;
	SetPixelFormatPtr SetPixelFormat;

	WNDPROC MainWndProc;
};



/**  
 * \brief 
 */
struct THookInfo
{
	GetAsyncKeyStatePtr GetAsyncKeyStateHook;
	GetAsyncKeyStateHookPtr GetAsyncKeyStateHook2;
	ChangeDisplaySettingsAPtr ChangeDisplaySettingsAHook;
	GetForegroundWindowPtr GetForegroundWindowHook;

	SwapBuffersPtr SwapBuffersHook;

	sendPtr sendHook;
	recvPtr recvHook;
	recvPtr recvWspAdapter;

	WNDPROC MainWndProcHook;
};


/**  
 * \brief 
 */
struct THardPatchInfo
{
	sendPtr	ws2_original_send;
	sendPtr	ws2_last_send;
	sendPtr* ws2_send_iat_entry;
	BYTE ws2_send_bytes[16];


	recvPtr	ws2_original_recv;
	recvPtr* ws2_recv_iat_entry;
	BYTE ws2_recv_bytes[16];

	recvPtr ws2_ac_recv; // AntiCheat original proc

	GetAsyncKeyStatePtr OriginalGetAsyncKeyState;
	BYTE get_async_key_state_bytes[16];
};



/**  
 * \brief 
 */
struct TGlobals
{
	HMODULE ghInstance;
	char gszRootDir[512];

	TPatchInfo gOriginalAPIs;
	THookInfo gHookInfo;
	
	TModuleHandles gModHanldes;
	THardPatchInfo gHardPatch;

	char gszStartAppName[MAX_PATH+1];

	BOOL gfGameStarted;
	HWND ghwndGame;

	HANDLE hLogFile;
	HANDLE hLogFile2;
};


