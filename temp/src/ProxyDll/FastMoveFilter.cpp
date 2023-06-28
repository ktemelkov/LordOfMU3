#include "stdafx.h"
#include "FastMoveFilter.h"
#include "CommonPackets.h"
#include "DebugOut.h"


/**
 * \brief 
 */
CFastMoveFilter::CFastMoveFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy)
{
	m_fEnabled = FALSE;
	m_fSuspended = false;
}

/**
 * \brief 
 */
int CFastMoveFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_fEnabled && !m_fSuspended && pkt.GetInjected() == 0 
			&& (pkt == CObjectMovedPacket::Type()))
	{
		CObjectMovedPacket pktMoved(pkt);

		CPacketFilter *pFilter = GetProxy()->GetFilter("CharInfoFilter");
		if (!pFilter)
		{
			CDebugOut::PrintError("Bad software configuration. CharInfoFilter filter object not found.");
			return 0;
		}

		WORD wId = 0;
		pFilter->GetParam("PlayerId", &wId);

		if (wId == pktMoved.GetId())
			return -1;
	}

	return 0;
}


/**
 * \brief 
 */
int CFastMoveFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_fEnabled && !m_fSuspended && pkt.GetInjected() == 0)
	{
		int x = -1;
		int y = -1;
		BYTE dir = 0;

		int coef[8][2] = 
		{ 
			{-1, -1},
			{ 0, -1},
			{ 1, -1},
			{ 1,  0},
			{ 1,  1},
			{ 0,  1},
			{-1,  1},
			{-1,  0}
		};


		if (pkt == CCharMoveCTSPacket::Type())
		{
			CCharMoveCTSPacket pkt1(pkt);

			x = pkt1.GetX();
			y = pkt1.GetY();
			dir = pkt1.GetDir();

			int dx = pkt1.GetDX();

			x = (BYTE)((int)x + dx*coef[dir][0]);
			y = (BYTE)((int)y + dx*coef[dir][1]);
		}
/*		else if (pkt == CCharMoveCTSx2Packet::Type()
			|| pkt == CCharMoveCTSx3Packet::Type() 
			|| pkt == CCharMoveCTSx4Packet::Type()
			|| pkt == CCharMoveCTSx5Packet::Type())
		{
			CCharMoveCTSx2Packet pkt1(pkt);

			x = pkt1.GetX();
			y = pkt1.GetY();
			dir = pkt1.GetDir();
		}
*/

		if (x != -1 && y != -1)
		{
			CPacketFilter *pFilter = GetProxy()->GetFilter("CharInfoFilter");
			if (!pFilter)
			{
				CDebugOut::PrintError("Bad software configuration. CharInfoFilter filter object not found.");
				return 0;
			}

			WORD wId = 0;
			pFilter->GetParam("PlayerId", &wId);

			CUpdatePosSTCPacket pktSTC(wId, x, y);
			CUpdatePosCTSPacket pktCTS(x, y, dir);

			GetProxy()->send_packet(pktCTS);

			GetProxy()->recv_packet(pktSTC);

			CObjectAnimationPacket pktAnim(wId, wId, dir, 1);
			GetProxy()->recv_packet(pktAnim);

			return -1;
		}
	}

	return 0;
}


/**
 * \brief 
 */
bool CFastMoveFilter::GetParam(const char* pszParam, void* pData)
{
	return false;
}


/**
 * \brief 
 */
bool CFastMoveFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "enabled") == 0)
	{
		m_fEnabled = *((BOOL*)pData);
	}
	else if (_stricmp(pszParam, "suspended") == 0)
	{
		m_fSuspended = *((bool*)pData);
	}

	return true;
}
