#include "ProcessUtil.h"
#include "StringUtil.h"


#define _PE_MakePtr(Type, Base, Offset) ((Type)(DWORD_PTR(Base) + (DWORD_PTR)(Offset)))


namespace ProcessUtil
{
	/**  
	 * \brief 
	 */
	void* GetPeb()
	{
		void* retVal;
		retVal = 0;

		__asm
		{
			push eax
			mov eax, fs:[30h] //PEB
			mov retVal, eax
			pop eax
		}

		return retVal;
	}


	/**  
	 * \brief 
	 */
	void* GetLocalProcessBaseAddress()
	{
		void* retVal;
		retVal = 0;

		__asm
		{
			push eax
			mov eax, fs:[30h] //PEB
			mov eax, [eax+08h] // process base
			mov retVal, eax
			pop eax
		}

		return retVal;
	}


	/**  
	 * \brief 
	 */
	void* GetProcessPebAddress(HANDLE hProcess)
	{
		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.NtQueryInformationProcess = (NtQueryInformationProcessPtr)1;

		if (!GetNtApi(NtApi))
			return 0;

		PROCESS_BASIC_INFORMATION procInfo;
		ULONG retLen = 0;

		if (0 > NtApi.NtQueryInformationProcess(hProcess, ProcessBasicInformation, &procInfo, sizeof(PROCESS_BASIC_INFORMATION), &retLen))
			return 0;

		return (void*)((BYTE*)procInfo.PebBaseAddress);
	}



	/**  
	 * \brief 
	 */
	void* GetProcessBaseAddress(HANDLE hProcess)
	{
		void* pPeb;
		pPeb = GetProcessPebAddress(hProcess);

		if (!pPeb)
			return 0;


		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.NtReadVirtualMemory = (NtReadVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return 0;

		DWORD dwRead = 0;
		DWORD dwBaseAddr = 0;

		NtApi.NtReadVirtualMemory(hProcess, ((BYTE*)pPeb + 8), &dwBaseAddr, 4, &dwRead);

		return (void*)dwBaseAddr;
	}


	/**  
	 * \brief 
	 */
	void* GetLocalProcessHeap()
	{
		void* retVal;
		retVal = 0;

		__asm
		{
			push eax
			mov eax, fs:[30h] //PEB
			mov eax, [eax+18h] // process heap
			mov retVal, eax
			pop eax
		}

		return retVal;
	}


	/**  
	 * \brief 
	 */
	HMODULE FindModuleBase(const char* pszModSearch)
	{
		HMODULE modBase;
		wchar_t* pszModule; 	
		DWORD dwStart;
		DWORD dwNext;

		__asm 
		{
			push ebx
			mov eax, fs:[30h]
			mov eax, [eax+0ch]
			mov eax, [eax+14h]
			mov dwStart, eax
			push eax
		}


		do
		{
			__asm 
			{
				pop eax
				mov ebx, [eax+28h]
				mov pszModule, ebx
				mov ebx, [eax]
				mov dwNext, ebx
				push eax
			}

			if (!pszModule || dwNext == dwStart)
				break;

			int len;
			int pos;

			for (len=0; pszModule[len] != 0; len++);
			for (pos=len-1; pos >= 0 && pszModule[pos] != L'\\'; pos--);
			pos++;

			pszModule += pos;
			
			if (StringUtil::StrCmpI(pszModSearch, pszModule))
			{
				__asm 
				{
					pop eax
					mov eax, [eax+0x10]
					mov modBase, eax
					pop ebx
				}

				return modBase;
			}


			__asm
			{
				pop eax
				mov eax, [eax]
				push eax
			}
		}
		while (1);


		__asm 
		{
			pop eax
			pop ebx
		}

		return 0;
	}



	/**  
	 * \brief 
	 */
	HMODULE FindModuleBase2(const char* pszModSearch)
	{
		void* peb = GetPeb();
		PPEB_LDR_DATA pLdrData = *((PPEB_LDR_DATA*)((BYTE*)peb + 12));

		PLDR_MODULE pLdrMod = (PLDR_MODULE)pLdrData->InLoadOrderModuleList.Flink;

		while (pLdrMod->BaseAddress != 0)
		{
			if (StringUtil::StrCmpI(pszModSearch, pLdrMod->BaseDllName.Buffer))
				return (HMODULE)pLdrMod->BaseAddress;

			pLdrMod = (PLDR_MODULE)pLdrMod->InLoadOrderModuleList.Flink;
		}

		return 0;
	}



	/**  
	 * \brief 
	 */
	const wchar_t* FindModuleName(HMODULE hMod)
	{
		HMODULE modBase;
		wchar_t* pszModule; 	
		DWORD dwStart;
		DWORD dwNext;

		__asm 
		{
			push ebx
			mov eax, fs:[30h]
			mov eax, [eax+0ch]
			mov eax, [eax+14h]
			mov dwStart, eax
			push eax
		}


		do
		{
			__asm 
			{
				pop eax
				mov ebx, [eax+28h]
				mov pszModule, ebx
				mov ebx, [eax+10h]
				mov modBase, ebx
				mov ebx, [eax]
				mov dwNext, ebx
				push eax
			}

			if (!pszModule || dwNext == dwStart)
				break;

			if (modBase == hMod)
			{
				int len;
				int pos;

				for (len=0; pszModule[len] != 0; len++);
				for (pos=len-1; pos >= 0 && pszModule[pos] != L'\\'; pos--);
				pos++;

				pszModule += pos;
			
				__asm 
				{
					pop eax
					pop ebx
				}

				return pszModule;
			}

			__asm
			{
				pop eax
				mov eax, [eax]
				push eax
			}
		}
		while (1);


		__asm 
		{
			pop eax
			pop ebx
		}

		return 0;
	}



	/**  
	 * \brief 
	 */
	const wchar_t* FindModuleFullPath(HMODULE hMod)
	{
		HMODULE modBase;
		wchar_t* pszModule; 	

		__asm 
		{
			push ebx
			mov eax, fs:[30h]
			mov eax, [eax+0ch]
			mov eax, [eax+14h]
			push eax
		}


		do
		{
			__asm 
			{
				pop eax
				mov ebx, [eax+28h]
				mov pszModule, ebx
				mov ebx, [eax+10h]
				mov modBase, ebx
				push eax
			}

			if (!pszModule)
				break;

			if (modBase == hMod)
			{			
				__asm 
				{
					pop eax
					pop ebx
				}

				return pszModule;
			}

			__asm
			{
				pop eax
				mov eax, [eax]
				push eax
			}
		}
		while (1);


		__asm 
		{
			pop eax
			pop ebx
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	bool GetWinApi(TWinApiTable& winApiTable)
	{
		HMODULE hKernel32;
		bool fRes;
	
		hKernel32 = FindModuleBase("kernel32.dll");

		if (!hKernel32)
			return false;

		fRes = true;

		if (winApiTable.LoadLibraryA == (LoadLibraryAPtr)1)
			fRes &= 0 != (winApiTable.LoadLibraryA = (LoadLibraryAPtr)FindFnAddressByName(hKernel32, "LoadLibraryA"));

		if (winApiTable.GetModuleHandleA == (GetModuleHandleAPtr)1)
			fRes &= 0 != (winApiTable.GetModuleHandleA = (GetModuleHandleAPtr)FindFnAddressByName(hKernel32, "GetModuleHandleA"));

		if (winApiTable.GetProcAddress == (GetProcAddressPtr)1)
			fRes &= 0 != (winApiTable.GetProcAddress = (GetProcAddressPtr)FindFnAddressByName(hKernel32, "GetProcAddress"));

		if (winApiTable.IsBadReadPtr == (IsBadReadPtrPtr)1)
			fRes &= 0 != (winApiTable.IsBadReadPtr = (IsBadReadPtrPtr)FindFnAddressByName(hKernel32, "IsBadReadPtr"));

		if (winApiTable.VirtualProtect == (VirtualProtectPtr)1)
			fRes &= 0 != (winApiTable.VirtualProtect = (VirtualProtectPtr)FindFnAddressByName(hKernel32, "VirtualProtect"));

		if (winApiTable.Beep == (BeepPtr)1)
			fRes &= 0 != (winApiTable.Beep = (BeepPtr)FindFnAddressByName(hKernel32, "Beep"));

		if (winApiTable.Sleep == (SleepPtr)1)
			fRes &= 0 != (winApiTable.Sleep = (SleepPtr)FindFnAddressByName(hKernel32, "Sleep"));

		if (winApiTable.ResumeThread == (ResumeThreadPtr)1)
			fRes &= 0 != (winApiTable.ResumeThread = (ResumeThreadPtr)FindFnAddressByName(hKernel32, "ResumeThread"));

		return fRes;
	}


	/**  
	 * \brief 
	 */
	bool GetNtApi(TNtApiTable& NtApiTable)
	{
		HMODULE hNtDll; 
		bool fRes;

		hNtDll = FindModuleBase("ntdll.dll");

		if (!hNtDll)
			return false;

		fRes = true;

		if (NtApiTable.NtQueryInformationProcess == (NtQueryInformationProcessPtr)1)
			fRes &= 0 != (NtApiTable.NtQueryInformationProcess = (NtQueryInformationProcessPtr)FindFnAddressByName(hNtDll, "NtQueryInformationProcess"));

		if (NtApiTable.NtReadVirtualMemory == (NtReadVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtReadVirtualMemory = (NtReadVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtReadVirtualMemory"));
		
		if (NtApiTable.NtWriteVirtualMemory == (NtWriteVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtWriteVirtualMemory = (NtWriteVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtWriteVirtualMemory"));

		if (NtApiTable.NtAllocateVirtualMemory == (NtAllocateVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtAllocateVirtualMemory = (NtAllocateVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtAllocateVirtualMemory"));

		if (NtApiTable.NtOpenProcess == (NtOpenProcessPtr)1)
			fRes &= 0 != (NtApiTable.NtOpenProcess = (NtOpenProcessPtr)FindFnAddressByName(hNtDll, "NtOpenProcess"));

		if (NtApiTable.NtProtectVirtualMemory == (NtProtectVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtProtectVirtualMemory = (NtProtectVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtProtectVirtualMemory"));

		if (NtApiTable.NtClose == (NtClosePtr)1)
			fRes &= 0 != (NtApiTable.NtClose = (NtClosePtr)FindFnAddressByName(hNtDll, "NtClose"));

		if (NtApiTable.NtQueryVirtualMemory == (NtQueryVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtQueryVirtualMemory = (NtQueryVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtQueryVirtualMemory"));
		
		if (NtApiTable.NtFreeVirtualMemory == (NtFreeVirtualMemoryPtr)1)
			fRes &= 0 != (NtApiTable.NtFreeVirtualMemory = (NtFreeVirtualMemoryPtr)FindFnAddressByName(hNtDll, "NtFreeVirtualMemory"));

		if (NtApiTable.LdrGetDllHandle == (LdrGetDllHandlePtr)1)
			fRes &= 0 != (NtApiTable.LdrGetDllHandle = (LdrGetDllHandlePtr)FindFnAddressByName(hNtDll, "LdrGetDllHandle"));

		if (NtApiTable.LdrLoadDll == (LdrLoadDllPtr)1)
			fRes &= 0 != (NtApiTable.LdrLoadDll = (LdrLoadDllPtr)FindFnAddressByName(hNtDll, "LdrLoadDll"));
		
		if (NtApiTable.RtlCreateHeap == (RtlCreateHeapPtr)1)
			fRes &= 0 != (NtApiTable.RtlCreateHeap = (RtlCreateHeapPtr)FindFnAddressByName(hNtDll, "RtlCreateHeap"));

		if (NtApiTable.RtlAllocateHeap == (RtlAllocateHeapPtr)1)
			fRes &= 0 != (NtApiTable.RtlAllocateHeap = (RtlAllocateHeapPtr)FindFnAddressByName(hNtDll, "RtlAllocateHeap"));

		if (NtApiTable.RtlReAllocateHeap == (RtlReAllocateHeapPtr)1)
			fRes &= 0 != (NtApiTable.RtlReAllocateHeap = (RtlReAllocateHeapPtr)FindFnAddressByName(hNtDll, "RtlReAllocateHeap"));

		if (NtApiTable.RtlFreeHeap == (RtlFreeHeapPtr)1)
			fRes &= 0 != (NtApiTable.RtlFreeHeap = (RtlFreeHeapPtr)FindFnAddressByName(hNtDll, "RtlFreeHeap"));

		if (NtApiTable.RtlDestroyHeap == (RtlDestroyHeapPtr)1)
			fRes &= 0 != (NtApiTable.RtlDestroyHeap = (RtlDestroyHeapPtr)FindFnAddressByName(hNtDll, "RtlDestroyHeap"));


		return fRes;
	}



	/**
	 * \brief 
	 */
	bool CheckForIATHook(const char* szDll, const char* szFunc)
	{		
		PVOID pfnOrigAddr = FindFnAddressByName(FindModuleBase(szDll), szFunc);
		PVOID pfnIatAddr = 0;

		if (!GetFunctionPtrFromIAT(GetLocalProcessBaseAddress(), szDll, szFunc, &pfnIatAddr))
			return false;

		return pfnOrigAddr != pfnIatAddr;
	}


	/**
	 * \brief 
	 */
	bool CheckForHotPatch(const char* szDll, const char* szFunc)
	{
		HMODULE hMod = FindModuleBase(szDll);

		if (!hMod)
			return false;

		BYTE* pFunc = (BYTE*)FindFnAddressByName(hMod, szFunc);

		if (!pFunc)
			return false;

		return (pFunc[0] == 0xE9 || pFunc[0] == 0x90 || (pFunc[0] == 0xEB && pFunc[1] == 0xF9));
	}


	/**
	 * \brief 
	 */
	bool CheckForBreakPoint(const char* szDll, const char* szFunc)
	{
		HMODULE hMod = FindModuleBase(szDll);

		if (!hMod)
			return false;

		BYTE* pFunc = (BYTE*)FindFnAddressByName(hMod, szFunc);

		if (!pFunc)
			return false;

		return (pFunc[0] == 0xCC || pFunc[1] == 0xCC || pFunc[2] == 0xCC);
	}


	/**
	 * \brief 
	 */
	bool IsHooked(const char* szDll, const char* szFunc)
	{
		return CheckForHotPatch(szDll, szFunc) || CheckForIATHook(szDll, szFunc) || CheckForBreakPoint(szDll, szFunc);
	}


	/**  
	 * \brief 
	 */
	bool IsBadReadMem(void *ptr)
	{
		ULONG dw;
		MEMORY_BASIC_INFORMATION mbi;
		int ok;

		TNtApiTable NtApi;

		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));
		StringUtil::MemSet(&mbi, 0, sizeof(mbi));

		NtApi.NtQueryVirtualMemory = (NtQueryVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return true;

		return false;

		NtApi.NtQueryVirtualMemory((HANDLE)-1, ptr, MemoryBasicInformation, &mbi, sizeof(mbi), &dw);

		ok = ((mbi.Protect & PAGE_READONLY) ||
			(mbi.Protect & PAGE_READWRITE) ||
			(mbi.Protect & PAGE_WRITECOPY) ||
			(mbi.Protect & PAGE_EXECUTE_READ) ||
			(mbi.Protect & PAGE_EXECUTE_READWRITE) ||
			(mbi.Protect & PAGE_EXECUTE_WRITECOPY));

		// check the page is not a guard page

		if (mbi.Protect & PAGE_GUARD)
			ok = 0;

		if (mbi.Protect & PAGE_NOACCESS)
			ok = 0;

		return (ok == 0);
	}


	/**  
	 * \brief 
	 */
	int IsBadWriteMem(void* ptr)
	{
		ULONG dw;
		MEMORY_BASIC_INFORMATION mbi;
		int ok;

		TNtApiTable NtApi;

		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));
		StringUtil::MemSet(&mbi, 0, sizeof(mbi));

		NtApi.NtQueryVirtualMemory = (NtQueryVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return true;

		NtApi.NtQueryVirtualMemory((HANDLE)-1, ptr, MemoryBasicInformation, &mbi, sizeof(mbi), &dw);


		ok = ((mbi.Protect & PAGE_READWRITE) ||
			(mbi.Protect & PAGE_WRITECOPY) ||
			(mbi.Protect & PAGE_EXECUTE_READWRITE) ||
			(mbi.Protect & PAGE_EXECUTE_WRITECOPY));

		// check the page is not a guard page
		if (mbi.Protect & PAGE_GUARD)
			ok = 0;

		if (mbi.Protect & PAGE_NOACCESS)
			ok = 0;

		return (ok == 0);
	}



	/**
	 * \brief 
	 */
	BOOL GetFunctionPtrFromIAT(void* pDosHdr, LPCSTR pImportModuleName, LPCSTR pFunctionSymbol, PVOID* ppvFn)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pDosHdr;

		if (!ppvFn || !pDosHdr || !pImportModuleName || !pFunctionSymbol
				|| pImportModuleName[0] == 0 || pFunctionSymbol[0] == 0)
		{
			return FALSE;
		}

		*ppvFn = 0;

		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		DWORD dwTableSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;


		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
				|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwTableSize == 0
				|| pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader)
		{
			return FALSE;
		}

		while (pImportDesc->Name)
		{
			LPCSTR pszDllName = _PE_MakePtr(LPCSTR, pDOSHeader, pImportDesc->Name);

			
			if (StringUtil::StrCmpIA(pszDllName, pImportModuleName))
			{
				DWORD* thunkRef = 0;
				FARPROC *funcRef = 0;

				if (pImportDesc->OriginalFirstThunk) 
				{
					thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->OriginalFirstThunk);
					funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
				} 
				else 
				{
					// no hint table
					thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
					funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
				}

				for (; *thunkRef; thunkRef++, funcRef++) 
				{
					const char* pszProcName = 0;

					if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
					{
						HMODULE hModd = ProcessUtil::FindModuleBase(pImportModuleName);
						pszProcName = GetFnNameByOrdinal(hModd, IMAGE_ORDINAL(*thunkRef));
					} 
					else 
					{
						PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)((DWORD)pDOSHeader + (*thunkRef));
						pszProcName = (LPCSTR)&thunkData->Name;
					}

					
					if (pszProcName && StringUtil::StrCmpIA(pszProcName, pFunctionSymbol))
					{
						*ppvFn = *funcRef;
						return *ppvFn != 0;
					}
				}
			}

			pImportDesc++;
		}

		return FALSE;
	}


	/**  
	 * \brief 
	 */
	BOOL TraverseReloc(void* base, TraverseRelocProc fnCallback, LPVOID pvParam)
	{
		if (!base || !fnCallback)
			return FALSE;

		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)base;
		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);

		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		{
			return FALSE;
		}


		void* reloc = 0;
		size_t reloc_sz = 0; 


		// Use data directory to find RVA of reloc section
		if (pNTHeader->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_BASERELOC
			&& pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		{
			DWORD rva = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

			reloc_sz = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
			reloc = (void*)((DWORD)base + rva);	
		}
		else
		{
			reloc_sz = 0;
			reloc = NULL;
		}

		int ofs = (int)(DWORD)base - pNTHeader->OptionalHeader.ImageBase;
		void* end = (void*)((DWORD)reloc + reloc_sz);

		for (TRelocBlock* rb = (TRelocBlock*)reloc; (DWORD)rb < (DWORD)end; rb = (TRelocBlock*)((DWORD)rb + rb->block_sz))
		{
			void* b_end = (void*)((DWORD)rb + rb->block_sz);

			for (WORD* x = (WORD*)((DWORD)rb + sizeof(*rb)); x < b_end; x++) 
			{
				WORD type = (*x & 0xf000) >> 12;
				WORD addr = (*x & 0x0fff);

				switch(type) 
				{
				case IMAGE_REL_BASED_HIGHLOW:
					{
						DWORD* f = (DWORD*)((DWORD)base + rb->pg_rva + addr);						
						// *f += ofs;

						if (!fnCallback(base, ofs, f, pvParam))
							return TRUE;
					}
					break;
				case IMAGE_REL_BASED_ABSOLUTE:
					// nop
					break;
				default:
					// Not handled reloc type
					break;
				}
			}
		}

		return TRUE;
	}


	/**
	 * \brief 
	 */
	BOOL TraverseIAT(void* pDosHdr, TraverseIATProc fnCallback, LPVOID pvParam)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pDosHdr;

		if (!pDosHdr || !fnCallback)
			return FALSE;


		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		DWORD dwTableSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;


		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
				|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwTableSize == 0
				|| pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader)
		{
			return FALSE;
		}

		while (pImportDesc->Name)
		{
			LPCSTR pszDllName = _PE_MakePtr(LPCSTR, pDOSHeader, pImportDesc->Name);
			
			DWORD* thunkRef = 0;
			FARPROC *funcRef = 0;

			if (pImportDesc->OriginalFirstThunk) 
			{
				thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->OriginalFirstThunk);
				funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
			} 
			else 
			{
				// no hint table
				thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
				funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
			}

			for (; *thunkRef; thunkRef++, funcRef++) 
			{
				bool fCallbackRes = true;

				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
				{
					fCallbackRes = fnCallback(pszDllName, 0, IMAGE_ORDINAL(*thunkRef), funcRef, pvParam);
				} 
				else 
				{
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)((DWORD)pDOSHeader + (*thunkRef));
					const char* pszProcName = (LPCSTR)&thunkData->Name;

					fCallbackRes = fnCallback(pszDllName, pszProcName, 0, funcRef, pvParam);
				}

				if (!fCallbackRes)
					return TRUE;
			}

			pImportDesc++;
		}

		return TRUE;
	}




	/**
	 * \brief 
	 */
	const char* GetFnNameByOrdinal(HMODULE hModule, DWORD dwOrd)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;

		if (!pDOSHeader || pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY pExportDir = _PE_MakePtr(PIMAGE_EXPORT_DIRECTORY, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		DWORD dwExportDirSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwExportDirSize == 0
				|| pExportDir == (PIMAGE_EXPORT_DIRECTORY)pNTHeader)
		{
			return 0;
		}

		LPDWORD pNames = _PE_MakePtr(LPDWORD, pDOSHeader, pExportDir->AddressOfNames);
		LPWORD pOrdNames = _PE_MakePtr(LPWORD, pDOSHeader, pExportDir->AddressOfNameOrdinals);

		for (int i=0; i < (int)pExportDir->NumberOfNames; i++)
		{
			DWORD dwFoundOrd = pOrdNames[i] + pExportDir->Base;

			if (dwFoundOrd == dwOrd)
			{
				if (pOrdNames[i] >= pExportDir->NumberOfFunctions)
					return 0;

				const char* pszName = (char*)_PE_MakePtr(char*, pDOSHeader, pNames[i]);
				return pszName;
			}
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	FARPROC FindFnAddressByName(HMODULE hMod, const char* pszFn)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hMod;

		if (!pDOSHeader || pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY pExportDir = _PE_MakePtr(PIMAGE_EXPORT_DIRECTORY, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		DWORD dwExportDirSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwExportDirSize == 0
				|| pExportDir == (PIMAGE_EXPORT_DIRECTORY)pNTHeader)
		{
			return 0;
		}


		LPDWORD pNames = _PE_MakePtr(LPDWORD, pDOSHeader, pExportDir->AddressOfNames);
		LPWORD pOrdNames = _PE_MakePtr(LPWORD, pDOSHeader, pExportDir->AddressOfNameOrdinals);
		LPDWORD pAddresses = _PE_MakePtr(LPDWORD, pDOSHeader, pExportDir->AddressOfFunctions);

		for (int i=0; i < (int)pExportDir->NumberOfNames; i++)
		{
			const char* pszName;
			pszName = (char*)_PE_MakePtr(char*, pDOSHeader, pNames[i]);

			if (StringUtil::StrCmpA(pszName, pszFn))
			{
				if (pOrdNames[i] >= pExportDir->NumberOfFunctions)
					return 0;

				FARPROC ret = (FARPROC)(pAddresses[pOrdNames[i]] + (DWORD)hMod);

				if ((DWORD)ret > (DWORD)pExportDir && (DWORD)ret < (DWORD)pExportDir + dwExportDirSize)
				{
					// we have a forwarded export
					char szExportFwd[256];
					StringUtil::MemSet(szExportFwd, 0, 256);

					unsigned long len = StringUtil::StrLenA((char*)ret);
					StringUtil::MemCopy(szExportFwd, (char*)ret, len);
					
					char* pszExpName = szExportFwd + len - 1;

					while (pszExpName > szExportFwd && *pszExpName != '.') pszExpName--;

					*pszExpName = 0;
					pszExpName++;

					char szExportFwdDll[256];
					StringUtil::MemSet(szExportFwdDll, 0, 256);
					StringUtil::StrCopyA(szExportFwdDll, szExportFwd);
					StringUtil::StrConcatA(szExportFwdDll, ".dll");

					HMODULE hModFwd = FindModuleBase(szExportFwdDll);

					if (!hModFwd)
					{
						TNtApiTable NtApi;
						StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

						NtApi.LdrLoadDll = (LdrLoadDllPtr)1;

						if (!GetNtApi(NtApi))
							return 0;

						wchar_t szExportFwDllW[260];
						StringUtil::MemSet(szExportFwDllW, 0, sizeof(szExportFwDllW));
						StringUtil::AnsiToUnicode(szExportFwDllW, szExportFwdDll, 259);

						UNICODE_STRING uniStr;
						StringUtil::InitUnicodeString(&uniStr, szExportFwDllW);

						NtApi.LdrLoadDll(0, 0, &uniStr, (PHANDLE)&hModFwd);
					}

					if (!hModFwd)
						return 0;

					return FindFnAddressByName(hModFwd, pszExpName);
				}

				return ret;
			}
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	FARPROC* FindFnExportEntry(HMODULE hMod, const char* pszFn, bool* pfIsForwarded)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hMod;

		if (!pDOSHeader || pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_EXPORT_DIRECTORY pExportDir = _PE_MakePtr(PIMAGE_EXPORT_DIRECTORY, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		DWORD dwTableSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

		if (pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwTableSize == 0
				|| pExportDir == (PIMAGE_EXPORT_DIRECTORY)pNTHeader)
		{
			return 0;
		}


		LPDWORD pNames = _PE_MakePtr(LPDWORD, pDOSHeader, pExportDir->AddressOfNames);
		LPWORD pOrdNames = _PE_MakePtr(LPWORD, pDOSHeader, pExportDir->AddressOfNameOrdinals);
		LPDWORD pAddresses = _PE_MakePtr(LPDWORD, pDOSHeader, pExportDir->AddressOfFunctions);

		for (int i=0; i < (int)pExportDir->NumberOfNames; i++)
		{
			const char* pszName;
			pszName = (char*)_PE_MakePtr(char*, pDOSHeader, pNames[i]);

			if (StringUtil::StrCmpA(pszName, pszFn))
			{
				if (pOrdNames[i] >= pExportDir->NumberOfFunctions)
					return 0;

				FARPROC ret = (FARPROC)(pAddresses[pOrdNames[i]] + (DWORD)hMod);

				if (pfIsForwarded)
					*pfIsForwarded = ((DWORD_PTR)ret > (DWORD_PTR)pExportDir && (DWORD_PTR)ret < (DWORD_PTR)pExportDir + dwTableSize);

				return (FARPROC*)&pAddresses[pOrdNames[i]];
			}
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	static bool FindFnImportEntryCallback(LPCSTR pImportModuleName, LPCSTR pFunctionSymbol, DWORD dwOrd, FARPROC* ppfn, LPVOID pvParam)
	{
		struct TParam { const char* pszDll; const char* pszFn; WORD wOrd; FARPROC* pRes; };
		TParam* param = (TParam*)pvParam;

		if (!param)
			return false;

		if (StringUtil::StrCmpIA(pImportModuleName, param->pszDll))
		{
			if ((pFunctionSymbol && StringUtil::StrCmpA(pFunctionSymbol, param->pszFn))
					|| (dwOrd != 0 && dwOrd == (DWORD)param->wOrd))
			{
				param->pRes = (FARPROC*)ppfn;
				return false;
			}
		}

		return true;
	}


	/**
	 * \brief 
	 */
	FARPROC* FindFnImportEntry(HMODULE hMod, const char* pImportModuleName, const char* pFunctionSymbol, WORD wOrd)
	{
		struct { const char* pszDll; const char* pszFn; WORD wOrd; FARPROC* pRes; } param;
		param.pszDll = pImportModuleName;
		param.pszFn = pFunctionSymbol;
		param.wOrd = wOrd;
		param.pRes = 0;

		TraverseIAT((void*)hMod, FindFnImportEntryCallback, &param);
		return param.pRes;
	}


	/**
	 * \brief 
	 */
	DWORD* FindFnImportEntryByOrd(HMODULE hMod, const char* pImportModuleName, WORD ordSymbol)
	{
		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hMod;

		if (!hMod || ordSymbol == 0
				|| !pImportModuleName || pImportModuleName[0] == 0)
		{
			return 0;
		}


		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		DWORD dwTableSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;


		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
				|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
				|| dwTableSize == 0
				|| pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader)
		{
			return 0;
		}

		while (pImportDesc->Name)
		{
			LPCSTR pszDllName = _PE_MakePtr(LPCSTR, pDOSHeader, pImportDesc->Name);
			
			if (StringUtil::StrCmpIA(pszDllName, pImportModuleName))
			{
				DWORD* thunkRef = 0;
				FARPROC *funcRef = 0;

				if (pImportDesc->OriginalFirstThunk) 
				{
					thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->OriginalFirstThunk);
					funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
				} 
				else 
				{
					// no hint table
					thunkRef = (DWORD*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
					funcRef = (FARPROC*)((DWORD)pDOSHeader + pImportDesc->FirstThunk);
				}

				for (; *thunkRef; thunkRef++, funcRef++) 
				{
					if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
					{
						if (ordSymbol == IMAGE_ORDINAL(*thunkRef))
							return (DWORD*)funcRef;
					} 
				}
			}

			pImportDesc++;
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	FARPROC FindFnAddressByOrd(HMODULE hMod, WORD wOrd)
	{
		return FindFnAddressByName(hMod, GetFnNameByOrdinal(hMod, wOrd));
	}


	/**  
	 * \brief 
	 */
	const char* FindFnNameByOrd(HMODULE hMod, WORD wOrd)
	{
		return GetFnNameByOrdinal(hMod, wOrd);
	}


	/**  
	 * \brief 
	 */
	DWORD FindModuleEntryProc(HMODULE hMod)
	{
		PIMAGE_DOS_HEADER dos_header;
		PIMAGE_NT_HEADERS old_header;

		dos_header = (PIMAGE_DOS_HEADER)hMod;

		if (!dos_header || dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(hMod))[dos_header->e_lfanew];

		if (old_header->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		if (old_header->OptionalHeader.AddressOfEntryPoint == 0)
			return 0;

		return (DWORD)hMod + old_header->OptionalHeader.AddressOfEntryPoint;
	}


	/**  
	 * \brief 
	 */
	DWORD GetModuleSize(HMODULE hMod)
	{
		PIMAGE_DOS_HEADER dos_header;
		PIMAGE_NT_HEADERS old_header;

		dos_header = (PIMAGE_DOS_HEADER)hMod;

		if (!dos_header || dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		old_header = (PIMAGE_NT_HEADERS)&((const unsigned char *)(hMod))[dos_header->e_lfanew];

		if (old_header->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		return old_header->OptionalHeader.SizeOfImage;
	}



	/**  
	 * \brief Map a PE image and find the headers filling in a self_img structure 
	 */
	static int GetPEImage(TPEImage* pImage, void* base)
	{
		pImage->img_map = base;
		pImage->h_dos = (PIMAGE_DOS_HEADER)pImage->img_map;
		pImage->h_nt = (PIMAGE_NT_HEADERS)((DWORD)pImage->img_map + pImage->h_dos->e_lfanew);
		pImage->h_sec = (PIMAGE_SECTION_HEADER)((DWORD)&pImage->h_nt->OptionalHeader + pImage->h_nt->FileHeader.SizeOfOptionalHeader);

		return 1;
	}



	/**  
	 * \brief Relocate the image given a base offset
	 */ 
	static int RelocateImage(void *base, void *reloc, size_t reloc_sz, int ofs)
	{
		void* end = (void*)((DWORD)reloc + reloc_sz);

		for (TRelocBlock* rb = (TRelocBlock*)reloc; (DWORD)rb < (DWORD)end; rb = (TRelocBlock*)((DWORD)rb + rb->block_sz))
		{
			void* b_end = (void*)((DWORD)rb + rb->block_sz);

			for (WORD* x = (WORD*)((DWORD)rb + sizeof(*rb)); x < b_end; x++) 
			{
				WORD type = (*x & 0xf000) >> 12;
				WORD addr = (*x & 0x0fff);

				switch(type) 
				{
				case IMAGE_REL_BASED_HIGHLOW:
					{
						DWORD* f = (DWORD*)((DWORD)base + rb->pg_rva + addr);
						*f += ofs;
					}
					break;
				case IMAGE_REL_BASED_ABSOLUTE:
					// nop
					break;
				default:
					// Not handled reloc type
					return 0;
				}
			}
		}

		return 1;
	}


	/**  
	 * \brief 
	 */
	bool ReplicateImageIntoRemoteProcess(HANDLE hProcess, void* base, void** ppvNewBase, ULONG* pulImgSize)
	{
		TPEImage image = {0};

		if (!GetPEImage(&image, base))
			return false;


		// Allocate enough space for the prep image
		image.img_size = image.h_nt->OptionalHeader.SizeOfImage;


		ProcessUtil::TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.NtAllocateVirtualMemory = (ProcessUtil::NtAllocateVirtualMemoryPtr)1;
		NtApi.NtFreeVirtualMemory = (ProcessUtil::NtFreeVirtualMemoryPtr)1;
		NtApi.NtWriteVirtualMemory = (ProcessUtil::NtWriteVirtualMemoryPtr)1;

		if (!ProcessUtil::GetNtApi(NtApi))
			return false;

		void* prep_img = 0;
		ULONG ulAllocate = image.img_size;
		NtApi.NtAllocateVirtualMemory((HANDLE)-1, &prep_img, 0, &ulAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (prep_img == NULL) 
			return false;


		// Allocate remote image, get new base address
		DWORD new_base = 0;
		ulAllocate = image.img_size;
		NtApi.NtAllocateVirtualMemory(hProcess, (PVOID*)&new_base, 0, &ulAllocate, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (new_base == 0) 
		{
			ulAllocate = image.img_size;
			NtApi.NtFreeVirtualMemory((HANDLE)-1, &prep_img, &ulAllocate, MEM_RELEASE);
			return false;
		}	


		DWORD old_base = (DWORD)base;
		int ofs = new_base - old_base;

		// Copy PE headers
		StringUtil::MemCopy(prep_img, base, image.h_nt->OptionalHeader.SizeOfHeaders);


		// Copy section data in to prep image
		for (unsigned int i = 0; i < image.h_nt->FileHeader.NumberOfSections; i++) 
		{
			if ((image.h_sec[i].Characteristics & 0x2)
				|| image.h_sec[i].PointerToRawData == 0 
				|| image.h_sec[i].SizeOfRawData == 0)
			{
				continue;
			}

			StringUtil::MemCopy((void*)((DWORD)prep_img + image.h_sec[i].VirtualAddress), (void*)((DWORD)image.img_map + image.h_sec[i].VirtualAddress), image.h_sec[i].SizeOfRawData);
		}


		void* reloc_ptr = 0;
		size_t reloc_sz = 0; 


		// Use data directory to find RVA of reloc section
		if (image.h_nt->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_BASERELOC
			&& image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		{
			DWORD rva = image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

			reloc_sz = image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
			reloc_ptr = (void*)((DWORD)prep_img + rva);	
		}
		else
		{
			reloc_sz = 0;
			reloc_ptr = NULL;
		}


		void* idata_ptr = 0;
		size_t idata_sz = 0;

		// Use data directory to find RVA of idata section
		if (image.h_nt->OptionalHeader.NumberOfRvaAndSizes > IMAGE_DIRECTORY_ENTRY_IMPORT
				&& image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) 
		{
			DWORD rva = image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

			idata_sz = image.h_nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
			idata_ptr = (void*)((DWORD)prep_img + rva);
		}
		else
		{
			idata_sz = 0;
			idata_ptr = NULL;
		}


		// Apply relocations
		if (!RelocateImage(prep_img, reloc_ptr, reloc_sz, ofs))
		{
			ulAllocate = image.img_size;
			NtApi.NtFreeVirtualMemory(hProcess, (PVOID*)&new_base, &ulAllocate, MEM_RELEASE);
			NtApi.NtFreeVirtualMemory((HANDLE)-1, &prep_img, &ulAllocate, MEM_RELEASE);
			return false;
		}


		SIZE_T written = 0;

		
		// Copy to remote process
		if (0 != NtApi.NtWriteVirtualMemory(hProcess, (void*)new_base, prep_img, image.img_size, &written)
				|| written != image.img_size) 
		{
			ulAllocate = image.img_size;
			NtApi.NtFreeVirtualMemory(hProcess, (PVOID*)&new_base, &ulAllocate, MEM_RELEASE);
			NtApi.NtFreeVirtualMemory((HANDLE)-1, &prep_img, &ulAllocate, MEM_RELEASE);
			return false;
		}

		// TODO: Fix-up page access with VirtualProtectEx

		if (ppvNewBase)
			*ppvNewBase = (void*)new_base;

		if (pulImgSize)
			*pulImgSize = image.img_size;

		ulAllocate = image.img_size;
		NtApi.NtFreeVirtualMemory((HANDLE)-1, &prep_img, &ulAllocate, MEM_RELEASE);
		return true;
	}



	/**  
	 * \brief This is the real pre-entry point which finishes off DLL binding, which cannot be done remotely. 
	 *        We can only call to code which doesn't need to use any imported functions that aren't in kernel32.dll 
	 *        ie. no calls to printf, strcmp of whatever.
	 */
	DWORD __stdcall BindImageImports(HMODULE hImage)
	{
		HMODULE hKernel32 = FindModuleBase("kernel32.dll");

		if (!hKernel32)
			return 670;


		HMODULE(__stdcall* pfnLoadLibrary)(LPCSTR) = (HMODULE(__stdcall*)(LPCSTR))FindFnAddressByName(hKernel32, "LoadLibraryA");

		if (!pfnLoadLibrary)
			return 671;


		FARPROC(__stdcall* pfnGetProcAddress)(HMODULE,LPCSTR) = (FARPROC(__stdcall*)(HMODULE,LPCSTR))FindFnAddressByName(hKernel32, "GetProcAddress");

		if (!pfnGetProcAddress)
			return 669;

		VirtualProtectPtr pfnVirtualProtect = (VirtualProtectPtr)FindFnAddressByName(hKernel32, "VirtualProtect");


		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hImage;
		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);

		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
			|| pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
		{
			return false;
		}

		PIMAGE_IMPORT_DESCRIPTOR importDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		unsigned char* codeBase = (unsigned char*)hImage;

		for (; importDesc->Name; importDesc++) 
		{
			DWORD* thunkRef = 0;
			FARPROC *funcRef = 0;
			HMODULE handle;
				
			if (StringUtil::StrCmpIA((LPCSTR)(codeBase + importDesc->Name), "kernel32.dll"))
				handle = hKernel32;
			else
				handle = pfnLoadLibrary((LPCSTR)(codeBase + importDesc->Name));

			if (handle == INVALID_HANDLE_VALUE) 
				return 666;

			if (importDesc->OriginalFirstThunk) 
			{
				thunkRef = (DWORD*)(codeBase + importDesc->OriginalFirstThunk);
				funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
			} 
			else 
			{
				// no hint table
				thunkRef = (DWORD*)(codeBase + importDesc->FirstThunk);
				funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
			}

			for (; *thunkRef; thunkRef++, funcRef++) 
			{
				DWORD dwOldProt;
				pfnVirtualProtect(funcRef, 4, PAGE_EXECUTE_READWRITE, &dwOldProt);

				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
				{
					*funcRef = (FARPROC)pfnGetProcAddress(handle, (LPCSTR)IMAGE_ORDINAL(*thunkRef));
				} 
				else 
				{
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
					*funcRef = (FARPROC)pfnGetProcAddress(handle, (LPCSTR)&thunkData->Name);
				}

				if (*funcRef == 0) 
					return 673;

				pfnVirtualProtect(funcRef, 4, dwOldProt, &dwOldProt);
			}
		}

		return 0;
	}



	/**
	 * \brief 
	 */
	bool BindImportsForModule(HMODULE hBase, LPCSTR pImportModuleName, HMODULE hModBind)
	{
		if (!hBase || !hModBind || !pImportModuleName || pImportModuleName[0] == 0)
			return false;

		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hBase;
		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);

		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
			|| pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
		{
			return false;
		}

		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));
		NtApi.NtProtectVirtualMemory = (NtProtectVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return false;

		PIMAGE_IMPORT_DESCRIPTOR importDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		unsigned char* codeBase = (unsigned char*)hBase;
		bool fRet = true;

		for (; importDesc->Name; importDesc++) 
		{
			if (StringUtil::StrCmpIA((LPCSTR)(codeBase + importDesc->Name), pImportModuleName))
			{
				DWORD* thunkRef = 0;
				FARPROC *funcRef = 0;

				if (importDesc->OriginalFirstThunk) 
				{
					thunkRef = (DWORD*)(codeBase + importDesc->OriginalFirstThunk);
					funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
				} 
				else 
				{
					// no hint table
					thunkRef = (DWORD*)(codeBase + importDesc->FirstThunk);
					funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
				}

				for (; *thunkRef; thunkRef++, funcRef++) 
				{
					ULONG ulSize = 4;
					PVOID pvAddr = (PVOID)funcRef;
					DWORD dwOldProt;

					NtApi.NtProtectVirtualMemory((HANDLE)-1, &pvAddr, &ulSize, PAGE_EXECUTE_READWRITE, &dwOldProt);

					if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
					{
						*funcRef = (FARPROC)FindFnAddressByOrd(hModBind, IMAGE_ORDINAL(*thunkRef));
					} 
					else 
					{
						PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
						*funcRef = (FARPROC)FindFnAddressByName(hModBind, (LPCSTR)&thunkData->Name);
					}

					NtApi.NtProtectVirtualMemory((HANDLE)-1, &pvAddr, &ulSize, dwOldProt, &dwOldProt);

					fRet &= (*funcRef != 0);
				}

				break;
			}
		}

		return fRet;
	}


	/**  
	 * \brief 
	 */
	HMODULE GetDllHandleA(LPCSTR pszDllName)
	{
		wchar_t szDllNameW[256];
		StringUtil::AnsiToUnicode(szDllNameW, pszDllName, 256);

		return GetDllHandleW(szDllNameW);
	}


	/**  
	 * \brief 
	 */
	HMODULE GetDllHandleW(LPCWSTR pszDllName)
	{
		HMODULE hRet = 0;

		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.LdrGetDllHandle = (LdrGetDllHandlePtr)1;

		if (!GetNtApi(NtApi))
			return hRet;

		UNICODE_STRING ustr;
		StringUtil::InitUnicodeString(&ustr, (PWCHAR)pszDllName);

		NtApi.LdrGetDllHandle(0, 0, &ustr, (PHANDLE)&hRet);

		return hRet;
	}


	/**
	 * \brief 
	 */
	bool BindFnForModule(HMODULE hBase, LPCSTR pImportModuleName, LPCSTR pFnName, DWORD dwAddr)
	{
		if (!hBase || !pImportModuleName || !pFnName || pImportModuleName[0] == 0 || pFnName[0] == 0)
			return false;

		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hBase;
		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);

		if (pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE
			|| pNTHeader->Signature != IMAGE_NT_SIGNATURE
			|| pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
		{
			return false;
		}

		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));
		NtApi.NtProtectVirtualMemory = (NtProtectVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return false;


		PIMAGE_IMPORT_DESCRIPTOR importDesc = _PE_MakePtr(PIMAGE_IMPORT_DESCRIPTOR, pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		unsigned char* codeBase = (unsigned char*)hBase;

		for (; importDesc->Name; importDesc++) 
		{
			if (StringUtil::StrCmpIA((LPCSTR)(codeBase + importDesc->Name), pImportModuleName))
			{
				DWORD* thunkRef = 0;
				FARPROC *funcRef = 0;

				if (importDesc->OriginalFirstThunk) 
				{
					thunkRef = (DWORD*)(codeBase + importDesc->OriginalFirstThunk);
					funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
				} 
				else 
				{
					// no hint table
					thunkRef = (DWORD*)(codeBase + importDesc->FirstThunk);
					funcRef = (FARPROC*)(codeBase + importDesc->FirstThunk);
				}

				for (; *thunkRef; thunkRef++, funcRef++) 
				{
					const char* pszFuncName;

					if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
					{
						HMODULE hModBind = GetDllHandleA(pImportModuleName);
						pszFuncName = FindFnNameByOrd(hModBind, IMAGE_ORDINAL(*thunkRef));
					} 
					else 
					{
						PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)(codeBase + (*thunkRef));
						pszFuncName = (LPCSTR)&thunkData->Name;
					}

					if (pszFuncName && StringUtil::StrCmpA(pszFuncName, pFnName))
					{
						ULONG ulSize = 4;
						DWORD dwProtAddr = (DWORD)funcRef;
						DWORD dwOldProt;

						NtApi.NtProtectVirtualMemory((HANDLE)-1, (PVOID*)&dwProtAddr, &ulSize, PAGE_EXECUTE_READWRITE, &dwOldProt);

						*funcRef = (FARPROC)dwAddr;

						ulSize = 4;
						dwProtAddr = (DWORD)funcRef;
						NtApi.NtProtectVirtualMemory((HANDLE)-1, (PVOID*)&dwProtAddr, &ulSize, dwOldProt, &dwOldProt);
						return true;
					}
				}

				break;
			}
		}

		return false;
	}


	/**  
	 * \brief 
	 */
	int ListProcessModules(HANDLE hProcess, TModuleInfo* parrMods, ULONG ulCount)
	{
		void* peb = GetProcessPebAddress(hProcess);

		if (!peb)
			return 0;


		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.NtReadVirtualMemory = (NtReadVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return 0;

		int ret = 0;

		DWORD dwIO = 0;
		PPEB_LDR_DATA pLdrData = 0;
		ULONG ulSize = sizeof(PPEB_LDR_DATA);

		NtApi.NtReadVirtualMemory(hProcess, (BYTE*)peb + 12, &pLdrData, ulSize, &dwIO);

		PEB_LDR_DATA ldrData;
		ulSize = sizeof(PEB_LDR_DATA);
		NtApi.NtReadVirtualMemory(hProcess, (PVOID)pLdrData, &ldrData, ulSize, &dwIO);
		
		PLDR_MODULE pLdrMod = (PLDR_MODULE)ldrData.InLoadOrderModuleList.Flink;
		LDR_MODULE ldrMod;
		ulSize = sizeof(LDR_MODULE);
		NtApi.NtReadVirtualMemory(hProcess, (PVOID)pLdrMod, &ldrMod, ulSize, &dwIO);

		int i=0;

		while (ldrMod.BaseAddress != 0 && ulCount > 0)
		{
			ulSize = ldrMod.FullDllName.MaximumLength > sizeof(parrMods[i].szFullPath) ? sizeof(parrMods[i].szFullPath) : ldrMod.FullDllName.MaximumLength;
			NtApi.NtReadVirtualMemory(hProcess, (PVOID)ldrMod.FullDllName.Buffer, parrMods[i].szFullPath, ulSize, &dwIO);			

			ulSize = ldrMod.BaseDllName.MaximumLength > sizeof(parrMods[i].szDllName) ? sizeof(parrMods[i].szDllName) : ldrMod.BaseDllName.MaximumLength;
			NtApi.NtReadVirtualMemory(hProcess, (PVOID)ldrMod.BaseDllName.Buffer, parrMods[i].szDllName, ulSize, &dwIO);			

			parrMods[i].hBase = (HMODULE)ldrMod.BaseAddress;
			parrMods[i].SizeOfImage = ldrMod.SizeOfImage;
			parrMods[i].EntryPoint = ldrMod.EntryPoint;

			ulSize = sizeof(LDR_MODULE);
			NtApi.NtReadVirtualMemory(hProcess, (PVOID)ldrMod.InLoadOrderModuleList.Flink, &ldrMod, ulSize, &dwIO);

			ulCount--;
			i++;
		}

		return i;
	}


	/**  
	 * \brief 
	 */
	bool CopyImageCode(HANDLE hProcess)
	{
		void* peb = GetProcessPebAddress(hProcess);

		if (!peb)
			return false;


		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));

		NtApi.NtReadVirtualMemory = (NtReadVirtualMemoryPtr)1;

		if (!GetNtApi(NtApi))
			return 0;

		return true;
	}



	/**  
	 * \brief 
	 */
	DWORD_PTR FindCodeBytes(HMODULE hMod, BYTE* code, int len, int offs)
	{
		if (!hMod)
			hMod = (HMODULE)GetLocalProcessBaseAddress();

		if (!hMod || !code || len <= 0)
			return 0;

		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hMod;
		PIMAGE_NT_HEADERS pNTHeader = _PE_MakePtr(PIMAGE_NT_HEADERS, pDOSHeader, pDOSHeader->e_lfanew);
		
		PIMAGE_SECTION_HEADER h_sec = (PIMAGE_SECTION_HEADER)((DWORD)&pNTHeader->OptionalHeader + pNTHeader->FileHeader.SizeOfOptionalHeader);

		for (unsigned int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++) 
		{
			if ((h_sec[i].Characteristics & IMAGE_SCN_CNT_CODE) == IMAGE_SCN_CNT_CODE)
			{
				DWORD_PTR pPhysicalAddress = (DWORD_PTR)hMod + h_sec[i].VirtualAddress;
				int start = offs - pPhysicalAddress;

				if (start < 0)
					start = 0;

				for (int j=start; j <= ((int)h_sec[i].SizeOfRawData - len); j++)
				{
					if ((pPhysicalAddress + j) >= (DWORD_PTR)offs && StringUtil::MemCmp((void*)(pPhysicalAddress + j), code, len))
						return (DWORD_PTR)(pPhysicalAddress + j);					
				}
			}
		}

		return 0;
	}


	/**  
	 * \brief 
	 */
	HMODULE LdrLoadDllA(const char* pszDllName)
	{
		wchar_t wszDllName[MAX_PATH+1];
		StringUtil::MemSet(wszDllName, 0, sizeof(wszDllName));
		StringUtil::AnsiToUnicode(wszDllName, pszDllName, MAX_PATH);

		UNICODE_STRING str;
		StringUtil::InitUnicodeString(&str, wszDllName);

		TNtApiTable NtApi;
		StringUtil::MemSet(&NtApi, 0, sizeof(NtApi));
		NtApi.LdrLoadDll = (LdrLoadDllPtr)1;
		
		if (!GetNtApi(NtApi))
			return 0;

		HANDLE hRet = 0;
		NtApi.LdrLoadDll(0, 0, &str, &hRet);

		return (HMODULE)hRet;
	}

}

#undef _PE_MakePtr
