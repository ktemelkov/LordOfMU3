#include "stdafx.h"
#include "PacketLogger.h"
#include "DebugOut.h"
#include "BufferUtil.h"

/**
 *
 */
CPacketLogger::CPacketLogger(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
}

/**
 *
 */
CPacketLogger::~CPacketLogger()
{
}

/**
 * 
 */
const char* CPacketLogger::GetName()
{
	return "PacketLogger";
}

/**
 *
 */
WORD CPacketLogger::GetLevel()
{
	return FILTER_LEVEL_LOGGER;
}

/**
 *
 */
int CPacketLogger::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	CDebugOut::PrintInfo("[RECV] %s -> %s", 
				pkt.GetType().GetDescription(),
				CBufferUtil::BufferToHex((char*)pkt.GetDecryptedPacket(), pkt.GetDecryptedLen()));

	CStringA strDetails = pkt.GetType().DescribePacket(pkt);

	if (strDetails.GetLength() > 0)
		CDebugOut::PrintInfo("%s", (char*)(const char*)strDetails);

	return 0;
}


/**
 *
 */
int CPacketLogger::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	BYTE* buf = pkt.GetDecryptedPacket();
	int len = pkt.GetDecryptedLen();

	if (!buf || len <= 0)
	{
		buf = pkt.GetRawPacket();
		len = pkt.GetPktLen();

		CDebugOut::PrintInfo("[SEND] %s -> RAW: %s", 
			pkt.GetType().GetDescription(),
			CBufferUtil::BufferToHex((char*)buf, len));

	}
	else
	{
		CDebugOut::PrintInfo("[SEND] %s -> %s", 
			pkt.GetType().GetDescription(),
			CBufferUtil::BufferToHex((char*)buf, len));
	}

	CStringA strDetails = pkt.GetType().DescribePacket(pkt);

	if (strDetails.GetLength() > 0)
		CDebugOut::PrintInfo("%s", (char*)(const char*)strDetails);

	return 0;
}
