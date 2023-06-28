#include "stdafx.h"
#include "MyHooks.h"
#include "MyGlobals.h"
#include "MyUtils.h"
#include "StringUtil.h"
#include "ApiHook.h"



/**  
 * \brief 
 */
int __stdcall MyWSPStartup(WORD wVer, LPWSPDATA pData, LPWSAPROTOCOL_INFOW pProtocol, WSPUPCALLTABLE upcalltable, LPWSPPROC_TABLE pProcTable)
{
	WSPStartupPtr proc = gGlobals.gOriginalAPIs.WSPStartup;

	LoadBotDlls();

	if (gGlobals.gModHanldes.hMemLib)
	{
		WSPStartupPtr myStartup = (WSPStartupPtr)MemoryGetProcAddress(gGlobals.gModHanldes.hMemLib, "MacInitialize");

		if (myStartup)
		{
			int ret = proc(wVer, pData, pProtocol, upcalltable, pProcTable);

			myStartup(wVer, pData, pProtocol, upcalltable, pProcTable);

			return ret;
		}
	}

	return proc(wVer, pData, pProtocol, upcalltable, pProcTable);
}




/**  
 * \brief 
 */
static LRESULT CALLBACK FilterCBTHook(ApiHook::HookProcPtr pfnHook, int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HCBT_ACTIVATE || nCode == HCBT_CREATEWND 
			|| nCode == HCBT_DESTROYWND || nCode == HCBT_MINMAX 
			|| nCode == HCBT_SETFOCUS)
	{
		HWND hWnd = (HWND)wParam;

		if (ShouldHideWindow(hWnd))
			return 0;
	}

	return pfnHook(nCode, wParam, lParam);
}



/**  
 * \brief 
 */
HHOOK WINAPI MySetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
	if (idHook == WH_KEYBOARD || idHook == WH_KEYBOARD_LL || idHook == WH_MOUSE || idHook == WH_MOUSE_LL || dwThreadId == 0 || idHook == WH_SHELL)
		return (HHOOK)0xDEADBEEF;

	if (idHook == WH_CBT)
		lpfn = ApiHook::CreateWinHookFilterThunk(&FilterCBTHook, lpfn);


	return (HHOOK)gGlobals.gOriginalAPIs.SetWindowsHookExA(idHook, lpfn, hMod, dwThreadId);
}


/**  
 * \brief 
 */
HHOOK WINAPI MySetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId)
{
	if (idHook == WH_KEYBOARD || idHook == WH_KEYBOARD_LL || idHook == WH_MOUSE || idHook == WH_MOUSE_LL || dwThreadId == 0 || idHook == WH_SHELL)
		return (HHOOK)0xDEADBEEF;

	if (idHook == WH_CBT)
		lpfn = ApiHook::CreateWinHookFilterThunk(&FilterCBTHook, lpfn);

	return (HHOOK)gGlobals.gOriginalAPIs.SetWindowsHookExW(idHook, lpfn, hMod, dwThreadId);
}




/**  
 * \brief 
 */
HWND WINAPI MyFindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.FindWindowA(lpClassName, lpWindowName);

	if (ShouldHideWindow(hwndRet))
		return 0;

	return hwndRet;
}



/**  
 * \brief 
 */
HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.FindWindowW(lpClassName, lpWindowName);

	if (ShouldHideWindow(hwndRet))
		return 0;

	return hwndRet;
}



/**  
 * \brief 
 */
HWND WINAPI MyFindWindowExA(HWND hwndParent, HWND hwndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow)
{
	if (ShouldHideWindow(hwndParent))
		return 0;

	HWND hwndRet = gGlobals.gOriginalAPIs.FindWindowExA(hwndParent, hwndChildAfter, lpszClass, lpszWindow);

	if (ShouldHideWindow(hwndRet))
		return 0;

	return hwndRet;
}


/**  
 * \brief 
 */
HWND WINAPI MyFindWindowExW(HWND hwndParent, HWND hwndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow)
{
	if (ShouldHideWindow(hwndParent))
		return 0;

	HWND hwndRet = gGlobals.gOriginalAPIs.FindWindowExW(hwndParent, hwndChildAfter, lpszClass, lpszWindow);

	if (ShouldHideWindow(hwndRet))
		return 0;

	return hwndRet;
}




/**  
 * \brief 
 */
BOOL WINAPI MySwapBuffers(HDC hdc)
{
	if (gGlobals.gfGameStarted && gGlobals.gHookInfo.SwapBuffersHook)
		return gGlobals.gHookInfo.SwapBuffersHook(hdc);

	return gGlobals.gOriginalAPIs.SwapBuffers(hdc);
}


/**  
 * \brief 
 */
BOOL WINAPI MySetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
	if (!gGlobals.gfGameStarted)
	{
		HWND hWnd = WindowFromDC(hdc);

		if (hWnd)
		{
			RECT rc;
			StringUtil::MemSet(&rc, 0, sizeof(RECT));

			GetWindowRect(hWnd, &rc);

			if (rc.right - rc.left >= 800 && rc.bottom - rc.top >= 600)
			{
				gGlobals.gfGameStarted = TRUE;
				gGlobals.ghwndGame = hWnd; 

				DoGameStartupInit();
			}
		}
		else
		{
			Beep(1500,100);
		}
	}

	return gGlobals.gOriginalAPIs.SetPixelFormat(hdc, iPixelFormat, ppfd);
}



/**  
 * \brief 
 */
HWND WINAPI MyGetForegroundWindow()
{
	HWND hwnd = gGlobals.gOriginalAPIs.GetForegroundWindow();

	if (ShouldHideWindow(hwnd))
		return GetShellWindow();

	if (gGlobals.gHookInfo.GetForegroundWindowHook)
		return gGlobals.gHookInfo.GetForegroundWindowHook();

	return gGlobals.gOriginalAPIs.GetForegroundWindow();
}


/**  
 * \brief 
 */
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (ShouldHideWindow(hwnd))
		return TRUE;

	struct _param_t { WNDENUMPROC proc; LPARAM lParam; }* param = (_param_t*) lParam;
	return param->proc(hwnd, param->lParam);
}


/**  
 * \brief 
 */
BOOL WINAPI MyEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
	struct { WNDENUMPROC proc; LPARAM lParam; } param = { lpEnumFunc, lParam };
	return gGlobals.gOriginalAPIs.EnumWindows(EnumWindowsProc, (LPARAM)&param);
}


/**  
 * \brief 
 */
BOOL WINAPI MyEnumChildWindows(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
	struct { WNDENUMPROC proc; LPARAM lParam; } param = { lpEnumFunc, lParam };
	return gGlobals.gOriginalAPIs.EnumChildWindows(hWndParent, EnumWindowsProc, (LPARAM)&param);
}


/**  
 * \brief 
 */
BOOL WINAPI MyEnumThreadWindows(DWORD dwThreadId, WNDENUMPROC lpfn, LPARAM lParam)
{
	struct { WNDENUMPROC proc; LPARAM lParam; } param = { lpfn, lParam };
	return gGlobals.gOriginalAPIs.EnumThreadWindows(dwThreadId, EnumWindowsProc, (LPARAM)&param);
}


/**  
 * \brief 
 */
HWND WINAPI MyGetWindow(HWND hWnd, UINT uCmd)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.GetWindow(hWnd, uCmd);

	if (ShouldHideWindow(hwndRet))
		return 0;

	return hwndRet;
}


/**  
 * \brief 
 */
BOOL WINAPI MyRegisterShellHookWindow(HWND hWnd)
{
	return TRUE;
}


/**  
 * \brief 
 */
HWND WINAPI MyWindowFromPoint(POINT Point)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.WindowFromPoint(Point);

	if (ShouldHideWindow(hwndRet))
		return GetShellWindow();

	return hwndRet;
}


/**  
 * \brief 
 */
HWND WINAPI MyChildWindowFromPoint(HWND hWndParent, POINT Point)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.ChildWindowFromPoint(hWndParent, Point);

	if (ShouldHideWindow(hwndRet))
		return hWndParent;

	return hwndRet;
}


/**  
 * \brief 
 */
HWND WINAPI MyGetTopWindow(HWND hWnd)
{
	HWND hwndRet = gGlobals.gOriginalAPIs.GetTopWindow(hWnd);

	if (ShouldHideWindow(hwndRet))
		return GetShellWindow();

	return hwndRet;
}


/**  
 * \brief 
 */
BOOL __stdcall MyVirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect)
{
	BOOL ret = gGlobals.gOriginalAPIs.VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
	
	FixIatWinsockHooks();

	return ret;
}
