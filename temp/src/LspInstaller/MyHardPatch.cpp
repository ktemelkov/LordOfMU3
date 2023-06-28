#include "stdafx.h"
#include "MyHardPatch.h"
#include "MyGlobals.h"
#include "ApiHook.h"
#include "ProcessUtil.h"

using namespace ProcessUtil;




static bool HardPatch_AddCallLocation(ApiPatchInfo* pPathcInfo, void* pvAddr);
static void HardPatch_GetCodePatchPoints(ApiPatchInfo* pProcInfo, HMODULE hMod, FARPROC* ppfn);
static bool HardPatch_CheckCodePatchPoints(ApiPatchInfo* pProcInfo, FARPROC* ppFnDetour);
static bool HardPatch_CheckForIATHook(ApiPatchInfo* pProcInfo, const char* pszModule, const char* pszFnName, FARPROC* ppFnDetour);



/**  
 * \brief 
 */
bool HardPatch_Init()
{
	HMODULE hModule = (HMODULE)GetLocalProcessBaseAddress();

	if (!hModule)
		return false;


	FARPROC* ppfnSend = FindFnImportEntry(hModule, "ws2_32.dll", "send", 19);

	if (ppfnSend)
	{
		gGlobals.gWsSendPatchData.pIatFnAddr = ppfnSend;
		HardPatch_GetCodePatchPoints(&gGlobals.gWsSendPatchData, hModule, ppfnSend);
	}


	FARPROC* ppfnRecv = FindFnImportEntry(hModule, "ws2_32.dll", "recv", 16);

	if (ppfnRecv)
	{
		gGlobals.gWsRecvPatchData.pIatFnAddr = ppfnRecv;
		HardPatch_GetCodePatchPoints(&gGlobals.gWsRecvPatchData, hModule, ppfnRecv);
	}


	return true;
}


/**  
 * \brief 
 */
void HardPatch_ModulePreLoad()
{

}


/**  
 * \brief 
 */
void HardPatch_ModulePostLoad()
{

}



/**  
 * \brief 
 */
void HardPatch_AppStart()
{
	HMODULE hModule = (HMODULE)GetLocalProcessBaseAddress();

	if (!hModule)
		return;


	FARPROC pFnSendDetour = 0;
	
	if (
		HardPatch_CheckForIATHook(&gGlobals.gWsSendPatchData, "ws2_32.dll", "send", &pFnSendDetour)
//		|| 
//		HardPatch_CheckCodePatchPoints(&gGlobals.gWsSendPatchData, &pFnSendDetour)
		)
	{
//		Beep(3000, 50);

		FARPROC pFnHook = ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.sendHook, (PVOID*)&gGlobals.gOriginalAPIs.send);
		ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD_PTR)pFnSendDetour, (DWORD_PTR)pFnHook, (DWORD_PTR)&gGlobals.gOriginalAPIs.send);
	}

}



/**  
 * \brief 
 */
bool HardPatch_AddCallLocation(ApiPatchInfo* pPathcInfo, void* pvAddr)
{
	if (!pPathcInfo || !pvAddr)
		return false;

	ApiCallLocation* pNewLoc = (ApiCallLocation*)ApiHook::HeapMemAllocate(sizeof(ApiCallLocation));

	if (!pNewLoc)
		return false;

	pNewLoc->pNext = pPathcInfo->pCallLocations;
	pNewLoc->callAddress = pvAddr;

	pPathcInfo->pCallLocations = pNewLoc;
	return true;
}



/**  
 * \brief 
 */
void HardPatch_GetCodePatchPoints(ApiPatchInfo* pProcInfo, HMODULE hMod, FARPROC* ppfn)
{
	int offs = 0;
	int len = sizeof(DWORD_PTR);
	DWORD_PTR dwFind = (DWORD_PTR)ppfn;
	DWORD dwCodeBytes = (DWORD)FindCodeBytes(hMod, (BYTE*)&dwFind, len);

	while (dwCodeBytes)
	{
		WORD wInstr = *((WORD*)((DWORD)dwCodeBytes-2));

		if (wInstr == 0x25FF || wInstr == 0x15FF)
		{
			HardPatch_AddCallLocation(pProcInfo, (void*)dwCodeBytes);
		}

		offs = dwCodeBytes + len;
		dwCodeBytes = FindCodeBytes(hMod, (BYTE*)&dwFind, len, offs);
	}
}


/**  
 * \brief 
 */
bool HardPatch_CheckCodePatchPoints(ApiPatchInfo* pProcInfo, FARPROC* ppFnDetour)
{
	if (!pProcInfo || !pProcInfo->pIatFnAddr)
		return false;


	ApiCallLocation* pCallLoc = pProcInfo->pCallLocations;

	while (pCallLoc)
	{
		FARPROC* ppFnAddr = *((FARPROC**)pCallLoc->callAddress);

		if (ppFnAddr != pProcInfo->pIatFnAddr)
		{
			if (ppFnDetour)
				*ppFnDetour = *ppFnAddr;
			
			return true;
		}

		pCallLoc = pCallLoc->pNext;
	}

	return false;
}

/**  
 * \brief 
 */
bool HardPatch_CheckForIATHook(ApiPatchInfo* pProcInfo, const char* pszModule, const char* pszFnName, FARPROC* ppFnDetour)
{
	if (!pProcInfo || !pszFnName || !pszModule || !pProcInfo->pIatFnAddr)
		return false;

	FARPROC pfn = FindFnAddressByName(FindModuleBase(pszModule), pszFnName);

	if (pfn == *pProcInfo->pIatFnAddr)
		return false;

	if (ppFnDetour)
		*ppFnDetour = *pProcInfo->pIatFnAddr;

	return true;
}
