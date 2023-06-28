#ifndef __Packet_H
#define __Packet_H

#pragma once

#include "PacketType.h"

/**
 *
 */
class CPacket
{
public:
	CPacket();
	CPacket(BYTE type, WORD len, BYTE* buf);
	CPacket(BYTE* rawBuf, int len);
	CPacket(char* rawBuf, int len);
	CPacket(const CPacket& in);
	virtual ~CPacket();

	void ConstructFromBuffer(BYTE* rawBuf, int len);

public:
	CPacket& operator=(const CPacket& in);
	bool operator==(const CPacketType& in) const;
	operator CPacketType() { return GetType(); }

	// Packet Interface
	BYTE GetPktClass() const { return m_bClass; }
	BYTE GetHdrLen() const { return m_bHdrLen; }
	int  GetPktLen() const { return m_pBuff ? (int)(m_wLen + m_bHdrLen) : 0; }
	BYTE* GetData() { return m_pBuff ? (m_pBuff + m_bHdrLen) : 0; }
	BYTE* GetRawPacket() { return m_pBuff; }

	void SetDecryptedPacket(BYTE* pData, int len);
	BYTE* GetDecryptedPacket(){ return m_pDecBuff; }
	int GetDecryptedLen() const { return m_pDecBuff ? m_iDecLen : 0; }
	bool IsDecrypted() const { return m_pDecBuff != 0; }

	virtual CPacketType GetType() const { return m_cType; }
	virtual void SetType(const CPacketType& type) { m_cType = type; }
	
	static CStringA DescribePacket(CPacket& pkt){ return ""; }


	BYTE* AnyBuffer(){ return m_pDecBuff != 0 ? m_pDecBuff : m_pBuff; }

	int GetInjected(){ return m_fInjected; }
	void SetInjected(int fInjected = 1) { m_fInjected = fInjected; }

	void PutEvent(HANDLE hEvent){ m_hEvent = hEvent; }
	HANDLE GetEvent() const { return m_hEvent; }

protected:
	void SetDecryptedLen(int iNewLen){ m_iDecLen = iNewLen; }

private:
	BYTE m_bHdrLen;
	BYTE m_bClass;
	WORD m_wLen;
	BYTE* m_pBuff;
	
	int m_iDecLen;
	BYTE* m_pDecBuff;

	int m_fInjected;
	CPacketType m_cType;

	HANDLE m_hEvent;
};

#endif // __Packet_H