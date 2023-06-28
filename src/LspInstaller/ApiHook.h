#pragma once


#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <Windows.h>



/**  
 * \brief 
 */
namespace ApiHook
{
	bool Init();

	bool RemoteHotPatch(HANDLE hProcess, DWORD_PTR pFuncAddr, DWORD_PTR pCallback, DWORD_PTR pOrigProcAddr);
	bool RemoteUnPatch(HANDLE hProcess, DWORD_PTR pOrigProcAddr);
	bool RemoteFreeOriginallCallData(HANDLE hProcess, DWORD_PTR pOrigProcAddr);

	bool PatchExport(HMODULE hModule, const char* fnName, PVOID pAddr, PVOID* pvOrigAddr);
	bool PatchRetn(DWORD_PTR pFuncAddr, DWORD_PTR pCallback);

	DWORD_PTR FindPatchPoint(HANDLE hProcess, DWORD_PTR pFuncAddress, DWORD_PTR pCallback);

	void* AllocateMem(HANDLE hProcess, DWORD dwSize, DWORD dwProtection = PAGE_EXECUTE_READ);
	bool ReleaseMem(HANDLE hProcess, PVOID pRemMem);
	bool ReadProtectedMem(HANDLE hProcess, DWORD_PTR dwAddr, void* buff, DWORD dwSize);
	bool WriteProtectedMem(HANDLE hProcess, DWORD_PTR dwAddr, void* buff, DWORD dwSize);

	void* HeapMemAllocate(DWORD dwSize);
	void* HeapMemReAllocate(void* ptr, DWORD dwSize);
	void HeapMemFree(void* ptr);

/*
#pragma pack(push)
#pragma pack(1)
	struct ApiFantomLocation
	{
		ApiFantomLocation* pNext;

		BYTE  junkCode[11];
		BYTE  jumpOpCode; // 0xE9
		DWORD_PTR relAddrJmp;

	};
#pragma pack(pop)


	ApiCallLocation* AddCallLocation(ApiCallLocation* pCallLoc, void* pvAddr);

	ApiFantomLocation* CreateFantomLocation();
	void SetFantomLocationJump(ApiFantomLocation* pChainData, DWORD dwAddr);
*/
	FARPROC CreateHookThunk(PVOID* pHookAddr, PVOID* pOrigCallAddr);

	typedef LRESULT (CALLBACK* HookProcPtr)(int nCode, WPARAM wParam, LPARAM lParam);
	typedef LRESULT (CALLBACK* FilterHookCallPtr)(HookProcPtr pfnHook, int nCode, WPARAM wParam, LPARAM lParam);

	HOOKPROC CreateWinHookFilterThunk(FilterHookCallPtr pfnFilter, HOOKPROC pfnOrigHook);

	void ReplaceFnCall(HMODULE hMod, FARPROC* ppfn,  FARPROC* ppfnRepl);
}
