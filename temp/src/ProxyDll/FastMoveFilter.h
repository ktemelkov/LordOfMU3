#ifndef __FastMoveFilter_H
#define __FastMoveFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"


/**
 * \brief 
 */
class CFastMoveFilter
	: public CPacketFilter
{
public:
	CFastMoveFilter(CProxy* pProxy);

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "FastMoveFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

private:
	BOOL m_fEnabled;
	bool m_fSuspended;
};


#endif //__FastMoveFilter_H