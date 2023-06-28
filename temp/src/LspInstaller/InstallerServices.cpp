#include "stdafx.h"
#include "InstallerServices.h"
#include "Myglobals.h"


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
			if (gGlobals.gOriginalAPIs.wglSwapBuffers)
			{
				gGlobals.gHookInfo.wglSwapBuffersHook = (wglSwapBuffersPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.wglSwapBuffers;
			}
			else
				*pvOrigAddr = (PVOID)ProcessUtil::FindFnAddressByName(gGlobals.gModHanldes.hOpenGL32, "wglSwapBuffers");
			break;
		case _API_HOOK_WS2_SEND:
			if (gGlobals.gOriginalAPIs.send)
			{
				gGlobals.gHookInfo.sendHook = (sendPtr)pvAddr;
				*pvOrigAddr = (PVOID)gGlobals.gOriginalAPIs.send;
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
			gGlobals.gHookInfo.wglSwapBuffersHook = (wglSwapBuffersPtr)pvAddr;
			break;
		case _API_HOOK_WS2_SEND:
			gGlobals.gHookInfo.sendHook = (sendPtr)pvAddr;
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
	InstallerServiceTable GetServiceTable()
	{
		InstallerServiceTable servTable;
		servTable.SetApiHook = &InstallerServices::SetApiHook;
		servTable.ClearApiHook = &InstallerServices::ClearApiHook;
		servTable.IsGameStarted = &InstallerServices::IsGameStarted;

		return servTable;
	}
}
