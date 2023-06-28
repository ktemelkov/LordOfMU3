// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>



#if 0
	#include <stdio.h>

	#define _DBGPRINT(param) OutputDebugStringA(param)
	#define _DBGPRINT2(format, param1) { char szMsg[512] = {0}; sprintf(szMsg, format, param1); OutputDebugStringA(szMsg); }
#else
	#define _DBGPRINT(param)
	#define _DBGPRINT2(format, param1)
#endif

