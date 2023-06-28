#ifndef __AutoPickupFilter_H
#define __AutoPickupFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"
#include <map>
#include <set>
#include <deque>


/**
 * \brief 
 */
class CAutoPickupFilter
	: public CPacketFilter
{
	/**  
	 * \brief 
	 */
	struct CAutoLockQueue
	{
		CAutoLockQueue(CRITICAL_SECTION* pCS){ EnterCriticalSection(m_pCS = pCS); }
		virtual ~CAutoLockQueue(){ LeaveCriticalSection(m_pCS); }

		CRITICAL_SECTION* m_pCS;
	};

	/**  
	 * \brief 
	 */
	struct CDropInfo
	{
		CDropInfo(BYTE pos){ bInvPos = pos; dwTimestamp = GetTickCount(); }
		CDropInfo(const CDropInfo& in){ bInvPos = in.bInvPos; dwTimestamp = in.dwTimestamp; }

		BYTE bInvPos;
		DWORD dwTimestamp;

	private:
		void operator=(const CDropInfo& in){}
	};

	/**  
	 * \brief 
	 */
	struct CPickInfo
	{
		CPickInfo(WORD wId){ wItemId = wId; dwTimestamp = GetTickCount(); }
		CPickInfo(const CPickInfo& in){ wItemId = in.wItemId; dwTimestamp = in.dwTimestamp; }

		WORD wItemId;
		DWORD dwTimestamp;

	private:
		void operator=(const CPickInfo& in){}
	};

public:
	CAutoPickupFilter(CProxy* pProxy);
	virtual ~CAutoPickupFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "AutoPickupFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

private:
	static DWORD PickThreadProc(CAutoPickupFilter* pThis);

	void PickItem(WORD wId, HANDLE hEvent = 0);
	void DropItem(BYTE pos);
	void GoPickNextItem();
	void TeleportTo(WORD wPlayerId, BYTE x, BYTE y, BOOL fNoClientMove = FALSE, HANDLE hEvent = 0);

	void ProcessDropQueue();
	void ProcessNoMovePickupQueue();
	void ClearQueues();

private:
	std::map<WORD, WORD> m_vItemList;
	std::map<WORD, WORD> m_vDropList;
	std::set<ULONG> m_vPickQueue;
	std::deque<CDropInfo> m_vDropQueue;
	std::deque<CPickInfo> m_vNoMovePickQueue;

private:
	static const WORD TYPE_BLESS = 0x0E0D;
	static const WORD TYPE_SOUL  = 0x0E0E;
	static const WORD TYPE_CHAOS = 0x0C0F;
	static const WORD TYPE_LIFE  = 0x0E10;
	static const WORD TYPE_JOC   = 0x0E16;
	static const WORD TYPE_JOG   = 0x0E1F;
	static const WORD TYPE_ZEN   = 0x0E0F;

private:
	ULONG m_ulBlessFlags;
	ULONG m_ulSoulFlags;
	ULONG m_ulChaosFlags;
	ULONG m_ulLifeFlags;
	ULONG m_ulCreationFlags;
	ULONG m_ulGuardianFlags;

	ULONG m_ulExlFlags;
	ULONG m_ulZenFlags;
	ULONG m_ulCustomFlags;

	BOOL  m_fEnabled;
	BOOL  m_fDisplayCode;

	bool m_fSuspended;
	bool m_fSuspPick;
	bool m_fSuspZen;
	bool m_fSuspMove;
	int	 m_iDist;

	bool m_fPickAll;
private:
	HANDLE m_hThread;
	HANDLE m_hPickEvent;
	HANDLE m_hStopEvent;
	CRITICAL_SECTION m_csQueue;

	HANDLE m_hPacketEvent;
};


#endif // __AutoPickupFilter_H