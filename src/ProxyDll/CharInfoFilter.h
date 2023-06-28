#ifndef __CharInfoFilter_H
#define __CharInfoFilter_H

#pragma once


#include "Proxy.h"
#include "PacketFilter.h"
#include <map>
#include <set>


/**
 * \brief 
 */
class CCharInfoFilter
	: public CPacketFilter
{
public:
	CCharInfoFilter(CProxy* pProxy);
	virtual ~CCharInfoFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "CharInfoFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	void UpdateSuspendedFlag();
	bool IsPartner(const CStringA& strName);
	void LoadPartners();
	void ListPlayers();
	void ListPartners();
	void ListChars();

private:
	WORD m_wPlayerId;
	WORD m_wLevel;
	WORD m_wMaxAG;
	BYTE m_bX;
	BYTE m_bY;
	char m_szCharName[16];

	bool m_fExit400;
	bool m_fLockAg;
	WORD m_wAgility;
	WORD m_wFixLevel;
	WORD m_wStrength;
	WORD m_wEnergy;
	WORD m_wVitality;

	std::map<WORD,CStringA> m_vPlayers;
	std::map<CStringA,WORD> m_vPartners;

	bool m_fSuspended;
	bool m_fStealth;
	bool m_fMeetPlayerMessage;

	bool m_fShowSkill;

	std::map<CStringA,WORD> m_vCharList;
};


#endif //__CharInfoFilter_H