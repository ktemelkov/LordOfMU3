#include "stdafx.h"
#include "Debugger.h"

#include "../_Shared/KernelUtil.h"
#include "../_Shared/MuUtil.h"
#include "Loader.h"


volatile HandleDebugEventPtr CDebugger::m_sHandler = CDebugger::HandleDebugEvent;



/**  
 * \brief 
 */
CDebugger::CDebugger() 
	: m_pProcessInfo(0), m_fDebug(false) 
{
	m_dwProcId = 0;
	m_dwStartAddress = 0;
	m_cOriginalInstruction = 0;

	m_baseOffset = 0;
}


/**  
 * \brief 
 */
int CDebugger::Run()
{
	if (CKernelUtil::IsDebuggerPresent())
		return 0;


	m_dwProcId = GetCurrentProcessId();

	STARTUPINFOA si = {0};
	si.cb = sizeof(STARTUPINFOA);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
	
	PROCESS_INFORMATION pi = {0};

	if (!CKernelUtil::CreateProcessA(CPathUtil::GetModuleFileNameA().c_str(), 0, 0, 0, 0, CREATE_SUSPENDED | DEBUG_ONLY_THIS_PROCESS, 0, CPathUtil::GetModuleDir().c_str(), &si, &pi))
	{
		return 0;
	}

	ALLOC_CONSOLE;

	m_pProcessInfo = &pi;
	return ProcessDebugEvents();
}


/**  
 * \brief 
 */
int CDebugger::ProcessDebugEvents()
{
	if (!m_pProcessInfo)
		return -1;


	PROCESS_INFORMATION& pi = *m_pProcessInfo;

	m_fDebug = true;
	DEBUG_EVENT DebugEv = {0};

	CKernelUtil::DebugSetProcessKillOnExit(FALSE);
	CKernelUtil::ResumeThread(pi.hThread);

	while (m_fDebug) 
	{ 
		CKernelUtil::WaitForDebugEvent(&DebugEv, INFINITE);
		CKernelUtil::ContinueDebugEvent(DebugEv.dwProcessId, DebugEv.dwThreadId, m_sHandler(this, DebugEv));
	}

	ON_EXIT_HANDLER;
	return 0;
}


/**  
 * \brief 
 */
DWORD CDebugger::HandleDebugEvent(CDebugger* pThis, DEBUG_EVENT& DebugEv)
{
	PROCESS_INFORMATION& pi = *pThis->m_pProcessInfo;

	switch (DebugEv.dwDebugEventCode) 
	{ 
	case EXCEPTION_DEBUG_EVENT: 
		{
			switch (DebugEv.u.Exception.ExceptionRecord.ExceptionCode) 
			{ 
			case EXCEPTION_ACCESS_VIOLATION: 
				{
					if (DebugEv.u.Exception.dwFirstChance == 0)
						return DBG_EXCEPTION_NOT_HANDLED;
					else
						pThis->m_fDebug = false;	
				}
				break;

			case EXCEPTION_BREAKPOINT: 
				{
					CONTEXT lcContext = {0};
					lcContext.ContextFlags = CONTEXT_ALL;
					CKernelUtil::GetThreadContext(pi.hThread, &lcContext);

					if ((DWORD)(lcContext.Eip-1) == (DWORD)pThis->m_dwStartAddress)
					{
						_STROUT("Child process stopped at start address. pThis->m_dwProcId = %d\r\n", pThis->m_dwProcId);

						lcContext.Eip--;
						CKernelUtil::SetThreadContext(pi.hThread, &lcContext);

						DWORD dwWriteSize = 0;
						CKernelUtil::WriteProcessMemory(pi.hProcess, (LPVOID)pThis->m_dwStartAddress, (LPCVOID)&pThis->m_cOriginalInstruction, 1, &dwWriteSize);
						CKernelUtil::FlushInstructionCache(pi.hProcess, (LPVOID)pThis->m_dwStartAddress, 1);


						DWORD dwAddr = (DWORD)CLoader::FakeLoad + pThis->m_baseOffset;
						DWORD dwBytes = 0;
						CKernelUtil::ReadProcessMemory(pi.hProcess, (void*)dwAddr, &pThis->m_cOriginalInstruction, 1, &dwBytes);

						BYTE cInstruction = 0xCC;
						CKernelUtil::WriteProcessMemory(pi.hProcess, (void*)dwAddr, &cInstruction, 1, &dwBytes);
						CKernelUtil::FlushInstructionCache(pi.hProcess,(void*)dwAddr, 1);
					}
					else if ((DWORD)(lcContext.Eip-1) == (DWORD)CLoader::FakeLoad + pThis->m_baseOffset)
					{
						_STROUT("Break at: CLoader::FakeLoad() called.\r\n");

						lcContext.Eip = (DWORD)CLoader::Load + pThis->m_baseOffset;
//						lcContext.Ecx = pThis->m_dwProcId;

						extern DWORD gdwDebuggerPid;
						DWORD dwWriteSize = 0;
						CKernelUtil::WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD)&gdwDebuggerPid + pThis->m_baseOffset), (LPCVOID)&pThis->m_dwProcId, 4, &dwWriteSize);

						char szFileName[MAX_PATH+1] = {0};
						GetModuleFileNameA(GetModuleHandle(0), szFileName, MAX_PATH);

						char* pszExeName = szFileName + lstrlenA(szFileName);

						while (pszExeName > (szFileName + 1) && *(pszExeName-1) != '\\') pszExeName--;

						extern char gszDebuggerName[MAX_PATH+1];
						CKernelUtil::WriteProcessMemory(pi.hProcess, (LPVOID)((DWORD)gszDebuggerName + pThis->m_baseOffset), (LPCVOID)pszExeName, lstrlenA(pszExeName)+1, &dwWriteSize);


						CKernelUtil::SetThreadContext(pi.hThread, &lcContext);

						m_sHandler = HandleDebugEvent2;
					}
				}
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT: 
				if (DebugEv.u.Exception.dwFirstChance == 0)
					return DBG_EXCEPTION_HANDLED;
				break;

			case EXCEPTION_SINGLE_STEP: 
				if (DebugEv.u.Exception.dwFirstChance == 0)
					return DBG_EXCEPTION_HANDLED;
				break;

			case DBG_CONTROL_C: 
				if (DebugEv.u.Exception.dwFirstChance == 0)
					return DBG_EXCEPTION_HANDLED;

				break;

			default:
				if (DebugEv.u.Exception.dwFirstChance == 0)
					return DBG_EXCEPTION_HANDLED;

				break;
			} 
		}
		break;

	case CREATE_PROCESS_DEBUG_EVENT: 
		{
			_STROUT("Debug event: 0x%08X (Create Process)\r\n", DebugEv.dwDebugEventCode);

			pThis->m_dwStartAddress = (LPVOID)DebugEv.u.CreateProcessInfo.lpStartAddress;

			DWORD dwBytes = 0;
			CKernelUtil::ReadProcessMemory(pi.hProcess, (void*)pThis->m_dwStartAddress, &pThis->m_cOriginalInstruction, 1, &dwBytes);

			BYTE cInstruction = 0xCC;
			CKernelUtil::WriteProcessMemory(pi.hProcess, (void*)pThis->m_dwStartAddress, &cInstruction, 1, &dwBytes);
			CKernelUtil::FlushInstructionCache(pi.hProcess,(void*)pThis->m_dwStartAddress, 1);

			pThis->m_baseOffset = (DWORD)DebugEv.u.CreateProcessInfo.lpBaseOfImage - (DWORD)ghInstance;

			CKernelUtil::CloseHandle(DebugEv.u.CreateProcessInfo.hFile);
		}
		break;

	case CREATE_THREAD_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Create Thread)\r\n", DebugEv.dwDebugEventCode);
		break;

	case EXIT_THREAD_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Exit Thread)\r\n", DebugEv.dwDebugEventCode);
		break;

	case EXIT_PROCESS_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Exit Process)\r\n", DebugEv.dwDebugEventCode);
		pThis->m_fDebug = false;
		break;

	case LOAD_DLL_DEBUG_EVENT: 
		{
			_STROUT("Debug event: 0x%08X (Load DLL)\r\n", DebugEv.dwDebugEventCode);

			char szFilename[_MAX_PATH+1] = {0};
			GetFileNameFromHandle(DebugEv.u.LoadDll.hFile, szFilename, _MAX_PATH);

			_STROUT("    %s DLL loaded.\r\n", szFilename);

			CKernelUtil::CloseHandle(DebugEv.u.LoadDll.hFile);
		}
		break;

	case UNLOAD_DLL_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Unload DLL)\r\n", DebugEv.dwDebugEventCode);
		break;

	case OUTPUT_DEBUG_STRING_EVENT: 
		{
			if (!DebugEv.u.DebugString.fUnicode)
			{
				char* pszMessage = new char[DebugEv.u.DebugString.nDebugStringLength];
				CKernelUtil::ReadProcessMemory(pi.hProcess, DebugEv.u.DebugString.lpDebugStringData, pszMessage, DebugEv.u.DebugString.nDebugStringLength, 0);

				_STROUT(pszMessage);
				delete[] pszMessage;
			}			
		}
		break;
	} 

	return DBG_CONTINUE;
}



/**  
 * \brief 
 */
DWORD CDebugger::HandleDebugEvent2(CDebugger* pThis, DEBUG_EVENT& DebugEv)
{
	PROCESS_INFORMATION& pi = *pThis->m_pProcessInfo;
	LPVOID dwStartAddress = 0;
	BYTE cOriginalInstruction = 0;

	switch (DebugEv.dwDebugEventCode) 
	{ 
	case EXCEPTION_DEBUG_EVENT: 
		{
			switch (DebugEv.u.Exception.ExceptionRecord.ExceptionCode) 
			{ 
			case EXCEPTION_ACCESS_VIOLATION: 
				{
					if (DebugEv.u.Exception.dwFirstChance == 0)
						return DBG_EXCEPTION_NOT_HANDLED;
					else
						pThis->m_fDebug = false;	
				}
				break;

			case EXCEPTION_BREAKPOINT: 
				{
					pThis->m_fDebug = false;
				}
				break;

			case EXCEPTION_DATATYPE_MISALIGNMENT: 
				{
					_STROUT("Data type misalignment.\r\n");

					if (DebugEv.u.Exception.dwFirstChance == 0)
						return DBG_EXCEPTION_HANDLED;
				}
				break;

			case EXCEPTION_SINGLE_STEP: 
				break;

			case DBG_CONTROL_C: 
				_STROUT("Control+C.\r\n");

				if (DebugEv.u.Exception.dwFirstChance == 0)
					return DBG_EXCEPTION_HANDLED;

				break;

			default:
				_STROUT("Unknown exception: 0x%08X, first-chance: %X\r\n", DebugEv.u.Exception.ExceptionRecord.ExceptionCode, DebugEv.u.Exception.dwFirstChance);
				return DBG_EXCEPTION_NOT_HANDLED;
			} 
		}
		break;

	case CREATE_THREAD_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Create Thread)\r\n", DebugEv.dwDebugEventCode);
		break;

	case CREATE_PROCESS_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Create Process)\r\n", DebugEv.dwDebugEventCode);

		CKernelUtil::CloseHandle(DebugEv.u.CreateProcessInfo.hFile);
		break;

	case EXIT_THREAD_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Exit Thread)\r\n", DebugEv.dwDebugEventCode);
		break;

	case EXIT_PROCESS_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Exit Process)\r\n", DebugEv.dwDebugEventCode);

		pThis->m_fDebug = false;
		break;

	case LOAD_DLL_DEBUG_EVENT: 
		{
			_STROUT("Debug event: 0x%08X (Load DLL)\r\n", DebugEv.dwDebugEventCode);

			char szFilename[_MAX_PATH+1] = {0};
			GetFileNameFromHandle(DebugEv.u.LoadDll.hFile, szFilename, _MAX_PATH);

			_STROUT("    %s DLL loaded.\r\n", szFilename);

			CKernelUtil::CloseHandle(DebugEv.u.LoadDll.hFile);
		}
		break;

	case UNLOAD_DLL_DEBUG_EVENT: 
		_STROUT("Debug event: 0x%08X (Unload DLL)\r\n", DebugEv.dwDebugEventCode);
		break;

	case OUTPUT_DEBUG_STRING_EVENT: 
		if (!DebugEv.u.DebugString.fUnicode)
		{
			char* pszMessage = new char[DebugEv.u.DebugString.nDebugStringLength];
			ReadProcessMemory(pi.hProcess, DebugEv.u.DebugString.lpDebugStringData, pszMessage, DebugEv.u.DebugString.nDebugStringLength, 0);

			_STROUT(pszMessage);

			delete[] pszMessage;
		}			
		break;

	} 

	return DBG_CONTINUE;
}

