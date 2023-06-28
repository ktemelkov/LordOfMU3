#include "stdafx.h"
#include "WSPRedirect.h"
#include "ModuleLoader.h"
#include "..\_Shared\PeUtil.h"


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
			DisableThreadLibraryCalls(hinstDll);

			if (lpvReserved)
			{
				gDllInstance = hinstDll;

				CWSPRedirect::OnDllInit();
				CModuleLoader::OnDllInit((HMODULE)hinstDll, (InstallerInitStruct*)lpvReserved);

				PeUtil::WipeImportTable((HMODULE)hinstDll);
			}
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			if (gDllInstance)
			{
				CModuleLoader::OnDllTerm();
				CWSPRedirect::OnDllTerm();
			}
		}
		break;
	}

	return TRUE;
}


/**  
 * \brief 
 */
int WSPAPI MacInitialize(WORD wVersion, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpCallTable, LPWSPPROC_TABLE lpProcTable)
{
	if (!gDllInstance)
		return SOCKET_ERROR;
	
	return CWSPRedirect::WSPStartup(wVersion, lpWSPData, lpProtocolInfo, UpCallTable, lpProcTable);
}


/**  
 * \brief 
 */
int WSPAPI MacStartup()
{
	CModuleLoader::Init();
	return 0;
}


/**  
 * \brief 
 */
int WSPAPI MacRemoveSendHook()
{
	CWSPRedirect* pInstance = CWSPRedirect::GetInstance();

	if (pInstance)
		pInstance->RemoveSendHook();

	return 0;
}

