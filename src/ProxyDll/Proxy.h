#ifndef __Proxy_H
#define __Proxy_H

#pragma once

#include "Packet.h"
#include "PacketFilter.h"
#include "ConnectionInterface.h"


/**
 *
 */
class CProxy
	: public IConnectionProxy
{
protected:
	CProxy(IConnectionHandler* pConnectionHandler) { m_pConnectionHandler = pConnectionHandler; }

public:
	virtual ~CProxy(){}

public:
	// Packet Interface
	virtual bool send_packet(CPacket& pkt){ return false; }
	virtual bool send_lop_packet(CPacket& pkt){ return false; }
	virtual bool recv_packet(CPacket& pkt){ return false; }
	virtual bool send_direct(CPacket& pkt){ return false; }
	virtual bool recv_direct(CPacket& pkt){ return false; }

	// Packet Filter Interface
	virtual bool AddFilter(CPacketFilter* pFilter){ return false; }
	virtual CPacketFilter* GetFilter(const char* szName){ return 0; }
	virtual void ClearFilters(){}

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData){ return false; }
	virtual bool SetParam(const char* pszParam, void* pData){ return false; }


protected:
	IConnectionHandler* m_pConnectionHandler;
};

#endif //__Proxy_H