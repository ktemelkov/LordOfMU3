#include "stdafx.h"
#include "CharInfoFilter.h"
#include "CommonPackets.h"
#include "version.h"
#include "LordOfMUdll.h"
#include "DebugOut.h"



/**
 * \brief 
 */
CCharInfoFilter::CCharInfoFilter(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
	m_wPlayerId = 0;
	m_wLevel = 0;
	m_bX = 0;
	m_bY = 0;
	m_fExit400 = false;
	m_fSuspended = false;
	m_fStealth = true;
	m_fMeetPlayerMessage = true;
	m_fLockAg = false;

	m_wAgility = 0;

	m_wMaxAG = 100;
	memset(m_szCharName, 0, 16);

	LoadPartners();
}


/**
 * \brief 
 */
CCharInfoFilter::~CCharInfoFilter()
{
}


/**
 * \brief 
 */
int CCharInfoFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();

		CServerMessagePacket pkt2(">> %s ", CT2A(__SOFTWARE_VERSION_ABOUT));
		CServerMessagePacket pkt3(">> %s ", __SOFTWARE_COPYRIGHT);

		GetProxy()->recv_direct(pkt2);
		GetProxy()->recv_direct(pkt3);
	}
	else if (pkt == CCharListReplyPacket::Type())
	{
		m_pktCharListReply = pkt;
	}
	else if (pkt == CCharStatsPacket::Type())
	{
		CGMMessagePacket pkt2(CT2A(__SOFTWARE_VERSION_ABOUT));
		CGMMessagePacket pkt3(__SOFTWARE_COPYRIGHT);

		GetProxy()->recv_direct(pkt2);
		GetProxy()->recv_direct(pkt3);

		if (m_wAgility != 0)
		{
			CCharStatsPacket& pkt2 = (CCharStatsPacket&)pkt;

			pkt2.SetAgility(m_wAgility);
		}

	}
	else if (pkt == CObjectMovedPacket::Type())
	{
		CObjectMovedPacket& pktMove = (CObjectMovedPacket&)pkt;

		if (pktMove.GetId() == m_wPlayerId)
		{
			m_bX = pktMove.GetX();
			m_bY = pktMove.GetY();
		}
	}
	else if (pkt == CWarpReplyPacket::Type())
	{
		CWarpReplyPacket& pkt2 = (CWarpReplyPacket&)pkt;

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();

		m_vPlayers.clear();
		UpdateSuspendedFlag();

		CCharacterSayPacket pktSay(m_szCharName, "--afk off");
		GetProxy()->send_packet(pktSay);
	}
	else if (pkt == CCharRespawnPacket::Type())
	{
		CCharRespawnPacket& pkt2 = (CCharRespawnPacket&)pkt;

		BYTE xx = pkt2.GetX();
		BYTE yy = pkt2.GetY();

		if (abs((int)xx-m_bX) > 2 || abs((int)yy-m_bY) > 2)
		{
			CCharacterSayPacket pktSay(m_szCharName, "--afk off");
			GetProxy()->send_packet(pktSay);
		}

		m_bX = xx;
		m_bY = yy;

		m_vPlayers.clear();
		UpdateSuspendedFlag();
	}
	else if (pkt == CCharStatsPacket::Type())
	{
		CCharStatsPacket& pkt2 = (CCharStatsPacket&)pkt;

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();
	}
	else if (pkt == CUpdatePosSTCPacket::Type())
	{
		CUpdatePosSTCPacket& pkt2 = (CUpdatePosSTCPacket&)pkt;

		if (pkt2.GetId() == m_wPlayerId)
		{
			m_bX = pkt2.GetX();
			m_bY = pkt2.GetY();
		}
	}
	else if (pkt == CLevelUpPacket::Type())
	{
		CLevelUpPacket& pkt2 = (CLevelUpPacket&)pkt;

		if (m_fExit400 && pkt2.GetLevel() >= 400)
		{
			if (CProxyClickerModule::GetInstance()->m_pLoader)
			{
				HWND hWnd = 0;
				CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

				if (hWnd)
					PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}
	}
	else if (pkt == CSetMaxManaAGPacket::Type())
	{
		CSetMaxManaAGPacket& pkt2 = (CSetMaxManaAGPacket&)pkt;
		m_wMaxAG = pkt2.GetAG();
	}
	else if (pkt == CUpdateManaAGPacket::Type())
	{
		if (m_fLockAg)
		{
			CUpdateManaAGPacket& pkt2 = (CUpdateManaAGPacket&)pkt;
			pkt2.SetAG(m_wMaxAG);
		}
	}
	else
	{
		if (pkt == CMeetPlayerPacket::Type())
		{
			CMeetPlayerPacket& pkt2 = (CMeetPlayerPacket&)pkt;
			int extraBytes = 0;

			for (int i=0; i < pkt2.GetPlayerCount(); i++)
			{
				CStringA strPlayerName = pkt2.GetPlayerName(i);
				WORD wPlayerId = pkt2.GetPlayerId(i);


				if (m_fMeetPlayerMessage)
				{
					CServerMessagePacket pktMsg(">> meet player %s", strPlayerName);
					GetProxy()->recv_direct(pktMsg);
				}

				if (!IsPartner(strPlayerName))
					m_vPlayers.insert(std::pair<WORD,CStringA>(wPlayerId, strPlayerName));
			}

			UpdateSuspendedFlag();
		}
		else if (pkt == CForgetObjectPacket::Type())
		{
			CForgetObjectPacket& pkt2 = (CForgetObjectPacket)pkt;

			for (int i=0; i < pkt2.GetObjectCount(); i++)
				m_vPlayers.erase(pkt2.GetObjectId(i));

			UpdateSuspendedFlag();
		}
		else if (pkt == CObjectDeathPacket::Type())
		{
			CObjectDeathPacket& pkt2 = (CObjectDeathPacket&)pkt;
			m_vPlayers.erase(pkt2.GetObjectId());

			UpdateSuspendedFlag();
		}
	}

	return 0;
}


/**
 * \brief 
 */
int CCharInfoFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CCharSelectedPacket::Type())
	{
		CCharSelectedPacket pktSel(pkt);
		CCharListReplyPacket pktList(m_pktCharListReply);

		int idx = pktList.FindChar(pktSel.GetCharName());
		m_wLevel = (idx >= 0) ? pktList.GetCharLevel(idx) : 1;

		memset(m_szCharName, 0, 16);
		strncpy(m_szCharName, pktSel.GetCharName(), 10);

		if (m_szCharName[0] != 0)
			m_vPartners.insert(pktSel.GetCharName());
	}
	else if (pkt == CUpdatePosCTSPacket::Type())
	{
		CUpdatePosCTSPacket& pkt2 = (CUpdatePosCTSPacket&)pkt;

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();
	}

	return 0;
}


/**
 * \brief 
 */
bool CCharInfoFilter::GetParam(const char* pszParam, void* pData)
{
	if (!pData || !pszParam)
		return false;

	if (_stricmp(pszParam, "X") == 0)
	{
		*((BYTE*)pData) = m_bX;
		return true;
	}
	
	if (_stricmp(pszParam, "Y") == 0)
	{
		*((BYTE*)pData) = m_bY;
		return true;
	}
	
	if (_stricmp(pszParam, "PlayerId") == 0)
	{
		*((WORD*)pData) = m_wPlayerId;
		return true;
	}

	if (_stricmp(pszParam, "Level") == 0)
	{
		*((WORD*)pData) = m_wLevel;
		return true;
	}

	if (_stricmp(pszParam, "CharName") == 0)
	{
		*((char**)pData) = m_szCharName;
		return true;
	}

	if (_stricmp(pszParam, "Players") == 0)
	{
		*((void**)pData) = (void*)&m_vPlayers;
		return true;
	}

	if (_stricmp(pszParam, "meetpl") == 0)
	{
		*((bool*)pData) = m_fMeetPlayerMessage;
		return true;		
	}

	return false;
}


/**
 * \brief 
 */
bool CCharInfoFilter::SetParam(const char* pszParam, void* pData)
{
	if (!pszParam || !pData)
		return false;

	if (_stricmp(pszParam, "exit400") == 0)
	{
		m_fExit400 = *((BOOL*)pData) != 0;
		return true;
	}
	else if (_stricmp(pszParam, "stealth") == 0)
	{
		bool fNewState = *((bool*)pData);

		if (fNewState != m_fStealth)
		{
			m_fStealth = fNewState;
			UpdateSuspendedFlag();
		}

		return true;
	}
	else if (_stricmp(pszParam, "lockag") == 0)
	{
		m_fLockAg = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "meetpl") == 0)
	{
		m_fMeetPlayerMessage = *((bool*)pData);
		return true;
	}
	else if (_stricmp(pszParam, "fixagi") == 0)
	{
		m_wAgility = *((WORD*)pData);
		return true;
	}

	return false;
}


/**
 * \brief 
 */
void CCharInfoFilter::UpdateSuspendedFlag()
{
	bool fNewSuspended = (m_vPlayers.size() != 0) && m_fStealth;

	if (m_fSuspended != fNewSuspended)
	{
		m_fSuspended = fNewSuspended;

		const char* filters[] = 
		{
			"AutoPickupFilter",
			"MultihitFilter",
			"AutoKillFilter",
			"FastMoveFilter",
			"ScriptProcessorFilter",
		};

		for (int i=0; i < sizeof(filters)/sizeof(filters[0]); i++)
		{
			CPacketFilter* pFilter = GetProxy()->GetFilter(filters[i]);

			if (pFilter)
				pFilter->SetParam("suspended", &m_fSuspended);
		}
	}
}


/**
 * \brief 
 */
bool CCharInfoFilter::IsPartner(const CStringA& strName)
{
	return m_vPartners.find(strName) != m_vPartners.end();
}


/**
 * \brief 
 */
void CCharInfoFilter::LoadPartners()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "partners.txt");

	m_vPartners.clear();

	FILE* f = fopen(szPath, "r");

	if (!f)
		return;

	char szLine[256] = {0};
	char ch = 0;
	int pos = 0;

	while(1)
	{
		bool fStop = (1 != fread(&ch, 1, 1, f));

		if (fStop || ch == '\n' || pos >= 256)
		{
			if (szLine[0] != 0)
			{
				m_vPartners.insert(CStringA(szLine));
			}

			pos = 0;
			memset(szLine, 0, 256);
		}
		else
		{
			szLine[pos++] = ch;
		}

		if (fStop)
			break;
	}

	fclose(f);
}
