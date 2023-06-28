#ifndef __ScriptCommands_H
#define __ScriptCommands_H

#pragma once

#include <vector>

class CProxy;

/**
 * \brief 
 */
class CExecutionContext
{
public:
	struct LoopPoint
	{
		LoopPoint(int idx, int hc) : iCommandIdx(idx), iHitCount(hc) {}
		LoopPoint(const LoopPoint& in) : iCommandIdx(in.iCommandIdx), iHitCount(in.iHitCount) {}

		int iCommandIdx;
		int iHitCount;
	};

public:
	CExecutionContext(CProxy* pProxy);
	virtual ~CExecutionContext();

	void Reset();
	bool IsLocked() const { return WaitForSingleObject(m_hLockedEvent, 0) != WAIT_TIMEOUT; }

	void Lock(){ SetEvent(m_hLockedEvent); }
	void Unlock() { ResetEvent(m_hLockedEvent); }

	void PushLoopPoint(){ m_vLoopStack.push_back(LoopPoint(m_iNextCommand, 0)); }
	void PopLoopPoint() { m_vLoopStack.pop_back(); }

	bool GetTopLoopPoint(int& iCommandIdx, int& iHitCount);
	bool SetTopLoopPoint(int iCommandIdx, int iHitCount);

	int GetCommandIdx() const { return m_iNextCommand; }
	void Advance();
	void SetNextCommand(int iCommandIdx);

	int GetDelay() const { return m_iDelay; }
	void SetDelay(int iDelay) { m_iDelay = iDelay; }

	CProxy* GetProxy() { return m_pProxy; }

protected:
	std::vector<LoopPoint> m_vLoopStack;
	int m_iNextCommand;
	int m_iDelay;
	HANDLE m_hLockedEvent;

private:
	CProxy* m_pProxy;
};


/**
 * \brief 
 */
class CScriptCommand
{
public:
	CScriptCommand(){}
	virtual ~CScriptCommand(){}

	virtual void Execute(CExecutionContext& context){}
};


/**
 * \brief 
 */
class CSayScriptCommand 
	: public CScriptCommand
{
public:
	CSayScriptCommand(const char* pszMessage) : m_strMessage(pszMessage) {}
	virtual ~CSayScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	CStringA m_strMessage;
};


/**
 * \brief 
 */
class CDelayScriptCommand
	: public CScriptCommand
{
public:
	CDelayScriptCommand(int iDelay) : m_iDelay(iDelay) {}
	virtual ~CDelayScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	int m_iDelay;
};


/**
 * \brief 
 */
class CMsgScriptCommand 
	: public CScriptCommand
{
public:
	CMsgScriptCommand(const char* pszMessage) : m_strMessage(pszMessage) {}
	virtual ~CMsgScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	CStringA m_strMessage;
};

/**
 * \brief 
 */
class CSleepScriptCommand
	: public CScriptCommand
{
public:
	CSleepScriptCommand(int iDelay) : m_iDelay(iDelay) {}
	virtual ~CSleepScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	int m_iDelay;
};


/**
 * \brief 
 */
class CLockScriptCommand
	: public CScriptCommand
{
public:
	CLockScriptCommand(){}
	virtual ~CLockScriptCommand(){}

	virtual void Execute(CExecutionContext& context);
};



/**
 * \brief 
 */
class CUnlockScriptCommand
	: public CScriptCommand
{
public:
	CUnlockScriptCommand(){}
	virtual ~CUnlockScriptCommand(){}

	virtual void Execute(CExecutionContext& context);
};


/**
 * \brief 
 */
class CRepeatScriptCommand
	: public CScriptCommand
{
public:
	CRepeatScriptCommand(){}
	virtual ~CRepeatScriptCommand(){}

	virtual void Execute(CExecutionContext& context);
};


/**
 * \brief 
 */
class CLoopScriptCommand
	: public CScriptCommand
{
public:
	CLoopScriptCommand(int iCount) : m_iCount(iCount) {}
	virtual ~CLoopScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	int m_iCount;
};


/**
 * \brief 
 */
class CMoveNoTargetScriptCommand 
	: public CScriptCommand
{
public:
	CMoveNoTargetScriptCommand(int x, int y) : m_x(x), m_y(y) {}
	virtual ~CMoveNoTargetScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	int m_x;
	int m_y;
};


/**
 * \brief 
 */
class CMoveScriptCommand 
	: public CScriptCommand
{
public:
	CMoveScriptCommand(int x, int y) : m_x(x), m_y(y) {}
	virtual ~CMoveScriptCommand(){}

	virtual void Execute(CExecutionContext& context);

protected:
	int m_x;
	int m_y;
};

#endif //__ScriptCommands_H