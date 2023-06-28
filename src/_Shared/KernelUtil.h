#pragma once

#include "ApiTable.h"


/**  
 * \brief 
 */
class CKernelUtil
{
public:
	/**  
	 * \brief 
	 */
	static DWORD QueueUserAPC(PAPCFUNC pfnAPC, HANDLE hThread, ULONG_PTR dwData)
	{
		DWORD (__stdcall* proc)(PAPCFUNC, HANDLE, ULONG_PTR) = 
			(DWORD(__stdcall*)(PAPCFUNC, HANDLE, ULONG_PTR)) CApiTable::GetProc(_KERNEL_API_QueueUserAPC);

		return proc(pfnAPC, hThread, dwData);
	}


	/**  
	 * \brief 
	 */
	static BOOL GetVolumeInformationA(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, 
										LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
	{
		BOOL (__stdcall* proc)(LPCSTR,LPSTR,DWORD,LPDWORD,LPDWORD,LPDWORD,LPSTR,DWORD) = 
			(BOOL(__stdcall*)(LPCSTR,LPSTR,DWORD,LPDWORD,LPDWORD,LPDWORD,LPSTR,DWORD)) CApiTable::GetProc(_KERNEL_API_GetVolumeInformationA);

		return proc(lpRootPathName, lpVolumeNameBuffer, nVolumeNameSize, lpVolumeSerialNumber, lpMaximumComponentLength, 
						lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
	}


	/**  
	 * \brief 
	 */
	static __forceinline BOOL WINAPI IsDebuggerPresent(void)
	{
		BOOL(__stdcall* proc)(void) = 
			(BOOL(__stdcall*)(void)) CApiTable::GetProc(_KERNEL_API_IsDebuggerPresent);

		return proc();
	}


	/**  
	 * \brief 
	 */
	static HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName)
	{
		HANDLE(__stdcall* proc)(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR) = 
			(HANDLE(__stdcall*)(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR)) CApiTable::GetProc(_KERNEL_API_CreateMutexA);

		return proc(lpMutexAttributes, bInitialOwner, lpName);
	}


	/**  
	 * \brief 
	 */
	static DWORD WINAPI GetLastError(void)
	{
		DWORD(__stdcall* proc)(void) = 
			(DWORD(__stdcall*)(void)) CApiTable::GetProc(_KERNEL_API_GetLastError);

		return proc();
	}


	/**  
	 * \brief 
	 */
	static BOOL WaitForDebugEvent(LPDEBUG_EVENT lpDebugEvent, DWORD dwMilliseconds)
	{
		BOOL(__stdcall* proc)(LPDEBUG_EVENT, DWORD) = 
			(BOOL(__stdcall*)(LPDEBUG_EVENT, DWORD)) CApiTable::GetProc(_KERNEL_API_WaitForDebugEvent);

		return proc(lpDebugEvent, dwMilliseconds);
	}

	
	/**  
	 * \brief 
	 */
	static BOOL DebugActiveProcess(DWORD dwProcessId)
	{
		BOOL(__stdcall* proc)(DWORD) = 
			(BOOL(__stdcall*)(DWORD)) CApiTable::GetProc(_KERNEL_API_DebugActiveProcess);

		return proc(dwProcessId);
	}



	/**  
	 * \brief 
	 */
	static BOOL ContinueDebugEvent(DWORD dwProcessId, DWORD dwThreadId, DWORD dwContinueStatus)
	{
		BOOL(__stdcall* proc)(DWORD,DWORD,DWORD) = 
			(BOOL(__stdcall*)(DWORD,DWORD,DWORD)) CApiTable::GetProc(_KERNEL_API_ContinueDebugEvent);

		return proc(dwProcessId, dwThreadId, dwContinueStatus);
	}


	/**  
	 * \brief 
	 */
	static BOOL DebugSetProcessKillOnExit(BOOL fKill)
	{
		BOOL(__stdcall* proc)(BOOL) = 
			(BOOL(__stdcall*)(BOOL)) CApiTable::GetProc(_KERNEL_API_DebugSetProcessKillOnExit);

		return proc(fKill);
	}

	
	/**  
	 * \brief 
	 */
	static BOOL GetThreadContext(HANDLE hThread, LPCONTEXT lpContext)
	{
		BOOL(__stdcall* proc)(HANDLE,LPCONTEXT) = 
			(BOOL(__stdcall*)(HANDLE,LPCONTEXT)) CApiTable::GetProc(_KERNEL_API_GetThreadContext);

		return proc(hThread, lpContext);
	}

		
	/**  
	 * \brief 
	 */
	static BOOL SetThreadContext(HANDLE hThread, const CONTEXT* lpContext)
	{
		BOOL(__stdcall* proc)(HANDLE, const CONTEXT*) = 
			(BOOL(__stdcall*)(HANDLE, const CONTEXT*)) CApiTable::GetProc(_KERNEL_API_SetThreadContext);

		return proc(hThread, lpContext);
	}


	/**  
	 * \brief 
	 */
	static DWORD SuspendThread(HANDLE hThread)
	{
		DWORD(__stdcall* proc)(HANDLE) = 
			(DWORD(__stdcall*)(HANDLE)) CApiTable::GetProc(_KERNEL_API_SuspendThread);

		return proc(hThread);
	}


	/**  
	 * \brief 
	 */
	static DWORD ResumeThread(HANDLE hThread)
	{
		DWORD(__stdcall* proc)(HANDLE) = 
			(DWORD(__stdcall*)(HANDLE)) CApiTable::GetProc(_KERNEL_API_ResumeThread);

		return proc(hThread);
	}
				

	/**  
	 * \brief 
	 */
	static BOOL ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead)
	{
		BOOL(__stdcall* proc)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*) = 
			(BOOL(__stdcall*)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T*)) CApiTable::GetProc(_KERNEL_API_ReadProcessMemory);

		return proc(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
	}
		

	/**  
	 * \brief 
	 */
	static BOOL WriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten)
	{
		BOOL(__stdcall* proc)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*) = 
			(BOOL(__stdcall*)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*)) CApiTable::GetProc(_KERNEL_API_WriteProcessMemory);

		return proc(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
	}

	
	/**  
	 * \brief 
	 */
	static BOOL FlushInstructionCache(HANDLE hProcess, LPCVOID lpBaseAddress, SIZE_T dwSize)
	{
		BOOL(__stdcall* proc)(HANDLE, LPCVOID, SIZE_T) = 
			(BOOL(__stdcall*)(HANDLE, LPCVOID, SIZE_T)) CApiTable::GetProc(_KERNEL_API_FlushInstructionCache);

		return proc(hProcess, lpBaseAddress, dwSize);
	}


	/**  
	 * \brief 
	 */
	static BOOL DebugActiveProcessStop(DWORD dwProcessId)
	{
		BOOL(__stdcall* proc)(DWORD) = 
			(BOOL(__stdcall*)(DWORD)) CApiTable::GetProc(_KERNEL_API_DebugActiveProcessStop);

		return proc(dwProcessId);
	}


	/**  
	 * \brief 
	 */
	static BOOL CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes,
								BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, 
								LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
	{
		BOOL(__stdcall* proc)(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION) = 
			(BOOL(__stdcall*)(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION)) 
			CApiTable::GetProc(_KERNEL_API_CreateProcessA);

		return proc(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
						bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, 
						lpStartupInfo, lpProcessInformation);
	}


	/**  
	 * \brief 
	 */
	static BOOL CloseHandle(HANDLE hObject)
	{
		BOOL(__stdcall* proc)(HANDLE) = 
			(BOOL(__stdcall*)(HANDLE)) CApiTable::GetProc(_KERNEL_API_CloseHandle);

		return proc(hObject);
	}

	/**  
	 * \brief 
	 */
	static __forceinline BOOL CheckRemoteDebuggerPresent(HANDLE hProcess, PBOOL pbDebuggerPresent)
	{
		BOOL(__stdcall* proc)(HANDLE,PBOOL) = 
			(BOOL(__stdcall*)(HANDLE,PBOOL)) CApiTable::GetProc(_KERNEL_API_CheckRemoteDebuggerPresent);

		return proc(hProcess, pbDebuggerPresent);
	}


	/**  
	 * \brief 
	 */
	static __forceinline DWORD SleepEx(DWORD dwTime, BOOL fAlt)
	{
		DWORD(__stdcall* proc)(DWORD,BOOL) = 
			(DWORD(__stdcall*)(DWORD,BOOL)) CApiTable::GetProc(_KERNEL_API_SleepEx);

		return proc(dwTime, fAlt);
	}
};