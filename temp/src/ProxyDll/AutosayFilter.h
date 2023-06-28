#ifndef __AutosayFilter_H
#define __AutosayFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"
#include <vector>
#include "AutosayParser.h"


/**
 * \brief 
 */
class CAutosayFilter
	: public CPacketFilter
{
public:
	CAutosayFilter(CProxy* pProxy);
	virtual ~CAutosayFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "AutosayFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	void LoadData();
	void DoReply();

	int MatchChar(const char* pszCharName);
	int MatchMessage(int iCharIdx, const char* pszMessage);
	bool IsNonsense(const char* pszMessage);
	int FindNonsenseMsg(int iCharIdx);
	int FindRepeatMsg(int iCharIdx);
	void SendReplyMessage(const char* pszMessage);


private:
	BOOL m_fEnabled;
	BOOL m_fDoReply;
	BOOL m_fNewConfig;
	DWORD m_dwTicks;
	CPacket m_cLastSayPkt;
	CStringA m_strLastMessage;
	std::vector<CStringA> m_vMessages;

	CAutosayParser m_cAutosayParser;
	int m_iNonsenseCounter;
	int m_iRepeatCounter;

	static const DWORD AUTOREPLY_DELAY = 3000;
};


#endif //__AutosayFilter_H