// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "MyGlobals.h"
#include "MyHooks.h"
#include "ApiHook.h"
#include "StringUtil.h"
#include "MyUtils.h"
#include "version.h"
#include "MyLoader.h"


#ifdef _DEBUG

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
void Test(HMODULE hModule)
{
	HMODULE hAsdf = ProcessUtil::FindModuleBase("asdf.dll");

	ApiHook::Init();

	HMODULE hMod = ProcessUtil::FindModuleBase("user32.dll");
	
	if (hMod)
	{
		FARPROC pfn = ProcessUtil::FindFnAddressByName(hMod, "FindWindowA");

		if (pfn)
			ApiHook::PatchRetn((DWORD_PTR)pfn, (DWORD_PTR)&MyFindWindowRet);
	}

	HWND hShellWnd = FindWindowA("Shell_TrayWnd", 0);

	if (hShellWnd == 0)
	{
		return;
	}
}

#endif



/**  
 * \brief 
 */
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			if (lpReserved != 0)
			{
 				InitLoader(hModule, (const char*)lpReserved);
			}
			else
			{
#ifdef _DEBUG
				Test(hModule);
#endif
			}
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}


