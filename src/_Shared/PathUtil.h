#pragma once

#include "StdString.h"
#include "ApiTable.h"
#include <stdlib.h>




/**  
 * \brief 
 */
class CPathUtil
{
public:
	/**  
	 * \brief 
	 */
	static CStdString ConstructModulePath(const char* pszFileName, HMODULE hMod = 0)
	{
		char szModuleFile[_MAX_PATH+1] = {0};

		if (hMod == 0)
			hMod = GetModuleHandleA(0);

		CPathUtil::GetModuleFileNameA(hMod, szModuleFile, _MAX_PATH);

		for (int i=(int)lstrlenA(szModuleFile)-1; i >= 0 && szModuleFile[i] != '\\'; szModuleFile[i--] = 0);
		lstrcatA(szModuleFile, pszFileName);

		return CStdString(szModuleFile);
	}


	/**  
	 * \brief 
	 */
	static CStdString ConstructModulePath(const char* pszModuleFilePath, const char* pszFileName)
	{
		char szModuleFile[_MAX_PATH+1] = {0};
		lstrcpyA(szModuleFile, pszModuleFilePath);

		for (int i=(int)lstrlenA(szModuleFile)-1; i >= 0 && szModuleFile[i] != '\\'; szModuleFile[i--] = 0);
		lstrcatA(szModuleFile, pszFileName);

		return CStdString(szModuleFile);
	}


	/**  
	 * \brief 
	 */
	static CStdString GetModuleDir(HMODULE hMod = 0)
	{
		char szModuleFile[_MAX_PATH+1] = {0};

		if (hMod == 0)
			hMod = GetModuleHandleA(0);

		CPathUtil::GetModuleFileNameA(hMod, szModuleFile, _MAX_PATH);

		for (int i=(int)lstrlenA(szModuleFile)-1; i >= 0 && szModuleFile[i] != '\\'; szModuleFile[i--] = 0);

		return CStdString(szModuleFile);
	}


	/**  
	 * \brief 
	 */
	static CStdString ExpandPath(const char* pszPath)
	{
		char szRes[_MAX_PATH+1] = {0};
		CPathUtil::ExpandEnvironmentStringsA(pszPath, szRes, _MAX_PATH);

		return CStdString(szRes);
	}


	/**  
	 * \brief 
	 */
	static BOOL PathFileExistsA(LPCSTR pszPath)
	{
		BOOL (__stdcall* proc)(LPCSTR) =
			(BOOL (__stdcall*)(LPCSTR)) CApiTable::GetProc(_SHLW_API_PathFileExistsA);

		return proc(pszPath);
	}

	/**  
	 * \brief 
	 */
	static BOOL CopyFileA(LPCSTR src, LPCSTR dst, BOOL fFailIfExists = FALSE)
	{
		BOOL (__stdcall* proc)(LPCSTR,LPCSTR,BOOL) =
			(BOOL (__stdcall*)(LPCSTR,LPCSTR,BOOL)) CApiTable::GetProc(_KERNEL_API_CopyFileA);

		return proc(src, dst, fFailIfExists);
	}


	/**  
	 * \brief 
	 */
	static DWORD GetModuleFileNameA(HMODULE hMod, LPCH lpszPath, DWORD dwSize)
	{
		DWORD (__stdcall* proc)(HMODULE, LPCH, DWORD) =
			(DWORD (__stdcall*)(HMODULE, LPCH, DWORD)) CApiTable::GetProc(_KERNEL_API_GetModuleFileNameA);

		return proc(hMod, lpszPath, dwSize);
	}


	/**  
	 * \brief 
	 */
	static CStdString GetModuleFileNameA(HMODULE hMod = 0)
	{
		if (hMod == 0)
			hMod = GetModuleHandleA(0);

		char szBuff[_MAX_PATH+1] = {0};
		CPathUtil::GetModuleFileNameA(hMod, szBuff, _MAX_PATH);

		return CStdString(szBuff);
	}


	/**  
	 * \brief 
	 */
	static LPCH PathFindFileNameA(LPCH lpszPath)
	{
		LPCH (__stdcall* proc)(LPCH) =
			(LPCH(__stdcall*)(LPCH)) CApiTable::GetProc(_SHLW_API_PathFindFileNameA);

		return proc(lpszPath);
	}
	
	/**  
	 * \brief 
	 */
	static LPCH PathAddBackslashA(LPCH lpszPath)
	{
		LPCH (__stdcall* proc)(LPCH) =
			(LPCH(__stdcall*)(LPCH)) CApiTable::GetProc(_SHLW_API_PathAddBackslashA);

		return proc(lpszPath);
	}


	/**  
	 * \brief 
	 */
	static DWORD ExpandEnvironmentStringsA(LPCSTR lpSrc, LPCSTR lpDst, DWORD nSize)
	{
		DWORD (__stdcall* proc)(LPCSTR,LPCSTR,DWORD) =
			(DWORD(__stdcall*)(LPCSTR,LPCSTR,DWORD)) CApiTable::GetProc(_KERNEL_API_ExpandEnvironmentStringsA);

		return proc(lpSrc, lpDst, nSize);
	}
};

