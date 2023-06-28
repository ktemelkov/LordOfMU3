#include "stdafx.h"
#include "ApiHooker.h"
#include "Disasm.h"



/**
 * \brief 
 */
CApiHooker::CApiHooker()
{
	m_pfnApi = 0;
	m_dwPatchSize = 0;
	memset(m_abTrampBuff, 0, sizeof(m_abTrampBuff));
}


/**
 * \brief 
 */
bool CApiHooker::PatchFunction(const char* szDll, const char* szFunc, DWORD_PTR dwHookProc)
{
	HMODULE hMod = GetModuleHandleA(szDll);

	if (!hMod)
		return false;

	BYTE* pFunc = (BYTE*)GetProcAddress(hMod, szFunc);

	if (!pFunc)
		return false;

	return PatchFunction(pFunc, dwHookProc);
}


/**
 * \brief 
 */
bool CApiHooker::UnPatchFunction(const char* szDll, const char* szFunc)
{
	HMODULE hMod = GetModuleHandleA(szDll);

	if (!hMod)
		return false;

	BYTE* pFunc = (BYTE*)GetProcAddress(hMod, szFunc);

	if (!pFunc)
		return false;

	return UnPatchFunction(pFunc);
}


/**
 * \brief 
 */
BYTE* CApiHooker::FindPatchPoint(BYTE* pFuncAddress, DWORD_PTR dwHookProc)
{
	BYTE* pFunc = pFuncAddress;

	while (pFunc[0] == 0xE9 || pFunc[0] == 0x90)
	{
		BYTE* pFuncRet = pFunc;
		
		while (*pFunc == 0x90) 
			pFunc++;

		if (pFunc[0] != 0xE9)
			return pFuncRet;

		DWORD_PTR dwRelAddr = 0;
		memcpy(&dwRelAddr, pFunc+1, 4);

		DWORD_PTR dwCurrHook = dwRelAddr + PtrToUlong(pFunc) + 5;

		if (dwCurrHook == dwHookProc)
			return 0;

		pFunc = (BYTE*)dwCurrHook;
	}

	return pFunc;
}


/**
 * \brief 
 */
BYTE* CApiHooker::FindPatchedAddress(BYTE* pFuncAddress, DWORD_PTR dwHookProc)
{
	BYTE* pFunc = pFuncAddress;

	while (pFunc[0] == 0xE9 || pFunc[0] == 0x90)
	{
		while (*pFunc == 0x90) 
			pFunc++;

		if (pFunc[0] != 0xE9)
			return 0;

		DWORD_PTR dwRelAddr = 0;
		memcpy(&dwRelAddr, pFunc+1, 4);

		DWORD_PTR dwCurrHook = dwRelAddr + PtrToUlong(pFunc) + 5;

		if (dwCurrHook == dwHookProc)
			return pFunc;

		pFunc = (BYTE*)dwCurrHook;
	}

	return 0;
}


/**
 * \brief 
 */
bool CApiHooker::PatchFunction(BYTE* pFunc, DWORD_PTR dwHookProc)
{
	if (!dwHookProc)
		return false;

	// 8b ff 55 8b ec - mov edi,edi; push ebp; mov ebp,esp;
	// 8b c0 55 8b ec - mov eax,eax; push ebp; mov ebp,esp;
	// "\xE9\x00\x00\x00\x00"
	// "\xEb\xF9"
	
	if (pFunc[0] == 0x8b && pFunc[2] == 0x55 && pFunc[3] == 0x8b && pFunc[4] == 0xec)
	{
		DoHotPatch(pFunc, (BYTE*)dwHookProc);
		return true;
	}


	if (0 == (pFunc = FindPatchPoint(pFunc, dwHookProc)))
		return true;


	// 0x83 0xf9 0x08 - CMP ECX 8
//	BYTE fakeCode[] = { 0x83, 0xf9, 0x08 };

	// 0x50 - push eax
	// 0x58 - pop eax
	BYTE fakeCode[] = { 0x50, 0x58 };

	int nops = sizeof(fakeCode);

	ULONG ulSizeToProtect = 5+nops;
	DWORD dwIdx = 0;
	DISASSEMBLY disasm = {0};
	disasm.Address = PtrToUlong(pFunc);

	FlushDecoded(&disasm);

//	for (dwIdx = 0; dwIdx <= ulSizeToProtect; dwIdx++)
	for (; dwIdx <= ulSizeToProtect; )
	{
		dwIdx = 0;
		Decode(&disasm, (char*)UlongToPtr(disasm.Address), &dwIdx);
		disasm.Address += disasm.OpcodeSize + disasm.PrefixSize;

		dwIdx = disasm.Address - PtrToUlong(pFunc);
		FlushDecoded(&disasm);
	}

	m_dwPatchSize = dwIdx;

	DWORD dwAddr = PtrToUlong(dwHookProc) - PtrToUlong(pFunc) - 5 - nops;
	DWORD dwAddrBack = (PtrToUlong(pFunc) + dwIdx) - (PtrToUlong(m_abTrampBuff) + dwIdx) - 5;
	DWORD dwProtect = 0;

	memcpy(m_abTrampBuff, pFunc, dwIdx);
	m_abTrampBuff[dwIdx] = 0xE9;
	memcpy(m_abTrampBuff + dwIdx + 1, &dwAddrBack, 4);

	if (!::VirtualProtect((PVOID)pFunc, dwIdx, PAGE_EXECUTE_READWRITE, &dwProtect))
		return false;


	DWORD dwNewProtect = dwProtect;

	for (int i=0; i < nops; i++)
		pFunc[i] = fakeCode[i]; 

	pFunc[nops] = 0xE9; 
	memcpy(pFunc+nops+1, &dwAddr, 4);

	m_dwHookProc = dwHookProc;

	::VirtualProtect((PVOID)pFunc, 5+nops, dwNewProtect, &dwProtect);

	::VirtualProtect((PVOID)m_abTrampBuff, 64, PAGE_EXECUTE_READWRITE, &dwProtect);
	return true;
}


/**
 * \brief 
 */
bool CApiHooker::UnPatchFunction(BYTE* pFunc)
{
	// "\xE9\x00\x00\x00\x00"
	// "\xEb\xF9"

	if (pFunc[0] == 0xeb && pFunc[1] == 0xf9 && *(pFunc-5) == 0xe9)
	{
		UndoHotPatch(pFunc);
		return true;
	}


	DWORD dwProtect = 0;
	DWORD dwAddr = PtrToUlong(m_abTrampBuff) - PtrToUlong(pFunc) - 5;

	if (m_abTrampBuff[0] != 0 && m_abTrampBuff[0] != 0xE9 && pFunc[0] == 0xE9 && m_dwPatchSize != 0)
	{
		if (0 != (pFunc = FindPatchedAddress(pFunc, m_dwHookProc)))
		{
			if (!::VirtualProtect((PVOID)pFunc, m_dwPatchSize, PAGE_EXECUTE_READWRITE, &dwProtect))
				return false;

			DWORD dwNewProtect = dwProtect;
			memcpy(pFunc, m_abTrampBuff, m_dwPatchSize);

			::VirtualProtect((PVOID)pFunc, m_dwPatchSize, dwNewProtect, &dwProtect);
		}
	}

	// Set out trampoline jump directly to original function
	::VirtualProtect((PVOID)m_abTrampBuff, 64, PAGE_EXECUTE_READWRITE, &dwProtect);
	memset(m_abTrampBuff, 0, sizeof(m_abTrampBuff));

	m_abTrampBuff[0] = 0xE9;
	memcpy(m_abTrampBuff+1, &dwAddr, 4);

	m_dwHookProc = 0;
	m_dwPatchSize = 0;
	return true;
}


/**  
 * \brief 
 */
DWORD CApiHooker::GetJMP(DWORD dwSrcAddr, DWORD dwDestAddr)
{
	return (DWORD)dwDestAddr-dwSrcAddr-5;
}


/**  
 * \brief 
 */
VOID CApiHooker::DoHotPatch(BYTE* pFuncAddr, BYTE* pCallback)
{
	DWORD dwOldProtect = 0;
	BYTE JMP[] = "\xE9\x00\x00\x00\x00";
	*(PDWORD)(JMP+1) = GetJMP((DWORD)((PUCHAR)pFuncAddr-0x5), (DWORD)pCallback);


	VirtualProtect((PUCHAR)pFuncAddr - 0x5, 0x7, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pFuncAddr, "\xEb\xF9", 0x2);
	memcpy((PUCHAR)pFuncAddr-0x5, JMP, 0x5);

	VirtualProtect((PUCHAR)pFuncAddr - 0x5, 0x7, dwOldProtect, &dwOldProtect);


	DWORD dwAddrBack = GetJMP((DWORD)m_abTrampBuff, (DWORD)(pFuncAddr+2));
	
	m_abTrampBuff[0] = 0xE9;
	memcpy(m_abTrampBuff+1, &dwAddrBack, sizeof(dwAddrBack));

	::VirtualProtect((PVOID)m_abTrampBuff, 64, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	m_dwHookProc = (DWORD)pCallback;
}


/**  
 * \brief 
 */
VOID CApiHooker::UndoHotPatch(BYTE* pFuncAddr)
{
	DWORD dwOldProtect = 0;

	VirtualProtect((PUCHAR)pFuncAddr, 0x2, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	memcpy(pFuncAddr, "\x8b\xFF", 0x2);

	VirtualProtect((PUCHAR)pFuncAddr, 0x2, dwOldProtect, &dwOldProtect);
}
