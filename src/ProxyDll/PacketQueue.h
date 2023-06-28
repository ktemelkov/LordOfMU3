#ifndef __PacketQueue_H
#define __PacketQueue_H

#pragma once

#include "Packet.h"
#include <deque>
#include <vector>

/**
 *
 */
class CPacketQueue
{
public:
	CPacketQueue();
	virtual ~CPacketQueue();

public:
	void QueuePacket(const CPacket& pkt, ULONG ulFlags = 0);
	bool GetNextPacket(CPacket& out, ULONG* pulFlags = 0);
	bool GetPackets(std::vector<CPacket>& out, std::vector<ULONG>& vFlags, int& iCount, int maxSize = 0, bool* pfHasMore = 0);
	bool IsPacketAvailable();
	void UnGetPacket(CPacket& pkt, ULONG ulFlags = 0);
	int  GetPacketCount();

private:
	CRITICAL_SECTION m_cs;
	std::deque<CPacket> m_vQueue;
	std::deque<ULONG> m_vQueueFlags;
};

#endif // __PacketQueue_H