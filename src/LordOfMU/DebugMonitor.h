#pragma once


/**  
 * \brief 
 */
class CDebugMonitor
{
public:
	CDebugMonitor(DWORD dwProcId) 
		: m_fDryRun(false), m_hThread(0) 
	{ 
		s_dwDbgProcessId = dwProcId; 

		m_hStopEv = CreateEvent(0, 1, 0, 0);

		if (m_hStopEv == INVALID_HANDLE_VALUE)
			m_hStopEv = 0;
	}

	virtual ~CDebugMonitor()
	{
		if (m_hStopEv)
			CloseHandle(m_hStopEv);
	}


	bool Run();
	bool DryRun(){ m_fDryRun = true; return true; }
	void Stop();

	bool CheckRunning();

	static DWORD CALLBACK MonitorThread(CDebugMonitor* pThis);

public:
	static DWORD s_dwDbgProcessId;
	bool m_fDryRun;

private:
	HANDLE m_hThread;
	HANDLE m_hStopEv;
};