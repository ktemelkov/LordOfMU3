#include "stdafx.h"
#include "InventoryManagerFilter.h"
#include "CommonPackets.h"
#include "version.h"
#include "DebugOut.h"


/**  
 * \brief 
 */
CInventoryManagerFilter::CInventoryManagerFilter(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
	m_fAutoRepair = false;
	m_iRepairSent = 0;
}


/**  
 * \brief 
 */
CInventoryManagerFilter::~CInventoryManagerFilter()
{

}


/**  
 * \brief 
 */
int CInventoryManagerFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CInitInventoryPacket::Type())
	{
		m_vInventory.clear();

		CInitInventoryPacket& pkt2 = (CInitInventoryPacket&)pkt;
		int iCount = pkt2.GetCount();
		
		for (int i=0; i < iCount; ++i)
		{
			BYTE bPos = pkt2.GetPosition(i);
			WORD wCode = pkt2.GetItemType(i);
			BYTE* pData = pkt2.GetItemData(i);

			m_vInventory.insert(std::pair<BYTE,CItemInfo>(bPos, CItemInfo(wCode, pData)));
		}
	}
	else if (pkt == CRemoveFromInventoryPacket::Type())
	{
		CRemoveFromInventoryPacket& pkt2 = (CRemoveFromInventoryPacket&)pkt;

		int iCount = pkt2.GetCount();
		for (int i=0; i < iCount; ++i)
		{
			BYTE bPos = pkt2.GetPosition(i);
			m_vInventory.erase(bPos);
		}
	}
	else if (pkt == CPutInventoryPacket::Type())
	{
		CPutInventoryPacket& pkt2 = (CPutInventoryPacket&)pkt;

		BYTE bPos = pkt2.GetInvPos();
		WORD wCode = pkt2.GetItemType();
		BYTE* pData = pkt2.GetItemData();
		
		m_vInventory.insert(std::pair<BYTE,CItemInfo>(bPos, CItemInfo(wCode, pData)));
	}
	else if (pkt == CMoveToInventoryPacket::Type())
	{
		CMoveToInventoryPacket& pkt2 = (CMoveToInventoryPacket&)pkt;

		BYTE bPos = pkt2.GetInvPos();
		WORD wCode = pkt2.GetItemType();
		BYTE* pData = pkt2.GetItemData();

		m_vInventory.insert(std::pair<BYTE,CItemInfo>(bPos, CItemInfo(wCode, pData)));
	}
	else if (pkt == CItemLostPacket::Type())
	{
		CItemLostPacket& pkt2 = (CItemLostPacket&)pkt;

		if (pkt2.GetStgType() == 1)
		{
			m_vInventory.erase(pkt2.GetPos());			
		}
	}
	else if (pkt == CItemUsedPacket::Type())
	{
		CItemUsedPacket& pkt2 = (CItemUsedPacket&)pkt;
		BYTE bPos = pkt2.GetInvPos();

		std::map<BYTE,CItemInfo>::iterator it = m_vInventory.find(bPos);

		if (it != m_vInventory.end())
		{
			BOOL bRemaining = pkt2.GetRemaining();

			if (m_fAutoRepair && bPos < 0x0C)
			{
				if ((int)it->second.bQuantity > (int)bRemaining || it->second.bQuantity == 0)
				{
					if (m_iRepairSent == 0)
					{
						GetProxy()->send_packet(CRepairInvPacket(bPos));
						m_iRepairSent++;
					}
				}
			}

			it->second.bQuantity = bRemaining;

			if (it->second.bUsed > 0)
				it->second.bUsed--;
		}
	}
	else if (pkt == CCreateInvItemPacket::Type())
	{
		CCreateInvItemPacket& pkt2 = (CCreateInvItemPacket&)pkt;

		BYTE bPos = pkt2.GetInvPos();
		WORD wCode = pkt2.GetItemType();
		BYTE* pData = pkt2.GetItemData();

		if (bPos != 0xFF)
			m_vInventory.insert(std::pair<BYTE,CItemInfo>(bPos, CItemInfo(wCode, pData)));
	}


	return 0;
}


/**  
 * \brief 
 */
int CInventoryManagerFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CMoveItemReqPacket::Type())
	{
		CMoveItemReqPacket& pkt2 = (CMoveItemReqPacket&)pkt;

		if (pkt2.GetSource() == CMoveItemReqPacket::PLACE_INVENTORY)
		{
			BYTE bPos = pkt2.GetInvPos();
			m_vInventory.erase(bPos);
		}
	}
	else if (pkt == CUseItemPacket::Type())
	{
		CUseItemPacket& pkt2 = (CUseItemPacket&)pkt;

		std::map<BYTE,CItemInfo>::iterator it = m_vInventory.find(pkt2.GetInvPos());

		if (it != m_vInventory.end())
			it->second.bUsed++;
	}
	else if (pkt == CRepairInvPacket::Type())
	{
		if (pkt.GetInjected() != 0)
		{
			if (--m_iRepairSent < 0)
				m_iRepairSent = 0;
		}
	}

	return 0;
}


/**  
 * \brief 
 */
bool CInventoryManagerFilter::GetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "get_quantity") == 0 && pData)
	{
		BYTE idx = *((BYTE*)pData);

		std::map<BYTE,CItemInfo>::iterator it = m_vInventory.find(idx);

		if (it != m_vInventory.end())
		{
			*((BYTE*)pData) = it->second.bQuantity;
			return true;
		}
		else
			*((BYTE*)pData) = 0;
	}

	return false;
}


/**  
 * \brief 
 */
bool CInventoryManagerFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "autorepair") == 0)
	{
		m_fAutoRepair = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "throw") == 0)
	{
		DWORD dwData = *((DWORD*)pData);

		WORD wCode = LOWORD(dwData);
		WORD wMask = HIWORD(dwData);

		int iPos = FindItem(wCode, wMask);
		
		if (iPos == -1)
		{
			CDebugOut::PrintInfo("No item(s) to throw");
			return false;
		}

		DropItem((BYTE)iPos);
	}
	else if (_stricmp(pszParam, "use") == 0)
	{
		DWORD dwData = *((DWORD*)pData);

		WORD wCode = LOWORD(dwData);
		WORD wMask = HIWORD(dwData);

		int iPos = FindItem(wCode, wMask);

		if (iPos == -1)
		{
			CDebugOut::PrintInfo("No item(s) to use");
			return false;
		}

		UseItem((BYTE)iPos);
	}
	else if (_stricmp(pszParam, "use_potion") == 0) // 0 - life, 1 - mana, 2 - sd, 3 - ag, 4 - antidote
	{
		DWORD dwData = *((DWORD*)pData);

		int iPos = -1;
		
		switch (dwData)
		{
			case 0:
				iPos = FindItemInRange(14,14,1,3,0,16);
				break;
			case 1:
				iPos = FindItemInRange(14,14,4,6,0,16);
				break;
			case 2:
				iPos = FindItemInRange(14,14,35,37,0,16);
				break;
			case 3:
				iPos = FindItemInRange(14,14,38,40,0,16);
				break;
			case 4:
				iPos = FindItemInRange(14,14,8,8,0,16);
				break;
		}

		if (iPos == -1)
			return false;

		UseItem((BYTE)iPos);
	}
	else if (_stricmp(pszParam, "sell") == 0)
	{
		DWORD dwData = *((DWORD*)pData);

		WORD wCode = LOWORD(dwData);
		WORD wMask = HIWORD(dwData);

		int iPos = FindItem(wCode, wMask);

		if (iPos == -1)
			return false;

		SellItem((BYTE)iPos);
	}
	else if (_stricmp(pszParam, "sellall") == 0)
	{
		SellItems();
	}
	else if (_stricmp(pszParam, "sellany") == 0)
	{
		SellItems(1);
	}
	else if (_stricmp(pszParam, "throwall") == 0)
	{
		ThrowItems();
	}
	else if (_stricmp(pszParam, "throwany") == 0)
	{
		ThrowItems(1);
	}
	else if (_stricmp(pszParam, "moveitem") == 0)
	{
		WORD wData = *((WORD*)pData);

		BYTE src = LOBYTE(wData);
		WORD dst = HIBYTE(wData);

		std::map<BYTE,CItemInfo>::iterator it = m_vInventory.find(src);

		if (it == m_vInventory.end())
			return false;

		if (dst >= 76)
			dst = MAKEWORD(dst, 4);

		GetProxy()->send_packet(CMoveItemReqPacket(src, dst, it->second.vbData));
		GetProxy()->recv_packet(CRemoveFromInventoryPacket(src));
	}
	else
		return false;

	return true;
}


/**  
 * \brief 
 */
int CInventoryManagerFilter::FindItem(WORD wCode, WORD wMask)
{
	for (std::map<BYTE,CItemInfo>::iterator it = m_vInventory.begin(); it != m_vInventory.end(); ++it)
	{
		WORD wItem = it->second.wCode;
		if (wCode == (wItem & 0x0FFF) && (wMask & (1 << (wItem >> 12))) != 0)
		{
			if (it->second.bUsed != 0
					&& ((int)it->second.bQuantity - (int)it->second.bUsed) <= 0)
			{
				continue;
			}
			
			return (int)it->first;
		}
	}

	return -1;
}


/**  
 * \brief 
 */
int CInventoryManagerFilter::FindItemInRange(BYTE grpLo, BYTE grpHi, BYTE subLo, BYTE subHi, BYTE lvlLo, BYTE lvlHi)
{
	for (std::map<BYTE,CItemInfo>::iterator it = m_vInventory.begin(); it != m_vInventory.end(); ++it)
	{
		WORD wItem = it->second.wCode;

		BYTE grp = ITEM_GROUP(wItem);
		BYTE sub = ITEM_SUBGR(wItem);
		BYTE lvl = ITEM_LEVEL(wItem);

		if (grp >= grpLo && grp <= grpHi && sub >= subLo && sub <= subHi && lvl >= lvlLo && lvl <= lvlHi)
		{
			if (it->second.bUsed != 0
				&& ((int)it->second.bQuantity - (int)it->second.bUsed) <= 0)
			{
				continue;
			}

			return (int)it->first;
		}
	}

	return -1;
}


/**
 * \brief 
 */
void CInventoryManagerFilter::DropItem(BYTE pos)
{
	CPacketFilter* pCharInfo = GetProxy()->GetFilter("CharInfoFilter");

	BYTE x = 0;
	BYTE y = 0;

	if (!pCharInfo 
		|| !pCharInfo->GetParam("X", &x)
		|| !pCharInfo->GetParam("Y", &y))
	{
		return;
	}

	CDropItemPacket pkt(x+1, y+1, pos);
	GetProxy()->send_packet(pkt);
}



/**
 * \brief 
 */
void CInventoryManagerFilter::SellItem(BYTE pos)
{
	CSellItemPacket pkt(pos);
	GetProxy()->send_packet(pkt);

	CRemoveFromInventoryPacket pkt2(pos);
	GetProxy()->recv_packet(pkt2);
}

/**
 * \brief 
 */
void CInventoryManagerFilter::SellItems(int iCount)
{
	bool fFound = false;

	for (std::map<BYTE,CItemInfo>::iterator it = m_vInventory.begin(); it != m_vInventory.end() && iCount > 0; ++it)
	{
		if ((int)it->first >= (int)0x0C && (int)it->first <= (int)0x4C)
		{
			if ((it->second.wCode & 0x0FFF) != 0x0E0B)
			{
				SellItem(it->first);
				iCount--;

				fFound = true;
			}
		}
	}

	if (!fFound)
	{
		CDebugOut::PrintInfo("No item(s) to sell");
	}
}


/**
 * \brief 
 */
void CInventoryManagerFilter::ThrowItems(int iCount)
{
	for (std::map<BYTE,CItemInfo>::iterator it = m_vInventory.begin(); it != m_vInventory.end() && iCount > 0; ++it)
	{
		if (it->first >= 0x0C && it->first <= 0x4C)
		{
			DropItem(it->first);
			iCount--;
		}
	}
}


/**
 * \brief 
 */
void CInventoryManagerFilter::UseItem(BYTE pos)
{
	CUseItemPacket pkt(pos);
	GetProxy()->send_packet(pkt);
}
