#pragma once

#include <winsock2.h>
#include "StringTable.h"
#include "ApiTable.h"


/**  
 * \brief 
 */
class CWinsockUtil
{
public:

	/**  
	 * \brief 
	 */
	static int WSCEnumProtocols(LPINT lpiProtocols, LPWSAPROTOCOL_INFOW lpProtocolBuffer, LPDWORD lpdwBufferLength, LPINT lpErrno)
	{
		int (__stdcall* proc)(LPINT,LPWSAPROTOCOL_INFOW,LPDWORD,LPINT) = 
			(int(__stdcall*)(LPINT,LPWSAPROTOCOL_INFOW,LPDWORD,LPINT)) CApiTable::GetProc(_WINSOCK_API_WSCEnumProtocols);

		return proc(lpiProtocols, lpProtocolBuffer, lpdwBufferLength, lpErrno);
	}


	/**  
	 * \brief 
	 */
	static int WSCInstallProvider(const LPGUID lpProviderId, const LPWSTR lpszProviderDllPath, const LPWSAPROTOCOL_INFOW lpProtocolInfoList, DWORD dwNumberOfEntries, LPINT lpErrno)
	{
		int (__stdcall* proc)(const LPGUID, const LPWSTR, const LPWSAPROTOCOL_INFOW, DWORD, LPINT) =
			(int(__stdcall*)(const LPGUID, const LPWSTR, const LPWSAPROTOCOL_INFOW, DWORD, LPINT)) CApiTable::GetProc(_WINSOCK_API_WSCInstallProvider);

		return proc(lpProviderId, lpszProviderDllPath, lpProtocolInfoList, dwNumberOfEntries, lpErrno);
	}


	/**  
	 * \brief 
	 */
	static int WSCWriteProviderOrder(LPDWORD lpwdCatalogEntryId, DWORD dwNumberOfEntries)
	{
		int (__stdcall* proc)(LPDWORD,DWORD) =
			(int (__stdcall*)(LPDWORD,DWORD)) CApiTable::GetProc(_WINSOCK_API_WSCWriteProviderOrder);

		return proc(lpwdCatalogEntryId, dwNumberOfEntries);
	}


	/**  
	 * \brief 
	 */
	static int WSCDeinstallProvider(LPGUID lpProviderId, LPINT lpErrno)
	{
		int (__stdcall* proc)(LPGUID, LPINT) =
			(int (__stdcall*)(LPGUID, LPINT)) CApiTable::GetProc(_WINSOCK_API_WSCDeinstallProvider);

		return proc(lpProviderId, lpErrno);
	}


	/**  
	 * \brief 
	 */
	static int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
	{
		int (__stdcall* proc)(WORD, LPWSADATA) = 
			(int(__stdcall*)(WORD, LPWSADATA)) CApiTable::GetProc(_WINSOCK_API_WSAStartup);

		return proc(wVersionRequested, lpWSAData);
	}


	/**  
	 * \brief 
	 */
	static int WSACleanup()
	{
		int(__stdcall* proc)(void) = 
			(int(__stdcall*)(void)) CApiTable::GetProc(_WINSOCK_API_WSACleanup);

		return proc();
	}


	/**  
	 * \brief 
	 */
	static int WPUCompleteOverlappedRequest(SOCKET s, LPWSAOVERLAPPED lpOverlapped, DWORD dwError, DWORD cbTransferred, LPINT lpErrno)
	{
		int(__stdcall* proc)(SOCKET,LPWSAOVERLAPPED,DWORD,DWORD,LPINT) = 
			(int(__stdcall*)(SOCKET,LPWSAOVERLAPPED,DWORD,DWORD,LPINT)) CApiTable::GetProc(_WINSOCK_API_WPUCompleteOverlappedRequest);

		return proc(s, lpOverlapped, dwError, cbTransferred, lpErrno);
	}


	/**  
	 * \brief 
	 */
	static int WSCGetProviderPath(LPGUID lpProviderId, LPWSTR lpszProviderDllPath, LPINT lpProviderDllPathLen, LPINT lpErrno)
	{
		int(__stdcall* proc)(LPGUID,LPWSTR,LPINT,LPINT) = 
			(int(__stdcall*)(LPGUID,LPWSTR,LPINT,LPINT)) CApiTable::GetProc(_WINSOCK_API_WSCGetProviderPath);

		return proc(lpProviderId, lpszProviderDllPath, lpProviderDllPathLen, lpErrno);
	}


	/**  
	 * \brief 
	 */
	static void WSASetLastError(int iError)
	{
		void(__stdcall* proc)(int) = 
			(void(__stdcall*)(int)) CApiTable::GetProc(_WINSOCK_API_WSASetLastError);

		return proc(iError);
	}


	/**  
	 * \brief 
	 */
	static int WSAGetLastError(void)
	{
		int(__stdcall* proc)(void) = 
			(int(__stdcall*)(void)) CApiTable::GetProc(_WINSOCK_API_WSAGetLastError);

		return proc();
	}

};