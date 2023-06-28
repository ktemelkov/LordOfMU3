#include "stdafx.h"
#include "Packet.h"


/**
 *
 */
CPacket::CPacket()
{
	m_bHdrLen = 0;
	m_bClass = 0;
	m_wLen = 0;
	m_pBuff = 0;
	
	m_iDecLen = 0;
	m_pDecBuff = 0;

	m_fInjected = 0;

	m_hEvent = 0;
}


/**
 *
 */
CPacket::CPacket(BYTE pktClass, WORD len, unsigned char* buf)
{
	m_hEvent = 0;
	m_fInjected = 0;

	m_bHdrLen = 0;
	m_bClass = 0;
	m_wLen = 0;
	m_pBuff = 0;
	
	m_iDecLen = 0;
	m_pDecBuff = 0;

	if (pktClass != 0 && len != 0 && buf != 0)
	{
		if (pktClass == 0xC1 || pktClass == 0xC3)
		{
			m_bHdrLen = 2;
			m_pBuff = new BYTE[len+m_bHdrLen+8];
			m_pBuff[0] = pktClass;
			m_pBuff[1] = (BYTE)(len+m_bHdrLen);
		}
		else if (pktClass == 0xC2 || pktClass == 0xC4)
		{
			m_bHdrLen = 3;
			m_pBuff = new BYTE[len+m_bHdrLen+8];
			m_pBuff[0] = pktClass;
			m_pBuff[1] = HIBYTE(len+m_bHdrLen);
			m_pBuff[2] = LOBYTE(len+m_bHdrLen);
		}

		if (m_bHdrLen != 0)
		{
			m_wLen = len;
			m_bClass = pktClass;
			memcpy(m_pBuff + m_bHdrLen, buf, len);
		}
	}
}


/**
 *
 */
CPacket::CPacket(BYTE* rawBuf, int len)
{
	ConstructFromBuffer(rawBuf, len);
}

/**
 * \brief 
 */
CPacket::CPacket(char* rawBuf, int len)
{
	ConstructFromBuffer((BYTE*)rawBuf, len);
}

/**
 * \brief 
 */
void CPacket::ConstructFromBuffer(BYTE* rawBuf, int len)
{
	m_hEvent = 0;
	m_fInjected = 0;
	m_bHdrLen = 0;
	m_bClass = 0;
	m_wLen = 0;
	m_pBuff = 0;
	m_iDecLen = 0;
	m_pDecBuff = 0;

	if (rawBuf && len > 0)
	{
		if (rawBuf[0] == 0xC1 || rawBuf[0] == 0xC3 && len > 2)
		{
			m_pBuff = new BYTE[len+8];
			m_bHdrLen = 2;
		}
		else if (rawBuf[0] == 0xC2 || rawBuf[0] == 0xC4 && len > 3)
		{
			m_pBuff = new BYTE[len+3+8];
			m_bHdrLen = 3;
		}

		if (m_bHdrLen != 0)
		{
			m_bClass = rawBuf[0];
			m_wLen = (WORD)(len - m_bHdrLen);
			memcpy(m_pBuff, rawBuf, len);	
		}
	}
}

/**
 *
 */
CPacket::CPacket(const CPacket& in)
{
	m_bHdrLen = 0;
	m_bClass = 0;
	m_wLen = 0;
	m_pBuff = 0;
	m_iDecLen = 0;
	m_pDecBuff = 0;
	m_hEvent = 0;

	operator=(in);
}


/**
 *
 */
CPacket::~CPacket()
{
	if (m_pBuff)
		delete[] m_pBuff;

	if (m_pDecBuff)
		delete[] m_pDecBuff;
}


/**
 *
 */
CPacket& CPacket::operator=(const CPacket& in)
{
	if (m_pBuff)
		delete[] m_pBuff;

	if (m_pDecBuff)
		delete[] m_pDecBuff;

	m_bHdrLen = in.m_bHdrLen;
	m_bClass = in.m_bClass;
	m_wLen = in.m_wLen;
	m_iDecLen = in.m_iDecLen;

	m_fInjected = in.m_fInjected;

	m_pBuff = 0;
	m_pDecBuff = 0;

	if (in.GetPktLen() > 0)
	{
		WORD len = in.GetPktLen();
		m_pBuff = new BYTE[len+8]; 
		memcpy(m_pBuff, in.m_pBuff, len);
	}

	if (in.m_iDecLen > 0)
	{
		m_pDecBuff = new BYTE[in.m_iDecLen+8]; 
		memcpy(m_pDecBuff, in.m_pDecBuff, in.m_iDecLen);
	}

	m_cType = in.GetType();
	m_hEvent = in.m_hEvent;

	return *this;
}

/**
 * \brief 
 */
bool CPacket::operator==(const CPacketType& in) const
{
	const BYTE *mask = in.GetMask();
	const BYTE *pat = in.GetPattern();

	if (!mask || !pat || mask[0] > pat[0])
		return false;

	int len = (int)mask[0];

	int lenPkt = (int)GetPktLen();
	BYTE* buffPkt = m_pBuff;

	if (m_pDecBuff)
	{
		lenPkt = m_iDecLen;
		buffPkt = m_pDecBuff;
	}

	if (len > lenPkt)
		return false;

	mask++;
	pat++;

	for (int i=0; i < len; i++)
	{
		if ((buffPkt[i] & mask[i]) != (pat[i] & mask[i]))
			return false;
	}

	return true;
}

/**
 * \brief 
 */
void CPacket::SetDecryptedPacket(BYTE* pData, int len)
{
	if (m_pDecBuff)
		delete[] m_pDecBuff;

	m_pDecBuff = 0;
	m_iDecLen = len;

	if (len > 0)
	{
		m_pDecBuff = new BYTE[len+8];
		memcpy(m_pDecBuff, pData, len);
	}

	if (!m_pBuff)
	{
		m_bClass = pData[0];

		if (m_bClass == 0xC1 || m_bClass == 0xC3)
		{
			m_bHdrLen = 2;
		}
		else if (m_bClass == 0xC2 || m_bClass == 0xC4)
		{
			m_bHdrLen = 3;
		}
	}
}
