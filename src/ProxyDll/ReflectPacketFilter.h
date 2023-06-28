#ifndef __ReflectPacketFilter_H
#define __ReflectPacketFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"

/**  
 * \brief 
 */
class CReflectPacketFilter
	: public CPacketFilter
{
public:
	CReflectPacketFilter(CProxy* pProxy);
	virtual ~CReflectPacketFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "ReflectFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

private:
	void KillObject(WORD wId, DWORD wSkill);

private:
	BOOL m_fEnabled;

	WORD m_wPlayerId;
	DWORD m_dwDelay;
	DWORD m_dwSkill;

	WORD m_wTarget;
	DWORD m_dwTicks;
	DWORD m_dwLastSkill;
};


#endif //__ReflectPacketFilter_H