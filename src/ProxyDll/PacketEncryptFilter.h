#ifndef __PacketEncryptFilter_H
#define __PacketEncryptFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"

/**
 * \brief 
 */
class CPacketEncryptFilter
	: public CPacketFilter
{
public:
	CPacketEncryptFilter(CProxy* pProxy);
	virtual ~CPacketEncryptFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName() { return "PacketEncryptFilter"; }
	virtual WORD GetLevel() { return FILTER_LEVEL_ENCDEC; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

protected:
	void EncryptXOR(CPacket& pkt, bool fSend = false);

private:
	ULONG m_ulSendC3Counter;
	BYTE m_bLastSendC3Counter;
	ULONG m_ulRecvC3Counter;
};

#endif //__PacketEncryptFilter_H