#include "stdafx.h"
#include "MultihitPacketFilter.h"
#include "CommonPackets.h"

/**
 * \brief 
 */
CMultihitPacketFilter::CMultihitPacketFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy)
{
	m_ulHitCount = 0;
	m_dwHitDelay = 25;

	m_fSuspended = false;

	InitializeCriticalSection(&m_csQueue);

	m_hHitEvent = CreateEvent(0, 1, 0, 0);
	m_hStopEvent = CreateEvent(0, 1, 0, 0);
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HitThreadProc, this, 0, 0);
}


/**
 * \brief 
 */
CMultihitPacketFilter::~CMultihitPacketFilter()
{	
	if (m_hThread && m_hThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hStopEvent);

		Sleep(100);
		
		if (WaitForSingleObject(m_hThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 0);

		CloseHandle(m_hThread);
	}

	CloseHandle(m_hStopEvent);
	CloseHandle(m_hHitEvent);
	DeleteCriticalSection(&m_csQueue);
}


/**
 * \brief 
 */
int CMultihitPacketFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CObjectDeathPacket::Type())
	{
		EnterCriticalSection(&m_csQueue);
		m_vHitQueue.clear();
		LeaveCriticalSection(&m_csQueue);
	}

	return 0;
}


/**
 * \brief 
 */
int CMultihitPacketFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_ulHitCount != 0 && pkt.GetInjected() < 2
			&& (pkt == CSingleSkillPacket::Type() 
				|| pkt == CMassiveSkillPacket::Type()
				|| pkt == CNormalAttackPacket::Type()))
	{
		CAutoLockQueue cs(&m_csQueue);
		m_vHitQueue.clear();

		int iSize = (int)m_ulHitCount;

		for (int i = 0; i < iSize; i++)
		{
			CPacket newPkt = pkt;
			newPkt.SetInjected(2);

			m_vHitQueue.push_back(newPkt);
		}

		SetEvent(m_hHitEvent);
		Sleep(0);
	}


	return 0;
}


/**
 * \brief 
 */
bool CMultihitPacketFilter::GetParam(const char* pszParam, void* pData)
{
	return false;
}


/**
 * \brief 
 */
bool CMultihitPacketFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "hitcount") == 0)
	{
		m_ulHitCount = *((ULONG*)pData);

		if (m_ulHitCount == 0)
		{
			CAutoLockQueue cs(&m_csQueue);
			m_vHitQueue.clear();
		}
		else if (m_ulHitCount > 1000)
			m_ulHitCount = 1000;
	}
	else if (_stricmp(pszParam, "hitdelay") == 0)
	{
		CAutoLockQueue cs(&m_csQueue);
		m_dwHitDelay = *((DWORD*)pData);

		if (m_dwHitDelay > 2000)
			m_dwHitDelay = 2000;
	}
	else if (_stricmp(pszParam, "suspended") == 0)
	{
		m_fSuspended = *((bool*)pData);
	}

	return true;
}


/**
 * \brief 
 */
DWORD CMultihitPacketFilter::HitThreadProc(CMultihitPacketFilter* pThis)
{
	while (1)
	{
		HANDLE objs[] = {pThis->m_hStopEvent, pThis->m_hHitEvent};
		DWORD dwRes = WaitForMultipleObjects(2, objs, 0, INFINITE);

		if (dwRes == WAIT_OBJECT_0)
			break;
		
		pThis->DoNextHit();
		Sleep(0);
	}

	return 0;
}


/**
 * \brief 
 */
void CMultihitPacketFilter::DoNextHit()
{
	EnterCriticalSection(&m_csQueue);

	DWORD dwDelay = m_dwHitDelay;
	int   iSize = (int)m_vHitQueue.size();
	bool  fSuspended = m_fSuspended;
	CPacket pkt;

	if (iSize > 0)
	{
		pkt = m_vHitQueue[0];
		m_vHitQueue.erase(m_vHitQueue.begin());
	}

	LeaveCriticalSection(&m_csQueue);

	if (iSize > 0 && !m_fSuspended)
		GetProxy()->send_lop_packet(pkt);


	if (iSize <= 1)
		ResetEvent(m_hHitEvent);
	else
		Sleep(dwDelay);
}
