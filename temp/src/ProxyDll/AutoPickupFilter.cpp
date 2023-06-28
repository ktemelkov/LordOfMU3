#include "stdafx.h"
#include "AutoPickupFilter.h"
#include "CommonPackets.h"
#include <math.h>


/**
 * \brief 
 */
CAutoPickupFilter::CAutoPickupFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy)
{
	m_ulBlessFlags = 0;
	m_ulSoulFlags = 0;
	m_ulChaosFlags = 0;
	m_ulLifeFlags = 0;
	m_ulCreationFlags = 0;
	m_ulGuardianFlags = 0;
	m_ulExlFlags = 0;
	m_ulZenFlags = 0;
	m_ulCustomFlags = 0;
	m_iDist = -1;

	m_fEnabled = FALSE;
	m_fDisplayCode = FALSE;
	m_fPickAll = false;

	m_fSuspended = false;
	m_fSuspPick = false;
	m_fSuspZen = false;
	m_fSuspMove = false;

	InitializeCriticalSection(&m_csQueue);

	m_hPacketEvent = CreateEvent(0, 1, 0, 0);
	m_hPickEvent = CreateEvent(0, 1, 0, 0);
	m_hStopEvent = CreateEvent(0, 1, 0, 0);
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PickThreadProc, this, 0, 0);
}


/**
 * \brief 
 */
CAutoPickupFilter::~CAutoPickupFilter()
{	
	if (m_hThread && m_hThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hStopEvent);

		Sleep(100);

		if (WaitForSingleObject(m_hThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 0);

		CloseHandle(m_hThread);
	}

	CloseHandle(m_hStopEvent);
	CloseHandle(m_hPickEvent);
	DeleteCriticalSection(&m_csQueue);
}


/**
 * \brief 
 */
int CAutoPickupFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_fEnabled && pkt == CMeetItemPacket::Type())
	{
		if (m_fSuspended && m_fSuspPick)
			return 0;

		CMeetItemPacket& pktMItem = (CMeetItemPacket&)pkt;

		ULONG ulZenFlags = (m_fSuspended && m_fSuspZen) ? 0 : m_ulZenFlags;

		struct { WORD wType; ULONG* pFlags; } _map[] =
			{
				{TYPE_BLESS, &m_ulBlessFlags}, {TYPE_SOUL, &m_ulSoulFlags}, {TYPE_CHAOS, &m_ulChaosFlags},
				{TYPE_LIFE, &m_ulLifeFlags}, {TYPE_JOC, &m_ulCreationFlags}, {TYPE_JOG, &m_ulGuardianFlags},  
				{TYPE_ZEN, &ulZenFlags}
			};

		BYTE bPlX = 0;
		BYTE bPlY = 0;

		CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
		if (pFilter)
		{
			pFilter->GetParam("X", (void*)&bPlX);
			pFilter->GetParam("Y", (void*)&bPlY);
		}

		for (int i = pktMItem.GetItemCount()-1; i >= 0; --i)
		{
			BYTE x = 0, y = 0;
			pktMItem.GetItemPos(i, x, y);

			WORD wType = pktMItem.GetItemType(i);
			WORD wMask = 1 << (wType >> 12);
			WORD wId = pktMItem.GetItemId(i);

			wType &= 0x0FFF;

			int xdiff = abs((int)bPlX - (int)x);
			int ydiff = abs((int)bPlY - (int)y);

			std::map<WORD, WORD>::iterator it = m_vItemList.find(wType);

			if (m_fPickAll || (it != m_vItemList.end() && (wMask & it->second) != 0))
			{
				if (m_iDist < 0 || (xdiff <= m_iDist && ydiff <= m_iDist && (m_ulCustomFlags & 1)))
				{
					if ((m_ulCustomFlags & 2) != 0 && (CPacketType::GetFeatures() & FEATURE_MOVE_TO_PICK) != 0)
					{
						CAutoLockQueue autoCS(&m_csQueue);

						m_vPickQueue.insert((ULONG)wId | (ULONG)x << 16 | (ULONG)y << 24);
						SetEvent(m_hPickEvent);
					}
					else
					{
						PickItem(wId);
					}
				}
			}
			else
			{
				BOOL fFound = FALSE;
				ULONG ulFlags = 0;

				for (int j=0; j < sizeof(_map)/sizeof(_map[0]); j++)
				{
					if (wType == _map[j].wType)
					{
						ulFlags = *(_map[j].pFlags);
						fFound = TRUE;
						break;
					}
				}


				BYTE* pItemCode = pktMItem.GetItemData(i) + 4;

				if (!fFound && (m_ulExlFlags & 1)
						&& ((pItemCode[3] & 0x3F) || (pItemCode[4] & 0x01)))
				{
					ulFlags = m_ulExlFlags;
					fFound = TRUE;
				}

				if (fFound && (ulFlags & 1))
				{
					if ((ulFlags & 2) != 0)
					{
						if (m_iDist < 0 || (xdiff <= m_iDist && ydiff <= m_iDist))
						{
							CAutoLockQueue autoCS(&m_csQueue);

							if ((CPacketType::GetFeatures() & FEATURE_MOVE_TO_PICK) != 0)
							{
								m_vPickQueue.insert((ULONG)wId | (ULONG)x << 16 | (ULONG)y << 24);
								SetEvent(m_hPickEvent);
							}
							else
							{
								m_vNoMovePickQueue.push_back(CPickInfo(wId));							
							}
						}
					}
					else
					{
						m_vNoMovePickQueue.push_back(CPickInfo(wId));
					}
				}
			}
		}
	}
	else if (pkt == CPutInventoryPacket::Type())
	{
		CPutInventoryPacket& pkt2 = (CPutInventoryPacket&)pkt;

		WORD wType = pkt2.GetItemType();
		WORD wMask = 1 << (wType >> 12);
		BYTE bPos = pkt2.GetInvPos();

		wType &= 0x0FFF;

		if (m_fEnabled)
		{
			if (m_fSuspended && m_fSuspPick)
				return 0;

			std::map<WORD, WORD>::iterator it = m_vDropList.find(wType);

			if (it != m_vDropList.end() && (wMask & it->second) != 0)
				m_vDropQueue.push_back(CDropInfo(bPos));
		}
		else if (m_fDisplayCode)
		{
			CServerMessagePacket pktMsg(">> Item code: %d %d %d", HIBYTE(wType) & 0x0F, LOBYTE(wType), HIBYTE(wType) >> 4);
			GetProxy()->recv_direct(pktMsg);
		}
	}
	else if (pkt == CCreateInvItemPacket::Type())
	{
		CCreateInvItemPacket& pkt2 = (CCreateInvItemPacket&)pkt;

		WORD wType = pkt2.GetItemType();
		WORD wMask = 1 << (wType >> 12);
		BYTE bPos = pkt2.GetInvPos();

		wType &= 0x0FFF;

		if (m_fEnabled)
		{
			if (m_fSuspended && m_fSuspPick)
				return 0;

			std::map<WORD, WORD>::iterator it = m_vDropList.find(wType);

			if (it != m_vDropList.end() && (wMask & it->second) != 0)
				m_vDropQueue.push_back(CDropInfo(bPos));
		}
		else if (m_fDisplayCode)
		{
			CServerMessagePacket pktMsg(">> Item code: %d %d %d", HIBYTE(wType) & 0x0F, LOBYTE(wType), HIBYTE(wType) >> 4);
			GetProxy()->recv_direct(pktMsg);
		}
	}
	else if (m_fDisplayCode)
	{
		WORD wType = 0;

		if (pkt == CMoveToInventoryPacket::Type())
		{
			wType = ((CMoveToInventoryPacket&)pkt).GetItemType();
		}
		else if (pkt == CCreateInvItemPacket::Type())
		{
			wType = ((CCreateInvItemPacket&)pkt).GetItemType();
		}

		if (wType != 0)
		{
			CServerMessagePacket pktMsg(">> Item code: %d %d %d", HIBYTE(wType) & 0x0F, LOBYTE(wType), HIBYTE(wType) >> 4);
			GetProxy()->recv_direct(pktMsg);
		}
	}
	else if (pkt == CWarpReplyPacket::Type())
	{
		m_fEnabled = FALSE;
		ClearQueues();
	}


	ProcessDropQueue();
	ProcessNoMovePickupQueue();
	return 0;
}


/**
 * \brief 
 */
int CAutoPickupFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	ProcessDropQueue();
	ProcessNoMovePickupQueue();
	return 0;
}


/**
 * \brief 
 */
bool CAutoPickupFilter::GetParam(const char* pszParam, void* pData)
{
	return false;
}


/**
 * \brief 
 */
bool CAutoPickupFilter::SetParam(const char* pszParam, void* pData)
{
	struct { const char* pszOpt; ULONG* pFlags; } _map[] = 
		{
			{"bless", &m_ulBlessFlags},
			{"soul", &m_ulSoulFlags},
			{"chaos", &m_ulChaosFlags},
			{"jol", &m_ulLifeFlags},
			{"joc", &m_ulCreationFlags},
			{"jog", &m_ulGuardianFlags},
			{"exl", &m_ulExlFlags},
			{"zen", &m_ulZenFlags},
			{"custom", &m_ulCustomFlags},
		};

	for (int i=0; i < sizeof(_map)/sizeof(_map[0]); i++)
	{
		if (_stricmp(pszParam, _map[i].pszOpt) == 0)
		{
			*_map[i].pFlags = *((ULONG*)pData);
			return true;
		}
	}

	if (_stricmp(pszParam, "autopick") == 0)
	{
		m_fEnabled = *((BOOL*)pData);

		if (!m_fEnabled)
			ClearQueues();
	}
	else if (_stricmp(pszParam, "pick") == 0)
	{
		DWORD dwData = *((DWORD*)pData);
		WORD wCode = LOWORD(dwData);
		WORD wMask = HIWORD(dwData);

		std::map<WORD,WORD>::iterator  it = m_vItemList.find(wCode);

		if (it != m_vItemList.end())
			it->second = it->second | wMask;
		else
			m_vItemList.insert(std::pair<WORD,WORD>(wCode, wMask));
	}
	else if (_stricmp(pszParam, "pick_clear") == 0)
	{
		m_vItemList.clear();
		m_fPickAll = false;
	}
	else if (_stricmp(pszParam, "pick_all") == 0)
	{
		m_fPickAll = true;
	}
	else if (_stricmp(pszParam, "drop") == 0)
	{
		DWORD dwData = *((DWORD*)pData);
		WORD wCode = LOWORD(dwData);
		WORD wMask = HIWORD(dwData);

		std::map<WORD,WORD>::iterator  it = m_vDropList.find(wCode);

		if (it != m_vDropList.end())
			it->second = it->second | wMask;
		else
			m_vDropList.insert(std::pair<WORD,WORD>(wCode, wMask));
	}
	else if (_stricmp(pszParam, "drop_clear") == 0)
	{
		m_vDropList.clear();
	}
	else if (_stricmp(pszParam, "suspended") == 0)
	{
		m_fSuspended = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "itemcode") == 0)
	{
		m_fDisplayCode = *((bool*)pData);	
	}
	else if (_stricmp(pszParam, "susp_move_pick") == 0)
	{
		m_fSuspMove = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "susp_pick") == 0)
	{
		m_fSuspPick = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "susp_zen_pick") == 0)
	{
		m_fSuspZen = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "pdist") == 0)
	{
		m_iDist = *((int*)pData);
	}

	return true;
}


/**
 * \brief 
 */
DWORD CAutoPickupFilter::PickThreadProc(CAutoPickupFilter* pThis)
{
	while (1)
	{
		HANDLE objs[] = {pThis->m_hStopEvent, pThis->m_hPickEvent};
		DWORD dwRes = WaitForMultipleObjects(2, objs, 0, INFINITE);

		if (dwRes == WAIT_OBJECT_0)
			break;
		
		pThis->GoPickNextItem();
	}

	return 0;
}


/**
 * \brief 
 */
void CAutoPickupFilter::GoPickNextItem()
{
	WORD wItem = 0;
	BYTE x = 0;
	BYTE y = 0;
	bool fSuspended = false;
	bool fSuspMove = false;
	bool fPickValid = false;

	EnterCriticalSection(&m_csQueue);

	if (m_vPickQueue.size() != 0)
	{
		fPickValid = true;

		ULONG ulItemInfo = *(m_vPickQueue.begin());
		m_vPickQueue.erase(ulItemInfo);

		wItem = LOWORD(ulItemInfo);
		x = LOBYTE(HIWORD(ulItemInfo));
		y = HIBYTE(HIWORD(ulItemInfo));
	}
	else
	{
		ResetEvent(m_hPickEvent);
	}

	fSuspended = m_fSuspended;
	fSuspMove = m_fSuspMove;
	LeaveCriticalSection(&m_csQueue);

	if (!fPickValid)
		return;

	CPacketFilter* pCharInfo = GetProxy()->GetFilter("CharInfoFilter");
	CPacketFilter* pScript = GetProxy()->GetFilter("ScriptProcessorFilter");
	CPacketFilter* pAK = GetProxy()->GetFilter("AutoKillFilter");

	BYTE xOld = 0;
	BYTE yOld = 0;
	WORD wPlayerId = 0;

	if (!pCharInfo 
			|| !pCharInfo->GetParam("X", &xOld)
			|| !pCharInfo->GetParam("Y", &yOld)
			|| !pCharInfo->GetParam("PlayerId", &wPlayerId))
	{
		return;
	}

	BOOL fDisableClientMove = FALSE;
	BOOL fAkEn = FALSE;

	if (pAK)
	{
		pAK->GetParam("autokill", &fDisableClientMove);
		pAK->GetParam("afkstat", &fAkEn);
	}

	if (fAkEn)
		fDisableClientMove = FALSE;


	if (!fSuspended || !fSuspMove)
	{
		bool fTemp = true;
		if (pScript && !fSuspended)
			pScript->SetParam("suspended", (void*)&fTemp);

		if (pAK && !fSuspended)
			pAK->SetParam("suspend_move", (void*)&fTemp);

		Sleep(10);

		// Teleport to item
		TeleportTo(wPlayerId, x, y, fDisableClientMove, m_hPacketEvent);
		WaitForSingleObject(m_hPacketEvent, 850);
	}

	// pick item
	PickItem(wItem, m_hPacketEvent);
	WaitForSingleObject(m_hPacketEvent, 700);

	if (!fSuspended || !fSuspMove)
	{
		if (!fAkEn)
		{
			TeleportTo(wPlayerId, xOld, yOld, fDisableClientMove, m_hPacketEvent);
			WaitForSingleObject(m_hPacketEvent, 350);
		}

		bool fTemp = false;
		if (pScript && !fSuspended)
			pScript->SetParam("suspended", (void*)&fTemp);

		if (pAK && !fSuspended)
			pAK->SetParam("suspend_move", (void*)&fTemp);
	}
}


/**
 * \brief 
 */
void CAutoPickupFilter::PickItem(WORD wId, HANDLE hEvent)
{
	if (hEvent)
		ResetEvent(hEvent);

	CPickItemPacket pkt(wId);
	pkt.PutEvent(hEvent);
	GetProxy()->send_packet(pkt);
}


/**
 * \brief 
 */
void CAutoPickupFilter::DropItem(BYTE pos)
{
	CPacketFilter* pCharInfo = GetProxy()->GetFilter("CharInfoFilter");

	BYTE x = 0;
	BYTE y = 0;

	if (!pCharInfo 
		|| !pCharInfo->GetParam("X", &x)
		|| !pCharInfo->GetParam("Y", &y))
	{
		return;
	}

	CDropItemPacket pkt(x, y, pos);
	GetProxy()->send_packet(pkt);
}


/**
 * \brief 
 */
void CAutoPickupFilter::TeleportTo(WORD wPlayerId, BYTE x, BYTE y, BOOL fNoClientMove, HANDLE hEvent)
{
	if (hEvent)
		ResetEvent(hEvent);

	CUpdatePosCTSPacket pktMoveCTS(x, y);
	pktMoveCTS.PutEvent(hEvent);
	GetProxy()->send_packet(pktMoveCTS);

	if (!fNoClientMove)
	{
		CUpdatePosSTCPacket pktMoveSTC(wPlayerId, x, y);
		GetProxy()->recv_packet(pktMoveSTC);
	}
}


/**  
 * \brief 
 */
void CAutoPickupFilter::ProcessDropQueue()
{
	while (!m_vDropQueue.empty() && (GetTickCount() - m_vDropQueue.front().dwTimestamp) > 700)
	{
		DropItem(m_vDropQueue.front().bInvPos);
		m_vDropQueue.pop_front();
	}
}


/**  
 * \brief 
 */
void CAutoPickupFilter::ProcessNoMovePickupQueue()
{
	while (!m_vNoMovePickQueue.empty() && (GetTickCount() - m_vNoMovePickQueue.front().dwTimestamp) > 2000)
	{
		PickItem(m_vNoMovePickQueue.front().wItemId);
		m_vNoMovePickQueue.pop_front();
	}
}


/**  
 * \brief 
 */
void CAutoPickupFilter::ClearQueues()
{
	m_vNoMovePickQueue.clear();

	EnterCriticalSection(&m_csQueue);
	m_vPickQueue.clear();
	LeaveCriticalSection(&m_csQueue);
}
