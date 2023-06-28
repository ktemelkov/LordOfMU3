#ifndef __PacketLogger_H
#define __PacketLogger_H

#pragma once

#include "PacketFilter.h"

/**
 *
 */
class CPacketLogger
	: public CPacketFilter
{
public:
	CPacketLogger(CProxy* pProxy);
	virtual ~CPacketLogger();

public:
	// Filter Interface Methods
	virtual const char* GetName();
	virtual WORD GetLevel();
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);
};

#endif //__PacketLogger_H