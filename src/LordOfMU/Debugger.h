#pragma once

class CDebugger;

typedef DWORD (*HandleDebugEventPtr)(CDebugger*, DEBUG_EVENT&);


/**  
 * \brief 
 */
class CDebugger
{
public:
	CDebugger();

	int Run();
	int ProcessDebugEvents();

	static DWORD HandleDebugEvent(CDebugger* pThis, DEBUG_EVENT& DebugEv);
	static DWORD HandleDebugEvent2(CDebugger* pThis, DEBUG_EVENT& DebugEv);


private:
	LPPROCESS_INFORMATION m_pProcessInfo;
	bool m_fDebug;
	DWORD m_dwProcId;
	LPVOID m_dwStartAddress;
	BYTE m_cOriginalInstruction;

	long m_baseOffset;

public:
	static volatile HandleDebugEventPtr m_sHandler;
};