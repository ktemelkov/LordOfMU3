#pragma once

#include "ApiTable.h"
#include <Wininet.h>

#define _DEFAULT_DOWNLOAD_BUFFER_SIZE 500000


/**  
 * \brief 
 */
class CInternetUtil
{
public:

	/**  
	 * \brief 
	 */
	static HINTERNET InternetOpenUrlA(HINTERNET hInternet, LPCSTR lpszUrl, LPCSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext)
	{
		HINTERNET (__stdcall* proc)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR) = 
			(HINTERNET(__stdcall*)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR)) CApiTable::GetProc(_INETAPI_InternetOpenUrlA);

		return proc(hInternet, lpszUrl, lpszHeaders, dwHeadersLength, dwFlags, dwContext);
	}


	/**  
	 * \brief 
	 */
	static BOOL InternetReadFile(HINTERNET hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead)
	{
		BOOL (__stdcall* proc)(HINTERNET, LPVOID, DWORD, LPDWORD) = 
			(BOOL(__stdcall*)(HINTERNET, LPVOID, DWORD, LPDWORD)) CApiTable::GetProc(_INETAPI_InternetReadFile);

		return proc(hFile, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
	}


	/**  
	 * \brief 
	 */
	static HINTERNET InternetOpenA(LPCSTR lpszAgent, DWORD dwAccessType, LPCSTR lpszProxyName, LPCSTR lpszProxyBypass, DWORD dwFlags)
	{
		HINTERNET (__stdcall* proc)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD) = 
			(HINTERNET(__stdcall*)(LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD)) CApiTable::GetProc(_INETAPI_InternetOpenA);

		return proc(lpszAgent, dwAccessType, lpszProxyName, lpszProxyBypass, dwFlags);
	}


	/**  
	 * \brief 
	 */
	static BOOL InternetCloseHandle(HINTERNET hInternet)
	{
		BOOL (__stdcall* proc)(HINTERNET) = 
			(BOOL(__stdcall*)(HINTERNET)) CApiTable::GetProc(_INETAPI_InternetCloseHandle);

		return proc(hInternet);
	}


	/**  
	 * \brief 
	 */
	static BOOL InternetGetLastResponseInfoA(LPDWORD lpdwError, LPSTR lpszBuffer, LPDWORD lpdwBufferLength)
	{
		BOOL (__stdcall* proc)(LPDWORD,LPSTR,LPDWORD) = 
			(BOOL(__stdcall*)(LPDWORD,LPSTR,LPDWORD)) CApiTable::GetProc(_INETAPI_InternetGetLastResponseInfoA);

		return proc(lpdwError, lpszBuffer, lpdwBufferLength);
	}

	/**  
	 * \brief 
	 */
	static BOOL InternetCanonicalizeUrlA(LPCSTR lpszUrl, LPSTR lpszBuffer, LPDWORD lpdwBufferLength, DWORD dwFlags)
	{
		BOOL (__stdcall* proc)(LPCSTR,LPSTR,LPDWORD,DWORD) = 
			(BOOL(__stdcall*)(LPCSTR,LPSTR,LPDWORD,DWORD)) CApiTable::GetProc(_INETAPI_InternetCanonicalizeUrlA);

		return proc(lpszUrl, lpszBuffer, lpdwBufferLength, dwFlags);
	}
};