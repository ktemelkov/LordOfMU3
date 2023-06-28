// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once



#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// Global Variables
extern HINSTANCE ghInstance;


bool GetFileNameFromHandle(HANDLE hFile, char* pszOut, int iSize);



#include <conio.h>

#if 0
	#define _DBGPRINT(param) OutputDebugStringA(param)
	#define _DBGPRINT2(format, param1) { char szMsg[512] = {0}; _snprintf_s(szMsg, 511, format, param1); OutputDebugStringA(szMsg); }
	#define _STROUT	_cprintf

	#define IS_INSIDE_DEBUGGER CKernelUtil::IsDebuggerPresent()

	#define SIMM_EXT_DEBUGGER 0
	#define ALLOC_CONSOLE AllocConsole()
	#define ON_EXIT_HANDLER { _cprintf("Press any key to exit ..."); _getch(); }
#else
	#define _DBGPRINT(param)
	#define _DBGPRINT2(format, param1)
	#define _STROUT

	#define IS_INSIDE_DEBUGGER CKernelUtil::IsDebuggerPresent()
	#define SIMM_EXT_DEBUGGER 0

	#define ALLOC_CONSOLE
	#define ON_EXIT_HANDLER
#endif


#define DEBUG_HANDLER_SYNC_EVENT "__na_baba_ti_farchiloto_256"
