#include "stdafx.h"
#include "MyUtils.h"
#include "MyGlobals.h"
#include "ApiHook.h"
#include "MyHooks.h"
#include "StringUtil.h"

#include "..\_Shared\PackUtil.h"
#include "..\_Shared\MemoryModule.h"
#include "shellapi.h"
#include "psapi.h"
#include "version.h"
#include "InstallerServices.h"
#include "tlhelp32.h"



/**  
 * \brief 
 */
void KillLauncher()
{
	if (gGlobals.ghParentProcess != 0 && gGlobals.ghParentProcess != INVALID_HANDLE_VALUE)
	{
		if (gGlobals.gdwPatchProcessEnum)
			TerminateProcess(gGlobals.ghParentProcess, 0);
		
		CloseHandle(gGlobals.ghParentProcess);
		gGlobals.ghParentProcess = 0;
	}
}



/**  
 * \brief 
 */
bool ShouldHideWindow(HWND hwnd)
{
	if (!hwnd)
		return false;

	wchar_t szTitle[MAX_PATH+1];
	StringUtil::MemSet(szTitle, 0, sizeof(szTitle));

	GetWindowTextW(hwnd, szTitle, MAX_PATH);

	if (StringUtil::StrFindIW(szTitle, L"clicker") >= 0
			|| StringUtil::StrFindIW(szTitle, L"packet") >= 0
			|| StringUtil::StrFindIW(szTitle, L"studio") >= 0)
	{
		return true;
	}

	return false;
}




/**  
 * \brief 
 */
void DoModuleInit()
{
	if (gGlobals.gModHanldes.hMemLib)
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

	gGlobals.gModHanldes.hMemLib = MemoryLoadLibrary(lspDllData, (LPVOID)&init);

	HeapFree(GetProcessHeap(), 0, lspDllData);
}



/**  
 * \brief 
 */
bool GetProcessFileName(HANDLE hProcess, wchar_t* pszOut, int iSize) 
{
	if (!pszOut || iSize <= 0)
		return false;

	bool bSuccess = false;
	wchar_t pszFilename[MAX_PATH+1];
	StringUtil::MemSet(pszFilename, 0, sizeof(pszFilename));

	GetProcessImageFileNameW(hProcess, pszFilename, MAX_PATH);

	// Translate path with device name to drive letters.
	wchar_t szTemp[MAX_PATH+1];
	StringUtil::MemSet(szTemp, 0, sizeof(szTemp));

	if (GetLogicalDriveStringsW(MAX_PATH, szTemp)) 
	{
		wchar_t szName[MAX_PATH+1];
		StringUtil::MemSet(szName, 0, sizeof(szName));

		wchar_t szDrive[3] = L" :";		
		bool bFound = false;
		wchar_t* p = szTemp;

		do 
		{
			// Copy the drive letter to the template string
			*szDrive = *p;

			// Look up each device name
			if (QueryDosDeviceW(szDrive, szName, MAX_PATH))
			{
				size_t uNameLen = lstrlenW(szName);
				wchar_t szTemp2[MAX_PATH+1];
				StringUtil::MemSet(szTemp2, 0, sizeof(szTemp2));

				lstrcpynW(szTemp2, pszFilename, uNameLen+1);

				if (uNameLen < MAX_PATH) 
				{
					bFound = lstrcmpiW(szTemp2, szName) == 0;

					if (bFound) 
					{
						if (iSize >= lstrlenW(pszFilename) - (int)uNameLen + 3)
						{
							bSuccess = true;

							lstrcpyW(pszOut, szDrive);
							lstrcatW(pszOut, pszFilename + uNameLen);
						}
					}
				}
			}

			// Go to the next NULL character.
			while (*p++);
		} while (!bFound && *p); // end of string
	}

	return bSuccess;
}



/**  
 * \brief 
 */
BOOL FixProcessScan(LPPROCESSENTRY32W lppe)
{
	if (!lppe)
		return TRUE;

	if (lppe->th32ProcessID == GetCurrentProcessId())
		return TRUE;
	
	wchar_t szThisFile[MAX_PATH+1];
	StringUtil::MemSet(szThisFile, 0, sizeof(szThisFile));

	GetCurrentDirectoryW(MAX_PATH, szThisFile);
	lstrcatW(szThisFile, L"\\");
	lstrcatW(szThisFile, lppe->szExeFile);


	HANDLE hFile = CreateFileW(szThisFile, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return TRUE;
	}


	StringUtil::MemSet(szThisFile, 0, sizeof(szThisFile));

	GetSystemDirectoryW(szThisFile, MAX_PATH);
	lstrcatW(szThisFile, L"\\");
	lstrcatW(szThisFile, lppe->szExeFile);

	hFile = CreateFileW(szThisFile, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile && hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return TRUE;
	}

	return FALSE;
}



