#include "stdafx.h"
#include "RecordRoutePacketFilter.h"
#include "CommonPackets.h"

/**  
 * \brief 
 */
CRecordRoutePacketFilter::CRecordRoutePacketFilter(CProxy* pProxy)
	: CPacketFilter(pProxy) 
{
	m_wPlayerId = 0;
	m_fEnabled = false;
}


/**  
 * \brief 
 */
int CRecordRoutePacketFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();
	}
	else if (pkt == CObjectMovedPacket::Type() && m_fEnabled)
	{
		CObjectMovedPacket& pkt2 = (CObjectMovedPacket&)pkt;
		
		if (pkt2.GetId() == m_wPlayerId)
		{
			BYTE x = pkt2.GetX();
			BYTE y = pkt2.GetY();

			m_vRoute.push_back(MAKEWORD(x, y));
		}
	}

	return 0;
}


/**  
 * \brief 
 */
int CRecordRoutePacketFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	return 0;
}


/**  
 * \brief 
 */
bool CRecordRoutePacketFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "start") == 0)
	{
		m_vRoute.clear();
		m_fEnabled = true;
	}
	else if (_stricmp(pszParam, "stop") == 0)
	{
		m_fEnabled = false;
	}
	else if (_stricmp(pszParam, "clear") == 0)
	{
		m_vRoute.clear();
	}
	else if (_stricmp(pszParam, "save") == 0)
	{
		if (!pData)
			return false;

		char szPath[_MAX_PATH+1] = {0};
		extern TCHAR g_szRoot[_MAX_PATH + 1];

		strcpy(szPath, CT2A(g_szRoot));
		strcat(szPath, (char*)pData);
			
		return SaveToFile(szPath);
	}
	else
		return false;

	return true;
}


/**  
 * \brief 
 */
bool CRecordRoutePacketFilter::GetParam(const char* pszParam, void* pData)
{
	return false;
}


/**  
 * \brief 
 */
bool CRecordRoutePacketFilter::SaveToFile(const char* pszFilename)
{
	FILE* f = fopen(pszFilename, "w");

	if (!f)
		return false;

	fprintf(f, "Recorded Route:\n");

	int count = (int)m_vRoute.size();

	for (int i=0; i < count; i++)
	{
		WORD coord = m_vRoute[i];
		BYTE x = LOBYTE(coord);
		BYTE y = HIBYTE(coord);

		fprintf(f, "%d %d\n", x, y);
	}


	fprintf(f, "\nBackward Route:\n");
	
	for (int i=count-1; i >= 0; i--)
	{
		WORD coord = m_vRoute[i];
		BYTE x = LOBYTE(coord);
		BYTE y = HIBYTE(coord);

		fprintf(f, "%d %d\n", x, y);
	}
	
	fclose(f);

	return true;
}
