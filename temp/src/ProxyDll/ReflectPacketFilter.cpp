#include "stdafx.h"
#include "ReflectPacketFilter.h"
#include "CommonPackets.h"


/**  
 * \brief 
 */
CReflectPacketFilter::CReflectPacketFilter(CProxy* pProxy)
	: CPacketFilter(pProxy) 
{
	m_fEnabled = false;

	m_wPlayerId = 0;
	m_dwDelay = 1000;
	m_wSkill = 0;

	m_wTarget = 0;
	m_dwTicks = 0;
	m_dwLastSkill = 0;
}


/**  
 * \brief 
 */
CReflectPacketFilter::~CReflectPacketFilter()
{

}


/**  
 * \brief 
 */
int CReflectPacketFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CObjectAnimationPacket::Type() && m_fEnabled)
	{
		CObjectAnimationPacket& pkt2 = (CObjectAnimationPacket&)pkt;

		if (pkt2.GetTarget() == m_wPlayerId && pkt2.GetActor() != m_wPlayerId)
		{
			if (m_wTarget == 0)
				m_dwTicks = GetTickCount();

			m_wTarget = pkt2.GetActor();
		}
	}
	else if (pkt == CSkillUsedPacket::Type() && m_fEnabled)
	{
		CSkillUsedPacket& pkt2 = (CSkillUsedPacket&)pkt;

		if (pkt2.GetTarget() == m_wPlayerId && pkt2.GetAttacker() != m_wPlayerId)
		{
			if (m_wTarget == 0)
				m_dwTicks = GetTickCount();

			m_wTarget = pkt2.GetAttacker();
		}
	}
	else if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();
	}
	else if (pkt == CObjectDeathPacket::Type())
	{
		CObjectDeathPacket& pkt2 = (CObjectDeathPacket&)pkt;

		if (pkt2.GetObjectId() == m_wTarget || pkt2.GetObjectId() == m_wPlayerId)
			m_wTarget = 0;
	}
	else if (pkt == CForgetObjectPacket::Type())
	{
		CForgetObjectPacket& pkt2 = (CForgetObjectPacket&)pkt;
		
		for (int i=pkt2.GetObjectCount()-1; i >= 0; --i)
		{
			if (pkt2.GetObjectId(i) == m_wTarget)
			{
				m_wTarget = 0;
				break;
			}
		}
	}
	else if (pkt == CWarpReplyPacket::Type() || pkt == CCharRespawnPacket::Type())
	{
		m_wTarget = 0;
	}

	if (m_wTarget != 0 && (GetTickCount() - m_dwTicks) > m_dwDelay)
	{
		m_dwTicks = GetTickCount();
		KillObject(m_wTarget, m_wSkill);
	}

	return 0;
}


/**  
 * \brief 
 */
int CReflectPacketFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if ((pkt == CSingleSkillPacket::Type() || pkt == CMassiveSkillPacket::Type()) && pkt.GetInjected() == 0)
	{
		DWORD dwDiff = GetTickCount() - m_dwLastSkill;
		m_dwLastSkill = GetTickCount();

		if (dwDiff < m_dwDelay)
			m_dwDelay = dwDiff;
	}

	if (m_wTarget != 0 && (GetTickCount() - m_dwTicks) > m_dwDelay)
	{
		m_dwTicks = GetTickCount();
		KillObject(m_wTarget, m_wSkill);
	}

	return 0;
}


/**  
 * \brief 
 */
bool CReflectPacketFilter::GetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "enable") == 0)
	{
		*((BOOL*)pData) = m_fEnabled;
	}

	return true;
}


/**  
 * \brief 
 */
bool CReflectPacketFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "enable") == 0)
	{
		m_fEnabled = *((BOOL*)pData);
	}
	else if (_stricmp(pszParam, "setskill") == 0)
	{
		char* szSkill = (char*)pData;

		struct { const char* szSkillName; WORD wSkill; } _map[] =
		{
			{"Normal"		, 0x0000},
			{"FireBurst"	, 0x193D},
			{"Force"		, 0x193C}, 
			{"TwistingSlash", 0x1E29},
			{"DeathStab"	, 0x192B},
			{"FallingSlash"	, 0x1913},
			{"Cyclone"		, 0x1916},
			{"Lunge"		, 0x1914},
			{"EnergyBall"	, 0x1911},
			{"Poison"		, 0x1901},
			{"Meteorite"	, 0x1902},
			{"Lightning"	, 0x1903},
			{"Fireball"		, 0x1904},
			{"PowerWave"	, 0x190B},
			{"Ice"			, 0x1907},
			{"IceArrow"		, 0x1933},
			{"ChainLight"	, 0x19D6},
			{"Sleep"		, 0x19DB},
			{"DrainLife"	, 0x1ED6},
			{"EvilSpirit"	, 0x1E09},
			{"DarkHorse"	, 0x1E3E},
		};

		for (int i=sizeof(_map)/sizeof(_map[0])-1; i >=0; --i)
		{
			if (0 == _stricmp(szSkill, _map[i].szSkillName))
			{
				m_wSkill = _map[i].wSkill;
				return true;
			}
		}

		return false;
	}
	else if (_stricmp(pszParam, "suicide") == 0)
	{
		m_wTarget = m_wPlayerId;
		m_dwTicks = GetTickCount();
	}

	return true;
}


/**  
 * \brief 
 */
void CReflectPacketFilter::KillObject(WORD wId, WORD wSkill)
{
	if (wSkill == 0)
	{
		GetProxy()->send_packet(CNormalAttackPacket(wId));
	}
	else if ((wSkill & 0xFF00) == 0x1900)
	{
		GetProxy()->send_packet(CSingleSkillPacket(LOBYTE(wSkill), wId));
	}
	else if ((wSkill & 0xFF00) == 0x1E00)
	{
		CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");

		if (pFilter)
		{
			BYTE x = 0, y = 0;
			pFilter->GetParam("X", &x);
			pFilter->GetParam("Y", &y);

			GetProxy()->send_packet(CMassiveSkillPacket(LOBYTE(wSkill), x, y, 0, wId));
		}
	}
}

