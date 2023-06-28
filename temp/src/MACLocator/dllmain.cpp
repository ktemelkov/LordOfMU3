#include "stdafx.h"
#include "WSPRedirect.h"
#include "ModuleLoader.h"


HINSTANCE gDllInstance = 0;


/**  
 * \brief 
 */
BOOL WINAPI DllMain(IN HINSTANCE hinstDll, IN DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			gDllInstance = hinstDll;

			if (lpvReserved)
			{
				CWSPRedirect::OnDllInit();
				CModuleLoader::Init((HMODULE)hinstDll, (InstallerInitStruct*)lpvReserved);
			}
		}
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		CModuleLoader::Term();
		CWSPRedirect::OnDllTerm();
		break;
	}

	return TRUE;
}


/**  
 * \brief 
 */
int WSPAPI WSPStartup(WORD wVersion, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpCallTable, LPWSPPROC_TABLE lpProcTable)
{
	int ret = CWSPRedirect::WSPStartup(wVersion, lpWSPData, lpProtocolInfo, UpCallTable, lpProcTable);

	CModuleLoader::Load();

	return ret;
}

