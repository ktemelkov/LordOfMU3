
#include "stdafx.h"
#include "DebugMonitor.h"
#include "../_Shared/KernelUtil.h"
#include "../_Shared/MuUtil.h"
#include "Loader.h"

DWORD CDebugMonitor::s_dwDbgProcessId = 0;

/**  
 * \brief 
 */
bool CDebugMonitor::Run()
{
	if (SIMM_EXT_DEBUGGER)
		return DryRun();
	else if (!CKernelUtil::IsDebuggerPresent())
		return false;

	Stop();

	if (m_hStopEv)
		ResetEvent(m_hStopEv);

	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MonitorThread, (LPVOID)this, 0, 0);

	if (m_hThread == INVALID_HANDLE_VALUE)
		m_hThread = 0;

	return m_hThread != 0;
}


/**  
 * \brief 
 */
void CDebugMonitor::Stop()
{
	if (m_hThread)
	{
		if (m_hStopEv)
			SetEvent(m_hStopEv);

		if (WaitForSingleObject(m_hThread, 3000) != WAIT_TIMEOUT)
		{
			CloseHandle(m_hThread);
			m_hThread = 0;
		}
	}
}


/**  
 * \brief 
 */
bool CDebugMonitor::CheckRunning()
{
	if (m_fDryRun)
		return true;

	return (m_hThread != 0 && WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT);
}


/**  
 * \brief 
 */
DWORD CALLBACK CDebugMonitor::MonitorThread(CDebugMonitor* pThis)
{
	_DBGPRINT("CDebugMonitor::MonitorThread() started.\r\n");

	if (!pThis)
		return 0;

	if (!s_dwDbgProcessId)
	{
		CMuUtil::RegisterDebugUser();
		CMuUtil::CrashProcess();
		return 0;
	}


	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, CDebugMonitor::s_dwDbgProcessId);

	if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
	{
		CMuUtil::RegisterDebugUser();
		CMuUtil::CrashProcess();
		return 0;
	}

	
	BYTE* pAddr1 = (BYTE*)CLoader::FakeInstallAPC;
	BYTE* pAddr2 = (BYTE*)CLoader::FakeDownloadAPC;

	DWORD dwWritten = 0;
	char buff[] = "\x90\x90\x90\x90\x90";

	CKernelUtil::WriteProcessMemory(hProcess, pAddr1, buff, 5, &dwWritten);
	CKernelUtil::WriteProcessMemory(hProcess, pAddr2, buff, 5, &dwWritten);


	BOOL fRemDebug = FALSE;

	while (WaitForSingleObject(pThis->m_hStopEv, 250) == WAIT_TIMEOUT)
	{
		if (!CKernelUtil::IsDebuggerPresent() || !CKernelUtil::CheckRemoteDebuggerPresent(hProcess, &fRemDebug) || fRemDebug)
		{
			_DBGPRINT("CDebugMonitor::MonitorThread() terminating ....\r\n");

			CMuUtil::RegisterDebugUser();

			CloseHandle(hProcess);

			CMuUtil::CrashProcess();

			return 0;
		}
	}

	CloseHandle(hProcess);
	return 0;
}
