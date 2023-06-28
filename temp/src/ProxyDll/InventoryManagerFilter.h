#ifndef __InventoryManagerFilter_H
#define __InventoryManagerFilter_H

#pragma once


#include "Proxy.h"
#include "PacketFilter.h"
#include <map>
#include <set>


/**  
 * \brief 
 */
class CInventoryManagerFilter
	: public CPacketFilter
{
	struct CItemInfo
	{
	public:
		CItemInfo(){ wCode = 0; memset(vbData, 0, sizeof(vbData)); bQuantity = 1; bUsed = 0; }
		CItemInfo(const CItemInfo& in)
		{ 
			bQuantity = in.bQuantity;
			bUsed = in.bUsed;
			wCode = in.wCode; 
			memcpy(vbData, in.vbData, sizeof(vbData)); 
		}

		CItemInfo(WORD code, BYTE* pData)
		{ 
			wCode = code; 
			memset(vbData, 0, sizeof(vbData)); 
			bQuantity = 1;
			bUsed = 0;

			if (pData)
			{
				memcpy(vbData, pData, sizeof(vbData)); 
				bQuantity = pData[2];
			}
		}

	private:
		void operator=(const CItemInfo& in){}

	public:
		WORD wCode;
		BYTE vbData[7];
		BYTE bQuantity;
		BYTE bUsed;
	};

public:
	CInventoryManagerFilter(CProxy* pProxy);
	virtual ~CInventoryManagerFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "InventoryManagerFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	int FindItem(WORD wCode, WORD wMask);
	int FindItemInRange(BYTE grpLo, BYTE grpHi, BYTE subLo, BYTE subHi, BYTE lvlLo, BYTE lvlHi);
	void DropItem(BYTE pos);
	void UseItem(BYTE pos);
	void SellItem(BYTE pos);
	void SellItems(int iCount = 64);
	void ThrowItems(int iCount = 64);

private:
	std::map<BYTE,CItemInfo> m_vInventory;
};


#endif //__InventoryManagerFilter_H