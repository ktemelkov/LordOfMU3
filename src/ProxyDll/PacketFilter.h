#ifndef __PacketFilter_H
#define __PacketFilter_H

#pragma once

#include "Packet.h"
#include "FilterContext.h"

class CProxy;

#define FILTER_LEVEL_CRITICAL	0
#define FILTER_LEVEL_NORMAL		100
#define FILTER_LEVEL_LOW		200

#define FILTER_LEVEL_CLICKER	FILTER_LEVEL_NORMAL
#define FILTER_LEVEL_ANTIHACK   FILTER_LEVEL_CRITICAL
#define FILTER_LEVEL_ENCDEC		FILTER_LEVEL_CRITICAL
#define FILTER_LEVEL_COMMAND	FILTER_LEVEL_CRITICAL
#define FILTER_LEVEL_LOGGER		FILTER_LEVEL_CRITICAL
#define FILTER_LEVEL_ENCRYPTION FILTER_LEVEL_CRITICAL
#define FILTER_LEVEL_CHARINFO	FILTER_LEVEL_CRITICAL


/**
 *
 */
class CPacketFilter
{
public:
	CPacketFilter(CProxy* pProxy);
	virtual ~CPacketFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName() = 0;
	virtual WORD GetLevel() = 0;
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context) = 0;
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context) = 0;

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData){ return false; }
	virtual bool SetParam(const char* pszParam, void* pData){ return false; }

	// clean up
	virtual void Term(){}

protected:
	CProxy* GetProxy() { return m_pProxy; }

private:
	CProxy* m_pProxy;
};

#endif //__PacketFilter_H