#include "stdafx.h"
#include "MyUtils.h"
#include "MyGlobals.h"
#include "ApiHook.h"
#include "MyHooks.h"

#include "..\_Shared\PackUtil.h"
#include "InstallerServices.h"
#include "StringUtil.h"
#include "Disasm\distorm.h"



/**  
 * \brief 
 */
static void InitHooks()
{
	GdiApiHotPatch(SwapBuffers);

	FARPROC pfn = gGlobals.ghwndGame ? (FARPROC)GetWindowLongA(gGlobals.ghwndGame, GWL_WNDPROC) : 0;

	if (pfn)
	{
		DWORD dwHook = (DWORD)ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.MainWndProcHook, (PVOID*)&gGlobals.gOriginalAPIs.MainWndProc);
		
		if (dwHook)
			ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD)pfn, dwHook, (DWORD)&gGlobals.gOriginalAPIs.MainWndProc);
	}
}



/**  
 * \brief 
 */
int __stdcall MyRestoredWsSend(SOCKET s, char *buf, int len, int flags)
{
	if (gGlobals.gOriginalAPIs.send)
	{
		int res = gGlobals.gOriginalAPIs.send(s, buf, len, flags);

/*		if (gGlobals.hLogFile && res > 0)
		{
			DWORD dwWritten = 0;
			DWORD dwPad = 0xCCAAAACC;
			WriteFile(gGlobals.hLogFile, (LPVOID)&dwPad, (DWORD)4, &dwWritten, 0);

			WriteFile(gGlobals.hLogFile, (LPVOID)buf, (DWORD)res, &dwWritten, 0);
		}
*/
		return res;
	}

	return -1;
}


/**  
 * \brief 
 */
static void FixSendAcHook()
{
	if (gGlobals.gHardPatch.ws2_send_iat_entry)
	{
		gGlobals.gHardPatch.ws2_original_send = (sendPtr)ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.sendHook, (PVOID*)&gGlobals.gOriginalAPIs.send);
		gGlobals.gOriginalAPIs.send = *gGlobals.gHardPatch.ws2_send_iat_entry;

		ApiHook::ReplaceFnCall(0, (FARPROC*)gGlobals.gHardPatch.ws2_send_iat_entry, (FARPROC*)&gGlobals.gHardPatch.ws2_original_send);
	}

	return;

	if (!gGlobals.gHardPatch.ws2_last_send)
		return;

	if (gGlobals.gHardPatch.ws2_send_iat_entry && *gGlobals.gHardPatch.ws2_send_iat_entry != 0
			&& *gGlobals.gHardPatch.ws2_send_iat_entry != gGlobals.gHardPatch.ws2_last_send)
	{
		if (*gGlobals.gHardPatch.ws2_send_iat_entry == gGlobals.gHardPatch.ws2_original_send)
		{
			if (gGlobals.gModHanldes.hMemLib != 0)
			{
				int (__stdcall* MacRemoveSendHook)() = (int(__stdcall*)())MemoryGetProcAddress(gGlobals.gModHanldes.hMemLib, "MacRemoveSendHook");

				if (MacRemoveSendHook)
					MacRemoveSendHook();
			}
		}
		else
		{
			FARPROC pFnHook = ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.sendHook, (PVOID*)&gGlobals.gOriginalAPIs.send);
			ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD_PTR)(*gGlobals.gHardPatch.ws2_send_iat_entry), (DWORD_PTR)pFnHook, (DWORD_PTR)&gGlobals.gOriginalAPIs.send);
		}

		gGlobals.gHardPatch.ws2_last_send = *gGlobals.gHardPatch.ws2_send_iat_entry; 
	}
//	else if (!StringUtil::MemCmp(gGlobals.gHardPatch.ws2_send_bytes, gGlobals.gHardPatch.ws2_original_send, sizeof(gGlobals.gHardPatch.ws2_send_bytes)))
//	{
//		FARPROC pFnHook = ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.sendHook, (PVOID*)&gGlobals.gOriginalAPIs.send);
//		ApiHook::RemoteHotPatch((HANDLE)-1, (DWORD_PTR)gGlobals.gHardPatch.ws2_original_send, (DWORD_PTR)pFnHook, (DWORD_PTR)&gGlobals.gOriginalAPIs.send);
//	}
}






/**  
 * \brief 
 */
int __stdcall MyWspRecvAdapter(SOCKET s, char *buf, int len, int flags)
{
	if (gGlobals.gHookInfo.recvWspAdapter)
	{
		return gGlobals.gHookInfo.recvWspAdapter(s, buf, len, flags);
	}

	if (gGlobals.gOriginalAPIs.recv)
	{
		int res = gGlobals.gOriginalAPIs.recv(s, buf, len, flags);

/*		if (gGlobals.hLogFile2 && res > 0)
		{
			DWORD dwWritten = 0;
			DWORD dwPad = 0xCCAAAACC;
			WriteFile(gGlobals.hLogFile2, (LPVOID)&dwPad, (DWORD)4, &dwWritten, 0);
			WriteFile(gGlobals.hLogFile2, (LPVOID)buf, (DWORD)res, &dwWritten, 0);
		}
*/
		return res;
	}

	return -1;
}


/**  
 * \brief 
 */
int __stdcall MyRestoredWsRecv(SOCKET s, char *buf, int len, int flags)
{
	if (gGlobals.gOriginalAPIs.recv)
	{
		int res = gGlobals.gOriginalAPIs.recv(s, buf, len, flags);

/*		if (gGlobals.hLogFile && res > 0)
		{
			DWORD dwWritten = 0;
			DWORD dwPad = 0xCCAAAACC;
			WriteFile(gGlobals.hLogFile, (LPVOID)&dwPad, (DWORD)4, &dwWritten, 0);

			WriteFile(gGlobals.hLogFile, (LPVOID)buf, (DWORD)res, &dwWritten, 0);
		}
*/
		return res;
	}

	return -1;
}




/**  
 * \brief 
 */
void FixRecvAcHook()
{
	if (gGlobals.gHardPatch.ws2_recv_iat_entry)
	{
		gGlobals.gHardPatch.ws2_original_recv = (recvPtr)ApiHook::CreateHookThunk((PVOID*)&gGlobals.gHookInfo.recvHook, (PVOID*)&gGlobals.gOriginalAPIs.recv);
		gGlobals.gOriginalAPIs.recv = *gGlobals.gHardPatch.ws2_recv_iat_entry;

		ApiHook::ReplaceFnCall(0, (FARPROC*)gGlobals.gHardPatch.ws2_recv_iat_entry, (FARPROC*)&gGlobals.gHardPatch.ws2_original_recv);
	}

	return;


	if (gGlobals.gHardPatch.ws2_recv_iat_entry && *gGlobals.gHardPatch.ws2_recv_iat_entry != 0
		&& *gGlobals.gHardPatch.ws2_recv_iat_entry != gGlobals.gHardPatch.ws2_original_recv)
	{
	}
	else if (!StringUtil::MemCmp(gGlobals.gHardPatch.ws2_recv_bytes, gGlobals.gHardPatch.ws2_original_recv, sizeof(gGlobals.gHardPatch.ws2_recv_bytes)))
	{
		int i=sizeof(gGlobals.gHardPatch.ws2_recv_bytes)-1;
		
		for (i; i >= 0; i--)
		{
			if (*((BYTE*)gGlobals.gHardPatch.ws2_original_recv + i) != gGlobals.gHardPatch.ws2_recv_bytes[i])
				break;
		}

		BYTE bJmp = *((BYTE*)gGlobals.gHardPatch.ws2_original_recv);


		if (i >= 0 && bJmp == 0xE9)
		{

			DWORD dwDstAddr = *((DWORD*)((BYTE*)gGlobals.gHardPatch.ws2_original_recv+1)) + (DWORD)gGlobals.gHardPatch.ws2_original_recv + 5;

/*
			gGlobals.hLogFile = CreateFileA("C:\\Users\\KTemelkov\\Games\\Release\\mu_log.bin", GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

			if (gGlobals.hLogFile == INVALID_HANDLE_VALUE)
				gGlobals.hLogFile = 0;

			gGlobals.hLogFile2 = CreateFileA("C:\\Users\\KTemelkov\\Games\\Release\\mu_log2.bin", GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

			if (gGlobals.hLogFile2 == INVALID_HANDLE_VALUE)
				gGlobals.hLogFile2 = 0;
*/

			_DInst decodedInstructions[1];
			unsigned int decodedInstructionsCount = 0;
			_DecodeType dt = Decode32Bits;

			_CodeInfo ci;
			ci.code = (const BYTE*)dwDstAddr;
			ci.codeLen = 1024;
			ci.codeOffset = 0;
			ci.dt = dt;
			ci.features = DF_NONE;


			DWORD dwRetAddr = 0;
			DWORD dwRetSize = 0;
			DWORD dwSize = 0;

			while (1)
			{
				distorm_decompose(&ci, decodedInstructions, 1, &decodedInstructionsCount);

				if (decodedInstructionsCount == 0)
					break;

				DWORD dwInstrSize = decodedInstructions[0].size;

				if (dwInstrSize == 1 || dwInstrSize == 3)
				{
					BYTE opCode = *((BYTE*)ci.code);

					if (opCode == 0xC3 || opCode == 0xC2)
					{
						dwRetAddr = (DWORD)ci.code;
						dwRetSize = dwInstrSize;
						dwSize += dwInstrSize;
						break;
					}
				}
				else if (dwInstrSize == 6)
				{
					BYTE opCode1 = *((BYTE*)ci.code);
					BYTE opCode2 = *((BYTE*)ci.code+1);

					if (opCode1 == 0xFF || opCode2 == 0x15)
					{
						ApiHook::RemoteHotPatch((HANDLE)-1, dwDstAddr, (DWORD)&MyRestoredWsRecv, (DWORD)&gGlobals.gHardPatch.ws2_ac_recv);


						DWORD dwAcTrampAddr = *((DWORD*)(ci.code+2));
						gGlobals.gOriginalAPIs.recv = (recvPtr)*((DWORD*)dwAcTrampAddr);

						DWORD dwWspAddaptAddr = (DWORD)&MyWspRecvAdapter;
						ApiHook::WriteProtectedMem((HANDLE)-1, dwAcTrampAddr, &dwWspAddaptAddr, 4);	


//						ApiHook::WriteProtectedMem((HANDLE)-1, (DWORD)gGlobals.gHardPatch.ws2_original_recv, gGlobals.gHardPatch.ws2_recv_bytes, i+1);
//						gGlobals.gHardPatch.ws2_ac_recv = (recvPtr)dwDstAddr;

						break;
					}
				}

				ci.code += dwInstrSize;
				ci.codeLen -= dwInstrSize;
				dwSize += dwInstrSize;
			}

		}

	}
}



/**  
 * \brief 
 */
void FixIatWinsockHooks()
{
//	FixSendAcHook();
//	FixRecvAcHook();
}


/**  
 * \brief 
 */
void CollectWs2HardPatchInfo()
{
	gGlobals.gHardPatch.ws2_original_send = (sendPtr)FindFnAddressByOrd(gGlobals.gModHanldes.hWs2_32, 19);
	gGlobals.gHardPatch.ws2_original_recv = (recvPtr)FindFnAddressByOrd(gGlobals.gModHanldes.hWs2_32, 16);

	gGlobals.gHardPatch.ws2_last_send = gGlobals.gHardPatch.ws2_original_send;

	if (gGlobals.gHardPatch.ws2_original_send)
		StringUtil::MemCopy(gGlobals.gHardPatch.ws2_send_bytes, gGlobals.gHardPatch.ws2_original_send, sizeof(gGlobals.gHardPatch.ws2_send_bytes));

	if (gGlobals.gHardPatch.ws2_original_recv)
		StringUtil::MemCopy(gGlobals.gHardPatch.ws2_recv_bytes, gGlobals.gHardPatch.ws2_original_recv, sizeof(gGlobals.gHardPatch.ws2_recv_bytes));
}


/**  
 * \brief 
 */
void CollectIatHardPatchInfo()
{
	HMODULE hModule = (HMODULE)GetLocalProcessBaseAddress();

	if (hModule)
	{
		gGlobals.gHardPatch.ws2_send_iat_entry = (sendPtr*)FindFnImportEntryByOrd(hModule, "ws2_32.dll", 19);
		gGlobals.gHardPatch.ws2_recv_iat_entry = (recvPtr*)FindFnImportEntryByOrd(hModule, "ws2_32.dll", 16);
	}
}



/**  
 * \brief 
 */
void LoadBotDlls()
{
	if (gGlobals.gModHanldes.hMemLib != 0)
		return;

	char szModuleFile[MAX_PATH+1] = {0};
	StringUtil::StrCopyA(szModuleFile, gGlobals.gszRootDir);

	for (int i=(int)StringUtil::StrLenA(szModuleFile)-1; i >= 0 && szModuleFile[i] != '\\'; szModuleFile[i--] = 0);
	StringUtil::StrConcatA(szModuleFile, "Bootstrapper.dll");


	CPackUtil packer;
	packer.Init(szModuleFile);

	int lSize = packer.GetPackedFileSize(_PACKUTIL_LSP_FILE_ID);
	BYTE* lspDllData = (BYTE*)HeapAlloc(GetProcessHeap(), 0, lSize);

	packer.ExtractFileToMemory(lspDllData, lSize, _PACKUTIL_LSP_FILE_ID);

	InstallerServiceTable services = InstallerServices::GetServiceTable();

	InstallerInitStruct init = {0};
	init.pszRootDir = gGlobals.gszRootDir;
	init.pServices = &services;

	gGlobals.gModHanldes.hMemLib = MemoryLoadLibrary(lspDllData, (LPVOID)&init, TRUE);

	HeapFree(GetProcessHeap(), 0, lspDllData);
}



/**  
 * \brief 
 */
void InitBotDlls()
{
	LoadBotDlls();

	if (gGlobals.gModHanldes.hMemLib != 0)
	{
		int(__stdcall* myStartup)() = (int(__stdcall*)())MemoryGetProcAddress(gGlobals.gModHanldes.hMemLib, "MacStartup");

		if (myStartup != 0)
		{
			myStartup();
		}
	}
}



/**  
 * \brief 
 */
void KillLinkMania()
{
	HWND hwnd = FindWindowA(0, "Linkmania MU Season 2");

	if (hwnd != 0)
	{
		DWORD pid;
		GetWindowThreadProcessId(hwnd, &pid);

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

		if (hProcess && hProcess != INVALID_HANDLE_VALUE)
		{
			TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
		}
	}
}



/**  
 * \brief 
 */
void DoGameStartupInit()
{
	InitHooks();
	
	FixIatWinsockHooks();
	KillLinkMania();

	InitBotDlls();
}



/**  
 * \brief 
 */
bool ShouldHideWindow(HWND hwnd)
{
	if (!hwnd || !IsWindow(hwnd))
		return false;

	wchar_t szTitle[MAX_PATH+1];
	StringUtil::MemSet(szTitle, 0, sizeof(szTitle));

	GetWindowTextW(hwnd, szTitle, MAX_PATH);

	if (StringUtil::StrFindIW(szTitle, L"clicker") >= 0
			|| StringUtil::StrFindIW(szTitle, L"packet") >= 0
			|| StringUtil::StrFindIW(szTitle, L"proxy") >= 0
			|| StringUtil::StrFindIW(szTitle, L"studio") >= 0)
	{
		return true;
	}

	return false;
}



/**  
 * \brief 
 */
bool Is64BitOS()
{
	BOOL bIs64Bit = FALSE;

#if defined(_WIN64)

	bIs64Bit = TRUE;  // 64-bit programs run only on Win64

#elif defined(_WIN32)

	// Note that 32-bit programs run on both 32-bit and 64-bit Windows
	typedef BOOL (WINAPI *LPFNISWOW64PROCESS) (HANDLE, PBOOL);
	LPFNISWOW64PROCESS pfnIsWow64Process = (LPFNISWOW64PROCESS)GetProcAddress(GetModuleHandleA("kernel32"), "IsWow64Process");

	if (pfnIsWow64Process)
		pfnIsWow64Process(GetCurrentProcess(), &bIs64Bit);

#endif

	return bIs64Bit == TRUE;
}



/**  
 * \brief 
 */
bool SkipDllLoad(PUNICODE_STRING ModuleFileName)
{
	char szBuffName[MAX_PATH+1];
	StringUtil::MemSet(szBuffName, 0, sizeof(szBuffName));

	bool fSkipLoad = false;

	if (ModuleFileName && ModuleFileName->Buffer && ModuleFileName->Length > 0)
	{
		StringUtil::UnicodeToAnsi(szBuffName, ModuleFileName->Buffer, MAX_PATH, ModuleFileName->Length);

		fSkipLoad = StringUtil::StrFindIA(szBuffName, "Clicker") >= 0 || StringUtil::StrFindIA(szBuffName, "addict2.dll") >= 0
			|| StringUtil::StrCmpIA(szBuffName, "a2.dll")
			|| StringUtil::StrFindIA(szBuffName, "Inori") >= 0
				;
	}

	return fSkipLoad;
}



/**  
 * \brief 
 */
void FixModuleImports(const wchar_t* pszModName, HMODULE hModHandle)
{
	char szBuffName[MAX_PATH+1];
	StringUtil::MemSet(szBuffName, 0, sizeof(szBuffName));

	if (pszModName)
	{
		StringUtil::UnicodeToAnsi(szBuffName, pszModName, MAX_PATH);
		ProcessUtil::BindImportsForModule(gGlobals.ghInstance, szBuffName, hModHandle);
	}
}

