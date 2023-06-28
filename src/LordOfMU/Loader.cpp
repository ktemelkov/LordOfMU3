#include "stdafx.h"
#include "Loader.h"
#include "..\_Shared\InjectUtil.h"
#include "../_Shared/InternetUtil.h"
#include "../_Shared/PathUtil.h"
#include "../_Shared/KernelUtil.h"
#include "../_Shared/MemoryModule.h"
#include "../_Shared/PackUtil.h"
#include "../_Shared/MuUtil.h"
#include "../_Shared/StdString.h"
#include "../_Shared/ProtocolSettings.h"
#include "DebugMonitor.h"
#include "Debugger.h"
#include <stdio.h>
#include "../_Shared/Version.h"
#include <ShellAPI.h>
#include "psapi.h"
#include "../_Shared/BufferUtil.h"
#include "../_Shared/base64.h"
#include "../_Shared/ProtocolSettings.h"


LPVOID CLoader::s_pInetFile = 0;
DWORD CLoader::s_dwDlSize = 0;


/**  
 * \brief 
 */
int CLoader::FakeLoad()
{
	if (CKernelUtil::IsDebuggerPresent())
	{
		CMuUtil::RegisterDebugUser();
		CMuUtil::CrashProcess();
	}

	if (0 == FindWindowA(0, "MU AutoClicker (Elite) V" __SOFTWARE_VERSION_STR))
		RunMU();

	return 0;
}


/**  
 * \brief 
 */
void CLoader::RunMU()
{
	CProtocolSettings settings;
	settings.Load();

	char szDir[_MAX_PATH+1] = {0};
	strcpy_s(szDir, _MAX_PATH, settings.data.szMuPath);
	for (int i=(int)strlen(szDir)-1; i>=0 && szDir[i] != '\\'; szDir[i--] = 0);

	ShellExecuteA(0, "open",  settings.data.szMuPath, 0, szDir, SW_SHOWNORMAL);
}



/**  
 * \brief 
 */
void CALLBACK CLoader::InstallAPC(ULONG_PTR data)
{
	_DBGPRINT2("CLoader::InstallAPC() called. data = 0x%08X\r\n", data);

	if (s_pInetFile == 0)
	{
		return;
	}

	_DBGPRINT2("CLoader::InstallAPC() s_pInetFile = 0x%08X\r\n", s_pInetFile);

	BYTE code[] = __SOFTWARE_VERSION_STR;
	int codeLen = sizeof(code)-1;

	for (int i=0; i < (int)s_dwDlSize; i++)
	{
		((BYTE*)s_pInetFile)[i] ^= code[i % codeLen];
	}

	PDWORD pData = (PDWORD)CInjectUtil::SearchDWORD(s_pInetFile, LDR_CODE_END, s_dwDlSize);


	if (pData)
	{
		CProtocolSettings settings;
		settings.Load();

		if ((DWORD)(pData + 1) - (DWORD)s_pInetFile < s_dwDlSize)
		{
			settings.data.dwFlags = *(pData+1);
		}
		else
		{
			settings.data.dwFlags = 0;
		}

		settings.Save();

		CPackUtil packer;
		packer.Init(CPathUtil::ConstructModulePath(CStringTable::GetString(_STRING_LordOfMU_BootstrapperName).c_str()).c_str());

		int lSize = packer.GetPackedFileSize(_PACKUTIL_INSTALLER_FILE_ID);
		BYTE* lspDllData = new BYTE[lSize];

		packer.ExtractFileToMemory(lspDllData, lSize, _PACKUTIL_INSTALLER_FILE_ID);
		CPackUtil::VersionCrypt(lspDllData, lSize);


		char pszDllPath[_MAX_PATH+1] = {0};

		strcpy_s(pszDllPath, _MAX_PATH, CPathUtil::GetModuleFileNameA(ghInstance).c_str());


		DWORD dwOldProt = 0;
		VirtualProtect(s_pInetFile, s_dwDlSize, PAGE_EXECUTE_READWRITE, &dwOldProt);

		void(*proc)(BYTE*,int) = (void(*)(BYTE*,int))s_pInetFile;

		proc(lspDllData, lSize);

		_DBGPRINT2("  lspDllData = %s\r\n", (char*)lspDllData);
		_DBGPRINT2("  lSize = %d\r\n", lSize);
		_DBGPRINT2("  pszDllPath = %s\r\n", (char*)pszDllPath);

		MemoryLoadLibrary(lspDllData, (data == 1) ? (LPVOID)pszDllPath: 0, TRUE);

		VirtualProtect(s_pInetFile, s_dwDlSize, dwOldProt, 0);
		delete[] lspDllData;

		_DBGPRINT("  Loader initialized.\r\n");
	}

	delete[] s_pInetFile;
	s_pInetFile = 0;
	s_dwDlSize = 0;
}



/**  
 * \brief 
 */
DWORD CALLBACK CLoader::FakeInstallAPC(ULONG_PTR hWnd, ULONG_PTR fDebug, ULONG_PTR fInst)
{
	__asm
	{
		nop
		nop
		nop
		nop
		nop
	}

	if (!s_pInetFile)
		return 0;

	if (fDebug != 0)
		return 0;

	PDWORD pData = (PDWORD)CInjectUtil::SearchDWORD(s_pInetFile, LDR_CODE_END, s_dwDlSize);

	if (!pData && fInst != 0)
	{
		FILE* f = 0;

		if (0 == fopen_s(&f, CPathUtil::ConstructModulePath("MUAutoLoader.dll").c_str(), "rb"))
		{
			fseek(f, 0, SEEK_END);
			DWORD dwSize = (DWORD)ftell(f);

			fseek(f, 0, SEEK_SET);

			BYTE* pFile = new BYTE[dwSize];
			memset(pFile, 0, dwSize);

			fread(pFile, 1, dwSize, f);
			fclose(f);


			DWORD dwOldProt = 0;
			VirtualProtect(s_pInetFile, s_dwDlSize, PAGE_EXECUTE_READWRITE, &dwOldProt);

			void(*proc)(BYTE*,int) = (void(*)(BYTE*,int))s_pInetFile;
			proc(pFile, dwSize);

			VirtualProtect(s_pInetFile, s_dwDlSize, dwOldProt, 0);

			MemoryFreeLibrary(MemoryLoadLibrary(pFile, 0, TRUE));

			delete[] pFile;
		}
		else
			PostMessageA((HWND)hWnd, WM_USER + 125, 0, 0);
	}

	delete[] s_pInetFile;
	s_pInetFile = 0;
	s_dwDlSize = 0;

	return 0;
}



/**  
 * \brief 
 */
void CLoader::InetReadResponse(HINTERNET hConn)
{
	DWORD dwSize = _DEFAULT_DOWNLOAD_BUFFER_SIZE;	
	s_pInetFile = new BYTE[dwSize];
	memset(s_pInetFile, 0, dwSize);

	s_dwDlSize = 0;

	if (!CInternetUtil::InternetReadFile(hConn, s_pInetFile, dwSize, &s_dwDlSize))
	{
		_DBGPRINT("Cannot read Internet file\r\n");

		delete[] s_pInetFile;
		s_pInetFile = 0;
		s_dwDlSize = 0;
	}

	_DBGPRINT2("Internet file size: %d\r\n", s_dwDlSize);
}



/**
 * \brief
 */
CStdString CLoader::MyBufferToHex(BYTE* buf, int len)
{
	CStdString res("");

	if (buf && len != 0)
	{
		for (int i=0; i < len; ++i)
		{
			char hex_buff[4] = {0};

			hex_buff[0] = (buf[i] >> 4) & 0xF;
			hex_buff[1] = buf[i] & 0xF;

			hex_buff[0] += (hex_buff[0] < 10) ? 0x30 : 0x37;
			hex_buff[1] += (hex_buff[1] < 10) ? 0x30 : 0x37;

			hex_buff[2] = ' ';

			res += hex_buff;
		}
	}

	return res;
}


/**  
 * \brief 
 */
void CALLBACK CLoader::DownloadAPC(ULONG_PTR)
{
	_DBGPRINT("CLoader::DownloadAPC() called. \r\n");


	HINTERNET hInet = CInternetUtil::InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

	if (hInet != 0)
	{
		DWORD dwFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
			| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_IGNORE_CERT_CN_INVALID
			| INTERNET_FLAG_NEED_FILE | INTERNET_FLAG_KEEP_CONNECTION
			| INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI
			| INTERNET_FLAG_RELOAD;

		//
		// http://muonline.ktemelkov.info/util/v2.php?arg1=/f2Nj/mIjonh9Inh+v7h/4jhj//hj4jh9Ymbm+vpn+7o74eS74ecmIeZ7ofpmYfp7oeT7w==
		//
		char pVerBuff[] = __SOFTWARE_VERSION_STR;
		int len = sizeof(pVerBuff) - 1;

		CStdString strVer("");
		
		if (len > 0)
			strVer = CStdString("&arg3=") + base64_encode((BYTE*)pVerBuff, len);

		CStdString strUrl = CStringTable::GetString(_STRING_RegisterUrl) + CMuUtil::GetSerial() + strVer;
		const char* pszHeaders = "User-Agent: MUAutoClicker\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";

		_DBGPRINT2("Loader URL -> %s\r\n", strUrl.c_str());

		HINTERNET hConn = CInternetUtil::InternetOpenUrlA(hInet, strUrl.c_str(), pszHeaders, -1, dwFlags, 0);

		if (hConn)
		{
			_DBGPRINT("Internet connection successfully opened.\r\n");

			InetReadResponse(hConn);
			CInternetUtil::InternetCloseHandle(hConn);
		}
		else
		{
			DWORD dwError = 0;// GetLastError();
			char pszError[512] = {0};
			DWORD dwBufLen = 511;

			CInternetUtil::InternetGetLastResponseInfoA(&dwError, pszError, &dwBufLen);

			_DBGPRINT2("InternetOpenUrlA() failed. -> %s", pszError);
		}

		CInternetUtil::InternetCloseHandle(hInet);
	}

/*
	FILE* f=0;

	fopen_s(&f, CPathUtil::ConstructModulePath("LspDecrypt.bin").c_str(), "rb");
	fseek(f, 0, SEEK_END);
	s_dwDlSize = (DWORD)ftell(f);
	fseek(f, 0, SEEK_SET);

	s_pInetFile = new BYTE[s_dwDlSize];
	memset(s_pInetFile, 0, s_dwDlSize);

	fread(s_pInetFile, 1, s_dwDlSize, f);
	fclose(f);
*/
}



/**  
 * \brief 
 */
DWORD CALLBACK CLoader::FakeDownloadAPC(ULONG_PTR hWnd, ULONG_PTR fDebug, ULONG_PTR fDownload)
{
	__asm
	{
		nop
		nop
		nop
		nop
		nop
	}

	if (fDebug != 0)
		return 0;

	HINTERNET hInet = CInternetUtil::InternetOpenA("", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);

	if (hInet != 0)
	{
		DWORD dwFlags = INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS
			| INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_PRAGMA_NOCACHE
			| INTERNET_FLAG_RELOAD;

		HINTERNET hConn = CInternetUtil::InternetOpenUrlA(hInet, (CStdString("http://muonline.ktemelkov.info/util/v2.php?arg2=") + CMuUtil::FakeGetSerial()).c_str(), 0, -1, dwFlags, 0);

		if (hConn)
		{
			if (fDownload != 0)
				InetReadResponse(hConn);

			CInternetUtil::InternetCloseHandle(hConn);
		}
		else
		{
			PostMessageA((HWND)hWnd, WM_USER + 124, 0, 0);
		}

		CInternetUtil::InternetCloseHandle(hInet);
	}
	else
	{
		PostMessageA((HWND)hWnd, WM_USER + 125, 0, 0);
	}

	return 0;
}


/**  
 * \brief 
 */
DWORD CALLBACK CLoader::FakeSleepEx(DWORD dwTime, BOOL fAlert)
{
	__asm
	{
		nop
		nop
		nop
		nop
		nop
	}

	_DBGPRINT("CLoader::FakeSleepEx() called. \r\n");
	return (BOOL)PostMessageA((HWND)dwTime, (UINT)fAlert, 0, 0);
}




DWORD gdwDebuggerPid = 0;
char gszDebuggerName[MAX_PATH+1] = {0};


/**  
 * \brief 
 */
int CLoader::Load()
{
#ifndef _DEBUG
	DWORD dwPid = gdwDebuggerPid;


	_DBGPRINT2("Debugger pid = %d\r\n", dwPid);

	if (!CKernelUtil::IsDebuggerPresent())
	{
		CMuUtil::RegisterDebugUser();
		CMuUtil::CrashProcess();
	}

	BOOL fDbg = FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	
	if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
	{
		_DBGPRINT("Cannot open a handle to debugger process\r\n");
		return 0;
	}

	char szImageName[_MAX_PATH+1] = {0};
	GetProcessImageFileNameA(hProcess, szImageName, _MAX_PATH);

	CKernelUtil::CheckRemoteDebuggerPresent(hProcess, &fDbg);

	_DBGPRINT2("Debugger process filename: %s\r\n", szImageName);
	
	char* pszExeName = szImageName + lstrlenA(szImageName);

	while (pszExeName > (szImageName + 1) && *(pszExeName-1) != '\\') pszExeName--;
	
	if (fDbg || StrCmpIA(pszExeName, gszDebuggerName) != 0)
	{
		_DBGPRINT("Invalid debugger process\r\n");
		CMuUtil::RegisterDebugUser();
		CMuUtil::CrashProcess();
	}
#endif


	CKernelUtil::QueueUserAPC(DownloadAPC, GetCurrentThread(), 0);
	CKernelUtil::SleepEx(0, TRUE);

	CKernelUtil::QueueUserAPC(InstallAPC, GetCurrentThread(), 1);
	CKernelUtil::SleepEx(0, TRUE);

#ifndef _DEBUG
	__asm int 3;

	if (WAIT_TIMEOUT != WaitForSingleObject(hProcess, 15000))
	{
		RunMU();
	}

	CloseHandle(hProcess);
#else
	RunMU();
#endif

	return 0;
}
