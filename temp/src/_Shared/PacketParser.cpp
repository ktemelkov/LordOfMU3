#include "stdafx.h"
#include "PacketParser.h"
#include "DebugOut.h"
#include "BufferUtil.h"


/**
 *
 */
CPacketParser::CPacketParser()
{
	m_pStream = 0;
	m_cbStream = 0;
	m_iReadPos = 0;

	m_eState = eHeader;
	m_bPktType = 0;
	m_abPktSize[0] = 0;
	m_abPktSize[1] = 0;
	m_iPktSize = 0;
	m_iHdrSize = 0;
	m_pData = 0;

	m_fEncoded = false;
}

/**
 *
 */
CPacketParser::~CPacketParser()
{
	if (m_pData)
		delete[] m_pData;

	if (m_pStream)
		delete[] m_pStream;
}


/**
 * \brief 
 */
void CPacketParser::Reboot()
{
	if (m_pData)
		delete[] m_pData;

	if (m_pStream)
		delete[] m_pStream;

	m_pStream = 0;
	m_cbStream = 0;
	m_iReadPos = 0;

	m_eState = eHeader;
	m_bPktType = 0;
	m_abPktSize[0] = 0;
	m_abPktSize[1] = 0;
	m_iPktSize = 0;
	m_iHdrSize = 0;
	m_pData = 0;
}



/**
 * \brief 
 */
bool CPacketParser::AppendStream(BYTE* pBuff, int len)
{
	if (!pBuff || len <= 0)
		return false;

	if (m_iReadPos > m_cbStream)
		m_iReadPos = m_cbStream; // just a precaution

	BYTE* pNewBuff = new BYTE[m_cbStream - m_iReadPos + len];

	if (m_pStream)
	{	
		if (m_iReadPos < m_cbStream)
			memcpy(pNewBuff, m_pStream + m_iReadPos, m_cbStream - m_iReadPos);

		delete[] m_pStream;
	}

	memcpy(pNewBuff + m_cbStream - m_iReadPos, pBuff, len);

	m_pStream = pNewBuff;
	m_cbStream = m_cbStream - m_iReadPos + len;
	m_iReadPos = 0;
	return true;
}



/**
 *
 */
void CPacketParser::Reset()
{
	m_eState = eHeader;

	if (m_pData)
		delete[] m_pData;

	m_bPktType = 0;
	m_abPktSize[0] = 0;
	m_abPktSize[1] = 0;
	m_iHdrSize = 0;
	m_iPktSize = 0;
	m_pData = 0;
}


/**
 *
 */
bool CPacketParser::Next()
{
	switch (m_eState)
	{
	case eHeader:
		return ReadHeader();
	case eSize:
		return ReadSize();
	case eData:
		return ReadData();
	default:
	case eReady:
	case eError:
		return false;
	}
}


/**
 *
 */
bool CPacketParser::GetPacket(CPacket& pkt)
{
	if (m_eState != eReady)
		return false;

	pkt = CPacket(m_bPktType, (WORD)(m_iPktSize - m_iHdrSize), m_pData);
	
	Reset();
	return true;
}


/**
 *
 */
bool CPacketParser::HasError()
{
	return m_eState == eError;
}


/**
 *
 */
bool CPacketParser::ReadHeader()
{
	if (!m_pStream)
		return false;

	if (m_iReadPos >= m_cbStream)
		return false;

	if ((m_pStream[m_iReadPos] & 0xF0) == 0xE0)
	{
//		m_pStream[m_iReadPos] = m_pStream[m_iReadPos] ^ 0x20;
//		m_fEncoded = true;
	}

	m_bPktType = m_pStream[m_iReadPos];

	if (m_bPktType == 0xC1 || m_bPktType == 0xC3)
	{
		m_iHdrSize = 2;
	}
	else if (m_bPktType == 0xC2 || m_bPktType == 0xC4)
	{
		m_iHdrSize = 3;
	}
	else
	{
		CDebugOut::PrintError("Invalid packet type: 0x%02X", m_bPktType);
		CDebugOut::PrintError("%s", CBufferUtil::BufferToHex(m_pStream+m_iReadPos, 8));


		m_iHdrSize = 0;
		m_eState = eError; 
		return false;
	}

	m_abPktSize[0] = 0;
	m_abPktSize[1] = 0;
	m_iPktSize = 0;

	m_eState = eSize;
	return true;
}


/**
 *
 */
bool CPacketParser::ReadSize()
{
	if (m_iReadPos + m_iHdrSize > m_cbStream)
		return false;

//	if (m_fEncoded)
//		m_pStream[m_iReadPos + 1] = m_pStream[m_iReadPos + 1] ^ 0x10;

	for (int i = 0; i < m_iHdrSize - 1; i++)
		m_abPktSize[i] = m_pStream[m_iReadPos + 1 + i];

	m_iPktSize = (m_bPktType & 1) ? m_abPktSize[0] // C1 or C3 packet
								  : MAKEWORD(m_abPktSize[1], m_abPktSize[0]);

	if (m_iPktSize <= 0)
	{
		m_eState = eError;
		return false;
	}

	m_pData = new BYTE[m_iPktSize-m_iHdrSize];
	m_eState = eData;
	return true;
}


/**
 *
 */
bool CPacketParser::ReadData()
{
	if (!m_pData)
	{
		m_eState = eError;
		return false;
	}

	if (m_iReadPos + m_iPktSize > m_cbStream)
		return false;

	int read_len = m_iPktSize - m_iHdrSize;
	memcpy(m_pData, m_pStream + m_iReadPos + m_iHdrSize, read_len);

	m_iReadPos += m_iPktSize;
	m_eState = eReady;
	return false;
}
