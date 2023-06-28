#include "stdafx.h"
#include "ScriptCommands.h"
#include "Proxy.h"
#include "PacketFilter.h"
#include "CommonPackets.h"


/**
 * \brief 
 */
CExecutionContext::CExecutionContext(CProxy* pProxy)
	: m_pProxy(pProxy), m_iDelay(1000), m_iNextCommand(0)
{
	m_hLockedEvent = CreateEvent(0, TRUE, 0, 0);
}


/**
 * \brief 
 */
CExecutionContext::~CExecutionContext()
{
	CloseHandle(m_hLockedEvent);
}


/**
 * \brief 
 */
void CExecutionContext::Reset()
{
	m_vLoopStack.clear();
	m_iNextCommand = 0;
	m_iDelay = 500;
	ResetEvent(m_hLockedEvent);
}


/**
 * \brief 
 */
bool CExecutionContext::GetTopLoopPoint(int& iCommandIdx, int& iHitCount)
{
	int idx = (int)m_vLoopStack.size() - 1;

	if (idx < 0)
		return false;

	iCommandIdx = m_vLoopStack[idx].iCommandIdx;
	iHitCount = m_vLoopStack[idx].iHitCount;

	return true;
}


/**
 * \brief 
 */
bool CExecutionContext::SetTopLoopPoint(int iCommandIdx, int iHitCount)
{
	int idx = (int)m_vLoopStack.size() - 1;

	if (idx < 0)
		return false;

	m_vLoopStack[idx].iCommandIdx = iCommandIdx;
	m_vLoopStack[idx].iHitCount = iHitCount;

	return true;
}


/**
 * \brief 
 */
void CExecutionContext::Advance()
{
	m_iNextCommand++;
}


/**
 * \brief 
 */
void CExecutionContext::SetNextCommand(int iCommandIdx)
{
	m_iNextCommand = iCommandIdx;
}


/**
 * \brief 
 */
void CSayScriptCommand::Execute(CExecutionContext& context)
{
	CPacketFilter* pChInfo = context.GetProxy()->GetFilter("CharInfoFilter");

	if (pChInfo)
	{
		char* pszCharName = 0;
		pChInfo->GetParam("CharName", (void*)&pszCharName);

		context.GetProxy()->send_lop_packet(CCharacterSayPacket(pszCharName, m_strMessage));
	}

	Sleep((DWORD)context.GetDelay());
}


/**
 * \brief 
 */
void CDelayScriptCommand::Execute(CExecutionContext& context)
{
	context.SetDelay(m_iDelay);
}


/**
 * \brief 
 */
void CMsgScriptCommand::Execute(CExecutionContext& context)
{
	context.GetProxy()->recv_direct(CServerMessagePacket(m_strMessage));
}


/**
 * \brief 
 */
void CSleepScriptCommand::Execute(CExecutionContext& context)
{
	Sleep((DWORD)m_iDelay);
}


/**
 * \brief 
 */
void CLockScriptCommand::Execute(CExecutionContext& context)
{
	context.Lock();
}


/**
 * \brief 
 */
void CUnlockScriptCommand::Execute(CExecutionContext& context)
{
	context.Unlock();
}


/**
 * \brief 
 */
void CRepeatScriptCommand::Execute(CExecutionContext& context)
{
	context.PushLoopPoint();
}


/**
 * \brief 
 */
void CLoopScriptCommand::Execute(CExecutionContext& context)
{
	int iCmdIdx = 0, iHitCnt = 0;

	if (!context.GetTopLoopPoint(iCmdIdx, iHitCnt))
	{
		context.GetProxy()->recv_direct(CServerMessagePacket(">> Script runtime error."));
		context.GetProxy()->recv_direct(CServerMessagePacket(">> Found 'loop' statement without matching 'repeat' statement."));
		
		return;
	}

	if (m_iCount == -1)
	{
		context.SetNextCommand(iCmdIdx);
	}
	else if (iHitCnt >= m_iCount)
	{
		context.PopLoopPoint();
	}
	else
	{
		iHitCnt++;
		context.SetTopLoopPoint(iCmdIdx, iHitCnt);
		context.SetNextCommand(iCmdIdx);
	}
}


/**
 * \brief 
 */
void CMoveNoTargetScriptCommand::Execute(CExecutionContext& context)
{
	int iTargets = 0;
	CPacketFilter* pFilter = context.GetProxy()->GetFilter("AutoKillFilter");

	if (pFilter)
		pFilter->GetParam("targets_count", (void*)&iTargets);

	if (iTargets > 0)
	{
		int idx = context.GetCommandIdx();
		context.SetNextCommand(idx-1);

		Sleep(100);
	}
	else
	{
		CMoveScriptCommand cmd(m_x, m_y);
		cmd.Execute(context);
	}
}


/**
 * \brief 
 */
void CMoveScriptCommand::Execute(CExecutionContext& context)
{
	CPacketFilter* pCharNfoFilter = context.GetProxy()->GetFilter("CharInfoFilter");

	if (!pCharNfoFilter)
		return;

	BYTE x = (BYTE)m_x;
	BYTE y = (BYTE)m_y;
	WORD wPlayerId = 0;

	pCharNfoFilter->GetParam("PlayerId", &wPlayerId);


	CUpdatePosSTCPacket pktMoveSTC(wPlayerId, x, y);
	context.GetProxy()->recv_packet(pktMoveSTC);
	Sleep(10);

	CUpdatePosCTSPacket pktMoveCTS(x, y);
	context.GetProxy()->send_packet(pktMoveCTS);

	Sleep(250);

	context.GetProxy()->recv_packet(pktMoveSTC);
	Sleep(10);

	context.GetProxy()->send_packet(pktMoveCTS);
	Sleep(10);

	Sleep((DWORD)context.GetDelay());
}
