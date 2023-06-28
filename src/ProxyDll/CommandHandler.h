#ifndef __CommandHandler_H
#define __CommandHandler_H

#pragma once

class CProxy;

/**
 * \brief 
 */
class CCommandHandler
{
public:
	CCommandHandler(CProxy* pProxy);
	virtual ~CCommandHandler(){}

public:
	// Command Handler Interface
	virtual bool ProcessCommand(const char* cmd, const char* args) = 0;
	virtual const char* PrintUsage() = 0;

protected:
	CProxy* GetProxy() { return m_pProxy; }

private:
	CProxy* m_pProxy;
};


#endif //__CommandHandler_H