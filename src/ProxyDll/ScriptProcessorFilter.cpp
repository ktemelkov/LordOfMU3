#include "stdafx.h"
#include "ScriptProcessorFilter.h"
#include "ScriptParser.h"
#include "CommonPackets.h"


/**
 * \brief 
 */
CScriptProcessorFilter::CScriptProcessorFilter(CProxy* pProxy)
	: CPacketFilter(pProxy), m_cExecContext(pProxy)
{
	m_wPlayerId = 0;
	m_fRun = false;

	InitializeCriticalSection(&m_cs);

	m_hSuspendedEvent = CreateEvent(0, TRUE, 0, 0);
	m_hEnabledEvent = CreateEvent(0, TRUE, 0, 0);
	m_hStopEvent = CreateEvent(0, TRUE, 0, 0);
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, 0);
}


/**
 * \brief 
 */
CScriptProcessorFilter::~CScriptProcessorFilter()
{
	if (m_hThread != 0 && m_hThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hStopEvent);

		if (WaitForSingleObject(m_hThread, 3000) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 0);

		CloseHandle(m_hThread);
	}

	CloseHandle(m_hSuspendedEvent);
	CloseHandle(m_hEnabledEvent);
	CloseHandle(m_hStopEvent);
	DeleteCriticalSection(&m_cs);
}


/**
 * \brief 
 */
int CScriptProcessorFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();
	}
	else if (pkt == CObjectDeathPacket::Type())
	{
		CObjectDeathPacket& pkt2 = (CObjectDeathPacket&)pkt;

		if (pkt2.GetObjectId() == m_wPlayerId)
		{
			if (IsEnabled())
			{
				if (!m_cExecContext.IsLocked())
					Stop();
			}
		}
	}
	else if (pkt == CWarpReplyPacket::Type())
	{
		if (IsEnabled())
		{
			if (!m_cExecContext.IsLocked())
				Stop();
		}
	}

	return 0;
}


/**
 * \brief 
 */
int CScriptProcessorFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CCharListReqPacket::Type())
	{
		if (IsEnabled())
			Stop();
	}

	return 0;
}


/**
 * \brief 
 */
bool CScriptProcessorFilter::GetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "enable") == 0)
	{
		*((bool*)pData) = IsEnabled();
		return true;
	}

	return false;
}


/**
 * \brief 
 */
bool CScriptProcessorFilter::SetParam(const char* pszParam, void* pData)
{
	if (!pszParam || !pData)
		return false;

	if (_stricmp(pszParam, "enable") == 0)
	{
		m_fRun = false;

		bool fEnabled = *((bool*)pData);

		if (fEnabled)
			Start();
		else
			Stop();

		return true;
	}
	else if (_stricmp(pszParam, "run") == 0)
	{
		m_fRun = true;
		ResetEvent(m_hSuspendedEvent);

		Start();
		return true;
	}
	else if (_stricmp(pszParam, "suspended") == 0 && !m_fRun)
	{
		if (*((bool*)pData))
			SetEvent(m_hSuspendedEvent);
		else
			ResetEvent(m_hSuspendedEvent);

		return true;
	}
	else if (_stricmp(pszParam, "load") == 0)
	{
		m_fRun = false;

		Stop();

		CAutoLockQueue cs(&m_cs);

		if (!Load((char*)pData))
			return false;

		return true;
	}

	return false;
}


/**
 * \brief 
 */
void CScriptProcessorFilter::Start()
{
	SetEvent(m_hEnabledEvent);
}


/**
 * \brief 
 */
void CScriptProcessorFilter::Stop()
{ 
	ResetEvent(m_hEnabledEvent);
}


/**
 * \brief 
 */
void CScriptProcessorFilter::Reset()
{
	for (int i=(int)m_vCommands.size()-1; i >= 0; --i)
	{
		if (m_vCommands[i])
			delete m_vCommands[i];

		m_vCommands[i] = 0;
	}

	m_vCommands.clear();
	m_cExecContext.Reset();
}


/**
 * \brief 
 */
bool CScriptProcessorFilter::Load(const char* pszFile)
{
	if (!pszFile || pszFile[0] == 0)
		return false;

	char szFile[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szFile, CT2A(g_szRoot));
	strcat(szFile, pszFile);

	FILE* f = fopen(szFile, "r");

	if (!f)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Cannot open script file %s!", pszFile));
		return false;
	}

	char szLine[512] = {0};
	char ch = 0;
	int pos = 0;
	bool fResult = true;
	int iLineNo = 1;
	CScriptParser parser;

	while(1)
	{
		bool fStop = (1 != fread(&ch, 1, 1, f));

		if (fStop || ch == '\n' || pos >= 511)
		{
			if (!parser.ParseLine(szLine))
			{
				fResult = false;
				fStop = true;
				
				GetProxy()->recv_direct(CServerMessagePacket(">> %s", parser.GetError()));
				GetProxy()->recv_direct(CServerMessagePacket(">> Bad script error at line %d!", iLineNo));
			}

			iLineNo++;
			pos = 0;
			memset(szLine, 0, 512);
		}
		else
		{
			szLine[pos++] = ch;
		}

		if (fStop)
			break;
	}

	if (fResult)
	{
		CAutoLockQueue cs(&m_cs);

		Reset();
		m_vCommands = parser.Finalize();
	}

	fclose(f);
	return fResult;
}


/**
 * \brief 
 */
DWORD CScriptProcessorFilter::ThreadProc(CScriptProcessorFilter* pThis)
{
	if (!pThis)
		return 0;

	bool fSuspended = false;

	while (1)
	{
		HANDLE vHandles[2] = {pThis->m_hStopEvent, pThis->m_hEnabledEvent};
		DWORD dwRes = WaitForMultipleObjects(2, vHandles, 0, INFINITE);

		if (dwRes != WAIT_OBJECT_0 + 1)
			break;

		EnterCriticalSection(&pThis->m_cs);
		pThis->m_cExecContext.Reset();
		LeaveCriticalSection(&pThis->m_cs);

		while (pThis->IsEnabled())
		{
			bool fSuspended2 = (WaitForSingleObject(pThis->m_hSuspendedEvent, 0) != WAIT_TIMEOUT);
			if (!fSuspended2
					|| pThis->m_cExecContext.IsLocked())
			{
				if (fSuspended != fSuspended2 && !pThis->m_cExecContext.IsLocked())
				{
					Sleep(5000);
				}
				else
					pThis->ExecuteNext();
			}

			fSuspended = fSuspended2;
			Sleep(10);
		}
	}

	return 0;
}


/**
 * \brief 
 */
void CScriptProcessorFilter::ExecuteNext()
{
	CAutoLockQueue cs(&m_cs);

	int idx = m_cExecContext.GetCommandIdx();
	
	if (idx < (int)m_vCommands.size() && m_vCommands[idx] != 0)
	{
		m_vCommands[idx]->Execute(m_cExecContext);
		m_cExecContext.Advance();
	}
	else
	{
		Stop();
	}
}
