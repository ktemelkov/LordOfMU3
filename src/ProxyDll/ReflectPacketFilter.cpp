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
	m_dwSkill = 0;

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
		KillObject(m_wTarget, m_dwSkill);
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
		KillObject(m_wTarget, m_dwSkill);
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

		struct { const char* szSkillName; DWORD dwSkill; } _map[] =
		{
			{"Normal"		, 0x00000000},
			{"FireBurst"	, 0x0019003D},
			{"Force"		, 0x0019003C}, 
			{"TwistingSlash", 0x001E0029},
			{"DeathStab"	, 0x0019002B},
			{"FallingSlash"	, 0x00190013},
			{"Cyclone"		, 0x00190016},
			{"Lunge"		, 0x00190014},
			{"EnergyBall"	, 0x00190011},
			{"Poison"		, 0x00190001},
			{"Meteorite"	, 0x00190002},
			{"Lightning"	, 0x00190003},
			{"Fireball"		, 0x00190004},
			{"PowerWave"	, 0x0019000B},
			{"Ice"			, 0x00190007},
			{"IceArrow"		, 0x00190033},
			{"ChainLight"	, 0x001900D6},
			{"Sleep"		, 0x001900DB},
			{"DrainLife"	, 0x001E00D6},
			{"EvilSpirit"	, 0x001E0009},
			{"DarkHorse"	, 0x001E003E},
		};

		for (int i=sizeof(_map)/sizeof(_map[0])-1; i >=0; --i)
		{
			if (0 == _stricmp(szSkill, _map[i].szSkillName))
			{
				m_dwSkill = _map[i].dwSkill;
				return true;
			}
		}

		ULONG ulSkill = strtoul(szSkill, 0, 16);

		if ((HIWORD(ulSkill) != 0x0019 && HIWORD(ulSkill) != 0x001E) || LOWORD(ulSkill) == 0)
		{
			return false;
		}

		m_dwSkill = ulSkill;
		return true;
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
void CReflectPacketFilter::KillObject(WORD wId, DWORD dwSkill)
{
	if (dwSkill == 0)
	{
		GetProxy()->send_packet(CNormalAttackPacket(wId));
	}
	else if (HIWORD(dwSkill) == 0x19)
	{
		GetProxy()->send_packet(CSingleSkillPacket(LOWORD(dwSkill), wId));
	}
	else if (HIWORD(dwSkill) == 0x1E)
	{
		CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");

		if (pFilter)
		{
			BYTE x = 0, y = 0;
			pFilter->GetParam("X", &x);
			pFilter->GetParam("Y", &y);

			GetProxy()->send_packet(CMassiveSkillPacket(LOWORD(dwSkill), x, y, 0, wId));
		}
	}
}

