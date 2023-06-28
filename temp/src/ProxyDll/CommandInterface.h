#ifndef __CommandInterface_H
#define __CommandInterface_H

#pragma once

#include "PacketFilter.h"
#include "Proxy.h"
#include "CommandHandler.h"
#include <map>


/**
 * \brief 
 */
typedef std::map<CStringA, CCommandHandler*> CCommandList;


/**
 * \brief 
 */
class CCommandInterface
	: public CPacketFilter
{
public:
	CCommandInterface(CProxy* pProxy);
	virtual ~CCommandInterface();

public:
	// Filter Interface Methods
	virtual const char* GetName(){ return "CommandInterface"; }
	virtual WORD GetLevel(){ return FILTER_LEVEL_COMMAND; }
	virtual int FilterRecvPacket(CPacket& pkt, CFilterContext& context);
	virtual int FilterSendPacket(CPacket& pkt, CFilterContext& context);

public:
	// Command Interface
	virtual bool AddCommand(const char* szCommand, CCommandHandler* pHandler);
	virtual void ClearCommands();

private:
	void ProcessCommand(const char* szCommand);

protected:
	CCommandList m_cCommands;
};

#endif //__CommandInterface_H
