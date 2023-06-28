#pragma once


#include "Proxy.h"
#include "PacketFilter.h"


/**
 * \brief 
 */
class CAutoLoginFilter
	: public CPacketFilter
{
public:
	CAutoLoginFilter(CProxy* pProxy) : CPacketFilter(pProxy) {}
	virtual ~CAutoLoginFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "AutoLoginFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_ENCDEC; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData){ return false; }
	virtual bool SetParam(const char* pszParam, void* pData){ return false; }
};
