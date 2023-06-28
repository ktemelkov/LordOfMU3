#include "stdafx.h"
#include "GameProxy.h"
#include "DebugOut.h"
#include "BufferUtil.h"
#include "CommonPackets.h"
#include "LordOfMUdll.h"


/**
 *
 */
CGameProxy::CGameProxy(IConnectionHandler* pHandler)
	: CProxy(pHandler)
{
	CProxyClickerModule::GetInstance()->m_pGameProxy = this;
}


/**
 *
 */
CGameProxy::~CGameProxy()
{
	ClearFilters();	

	CProxyClickerModule::GetInstance()->m_pGameProxy = 0;
}



/**  
 * \brief 
 */
bool CGameProxy::ProcessSendStream(char* lpBuffer, int iLen)
{
	m_cSendPacketParser.AppendStream((BYTE*)lpBuffer, iLen);

	while (1)
	{
		while (m_cSendPacketParser.Next());

		CPacket pkt;
		if (m_cSendPacketParser.GetPacket(pkt))
		{
			m_cSendQueue.QueuePacket(pkt);
			continue;
		}
		else if (m_cSendPacketParser.HasError())
		{
			CDebugOut::PrintError("Error parsing send packet.");
		}

		break;
	}

	return true;
}


/**  
 * \brief 
 */
bool CGameProxy::ProcessRecvStream(char* lpBuffer, int iLen)
{
	m_cRecvPacketParser.AppendStream((BYTE*)lpBuffer, iLen);

	while (1)
	{
		while (m_cRecvPacketParser.Next());

		CPacket pkt;
		if (m_cRecvPacketParser.GetPacket(pkt))
		{
			m_cRecvQueue.QueuePacket(pkt);
			continue;
		}
		else if (m_cRecvPacketParser.HasError())
		{
			CDebugOut::PrintError("Error parsing received packet.");
		}

		break;
	}

	return true;
}

/**  
 * \brief 
 */
bool CGameProxy::GetRecvBuffer(char* lpBuffer, int& iLen, bool& fHasMore)
{
	int iCount = 25;
	std::vector<CPacket> vPackets; 
	std::vector<ULONG> vFlags;

	m_cRecvQueue.GetPackets(vPackets, vFlags, iCount, iLen, &fHasMore);

	iLen = 0;

	for (int i = 0; i < iCount; i++)
	{
		CPacket& pkt = vPackets[i];
		ULONG    ulFlags = vFlags[i];

		if (ulFlags == QUEUE_FLAG_DIRECT || !FilterRecvPacket(pkt))
		{
			char* buf = (char*)pkt.GetRawPacket();
			int len = (int)pkt.GetPktLen();

			memcpy(lpBuffer + iLen, buf, len);
			iLen += len;
		}
		else
		{
			CDebugOut::PrintDebug("- Packet blocked");
		}

		HANDLE hEvent = pkt.GetEvent();
		
		if (hEvent)
			SetEvent(hEvent);
	}

	return true;
}


/**  
 * \brief 
 */
bool CGameProxy::GetSendBuffer(char* lpBuffer, int& iLen, bool& fHasMore)
{
	int iCount = 25;
	std::vector<CPacket> vPackets; 
	std::vector<ULONG> vFlags;

	m_cSendQueue.GetPackets(vPackets, vFlags, iCount, iLen, &fHasMore);

	iLen = 0;

	for (int i = 0; i < iCount; i++)
	{
		CPacket& pkt = vPackets[i];
		ULONG    ulFlags = vFlags[i];

		if (ulFlags == QUEUE_FLAG_DIRECT || !FilterSendPacket(pkt))
		{
			char* buf = (char*)pkt.GetRawPacket();
			int len = (int)pkt.GetPktLen();

			memcpy(lpBuffer + iLen, buf, len);
			iLen += len;
		}
		else
		{
			CDebugOut::PrintDebug("- Packet blocked");
		}

		HANDLE hEvent = pkt.GetEvent();

		if (hEvent)
			SetEvent(hEvent);
	}

	return true;
}



/**  
 * \brief 
 */
bool CGameProxy::HasIngoingPackets()
{
	return m_cRecvQueue.IsPacketAvailable();
}


/**  
 * \brief 
 */
bool CGameProxy::HasOutgoingPackets()
{
	return m_cSendQueue.IsPacketAvailable();
}



/**
 *
 */
bool CGameProxy::FilterRecvPacket(CPacket& pkt)
{
	int iCount = (int)m_cFilterChain.size();
	int iLvl = INT_MAX;

	CFilterContext context;

	for (int i=0; i < iCount; ++i)
	{
		CPacketFilter* pFilter = m_cFilterChain[i];

		if (pFilter && (int)pFilter->GetLevel() < iLvl)
		{
			int iRet = pFilter->FilterRecvPacket(pkt, context);

			if (iRet == -1)
				return true;

			if (iRet > 0)
				iLvl = iRet;
		}
	}

	return false;
}


/**
 *
 */
bool CGameProxy::FilterSendPacket(CPacket& pkt)
{
	int iCount = (int)m_cFilterChain.size();
	int iLvl = INT_MAX;

	CFilterContext context;

	for (int i=0; i < iCount; ++i)
	{
		CPacketFilter* pFilter = m_cFilterChain[i];

		if (pFilter && (int)pFilter->GetLevel() < iLvl)
		{
			int iRet = pFilter->FilterSendPacket(pkt, context);

			if (iRet == -1)
				return true;

			if (iRet > 0)
				iLvl = iRet;
		}
	}

	return false;
}

/**
 *
 */
bool CGameProxy::send_packet(CPacket& pkt)
{
	m_cSendQueue.QueuePacket(pkt);
	return true;
}


/**
 * \brief 
 */
bool CGameProxy::send_lop_packet(CPacket& pkt)
{
	m_cSendQueue.QueuePacket(pkt);
	return true;
}


/**
 *
 */
bool CGameProxy::recv_packet(CPacket& pkt)
{
	m_cRecvQueue.QueuePacket(pkt);	
	return true;
}


/**
 *
 */
bool CGameProxy::send_direct(CPacket& pkt)
{
	m_cSendQueue.QueuePacket(pkt, QUEUE_FLAG_DIRECT);
	return true;
}

/**
 *
 */
bool CGameProxy::recv_direct(CPacket& pkt)
{
	m_cRecvQueue.QueuePacket(pkt, QUEUE_FLAG_DIRECT);
	return true;
}

/**
 *
 */
bool CGameProxy::AddFilter(CPacketFilter* pFilter)
{
	if (!pFilter)
		return false;

	if (GetFilter(pFilter->GetName()) != 0)
		return false; // already exists

	m_cFilterChain.push_back(pFilter);
	return true;
}

/**
 *
 */
CPacketFilter* CGameProxy::GetFilter(const char* szName)
{
	for (int i=(int)m_cFilterChain.size()-1; i >= 0; --i)
	{
		if (m_cFilterChain[i] != 0 && _stricmp(m_cFilterChain[i]->GetName(), szName) == 0)
			return m_cFilterChain[i];
	}

	return 0;
}

/**
 *
 */
void CGameProxy::ClearFilters()
{
	for (int i=(int)m_cFilterChain.size()-1; i >= 0; --i)
	{
		if (m_cFilterChain[i])
			m_cFilterChain[i]->Term();
	}

	for (int i=(int)m_cFilterChain.size()-1; i >= 0; --i)
	{
		if (m_cFilterChain[i])
		{
			delete m_cFilterChain[i];
			m_cFilterChain[i] = 0;
		}
	}

	m_cFilterChain.clear();
}


/**
 * \brief 
 */
bool CGameProxy::GetParam(const char* pszParam, void* pData)
{
	if (!pszParam)
		return false;

	return false;
}


/**
 * \brief 
 */
bool CGameProxy::SetParam(const char* pszParam, void* pData)
{
	return false;
}

