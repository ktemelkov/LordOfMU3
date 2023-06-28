#ifndef __DebugOut_H
#define __DebugOut_H

#pragma once

#include <conio.h>

extern TCHAR g_szRoot[_MAX_PATH + 1];

#define _DEBUGOUT_LOGFILENAME		_T("MUEliteClicker")
#define _DEBUGOUT_LOG_EXT			_T(".log")


#ifdef DEBUG
	#define __DEBUG_OUT
	#define __DEBUG_LOG
	#define __LOG_LEVEL LOG_LEVEL_INFO
//	#define __LOG_LEVEL LOG_LEVEL_SILENT
	#define __MAX_LOG_SIZE 2000000
	#define __CONNECT_LOG
#else
//	#define __DEBUG_OUT
	#define __DEBUG_LOG
	#define __LOG_LEVEL LOG_LEVEL_ERROR
//	#define __LOG_LEVEL LOG_LEVEL_INFO
	#define __MAX_LOG_SIZE 2000000
#endif 




/**
 *
 */
class CDebugOut
{
public:
	static bool Init(int iInstanceNo = 1)
	{
		logLevel() = __LOG_LEVEL;
		instanceNo() = iInstanceNo;

#ifdef __DEBUG_OUT
		AllocConsole();
#endif

#ifdef __DEBUG_LOG
		s_hFile() = 0;

		CreateLogFile();
		RotateLog();
#endif

		return true;
	}

	static void CleanUp()
	{
#ifdef __DEBUG_LOG
		if (s_hFile())
			CloseHandle(s_hFile());

		s_hFile() = 0;
#endif
	}

	static void SetLogLevel(int iLevel)
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		Lock();
		logLevel() = iLevel;
		UnLock();
#endif
	}

	static const int LOG_LEVEL_SILENT = -1;
	static const int LOG_LEVEL_ERROR = 0;
	static const int LOG_LEVEL_WARN = 1;
	static const int LOG_LEVEL_INFO = 2;
	static const int LOG_LEVEL_DEBUG = 3;

	static int& logLevel(){ static int s_iLogLevel = 0; return s_iLogLevel; };
	static int& instanceNo(){ static int s_iInstance = 0; return s_iInstance; };

	static void PrintLastError(char* lpszFunction) 
	{ 
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_ERROR)
			return;

		Lock();

		char szBuf[80] = {0}; 
		LPSTR lpMsgBuf = 0;
		DWORD dw = GetLastError(); 

		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&lpMsgBuf, 0, NULL);
#endif

#if defined(__DEBUG_OUT)
		_cprintf("%s failed with error %d: %s\n", lpszFunction, dw, lpMsgBuf); 
#endif

#if defined(__DEBUG_LOG)
		if (s_hFile())
		{
			PrintTimeStamp();

			char szMessage[512] = {0};
			_snprintf(szMessage, 511, "%s failed with error %d: %s\n", lpszFunction, dw, lpMsgBuf);

			DWORD dwWritten = 0;
			WriteFile(s_hFile(), szMessage, (DWORD)strlen(szMessage), &dwWritten, 0);
		}
#endif

#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		LocalFree(lpMsgBuf);
		UnLock();
#endif
	}

	static void PrintLastError(char* lpszFunction, DWORD dw) 
	{ 
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_ERROR)
			return;

		Lock();

		char szBuf[80] = {0}; 
		LPSTR lpMsgBuf = 0;

		FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&lpMsgBuf, 0, NULL);
#endif

#if defined(__DEBUG_OUT)
		_cprintf("%s failed with error %d: %s\n", lpszFunction, dw, lpMsgBuf); 
#endif

#if defined(__DEBUG_LOG)
		if (s_hFile())
		{
			PrintTimeStamp();

			char szMessage[512] = {0};
			_snprintf(szMessage, 511, "%s failed with error %d: %s\n", lpszFunction, dw, lpMsgBuf);

			DWORD dwWritten = 0;
			WriteFile(s_hFile(), szMessage, (DWORD)strlen(szMessage), &dwWritten, 0);
		}
#endif

#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		LocalFree(lpMsgBuf);

		UnLock();
#endif
	}

	static void PrintDebug(char* format, ...)
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_DEBUG)
			return;

		va_list args;
		va_start(args, format);

		PrintV(format, args);
#endif
	}

	static void PrintWarn(char* format, ...)
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_WARN)
			return;

		va_list args;
		va_start(args, format);

		PrintV(format, args);
#endif
	}

	static void PrintInfo(char* format, ...)
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_INFO)
			return;

		va_list args;
		va_start(args, format);

		PrintV(format, args);
#endif
	}

	static void PrintError(char* format, ...) 
	{ 
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		if (logLevel() < LOG_LEVEL_ERROR)
			return;

		va_list args;
		va_start(args, format);

		PrintV(format, args);
#endif
	}


	static void PrintAlways(const char* format, ...) 
	{ 
		va_list args;
		va_start(args, format);

		PrintV(format, args);
	}

protected:
	static void PrintV(const char* format, va_list args)
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		Lock();
		CStringA strMessage = GetTimeStamp();

		CStringA str;
		str.FormatV(format, args);

		CStringA strRepl;
		strRepl.Format("\n%s", strMessage);

		str.Replace("\r", "");
		str.Replace("\n", strRepl);
		str += "\n";

		strMessage += str;
#endif

#if defined(__DEBUG_OUT)
		_cprintf(strMessage); 
#endif

#if defined(__DEBUG_LOG)
		if (s_hFile())
		{
			DWORD dwWritten = 0;
			WriteFile(s_hFile(), (const char*)strMessage, strMessage.GetLength(), &dwWritten, 0);

			if (s_iCheckCount()++ >= 1000)
			{
				RotateLog();
				s_iCheckCount() = 0;
			}
		}
#endif

#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		UnLock();
#endif
	}


#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)

	/**  
	 * \brief 
	 */
	static CStringA GetTimeStamp()
	{
		CStringA strTS;
		SYSTEMTIME st = {0};
		GetLocalTime(&st);

		strTS.Format("[%02d/%02d/%04d %02d:%02d:%02d:%04d] ", st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		return strTS;
	}

#endif

	/**  
	 * \brief 
	 */
	static void PrintTimeStamp()
	{
#if defined(__DEBUG_OUT) || defined(__DEBUG_LOG)
		CStringA strTS = GetTimeStamp();
#endif

#if defined(__DEBUG_OUT)
		_cprintf(strTS);
#endif

#if defined(__DEBUG_LOG)
		if (s_hFile())
		{
			DWORD dwWritten = 0;
			WriteFile(s_hFile(), (const char*)strTS, strTS.GetLength(), &dwWritten, 0);
		}
#endif
	}

protected:
	static CRITICAL_SECTION* GetCS()
	{
		static CRITICAL_SECTION cs = {0};
		static bool fInit = false;

		if (!fInit)
		{
			InitializeCriticalSection(&cs);
			fInit = true;
		}

		return &cs;
	}

	static void Lock()
	{
		EnterCriticalSection(GetCS());
	}

	static void UnLock()
	{
		LeaveCriticalSection(GetCS());
	}

#ifdef __DEBUG_LOG
private:
	static HANDLE& s_hFile(){ static HANDLE sHFile = 0; return sHFile; };
	static int& s_iCheckCount(){ static int iCounter = 0; return iCounter; };

	static void RotateLog()
	{
		DWORD dwSize = GetFileSize(s_hFile(), 0);

		if (dwSize >= __MAX_LOG_SIZE)
		{
			CloseHandle(s_hFile());

			TCHAR szPath[_MAX_PATH+1] = {0};
			TCHAR szPath_old[_MAX_PATH+1] = {0};
			_sntprintf(szPath, _MAX_PATH, _T("%sLog\\") _DEBUGOUT_LOGFILENAME _T("%d") _DEBUGOUT_LOG_EXT, g_szRoot, instanceNo());
			_sntprintf(szPath_old, _MAX_PATH, _T("%sLog\\") _DEBUGOUT_LOGFILENAME _T("%d_old") _DEBUGOUT_LOG_EXT, g_szRoot, instanceNo());

			DeleteFile(szPath_old);
			MoveFile(szPath, szPath_old);

			CreateLogFile();
		}
	}

	static void CreateLogFile()
	{
		TCHAR szPath[_MAX_PATH+1] = {0};
		_tcscpy(szPath, g_szRoot);
		_tcscat(szPath, _T("Log"));

		CreateDirectory(szPath, 0);
		_sntprintf(szPath, _MAX_PATH, _T("%sLog\\") _DEBUGOUT_LOGFILENAME _T("%d") _DEBUGOUT_LOG_EXT, g_szRoot, instanceNo());

		s_hFile() = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if (INVALID_HANDLE_VALUE == s_hFile())
			s_hFile() = 0;
		else
		{
			SetFilePointer(s_hFile(), 0, 0, FILE_END);

			PrintAlways("    ----------  Logging started ----------------- ");
		}
	}
#endif
};



#endif //__DebugOut_H