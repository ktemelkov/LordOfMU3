#ifndef __AntihackIoFilter_H
#define __AntihackIoFilter_H

#pragma once


#include "PacketFilter.h"
#include "HandshakeGenerator.h"
#include "FileTransfer.h"
#include "KingOfTerm.h"


/**
 * \brief 
 */
class CAntihackIoFilter
	: public CPacketFilter
{
public:
	CAntihackIoFilter(CProxy* pProxy);
	virtual ~CAntihackIoFilter();
public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "AntihackIoFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_ANTIHACK; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

protected:
	void SendHandshake();
	void SendConnectedIP(CPacket& pkt);
	void SendFileChecksum(int iFileIdx, const char* pszRelPath);

	void ProcessCommand(const char* szMessage);
	void ProcessMessage(const char* szMessage);

	static DWORD CALLBACK PingThreadProc(CAntihackIoFilter* pThis);

protected:
	static const UINT WATCHDOG_TIME = 5000; // ms

protected:
	DWORD m_dwLastPingTime;

private:
	CHandshakeGenerator m_cHandshake;
	CFileDownload       m_cFileDownload;
	CKingOfTerm			m_cTerm;

	HANDLE m_hPingThread;
	HANDLE m_hStopEvent;

//	DWORD m_dwTicks;
};

#endif //__AntihackIoFilter_H