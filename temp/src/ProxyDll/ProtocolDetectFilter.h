#ifndef __ProtocolDetectFilter_H
#define __ProtocolDetectFilter_H

#pragma once 

#include "Proxy.h"
#include "PacketFilter.h"


/**
 * \brief 
 */
class CProtocolDetectFilter
	: public CPacketFilter
{
public:
	CProtocolDetectFilter(CProxy* pProxy);
	virtual ~CProtocolDetectFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "ProtocolDetectFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_ENCDEC; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

private:
	void FinalizeVersionDetect();
	void ApplyVersion();
	bool DetectProtocolVersion(CPacket& pkt);
	void CheckTrial();
	void EnforceFeaturesOff();

private:
	static bool m_fVersionDetected;
	
	DWORD m_dwGameStartTS;
	DWORD m_dwTrialCheckTS;
	ULONG m_ulVersion;
	int m_iState;


	DWORD m_dwTrialTime;
	DWORD m_dwCheckTime;

};

#endif // __ProtocolDetectFilter_H