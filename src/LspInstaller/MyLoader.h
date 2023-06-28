#pragma once

#include "ProcessUtil.h"


ULONG __stdcall MyLdrLoadDll(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
ULONG __stdcall MyLdrUnloadDll(HANDLE);
ULONG __stdcall MyLdrGetProcedureAddress(HMODULE hModule, PANSI_STRING pProcName, WORD wOrd, PVOID* pvProc);

bool InitLoader(HMODULE hInst, const char* pszRootDir);

