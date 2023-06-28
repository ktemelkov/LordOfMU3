#ifndef __GameProxy_H
#define __GameProxy_H

#pragma once

#include "Proxy.h"
#include "PacketQueue.h"
#include "PacketParser.h"
#include "AutoBuffer.h"
#include <vector>


/**
 * \brief
 */
class CGameProxy 
	: public CProxy
{
protected:
	typedef std::vector<CPacketFilter*> CFilterChain;


public:
	CGameProxy(IConnectionHandler* pHandler);
	virtual ~CGameProxy();

public:
	// IConnectionProxy interface
	virtual bool ProcessSendStream(char* lpBuffer, int iLen);
	virtual bool ProcessRecvStream(char* lpBuffer, int iLen);

	virtual bool GetRecvBuffer(char* lpBuffer, int& iLen, bool& fHasMore);
	virtual bool GetSendBuffer(char* lpBuffer, int& iLen, bool& fHasMore);

	virtual bool HasIngoingPackets();
	virtual bool HasOutgoingPackets();

	virtual bool ShouldCloseConnection(){ return false; }

	virtual void Destroy(){ delete this; }


	// Packet Interface
	virtual bool send_packet(CPacket& pkt);
	virtual bool send_lop_packet(CPacket& pkt);
	virtual bool recv_packet(CPacket& pkt);
	virtual bool send_direct(CPacket& pkt);
	virtual bool recv_direct(CPacket& pkt);

	// Filter Interface
	virtual bool AddFilter(CPacketFilter* pFilter);
	virtual CPacketFilter* GetFilter(const char* szName);
	virtual void ClearFilters();

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	void ProcessRecvStream(char* lpBuffer, char* newBuff, int& iLen);
	void ProcessSendStream(char* lpBuffer, char* newBuff, int& iLen);

	void ProcessSendQueue();
	void ProcessRecvQueue();

	bool FilterRecvPacket(CPacket& pkt);
	bool FilterSendPacket(CPacket& pkt);

private:
	CPacketQueue  m_cRecvQueue;
	CPacketQueue  m_cSendQueue;
	CPacketParser m_cRecvPacketParser;
	CPacketParser m_cSendPacketParser;
	CFilterChain  m_cFilterChain;

private:
	static const UINT QUEUE_FLAG_DIRECT	= 1;
};

#endif //__GameProxy_H