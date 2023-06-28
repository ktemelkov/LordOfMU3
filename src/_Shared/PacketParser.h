#ifndef __PacketParser_H
#define __PacketParser_H

#pragma once

#include "Packet.h"

/**
 *
 */
class CPacketParser
{
public:
	CPacketParser();
	virtual ~CPacketParser();

public:
	bool AppendStream(BYTE* pBuff, int len);

	void Reset();
	bool Next();
	bool GetPacket(CPacket& pkt);
	bool HasError();

	void Reboot();

protected:
	bool ReadHeader();
	bool ReadSize();
	bool ReadData();
	bool ReadRawData();

	bool IsPktType(BYTE b);

protected:
	enum STATE 
	{
		eHeader = 0,
		eSize = 1,
		eData = 2,
		eReady = 3,
		eError = 4,
		eRawData = 5
	};

private:
	STATE m_eState;

private:
	BYTE* m_pStream;
	int   m_cbStream;
	int   m_iReadPos;

	BYTE m_bPktType;
	BYTE m_abPktSize[2];
	int  m_iPktSize;
	int  m_iHdrSize;
	BYTE* m_pData;

	bool m_fEncoded;
};

#endif // __PacketParser_H