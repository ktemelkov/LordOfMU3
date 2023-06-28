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
void __declspec(naked) MyGetAsyncKeyStateRet2()
{
	__asm
	{
		push ebx
		push eax
		mov eax, gGlobals.gHookInfo.GetAsyncKeyStateHook
		cmp eax, 0
		jz MyGetAsyncKeyStateRet_End1

		mov ebx, [esp + 16]
		push ebx
		call [gGlobals.gHookInfo.GetAsyncKeyStateHook]
		jmp MyGetAsyncKeyStateRet_End

MyGetAsyncKeyStateRet_End1:
		pop eax

MyGetAsyncKeyStateRet_End:
		pop ebx
		ret
	}
}


SHORT WINAPI MyGetAsKeyStateHook(int vk, int orig)
{
	if (vk == VK_RBUTTON)
		return (SHORT)0x8000;

	return (SHORT)orig;
}


/**  
 * \brief 
 */
void Test(HMODULE hModule)
{
	ApiHook::Init();

	gGlobals.gHookInfo.GetAsyncKeyStateHook2 = MyGetAsKeyStateHook;

	gGlobals.gModHanldes.hUser32 = LoadLibraryA("user32.dll");

	FARPROC pfn = FindFnAddressByName(gGlobals.gModHanldes.hUser32, "GetAsyncKeyState"); 
	
	if (pfn) 
		ApiHook::PatchRetn((DWORD_PTR)pfn, (DWORD_PTR)&MyGetAsyncKeyStateRet2);

	SHORT res = GetAsyncKeyState(VK_RBUTTON);

	res = GetAsyncKeyState(VK_ESCAPE);
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


