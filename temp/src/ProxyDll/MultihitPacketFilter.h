#ifndef __MultihitPacketFilter_H
#define __MultihitPacketFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"
#include <vector>


/**
 * \brief 
 */
class CMultihitPacketFilter
	: public CPacketFilter
{
	struct CAutoLockQueue
	{
		CAutoLockQueue(CRITICAL_SECTION* pCS){ EnterCriticalSection(m_pCS = pCS); }
		virtual ~CAutoLockQueue(){ LeaveCriticalSection(m_pCS); }

		CRITICAL_SECTION* m_pCS;
	};

public:
	CMultihitPacketFilter(CProxy* pProxy);
	virtual ~CMultihitPacketFilter();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "MultihitFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

private:
	static DWORD HitThreadProc(CMultihitPacketFilter* pThis);
	void DoNextHit();

private:
	std::vector<CPacket> m_vHitQueue;

private:
	ULONG m_ulHitCount;
	DWORD m_dwHitDelay;

	bool m_fSuspended;

private:
	HANDLE m_hThread;
	HANDLE m_hHitEvent;
	HANDLE m_hStopEvent;

	CRITICAL_SECTION m_csQueue;
};


#endif //__MultihitPacketFilter_H