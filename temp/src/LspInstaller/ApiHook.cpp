#include "ApiHook.h"
#include "StringUtil.h"
#include "ProcessUtil.h"
#include "Disasm.h"

#pragma warning(push)
#pragma warning(disable: 4706)
#pragma warning(disable: 4312)
#pragma warning(disable: 4311)
#pragma warning(disable: 4244)


/**  
 * \brief 
 */
namespace ApiHook
{
#pragma pack(push)
#pragma pack(1)
	struct CPatchInfo
	{
		BYTE m_abTrampBuff[64];
		BYTE m_abOriginalInstr[64];
		DWORD m_dwPatchSize;
		DWORD m_dwProcAddr;
	};
#pragma pack(pop)

	#define InitVar(varType, varName) varType varName; StringUtil::MemSet(&varName, 0, sizeof(varName));
	#define InitBuf(elType, bufName, elCount) elType bufName[elCount]; StringUtil::MemSet(bufName, 0, sizeof(elType)*elCount);

	#define TestResultRet(expr, err_ret) if (!(expr)) return err_ret;
	#define TestResult(expr) TestResultRet(expr, false)

	#define InitNtMemApiRet(varName, retVal) \
		InitVar(ProcessUtil::TNtApiTable, varName) \
		TestResultRet(GetNtMemApi(varName), retVal)

	#define InitNtMemApi(varName) InitNtMemApiRet(varName, false)

	#define InitNtHeapApiRet(varName, retVal) \
		InitVar(ProcessUtil::TNtApiTable, varName) \
		TestResultRet(GetNtHeapApi(varName), retVal)

	#define InitNtHeapApi(varName) InitNtHeapApiRet(varName, false)


	static HANDLE ghAHPrivateHeap;
	static HANDLE& GetPrivateHeap(){ return ghAHPrivateHeap; }


	/**  
	 * \brief 
	 */
	bool GetNtMemApi(ProcessUtil::TNtApiTable& NtApi)
	{
		NtApi.NtProtectVirtualMemory = (ProcessUtil::NtProtectVirtualMemoryPtr)1;
		NtApi.NtWriteVirtualMemory = (ProcessUtil::NtWriteVirtualMemoryPtr)1;
		NtApi.NtReadVirtualMemory = (ProcessUtil::NtReadVirtualMemoryPtr)1;
		NtApi.NtAllocateVirtualMemory = (ProcessUtil::NtAllocateVirtualMemoryPtr)1;
		NtApi.NtFreeVirtualMemory = (ProcessUtil::NtFreeVirtualMemoryPtr)1;

		return ProcessUtil::GetNtApi(NtApi);
	}

	/**  
	 * \brief 
	 */
	bool GetNtHeapApi(ProcessUtil::TNtApiTable& NtApi)
	{
		NtApi.RtlCreateHeap = (ProcessUtil::RtlCreateHeapPtr)1;
		NtApi.RtlAllocateHeap = (ProcessUtil::RtlAllocateHeapPtr)1;
		NtApi.RtlReAllocateHeap = (ProcessUtil::RtlReAllocateHeapPtr)1;
		NtApi.RtlFreeHeap = (ProcessUtil::RtlFreeHeapPtr)1;
		NtApi.RtlDestroyHeap = (ProcessUtil::RtlDestroyHeapPtr)1;

		return ProcessUtil::GetNtApi(NtApi);
	}


	/**  
	 * \brief 
	 */
	DWORD GetJMP(DWORD dwSrcAddr, DWORD dwDestAddr)
	{
		return (DWORD)(dwDestAddr - dwSrcAddr - 5);
	}


	/**  
	 * \brief 
	 */
	bool Init()
	{
		InitNtHeapApiRet(NtApi, false);
		GetPrivateHeap() = NtApi.RtlCreateHeap(HEAP_GROWABLE | HEAP_CREATE_ENABLE_EXECUTE, 0, 0, 0, 0, 0);

		return true;
	}


	/**  
	 * \brief 
	 */
	DWORD ProtectMem(HANDLE hProcess, DWORD dwAddr, DWORD dwSize, DWORD dwProt)
	{
		InitNtMemApiRet(NtApi, 0);

		DWORD dwOldProt = 0;
		DWORD dwProtAddr = dwAddr;
		ULONG ulProtSize = dwSize;
		NtApi.NtProtectVirtualMemory(hProcess, (PVOID*)&dwProtAddr, &ulProtSize, dwProt, &dwOldProt);

		return dwOldProt;
	}

	/**  
	 * \brief 
	 */
	bool ReadProtectedMem(HANDLE hProcess, DWORD dwAddr, void* buff, DWORD dwSize)
	{
		InitNtMemApi(NtApi)

		DWORD dwOldProt = ProtectMem(hProcess, dwAddr, dwSize, PAGE_EXECUTE_READWRITE);

		if (!dwOldProt)
			return false;

		DWORD dwRead = 0;
		NtApi.NtReadVirtualMemory(hProcess,(LPVOID)dwAddr, buff, dwSize, &dwRead);

		ProtectMem(hProcess, dwAddr, dwSize, dwOldProt);

		return (dwRead == dwSize);
	}



	/**  
	 * \brief 
	 */
	bool WriteProtectedMem(HANDLE hProcess, DWORD dwAddr, void* buff, DWORD dwSize)
	{
		InitNtMemApi(NtApi)

		DWORD dwOldProt = ProtectMem(hProcess, dwAddr, dwSize, PAGE_EXECUTE_READWRITE);

		if (!dwOldProt)
			return false;

		DWORD dwWritten = 0;
		NtApi.NtWriteVirtualMemory(hProcess,(LPVOID)dwAddr, buff, dwSize, &dwWritten);

		ProtectMem(hProcess, dwAddr, dwSize, dwOldProt);

		return (dwWritten == dwSize);
	}


	/**  
	 * \brief 
	 */
	void* AllocateMem(HANDLE hProcess, DWORD dwSize, DWORD dwProtection)
	{
		InitNtMemApiRet(NtApi, 0)

		void* pRemMem = 0;
		ULONG ulAllocSize = dwSize;
		NtApi.NtAllocateVirtualMemory(hProcess, (PVOID*)&pRemMem, 0, &ulAllocSize, MEM_RESERVE | MEM_COMMIT, dwProtection);

		return pRemMem;
	}

	
	/**  
	 * \brief 
	 */
	void* HeapMemAllocate(DWORD dwSize)
	{
		InitNtHeapApiRet(NtApi, 0);
		return NtApi.RtlAllocateHeap(GetPrivateHeap(), HEAP_ZERO_MEMORY, dwSize);
	}


	/**  
	 * \brief 
	 */
	void* HeapMemReAllocate(void* ptr, DWORD dwSize)
	{
		InitNtHeapApiRet(NtApi, 0);
		return NtApi.RtlReAllocateHeap(GetPrivateHeap(), HEAP_ZERO_MEMORY, ptr, dwSize);
	}


	/**  
	 * \brief 
	 */
	void HeapMemFree(void* ptr)
	{
		InitVar(ProcessUtil::TNtApiTable, NtApi)
		
		if (GetNtHeapApi(NtApi))
			NtApi.RtlFreeHeap(GetPrivateHeap(), 0, ptr);
	}


	/**
	 * \brief
	 */
	DWORD_PTR FindPatchPoint(HANDLE hProcess, DWORD_PTR pFuncAddress, DWORD_PTR pCallback)
	{
		InitNtMemApiRet(NtApi, 0)
		InitBuf(BYTE, buff, 11)

		TestResultRet(!(pFuncAddress == pCallback), 0)
		TestResultRet(ReadProtectedMem(hProcess, pFuncAddress - 5, buff, sizeof(buff)), 0)

		TestResultRet(!(buff[5] == 0xC3), 0)
		TestResultRet(!(buff[5] == 0xE9 || buff[5] == 0xE8), FindPatchPoint(hProcess, *((DWORD*)(buff+6)) + pFuncAddress + 5, pCallback))
		TestResultRet(!(buff[0] == 0xE9 && buff[5] == 0xEb && buff[6] == 0xF9), FindPatchPoint(hProcess, *((DWORD*)(buff+1)) + pFuncAddress, pCallback))
		TestResultRet(!(buff[5] == 0x68 && buff[10] == 0xC3), FindPatchPoint(hProcess, *((DWORD*)(buff+6)), pCallback))

		if (buff[5] == 0xFF && (buff[6] == 0x25 || buff[6] == 0x15))
		{
			TestResultRet(ReadProtectedMem(hProcess, *((DWORD*)(buff+7)), buff, 4), 0)
			return FindPatchPoint(hProcess, *((DWORD*)buff), pCallback);
		}

		return pFuncAddress;
	}


	/**  
	 * \brief 
	 */
	bool IsHotPatchable(BYTE* buff)
	{
		return *((WORD*)(buff+5)) == 0xFF8B && 
					((buff[0] == 0x90 && *((DWORD*)(buff+1)) == 0x90909090) || (buff[0] == 0xE9));
	}


	/**  
	 * \brief 
	 */
	bool DisasmAndCopyTramp(BYTE* buff, DWORD_PTR pRemFuncAddr, CPatchInfo* pPatchInfo, CPatchInfo* pRemPatch, DWORD& dwIdx, ULONG ulSizeToProtect)
	{
		BYTE* trampBuff = pPatchInfo->m_abTrampBuff;
		BYTE* trampBuffO = pPatchInfo->m_abOriginalInstr;		
		BYTE* pRemoteBuf = pRemPatch->m_abTrampBuff;


		InitVar(DISASSEMBLY, disasm)
		disasm.Address = (DWORD)buff;

		pPatchInfo->m_dwPatchSize = 0;

		for (dwIdx = 0; dwIdx < ulSizeToProtect; )
		{
			DWORD dwTmp = 0;

			FlushDecoded(&disasm);
			Decode(&disasm, (char*)(DWORD)disasm.Address, &dwTmp);

			DWORD dwInstrSize = disasm.OpcodeSize + disasm.PrefixSize;
			StringUtil::MemCopy(trampBuff + dwIdx, (void*)disasm.Address, dwInstrSize);
			StringUtil::MemCopy(trampBuffO + pPatchInfo->m_dwPatchSize, (void*)disasm.Address, dwInstrSize);

			if (trampBuff[dwIdx] == 0xE9 || trampBuff[dwIdx] == 0xE8) // fix far relative jumps/calls
			{				
				DWORD jmp = *((DWORD*)(disasm.Address + 1));
				DWORD dwDstAddr = jmp + disasm.Address + 5;

				dwDstAddr = (DWORD)pRemFuncAddr + (dwDstAddr - (DWORD)buff);
				DWORD new_jmp = dwDstAddr - ((DWORD)pRemoteBuf + dwIdx) - 5; //new relative jump address

				*((DWORD*)(trampBuff + dwIdx + 1)) = new_jmp; 
			}
			else if (trampBuff[dwIdx] == 0xEb) // fix short jumps
			{	
				BYTE jmp = *((BYTE*)(disasm.Address + 1));
				DWORD dwDstAddr = (char)jmp + disasm.Address + 2;
				
				dwDstAddr = (DWORD)pRemFuncAddr + (dwDstAddr - (DWORD)buff);
				DWORD new_jmp = dwDstAddr - ((DWORD)pRemoteBuf + dwIdx) - 5;

				trampBuff[dwIdx] = 0xE9;
				*((DWORD*)(trampBuff + dwIdx + 1)) = new_jmp;

				dwIdx += 3;
			}

			disasm.Address += dwInstrSize;
			dwIdx += dwInstrSize;

			pPatchInfo->m_dwPatchSize += dwInstrSize;
		}

		return true;
	}


	/**  
	 * \brief 
	 *	
	 *	// FF 15 xx xx xx xx - call absolute address
	 *	// FF 25 xx xx xx xx - jump to absolute address
	 *	// E9 xx xx xx xx - jump to relative address
	 *	// E8 xx xx xx xx - call relative address
	 *	// EB xx - jump relative
	 *	// 
	 *	// FF D6 - call esi
	 *	// 8B 35 xx xx xx xx - mov esi, memory address
	 *
	 */
	bool RemoteHotPatch(HANDLE hProcess, DWORD_PTR pFuncAddr, DWORD_PTR pCallback, DWORD_PTR pOrigProcAddr)
	{
		InitBuf(BYTE, buff, 24)		
		InitNtMemApi(NtApi)

		TestResult(pFuncAddr = FindPatchPoint(hProcess, pFuncAddr, pCallback))
		TestResult(ReadProtectedMem(hProcess, pFuncAddr - 5, buff, 24))


		InitVar(CPatchInfo, info)
		InitVar(CPatchInfo*, pRemPatch)
		TestResult(pRemPatch = (CPatchInfo*)AllocateMem(hProcess, sizeof(CPatchInfo)))

		DWORD dwIdx = 0;
		DisasmAndCopyTramp(buff + 5, pFuncAddr, &info, pRemPatch, dwIdx, 6);


		DWORD Alignment = (dwIdx + 0x5) & ~0x3; // Four byte alignment of the preamble plus two to account for the jmp indirect


		// Append the indirect jump back to the original function
		info.m_abTrampBuff[dwIdx] = 0xff;
		info.m_abTrampBuff[dwIdx + 1] = 0x25;
		*((PVOID*)(info.m_abTrampBuff + dwIdx + 2)) = (PVOID)(pRemPatch->m_abTrampBuff + Alignment + 4);

		// Next comes the pointer to the original function.
		*((PVOID*)(info.m_abTrampBuff + Alignment + 4)) = (PVOID)(pFuncAddr + dwIdx);


		// And last, the pointer to our hook routine for use in our indirect jump that we overwrite the preamble with.
		*((PVOID*)(info.m_abTrampBuff + Alignment + 8)) = (PVOID)pCallback;

		info.m_dwProcAddr = pFuncAddr;

		// Now, let's overwrite the function.
		buff[5] = 0xff;
		buff[6] = 0x25;
		*((PVOID*)(buff + 7)) = (PVOID)(pRemPatch->m_abTrampBuff + Alignment + 8);


		for (int i=dwIdx; i >= 6; i--)
			buff[5 + i] = 0x90;

		return WriteProtectedMem(hProcess, (DWORD)pRemPatch, &info, sizeof(info)) 
			&& WriteProtectedMem(hProcess, pFuncAddr, buff+5, dwIdx)
			&& WriteProtectedMem(hProcess, pOrigProcAddr, &pRemPatch, 4);
	}


	/**  
	 * \brief 
	 */
	bool RemoteUnPatch(HANDLE hProcess, DWORD_PTR pOrigProcAddr)
	{
		if (!pOrigProcAddr)
			return 0;

		InitNtMemApi(NtApi)
		InitVar(CPatchInfo, info)

		TestResult(ReadProtectedMem(hProcess, pOrigProcAddr, &info, sizeof(info)));
		TestResult(WriteProtectedMem(hProcess, info.m_dwProcAddr, info.m_abOriginalInstr, info.m_dwPatchSize))

		return true;
	}



	/**  
	 * \brief 
	 */
	bool PatchRetn(DWORD_PTR pFuncAddr, DWORD_PTR pCallback)
	{
		InitVar(DISASSEMBLY, disasm)
		disasm.Address = (DWORD)pFuncAddr;

		DWORD dwRetAddr = 0;
		DWORD dwRetSize = 0;
		DWORD dwSize = 0;

		while (1)
		{
			DWORD dwIndex = 0;

			FlushDecoded(&disasm);
			Decode(&disasm, (char*)(DWORD)disasm.Address, &dwIndex);

			DWORD dwInstrSize = disasm.OpcodeSize + disasm.PrefixSize;

			if (dwInstrSize == 1 || dwInstrSize == 3)
			{
				BYTE opCode = *((BYTE*)disasm.Address);

				if (opCode == 0xC3 || opCode == 0xC2)
				{
					dwRetAddr = disasm.Address;
					dwRetSize = dwInstrSize;
					break;
				}
			}

			disasm.Address += dwInstrSize;
			dwSize += dwInstrSize;

			if (dwSize > 1024)
				break;
		}

		if (dwRetAddr == 0 || (dwRetAddr - (DWORD)pFuncAddr) < 5)
			return false;


		disasm.Address = (DWORD)pFuncAddr;
		DWORD dwCopyAddr = 0;

		while (1)
		{
			DWORD dwIndex = 0;

			FlushDecoded(&disasm);
			Decode(&disasm, (char*)(DWORD)disasm.Address, &dwIndex);

			DWORD dwInstrSize = disasm.OpcodeSize + disasm.PrefixSize;

			int diff = dwRetAddr - disasm.Address;

			if (diff < (int)(5 - dwRetSize))
				break;

			dwCopyAddr = disasm.Address;
			disasm.Address += dwInstrSize;

			if (disasm.Address > dwRetAddr)
				return false;
		}



		BYTE* trampBuff = (BYTE*)HeapMemAllocate(64);

		if (!trampBuff)
			return false;

		DWORD dwIdx = 0;
		disasm.Address = (DWORD)dwCopyAddr;

		while (disasm.Address < dwRetAddr)
		{
			DWORD dwIndex = 0;

			FlushDecoded(&disasm);
			Decode(&disasm, (char*)(DWORD)disasm.Address, &dwIndex);

			DWORD dwInstrSize = disasm.OpcodeSize + disasm.PrefixSize;

			StringUtil::MemCopy(trampBuff + dwIdx, (void*)disasm.Address, dwInstrSize);

			if (trampBuff[dwIdx] == 0xE9 || trampBuff[dwIdx] == 0xE8) // fix far relative jumps/calls
			{				
				DWORD jmp = *((DWORD*)(disasm.Address + 1));
				DWORD dwDstAddr = jmp + disasm.Address + 5;

				DWORD new_jmp = dwDstAddr - ((DWORD)trampBuff + dwIdx) - 5; //new relative jump address

				*((DWORD*)(trampBuff + dwIdx + 1)) = new_jmp; 
			}
			else if (trampBuff[dwIdx] == 0xEb) // fix short jumps
			{	
				BYTE jmp = *((BYTE*)(disasm.Address + 1));
				DWORD dwDstAddr = (char)jmp + disasm.Address + 2;

				DWORD new_jmp = dwDstAddr - ((DWORD)trampBuff + dwIdx) - 5;

				trampBuff[dwIdx] = 0xE9;
				*((DWORD*)(trampBuff + dwIdx + 1)) = new_jmp;

				dwIdx += 3;
			}

			disasm.Address += dwInstrSize;
			dwIdx += dwInstrSize;			
		}

		trampBuff[dwIdx] = 0xE8;

		*((DWORD*)(trampBuff + dwIdx + 1)) = GetJMP((DWORD)(trampBuff + dwIdx), (DWORD)pCallback);
		StringUtil::MemCopy(trampBuff + dwIdx + 5, (void*)dwRetAddr, dwRetSize);

		DWORD dwJump = GetJMP(dwCopyAddr, (DWORD)trampBuff);

		BYTE bJumpOp = 0xE9;
		WriteProtectedMem((HANDLE)-1, dwCopyAddr, (void*)&bJumpOp, 1);
		WriteProtectedMem((HANDLE)-1, dwCopyAddr+1, (void*)&dwJump, 4);
		return true;
	}




	/**  
	 * \brief 
	 */
	static void ParseForwardedExport(const char* pszEntry, char* szFnName, char* szDllName)
	{
		unsigned long len = StringUtil::StrLenA((char*)pszEntry);
		StringUtil::StrCopyA(szDllName, (char*)pszEntry);

		char* pszExpName = szDllName + len - 1;

		while (pszExpName > szDllName && *pszExpName != '.') pszExpName--;

		*pszExpName = 0;
		pszExpName++;

		StringUtil::StrCopyA(szFnName, pszExpName);
		StringUtil::StrConcatA(szDllName, ".dll");
	}



	/**  
	 * \brief 
	 */
	static FARPROC FindForwardedExportAddress(const char* pszName)
	{
		char szFn[256];
		char szDll[256];

		ParseForwardedExport(pszName, szFn, szDll);

		HMODULE hModFwd = ProcessUtil::FindModuleBase(szDll);

		if (!hModFwd)
		{
			ProcessUtil::TNtApiTable NtApi;
			StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

			NtApi.LdrLoadDll = (ProcessUtil::LdrLoadDllPtr)1;

			if (!ProcessUtil::GetNtApi(NtApi))
				return 0;

			wchar_t szExportFwDllW[260];
			StringUtil::MemSet(szExportFwDllW, 0, sizeof(szExportFwDllW));
			StringUtil::AnsiToUnicode(szExportFwDllW, szDll, 259);

			UNICODE_STRING uniStr;
			StringUtil::InitUnicodeString(&uniStr, szExportFwDllW);

			NtApi.LdrLoadDll(0, 0, &uniStr, (PHANDLE)&hModFwd);
		}

		if (!hModFwd)
			return 0;

		return (FARPROC)ProcessUtil::FindFnAddressByName(hModFwd, szFn);
	}



	/**  
	 * \brief 
	 */
	static PVOID CreateExportFwdThunk(const char* pszExport)
	{
		const char code_buff[] =
			"\xB8\x2B\x00\x00\x00"		// 00000000 mov eax, offset proc_addr (0000002Bh) 
			"\x8B\x00"					// 00000005 mov eax, dword ptr [eax] 
			"\x83\xF8\x00"				// 00000007 cmp eax, 0 
			"\x75\x18"					// 0000000A jne call_api (00000024h) 
			"\xB8\x33\x00\x00\x00"		// 0000000C mov eax, offset fwd_export_name (00000033h) 
			"\x8B\x00"					// 00000011 mov eax, dword ptr [eax] 
			"\x50"						// 00000013 push eax  
			"\xB8\x2F\x00\x00\x00"		// 00000014 mov eax, offset fwd_find_proc (0000002Fh) 
			"\xFF\x10"					// 00000019 call dword ptr [eax] 
			"\x83\xC4\x04"				// 0000001B add esp, 4 
			"\x89\x05\x2B\x00\x00\x00"	// 0000001E mov dword ptr [proc_addr (0000002Bh)], eax 

			// call_api:
			"\xB8\x2B\x00\x00\x00"		// 00000024 mov eax, offset proc_addr (0000002Bh) 
			"\xFF\x20"					// 00000029 jmp dword ptr [eax] 

			// proc_addr: 
			"\x00\x00\x00\x00"			// 0000002B _emit 0; _emit 0; _emit 0; _emit 0 

			// fwd_find_proc:
			"\x00\x00\x00\x00"			// 0000002F _emit 0; _emit 0; _emit 0; _emit 0 

			// fwd_export_name:
			"\x00\x00\x00\x00"			// 00000033 _emit 0; _emit 0; _emit 0; _emit 0 
			;

		void* pThunkWithNops = ApiHook::HeapMemAllocate(8 + sizeof(code_buff) + 260);
		StringUtil::MemSet(pThunkWithNops, 0x90, 8);

		void* pThunk = (void*)((DWORD_PTR)pThunkWithNops + 8);


		StringUtil::MemCopy(pThunk, (void*)code_buff, sizeof(code_buff));

		int reloc[] = {0x01, 0x0D, 0x15, 0x20, 0x25};

		for (int i=0; i < sizeof(reloc)/sizeof(reloc[0]); i++)
			*((DWORD_PTR*)((BYTE*)pThunk + reloc[i])) += (DWORD_PTR)pThunk;


		char* pszExportBuff = (char*)(((DWORD_PTR)pThunk + sizeof(code_buff) + 0x3) & ~0x3); // 4 byte aligned
		StringUtil::StrCopyA(pszExportBuff, pszExport);

		*((DWORD*)((BYTE*)pThunk + 0x33)) = (DWORD)pszExportBuff;
		*((DWORD*)((BYTE*)pThunk + 0x2F)) = (DWORD)&FindForwardedExportAddress;

		return pThunk;
	}


	/**  
	 * \brief 
	 */
	bool PatchExport(HMODULE hModule, const char* fnName, PVOID pAddr, PVOID* pvOrigAddr)
	{
		InitNtMemApi(NtApi);

		bool fForwarded = false;
		FARPROC* pEntry = ProcessUtil::FindFnExportEntry(hModule, fnName, &fForwarded);

		if (!pEntry)
			return false;


		PVOID pOriginalAddress = (PVOID)((DWORD_PTR)*pEntry + (DWORD_PTR)hModule);


		if (fForwarded)
		{
			// we have a forwarded export
			pOriginalAddress = CreateExportFwdThunk((const char*)pOriginalAddress);

			if (!pOriginalAddress)
				return false;
		}


		DWORD dwAddress = (DWORD)pEntry;
		ULONG ulSize = 4;
		DWORD dwOldProtect = 0;

		NtApi.NtProtectVirtualMemory((HANDLE)-1, (PVOID*)&dwAddress, &ulSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

		bool fRet = true;

		if ((int)(*pEntry) > 0 && (int)(*pEntry) < (int)ProcessUtil::GetModuleSize(hModule))
		{
			*pvOrigAddr = pOriginalAddress;
			*pEntry = (FARPROC)((DWORD_PTR)pAddr - (DWORD_PTR)hModule);
		}
		else
		{
			if (*pEntry == (FARPROC)((DWORD_PTR)pAddr - (DWORD_PTR)hModule))
			{
				// already patched
				fRet = false;
			}
			else
			{
				// patched by someone else
				*pvOrigAddr = pOriginalAddress;
				*pEntry = (FARPROC)((DWORD_PTR)pAddr - (DWORD_PTR)hModule);
			}
		}

		ulSize = 4;
		NtApi.NtProtectVirtualMemory((HANDLE)-1, (PVOID*)&dwAddress, &ulSize, dwOldProtect, &dwOldProtect);
		return fRet;
	}

#if 0
	/**  
	 * \brief 
	 */
	ApiFantomLocation* CreateFantomLocation()
	{
		InitNtMemApiRet(NtApi, 0);

		ApiFantomLocation* pChainData = (ApiFantomLocation*)AllocateMem((HANDLE)-1, sizeof(ApiFantomLocation), PAGE_EXECUTE_READWRITE);

		if (!pChainData)
			return 0;

		pChainData->jumpOpCode = 0xE9;
		pChainData->pNext = 0;
		pChainData->relAddrJmp = 0;

		StringUtil::MemSet(pChainData->junkCode, 0x90, sizeof(pChainData->junkCode));

		return pChainData;
	}


	/**  
	 * \brief 
	 */
	void SetFantomLocationJump(ApiFantomLocation* pChainData, DWORD dwAddr)
	{
		if (!pChainData)
			return;

		pChainData->relAddrJmp = GetJMP((DWORD)&pChainData->jumpOpCode, dwAddr);
	}


	/**  
	 * \brief 
	 */
	ApiDllInfo* CreateApiDllInfo(ApiDllInfo* pChain, const char* pszDllName)
	{
		ApiDllInfo* pApiDllInfo = FindApiDllInfo(pChain, pszDllName);

		if (pApiDllInfo)
			return pApiDllInfo;

		pApiDllInfo = (ApiDllInfo*)HeapMemAllocate(sizeof(ApiDllInfo));

		if (pApiDllInfo)
		{
			pApiDllInfo->pNext = pChain;
			pApiDllInfo->pProcInfo = 0;
			StringUtil::StrCopyA(pApiDllInfo->ApiDll, pszDllName);
		}

		return pApiDllInfo;
	}


	/**  
	 * \brief 
	 */
	ApiDllInfo* FindApiDllInfo(ApiDllInfo* pChain, const char* pszDllName)
	{
		ApiDllInfo* pIter = pChain;

		while (pIter)
		{
			if (StringUtil::StrCmpIA(pIter->ApiDll, pszDllName))
				break;

			pIter = pIter->pNext;
		}

		return pIter;
	}


	/**  
	 * \brief 
	 */
	bool AddApiProc(ApiDllInfo* pDllInfo, const char* pszName, WORD wOrd, WORD flags, const char* pszModules, FARPROC pfnHook, FARPROC* ppfnOrigApi)
	{
		if (!pDllInfo)
			return false;

		ApiProcInfo* pNewInfo = (ApiProcInfo*)HeapMemAllocate(sizeof(ApiProcInfo));
		StringUtil::MemSet(pNewInfo, 0, sizeof(ApiProcInfo));

		pNewInfo->pNext = pDllInfo->pProcInfo;
		pDllInfo->pProcInfo = pNewInfo;

		StringUtil::StrCopyA(pNewInfo->ApiName, pszName);
		pNewInfo->ApiOrdinal = wOrd;
		pNewInfo->flags = flags;

		if (pszModules)
			StringUtil::StrCopyA(pNewInfo->ModuleList, pszModules);

		pNewInfo->pfnHook = pfnHook;
		pNewInfo->ppfnOriginalApi = ppfnOrigApi;
		return true;
	}


	/**  
	 * \brief 
	 */
	ApiProcInfo* FindApiProcInfo(ApiProcInfo* pProcInfo, WORD wOrd)
	{
		ApiProcInfo* pIter = pProcInfo;

		while (pIter)
		{
			if (pIter->ApiOrdinal == wOrd)
				break;

			pIter = pIter->pNext;
		}

		return pIter;
	}


	/**  
	 * \brief 
	 */
	ApiCallLocation* AddCallLocation(ApiCallLocation* pCallLoc, void* pvAddr)
	{
		ApiCallLocation* pNewLoc = (ApiCallLocation*)HeapMemAllocate(sizeof(ApiCallLocation));
		pNewLoc->pNext = pCallLoc;
		pNewLoc->callAddress = pvAddr;

		return pNewLoc;
	}

#endif


	/**  
	 * \brief 
	 */
	FARPROC CreateHookThunk(PVOID* pHookAddr, PVOID* pOrigCallAddr)
	{
		if (!pHookAddr || !pOrigCallAddr)
			return 0;

		BYTE buff[] = "\x90\x90\x90\x90\x90\x90\x90\x90"
						"\x53\x8B\x1D"
						"\x00\x00\x00\x00" // Address of variable holding hook function pointer
						"\x83\xFB\x00\x74\x07\x5B\xFF\x25"
						"\x00\x00\x00\x00" // Address of variable holding hook function pointer
						"\x5B\xFF\x25"
						"\x00\x00\x00\x00"; // Address of variable holding original function pointer

		void* proc = ApiHook::HeapMemAllocate(sizeof(buff));
		StringUtil::MemCopy(proc, buff, sizeof(buff));

		*((DWORD_PTR*)((BYTE*)proc + 8 + 3)) = (DWORD_PTR)pHookAddr;
		*((DWORD_PTR*)((BYTE*)proc + 8 + 15)) = (DWORD_PTR)pHookAddr;
		*((DWORD_PTR*)((BYTE*)proc + 8 + 22)) = (DWORD_PTR)pOrigCallAddr;

		return (FARPROC)proc;
	}


	/**  
	 * \brief 
	 */
	HOOKPROC CreateWinHookFilterThunk(FilterHookCallPtr pfnFilter, HOOKPROC pfnOrigHook)
	{
		if (!pfnOrigHook)
			return 0;

		BYTE code_buff[] = 
		"\x55"						// push ebp
		"\x8B\xEC"					// mov ebp, esp
		"\xFF\x75\x10"				// push [ebp + 10h]
		"\xFF\x75\x0C"				// push [ebp + 0Ch]
		"\xFF\x75\x08"				// push [ebp + 08h]
		"\x68\x00\x00\x00\x00"		// push 00000000h 
		"\xE8\x00\x00\x00\x00"		// call 00000000h
		"\x5D"						// pop ebp
		"\xC2\x0C\x00"				// retn 0Ch
		;

		BYTE* pThunk = (BYTE*)HeapMemAllocate(sizeof(code_buff));

		if (!pThunk)
			return 0;

		StringUtil::MemCopy(pThunk, code_buff, sizeof(code_buff));
		*((DWORD_PTR*)(pThunk + 13)) = (DWORD_PTR)pfnOrigHook;
		*((DWORD_PTR*)(pThunk + 18)) = GetJMP((DWORD_PTR)(pThunk + 17), (DWORD_PTR)pfnFilter);

		return (HOOKPROC)pThunk;
	}

}


#pragma warning(pop)

