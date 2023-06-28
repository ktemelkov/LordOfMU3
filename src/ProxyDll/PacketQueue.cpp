#include "stdafx.h"
#include "PacketQueue.h"

/**
 *
 */
CPacketQueue::CPacketQueue()
{
	InitializeCriticalSection(&m_cs);
}

/**
 *
 */
CPacketQueue::~CPacketQueue()
{
	DeleteCriticalSection(&m_cs);
}

/**
 *
 */
void CPacketQueue::QueuePacket(const CPacket& pkt, ULONG ulFlags)
{
	EnterCriticalSection(&m_cs);
	m_vQueue.push_back(pkt);
	m_vQueueFlags.push_back(ulFlags);
	LeaveCriticalSection(&m_cs);
}

/**
 *
 */
bool CPacketQueue::GetNextPacket(CPacket& out, ULONG* pulFlags)
{
	EnterCriticalSection(&m_cs);
	int iCount = (int)m_vQueue.size();

	if (iCount > 0)
	{
		out = m_vQueue[0];

		if (pulFlags)
			*pulFlags = m_vQueueFlags[0];

		m_vQueue.pop_front();
		m_vQueueFlags.pop_front();
	}

	LeaveCriticalSection(&m_cs);

	return iCount > 0;
}


/**
 * \brief 
 */
bool CPacketQueue::GetPackets(std::vector<CPacket>& out, std::vector<ULONG>& vFlags, int& iCount, int maxSize, bool* pfHasMore)
{
	EnterCriticalSection(&m_cs);
	if (iCount > (int)m_vQueue.size())
		iCount = (int)m_vQueue.size();

	out.clear();
	vFlags.clear();

	int iSize = 0;

	if (iCount > 0)
	{
		for (int i=0; i < iCount; i++)
		{
			if (maxSize > 0)
			{
				iSize += m_vQueue[0].GetPktLen();

				if (iSize > maxSize)
				{
					iCount = i;
					break;
				}
			}
			
			out.push_back(m_vQueue[0]);
			m_vQueue.pop_front();

			vFlags.push_back(m_vQueueFlags[0]);
			m_vQueueFlags.pop_front();
		}
	}

	if (pfHasMore)
		*pfHasMore = m_vQueue.size() != 0;

	LeaveCriticalSection(&m_cs);

	return iCount > 0;
}

/**
 * \brief 
 */
void CPacketQueue::UnGetPacket(CPacket& pkt, ULONG ulFlags)
{
	EnterCriticalSection(&m_cs);
	m_vQueue.push_front(pkt);
	m_vQueueFlags.push_front(ulFlags);
	LeaveCriticalSection(&m_cs);
}


/**
 *
 */
bool CPacketQueue::IsPacketAvailable()
{
	EnterCriticalSection(&m_cs);
	int iCount = (int)m_vQueue.size();
	LeaveCriticalSection(&m_cs);

	return iCount > 0;
}


/**
 * \brief 
 */
int CPacketQueue::GetPacketCount()
{
	EnterCriticalSection(&m_cs);
	int iCount = (int)m_vQueue.size();
	LeaveCriticalSection(&m_cs);

	return iCount;
}
