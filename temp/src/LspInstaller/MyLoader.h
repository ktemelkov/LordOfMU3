#pragma once

#include "ProcessUtil.h"


ULONG __stdcall MyLdrInitializeThunk(PVOID,PVOID,PVOID);
ULONG __stdcall MyLdrLoadDll(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, PHANDLE ModuleHandle);
ULONG __stdcall MyLdrGetProcedureAddress(HMODULE, PANSI_STRING, WORD, PVOID*);


bool InitLoader(HMODULE hInst, const char* pszRootDir);

