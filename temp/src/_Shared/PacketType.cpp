#include "stdafx.h"
#include "PacketType.h"



ULONG CPacketType::m_ulVersion = 0;
DWORD CPacketType::m_dwFeatures = 0;



/**
 * \brief 
 */
CPacketType::CPacketType()
{
	m_fPostDetect = false;
	m_pfnDescribe = 0;

	m_abXorP[0] = 0;
	m_abXorP[1] = 0;

	m_pPat = new BYTE[1];
	m_pPat[0] = 0;

	m_pMask = new BYTE[1];
	m_pMask[0] = 0;

	m_szDescr = new char[128];
	strcpy(m_szDescr, "Unknown packet");
}

/**
 * \brief 
 */
CPacketType::CPacketType(const CPacketType& in)
{
	m_abXorP[0] = 0;
	m_abXorP[1] = 0;
	m_pPat = 0;
	m_pMask = 0;
	m_szDescr = 0;
	m_pfnDescribe = 0;
	m_fPostDetect = in.NeedPostDetect();

	operator=(in);
}

/**
 * \brief 
 */
CPacketType::~CPacketType()
{
	if (m_pPat)
		delete[] m_pPat;
	
	if (m_pMask)
		delete[] m_pMask;

	if (m_szDescr)
		delete[] m_szDescr;
}

/**
 * \brief 
 */
CPacketType& CPacketType::operator=(const CPacketType& in)
{
	if (m_pPat)
		delete[] m_pPat;
	
	if (m_pMask)
		delete[] m_pMask;

	if (m_szDescr)
		delete[] m_szDescr;

	m_pPat = 0;
	m_pMask = 0;
	m_szDescr = 0;

	m_pfnDescribe = in.GetDescribeProc();

	const BYTE* xor = in.GetXorParams();

	m_abXorP[0] = xor[0];
	m_abXorP[1] = xor[1];

	const BYTE* pat = in.GetPattern();
	const BYTE* msk = in.GetMask();
	const char* descr = in.GetDescription();

	if (pat)
	{
		m_pPat = new BYTE[pat[0]+1];
		memcpy(m_pPat, pat, pat[0]+1);
	}

	if (msk)
	{
		m_pMask = new BYTE[msk[0]+1];
		memcpy(m_pMask, msk, msk[0]+1);
	}

	if (descr)
	{
		int len = (int)strlen(descr);
		m_szDescr = new char[len+1];
		strcpy(m_szDescr, descr);
	}

	m_fPostDetect = in.NeedPostDetect();
	return *this;
}


/**  
 * \brief 
 */
CStringA CPacketType::DescribePacket(CPacket& pkt)
{
	return (!m_pfnDescribe) ? "" : m_pfnDescribe(pkt);
}
