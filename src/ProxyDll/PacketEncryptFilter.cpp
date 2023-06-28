#include "stdafx.h"
#include "PacketEncryptFilter.h"
#include "CommonPackets.h"
#include "EncDec.h"
#include "BufferUtil.h"
#include "DebugOut.h"


/**
 * \brief 
 */
CPacketEncryptFilter::CPacketEncryptFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy) 
{
	m_ulSendC3Counter = 0;
	m_bLastSendC3Counter = 0;
	m_ulRecvC3Counter = 0;
}

/**
 * \brief 
 */
int CPacketEncryptFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (!pkt.IsDecrypted())
		return 0;

	EncryptXOR(pkt);

	if (pkt.GetPktClass() == 0xC3 || pkt.GetPktClass() == 0xC4)
	{
		CEncDec::EncryptC3asServer(pkt);
	}
	else
	{
		pkt = CPacket(pkt.GetDecryptedPacket(), pkt.GetDecryptedLen());
	}
	
	return 0;
}

/**
 * \brief 
 */
int CPacketEncryptFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (!pkt.IsDecrypted())
		return 0;


	EncryptXOR(pkt, true);

	if (pkt.GetPktClass() == 0xC3 || pkt.GetPktClass() == 0xC4)
	{
/*
		if (pkt.GetInjected() != 0)
			m_ulSendC3Counter = (m_ulSendC3Counter + 1) % 256;
		else
			m_bLastSendC3Counter = *(pkt.GetDecryptedPacket() + pkt.GetHdrLen());

		DWORD counter = (m_bLastSendC3Counter + m_ulSendC3Counter) % 256;		
		*(pkt.GetDecryptedPacket() + pkt.GetHdrLen()) = (BYTE)counter;
*/
		
		*(pkt.GetDecryptedPacket() + pkt.GetHdrLen()) = (BYTE)m_ulSendC3Counter;

		m_ulSendC3Counter = (m_ulSendC3Counter + 1) % 256;

		CEncDec::EncryptC3asClient(pkt);
	}
	else
	{
		pkt = CPacket(pkt.GetDecryptedPacket(), pkt.GetDecryptedLen());
	}

	return 0;
}

/**
 * \brief 
 */
void CPacketEncryptFilter::EncryptXOR(CPacket& pkt, bool fSend)
{
	const BYTE* xorP = pkt.GetType().GetXorParams();

	if (xorP && (xorP[0] != 0 || xorP[1] != 0))
	{
		int len = pkt.GetDecryptedLen();
		BYTE* buf = pkt.GetDecryptedPacket();

		CEncDec::EncXor32(buf + xorP[0], xorP[1], len - xorP[0], fSend);
	}
}
