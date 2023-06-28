#ifndef __PacketDecryptFilter_H
#define __PacketDecryptFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"

/**
 * \brief 
 */
class CPacketDecryptFilter
	: public CPacketFilter
{
public:
	CPacketDecryptFilter(CProxy* pProxy);
	virtual ~CPacketDecryptFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "PacketDecryptFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_ENCDEC; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);


protected:
	void DetectRecvPacketType(CPacket& pkt);
	void DetectSendPacketType(CPacket& pkt);
	void PostDetectRecvPacketType(CPacket& pkt);
	void PostDetectSendPacketType(CPacket& pkt);
};

#endif //__PacketDecryptFilter_H