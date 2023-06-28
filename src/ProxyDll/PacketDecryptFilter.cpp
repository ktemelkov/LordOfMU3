#include "stdafx.h"
#include "PacketDecryptFilter.h"
#include "CommonPackets.h"
#include "EncDec.h"
#include "BufferUtil.h"
#include "DebugOut.h"



/**
 * \brief 
 */
#define DETECT_PACKET_TYPE(type) \
	__if_not_exists (__f_type_declared) \
	{ \
		bool __f_type_declared = true; \
		if (false) \
		{ \
		} \
	} \
	else if (pkt == type::Type()) \
	{ \
		pkt.SetType(type::Type()); \
		__if_exists(type::InitStatic) \
		{ \
			type::InitStatic(pkt);\
		}\
	} \


/**
 * \brief 
 */
CPacketDecryptFilter::CPacketDecryptFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy) 
{ 
}


/**
 * \brief 
 */
int CPacketDecryptFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt.IsDecrypted())
		return 0;

	if (pkt.GetPktClass() == 0xC3 || pkt.GetPktClass() == 0xC4)
	{
		CEncDec::DecryptC3asClient(pkt);
	}
	else
	{
		pkt.SetDecryptedPacket(pkt.GetRawPacket(), pkt.GetPktLen());
	}

	if (!pkt.IsDecrypted())
		return 0;

	DetectRecvPacketType(pkt);

	int len = pkt.GetDecryptedLen();
	BYTE* buf = pkt.GetDecryptedPacket();
	const BYTE* xorP = pkt.GetType().GetXorParams();

	if (xorP && (xorP[0] != 0 || xorP[1] != 0))
		CEncDec::DecXor32(buf + xorP[0], xorP[1], len - xorP[0]);

	if (pkt.GetType().NeedPostDetect())
		PostDetectRecvPacketType(pkt);

	return 0;
}


/**
 * \brief 
 */
int CPacketDecryptFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt.IsDecrypted())
		return 0;

	if (pkt.GetPktClass() == 0xC3 || pkt.GetPktClass() == 0xC4)
	{
		CEncDec::DecryptC3asServer(pkt);
	}
	else
	{
		pkt.SetDecryptedPacket(pkt.GetRawPacket(), pkt.GetPktLen());
	}

	DetectSendPacketType(pkt);

	int len = pkt.GetDecryptedLen();
	BYTE* buf = pkt.GetDecryptedPacket();
	const BYTE* xorP = pkt.GetType().GetXorParams();

	if (xorP && (xorP[0] != 0 || xorP[1] != 0))
		CEncDec::DecXor32(buf + xorP[0], xorP[1], len - xorP[0], true);

	if (pkt.GetType().NeedPostDetect())
		PostDetectSendPacketType(pkt);

	return 0;
}


/**
 * \brief 
 */
void CPacketDecryptFilter::DetectRecvPacketType(CPacket& pkt)
{
	DETECT_PACKET_TYPE(CConnectServHelloPacket)

	DETECT_PACKET_TYPE(CObjectMovedPacket)
	DETECT_PACKET_TYPE(CUpdatePosSTCPacket)


	DETECT_PACKET_TYPE(CHitInfoPacket)

	DETECT_PACKET_TYPE(CObjectAnimationPacket)
	DETECT_PACKET_TYPE(CSkillUsedPacket)
	DETECT_PACKET_TYPE(CMassiveSkillUsedPacket)
	DETECT_PACKET_TYPE(CMassiveSkillEffectPacket)

	DETECT_PACKET_TYPE(CPingReplyPacket)

	DETECT_PACKET_TYPE(CItemUsedPacket)
	DETECT_PACKET_TYPE(CUpdateManaAGPacket)
	DETECT_PACKET_TYPE(CUpdateLifeSDPacket)

	DETECT_PACKET_TYPE(CMeetMonsterPacket)
	DETECT_PACKET_TYPE(CMeetItemPacket)
	DETECT_PACKET_TYPE(CMeetPlayerPacket)
	DETECT_PACKET_TYPE(CMeetDisguisedPlayerPacket)
	DETECT_PACKET_TYPE(CForgetObjectPacket)
	DETECT_PACKET_TYPE(CForgetItemPacket)

	DETECT_PACKET_TYPE(CObjectDeathPacket)

	DETECT_PACKET_TYPE(CItemLostPacket)

	DETECT_PACKET_TYPE(CLevelUpPacket)

	DETECT_PACKET_TYPE(CWarpReplyPacket)
	DETECT_PACKET_TYPE(CCharRespawnPacket)

	DETECT_PACKET_TYPE(CServerMessagePacket)
	DETECT_PACKET_TYPE(CGMMessagePacket)
	DETECT_PACKET_TYPE(CCharacterWhisperSTCPacket)
	DETECT_PACKET_TYPE(CGlobalSpeakPacket)

	DETECT_PACKET_TYPE(CPutInventoryPacket)
	DETECT_PACKET_TYPE(CMoveToInventoryPacket)
	DETECT_PACKET_TYPE(CMoveToVaultPacket)
	DETECT_PACKET_TYPE(CRemoveFromInventoryPacket)

	DETECT_PACKET_TYPE(CAssignPlayerToGuildPacket)

	DETECT_PACKET_TYPE(CVaultContentsPacket)

	DETECT_PACKET_TYPE(CInitInventoryPacket)
	DETECT_PACKET_TYPE(CCharStatsPacket)

	DETECT_PACKET_TYPE(CSetMaxManaAGPacket)
	DETECT_PACKET_TYPE(CSetMaxLifeSDPacket)

	DETECT_PACKET_TYPE(CTradeReplyPacket)
	DETECT_PACKET_TYPE(CTradeCanceledPacket)

	DETECT_PACKET_TYPE(CCreateInvItemPacket)

	DETECT_PACKET_TYPE(CGameServerHelloPacket)

	DETECT_PACKET_TYPE(CServerListReplyPacket)
	DETECT_PACKET_TYPE(CServerInfoReplyPacket)

	DETECT_PACKET_TYPE(CUpdateSkillPacket)
	DETECT_PACKET_TYPE(CInitSkillsPacket)

	DETECT_PACKET_TYPE(CCharListReplyPacket)
	DETECT_PACKET_TYPE(CCharSetupPacket)
	DETECT_PACKET_TYPE(CCharCreateReplyPacket)
	DETECT_PACKET_TYPE(CClientAuthReplyPacket)
	DETECT_PACKET_TYPE(CClientDCReplyPacket)
}


/**
 * \brief 
 */
void CPacketDecryptFilter::DetectSendPacketType(CPacket& pkt)
{
	DETECT_PACKET_TYPE(CNormalAttackPacket)
	DETECT_PACKET_TYPE(CSingleSkillPacket)
	DETECT_PACKET_TYPE(CMassiveSkillPacket)

	DETECT_PACKET_TYPE(CCharRotateCTSPacket)
	DETECT_PACKET_TYPE(CCharMoveCTSPacket)
	DETECT_PACKET_TYPE(CCharMoveCTSx2Packet)
	DETECT_PACKET_TYPE(CCharMoveCTSx3Packet)
	DETECT_PACKET_TYPE(CCharMoveCTSx4Packet)
	DETECT_PACKET_TYPE(CUpdatePosCTSPacket)

	DETECT_PACKET_TYPE(CCharacterSayPacket)
	DETECT_PACKET_TYPE(CCharacterSayPacket2)
	DETECT_PACKET_TYPE(CCharacterSayPacket3)
	DETECT_PACKET_TYPE(CCharacterWhisperPacket)
	DETECT_PACKET_TYPE(CClientPingPacket)

	DETECT_PACKET_TYPE(CCharCommandsPacket)
	DETECT_PACKET_TYPE(CPickItemPacket)

	DETECT_PACKET_TYPE(CDropItemPacket)
	DETECT_PACKET_TYPE(CMoveItemReqPacket)
	DETECT_PACKET_TYPE(CUseItemPacket)
	DETECT_PACKET_TYPE(CBuyStoreItemPacket)
	DETECT_PACKET_TYPE(CSellItemPacket)
	
	DETECT_PACKET_TYPE(CTradeRequestPacket)
	DETECT_PACKET_TYPE(COpenVaultRequestPacket)
	DETECT_PACKET_TYPE(COpenStoreRequestPacket)


	DETECT_PACKET_TYPE(CServerListReqPacket)
	DETECT_PACKET_TYPE(CServerInfoReqPacket)

	DETECT_PACKET_TYPE(CClientF1Packet)

	DETECT_PACKET_TYPE(CUnkSendC1Packet)
	DETECT_PACKET_TYPE(CUnkSendC3Packet)	
}


/**
 * \brief 
 */
void CPacketDecryptFilter::PostDetectRecvPacketType(CPacket& pkt)
{
}


/**
 * \brief 
 */
void CPacketDecryptFilter::PostDetectSendPacketType(CPacket& pkt)
{
	DETECT_PACKET_TYPE(CCharSelectedPacket)
	DETECT_PACKET_TYPE(CCharListReqPacket)
	DETECT_PACKET_TYPE(CCharCreateReqPacket)

	DETECT_PACKET_TYPE(CClientAuthPacket)
	DETECT_PACKET_TYPE(CClientDCReqPacket)
}

