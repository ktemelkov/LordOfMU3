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

	std::map<WORD,CStringA> m_vPlayers;
	std::set<CStringA> m_vPartners;

	bool m_fSuspended;
	bool m_fStealth;
	bool m_fMeetPlayerMessage;

	CPacket m_pktCharListReply;
};


#endif //__CharInfoFilter_H