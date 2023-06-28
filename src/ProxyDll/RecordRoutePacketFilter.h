#ifndef __RecordRoutePacketFilter_H
#define __RecordRoutePacketFilter_H

#pragma once

#include "Proxy.h"
#include "PacketFilter.h"
#include <vector>


/**  
 * \brief 
 */
class CRecordRoutePacketFilter
	: public CPacketFilter
{
public:
	CRecordRoutePacketFilter(CProxy* pProxy);
	virtual ~CRecordRoutePacketFilter(){}

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "RecordRouteFilter"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_CLICKER; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

	// Parameter interface
	virtual bool GetParam(const char* pszParam, void* pData);
	virtual bool SetParam(const char* pszParam, void* pData);

protected:
	bool SaveToFile(const char* pszFilename);

	void ReplayRoute();
	void RunRoute();

private:
	std::vector<WORD> m_vRoute;
	bool m_fEnabled;
	WORD m_wPlayerId;
};


#endif //__RecordRoutePacketFilter_H