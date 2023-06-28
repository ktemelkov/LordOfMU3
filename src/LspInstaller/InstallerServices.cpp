#include "stdafx.h"
#include "InstallerServices.h"
#include "Myglobals.h"
#include "MyUtils.h"


/**  
 * \brief 
 */
namespace InstallerServices
{
	/**  
	 * \brief 
	 */
	static bool SetApiHook(int fnIdx, PVOID pvAddr, PVOID* pvOrigAddr)
	{
		switch (fnIdx)
		{
		case _API_HOOK_GETASYNCKEYSTATE:
			if (gGlobals.gOriginalAPIs.GetAsyncKeyState)
			{
				gGlobals.gHookInfo.GetAsyncKeyStateHook = (GetAsyncKeyStatePtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.GetAsyncKeyState;
			}
			else
			{
//				ApiHotPatch2(gGlobals.gModHanldes.hUser32, GetAsyncKeyState);
//				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.GetAsyncKeyState;

				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hUser32, "GetAsyncKeyState");
			}
			break;
		case _API_HOOK_GETASYNCKEYSTATE_FILTER:
			{
				gGlobals.gHookInfo.GetAsyncKeyStateHook2 = (GetAsyncKeyStateHookPtr)pvAddr;
				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hUser32, "GetAsyncKeyState");
			}
			break;			
		case _API_HOOK_CHANGEDISPLAYSETTINGSA:
			if (gGlobals.gOriginalAPIs.ChangeDisplaySettingsA)
			{
				gGlobals.gHookInfo.ChangeDisplaySettingsAHook = (ChangeDisplaySettingsAPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.ChangeDisplaySettingsA;
			}
			else
				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hUser32, "ChangeDisplaySettingsA");
			break;
		case _API_HOOK_GETFOREGROUNDWINDOW:
			if (gGlobals.gOriginalAPIs.GetForegroundWindow)
			{
				gGlobals.gHookInfo.GetForegroundWindowHook = (GetForegroundWindowPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.GetForegroundWindow;
			}
			else
				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hUser32, "GetForegroundWindow");
			break;
		case _API_HOOK_WGLSWAPBUFFERS:
			if (gGlobals.gOriginalAPIs.SwapBuffers)
			{
				gGlobals.gHookInfo.SwapBuffersHook = (SwapBuffersPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.SwapBuffers;
			}
			else
				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hGdi32, "SwapBuffers");

			break;
/*		case _API_HOOK_WS2_SEND:
			if (gGlobals.gOriginalAPIs.send)
			{
				gGlobals.gHookInfo.sendHook = (sendPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.send;
			}
			else
				*pvOrigAddr = 0;
			break;
		case _API_HOOK_WS2_RECV:
			if (gGlobals.gHardPatch.ws2_ac_recv)
			{
				gGlobals.gHookInfo.recvWspAdapter = (recvPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gHardPatch.ws2_ac_recv;
			}
			else
				*pvOrigAddr = 0;
			break;
			*/
		case _API_HOOK_WS2_SEND:
			if (gGlobals.gOriginalAPIs.send)
			{
				gGlobals.gHookInfo.sendHook = (sendPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.send;
			}
			else
				*pvOrigAddr = 0;
			break;
		case _API_HOOK_WS2_RECV:
			if (gGlobals.gOriginalAPIs.recv)
			{
				gGlobals.gHookInfo.recvHook = (recvPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.recv;
			}
			else
				*pvOrigAddr = 0;
			break;
		case _API_HOOK_MAINWND_PROC:
			if (gGlobals.gOriginalAPIs.MainWndProc)
			{
				gGlobals.gHookInfo.MainWndProcHook = (WNDPROC)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.MainWndProc;
			}
			else
				*pvOrigAddr = 0;

			break;
		default:
			return false;
		}

		return true;
	}



	/**  
	 * \brief 
	 */
	static bool ClearApiHook(int fnIdx)
	{
		PVOID pvAddr = 0;

		switch (fnIdx)
		{
		case _API_HOOK_GETASYNCKEYSTATE:
			gGlobals.gHookInfo.GetAsyncKeyStateHook = (GetAsyncKeyStatePtr)pvAddr;
			break;
		case _API_HOOK_CHANGEDISPLAYSETTINGSA:
			gGlobals.gHookInfo.ChangeDisplaySettingsAHook = (ChangeDisplaySettingsAPtr)pvAddr;
			break;
		case _API_HOOK_GETFOREGROUNDWINDOW:
			gGlobals.gHookInfo.GetForegroundWindowHook = (GetForegroundWindowPtr)pvAddr;
			break;
		case _API_HOOK_WGLSWAPBUFFERS:
			gGlobals.gHookInfo.SwapBuffersHook = (SwapBuffersPtr)pvAddr;
			break;
		case _API_HOOK_WS2_SEND:
			gGlobals.gHookInfo.sendHook = (sendPtr)pvAddr;
			break;
		case _API_HOOK_WS2_RECV:
			gGlobals.gHookInfo.recvWspAdapter = (recvPtr)pvAddr;
			break;
		case _API_HOOK_MAINWND_PROC:
			gGlobals.gHookInfo.MainWndProcHook = (WNDPROC)pvAddr;
			break;
		default:
			return false;
		}

		return true;
	}



	/**  
	 * \brief 
	 */
	static bool IsGameStarted()
	{
		return (gGlobals.gfGameStarted == TRUE);
	}

	/**  
	 * \brief 
	 */
	static HWND GetGameWindow()
	{
		return gGlobals.ghwndGame;
	}


	/**  
	 * \brief 
	 */
	static void CheckWsHooks()
	{
//		FixIatWinsockHooks();
	}


	/**  
	 * \brief 
	 */
	InstallerServiceTable GetServiceTable()
	{
		InstallerServiceTable servTable;
		servTable.SetApiHook = &InstallerServices::SetApiHook;
		servTable.ClearApiHook = &InstallerServices::ClearApiHook;
		servTable.IsGameStarted = &InstallerServices::IsGameStarted;
		servTable.GetGameWindow = &InstallerServices::GetGameWindow;
		servTable.CheckWsHooks = &InstallerServices::CheckWsHooks;

		return servTable;
	}
}
