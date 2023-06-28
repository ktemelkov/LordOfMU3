#ifndef __ScriptProcessorFilter_H
#define __ScriptProcessorFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"
#include <vector>
#include <map>
#include "ScriptCommands.h"


/**
 * \brief 
 */
class CScriptProcessorFilter
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

public:
	CScriptProcessorFilter(CProxy* pProxy);
	virtual ~CScriptProcessorFilter();


public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "ScriptProcessorFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);


protected:
	void Start();
	void Stop();
	bool IsEnabled() { return WaitForSingleObject(m_hEnabledEvent, 0) != WAIT_TIMEOUT; }

	bool Load(const char* pszFile);
	void Reset();
	void ExecuteNext();

private:
	static DWORD ThreadProc(CScriptProcessorFilter* pThis);


protected:
	WORD m_wPlayerId;
	bool m_fRun;

	std::vector<CScriptCommand*> m_vCommands;
	CExecutionContext m_cExecContext;

	HANDLE m_hThread;
	HANDLE m_hStopEvent;
	HANDLE m_hEnabledEvent;
	HANDLE m_hSuspendedEvent;
	CRITICAL_SECTION m_cs;
};


#endif //__ScriptProcessorFilter_H