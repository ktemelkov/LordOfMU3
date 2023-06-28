#include "stdafx.h"
#include "AutoPotFilter.h"
#include "CommonPackets.h"
#include "version.h"
#include "DebugOut.h"


/**  
 * \brief 
 */
CAutoPotFilter::CAutoPotFilter(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
	m_fEnabled = false;
	m_wPlayerId = 0;

	m_wLife = 0;
	m_wMaxLife = 0;
	m_wMana = 0;
	m_wMaxMana = 0;

	m_dwLastHpTs = 0;
	m_dwLastMnTs = 0;
}


/**  
 * \brief 
 */
int CAutoPotFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CHitInfoPacket::Type())
	{
		if (m_wPlayerId == ((CHitInfoPacket&)pkt).GetId())
		{
			WORD dwDamage = ((CHitInfoPacket&)pkt).GetDamage();

			int life = (int)m_wLife - (int)dwDamage;
			
			if (life < 0)
				life = 0;

			m_wLife = life;

			if (life != 0 && dwDamage != 0)
			{
				int error = (int)m_wMaxLife - life;
				int iHPInc = ((int)m_wMaxLife + 1)*40/100;

				if (error > 0 && iHPInc > 0 && error > iHPInc)
				{
					int iPotCnt = error/iHPInc;

					if (iPotCnt > 5)
						iPotCnt = 5;

					if ((int)GetTickCount() - (int)m_dwLastHpTs > 500 || m_wLife < m_wMaxLife/2)
					{
						for (int i=0; i < iPotCnt; i++)
						{
							UsePotion(LIFE_POTION);
							m_dwLastHpTs = GetTickCount();
						}
					}
				}
			}
		}
	}
	else if (pkt == CUpdateManaAGPacket::Type())
	{
		WORD wMana = ((CUpdateManaAGPacket&)pkt).GetMana();

		int diff = (int)wMana - (int)m_wMana;
		m_wMana = wMana;

		if (diff < 0)
		{
			int error = (int)m_wMaxMana - (int)m_wMana;
			int iInc = ((int)m_wMaxMana + 1)*40/100;

			if (error > 0 && iInc > 0 && error > iInc)
			{
				int iPotCnt = error/iInc;

				if (iPotCnt > 2)
					iPotCnt = 2;

				if ((int)GetTickCount() - (int)m_dwLastMnTs > 500 || m_wMana < m_wMaxMana/2)
				{
					for (int i=0; i < iPotCnt; i++)
					{
						UsePotion(MANA_POTION);
						m_dwLastMnTs = GetTickCount();
					}
				}
			}
		}
	}
	else if (pkt == CUpdateLifeSDPacket::Type())
	{
		WORD wLife = ((CUpdateLifeSDPacket&)pkt).GetLife();
		m_wLife = wLife;
	}
	else if (pkt == CSetMaxManaAGPacket::Type())
	{
		m_wMaxMana = ((CSetMaxManaAGPacket&)pkt).GetMana();

		if (m_wMana == 0)
			m_wMana = m_wMaxMana;
	}
	else if (pkt == CSetMaxLifeSDPacket::Type())
	{
		m_wMaxLife = ((CSetMaxLifeSDPacket&)pkt).GetLife();

		if (m_wLife == 0)
			m_wLife = m_wMaxLife;
	}
	else if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();
	}

	return 0;
}


/**  
 * \brief 
 */
int CAutoPotFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	return 0;
}


/**  
 * \brief 
 */
bool CAutoPotFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "autopot") == 0)
	{
		m_fEnabled = *((bool*)pData);
	}
	else
		return false;

	return true;
}


/**  
 * \brief 
 */
void CAutoPotFilter::UsePotion(DWORD dwPotion)
{
	if (!m_fEnabled)
		return;

	CPacketFilter* pFilter = GetProxy()->GetFilter("InventoryManagerFilter");

	if (pFilter)
	{
		DWORD dwCmd = dwPotion;
		pFilter->SetParam("use_potion", &dwCmd);
	}
}
