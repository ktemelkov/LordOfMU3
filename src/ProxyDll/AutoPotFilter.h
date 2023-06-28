#pragma once


#include "Proxy.h"
#include "PacketFilter.h"



/**  
 * \brief 
 */
class CAutoPotFilter
	: public CPacketFilter
{
public:
	CAutoPotFilter(CProxy* pProxy);
	virtual ~CAutoPotFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "AutoPotFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData){ return false; }
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	void UsePotion(DWORD dwPotion);

protected:
	static const DWORD LIFE_POTION = 0; //Large Healing Potion
	static const DWORD MANA_POTION = 1; //Large Mana Potion

private:
	bool m_fEnabled;

	WORD m_wLife;
	WORD m_wMana;

	WORD m_wMaxLife;
	WORD m_wMaxMana;

	int m_iLastDiff;

	WORD m_wPlayerId;

	DWORD m_dwLastHpTs;
	DWORD m_dwLastMnTs;
};

