#ifndef __PEUtil_H
#define __PEUtil_H

#pragma once


/**  
 * \brief 
 */
namespace PeUtil
{
	template<class T, class B, class O>
	inline T MakePtr(B Base, O Offset) 
	{
		return ((T)(DWORD_PTR(Base) + (DWORD_PTR)(Offset)));
	}


	/**  
	 * \brief 
	 */
	inline void WipeDosHeader(HMODULE hModule)
	{
		if (!hModule)
			return;

		DWORD rOffs = (GetTickCount() % (sizeof(IMAGE_DOS_HEADER) - 8)) + 2;
		BYTE* code = (BYTE*)hModule;

		DWORD oldProt = 0;
		VirtualProtect(code, ((PIMAGE_DOS_HEADER)code)->e_lfanew + 4, PAGE_READWRITE, &oldProt);
		memset(code, 0, sizeof(IMAGE_DOS_HEADER) - 4);

		int iFillSize = ((PIMAGE_DOS_HEADER)code)->e_lfanew - sizeof(IMAGE_DOS_HEADER) + 4;

		if (iFillSize > 0)
			memset(code + sizeof(IMAGE_DOS_HEADER), 0, iFillSize);

		*((WORD*)(code + rOffs)) = 'MZ';

		VirtualProtect(code, ((PIMAGE_DOS_HEADER)code)->e_lfanew + 4, oldProt, &oldProt);
	}


	/**  
	 * \brief 
	 */
	inline void WipeResSection(HMODULE hModule)
	{
		if (!hModule)
			return;

		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS pNTHeader = MakePtr<PIMAGE_NT_HEADERS>(pDOSHeader, pDOSHeader->e_lfanew);

		PIMAGE_SECTION_HEADER h_sec = (PIMAGE_SECTION_HEADER)((DWORD)&pNTHeader->OptionalHeader + pNTHeader->FileHeader.SizeOfOptionalHeader);

		for (unsigned int i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++) 
		{
			if (h_sec[i].SizeOfRawData == 0)
				continue;

			char secName[10] = {0};
			memcpy(secName, h_sec[i].Name, IMAGE_SIZEOF_SHORT_NAME);

			if (_strcmpi(secName, ".rsrc") == 0)
			{
				DWORD_PTR pPhysicalAddress = (DWORD_PTR)hModule + h_sec[i].VirtualAddress;

				DWORD oldProt = 0;
				VirtualProtect((PVOID)pPhysicalAddress, h_sec[i].SizeOfRawData, PAGE_EXECUTE_READWRITE, &oldProt);

				memset((PVOID)pPhysicalAddress, 0, h_sec[i].SizeOfRawData);

				VirtualProtect((PVOID)pPhysicalAddress, h_sec[i].SizeOfRawData, oldProt, &oldProt);
			}
		}
	}


	/**  
	 * \brief 
	 */
	inline void WipeImportTable(HMODULE hModule)
	{
		if (!hModule)
			return;


		PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
		PIMAGE_NT_HEADERS pNTHeader = MakePtr<PIMAGE_NT_HEADERS>(pDOSHeader, pDOSHeader->e_lfanew);
		PIMAGE_IMPORT_DESCRIPTOR pImportDesc = MakePtr<PIMAGE_IMPORT_DESCRIPTOR>(pDOSHeader, pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		DWORD dwTableSize = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;


		if (dwTableSize == 0
			|| pImportDesc == (PIMAGE_IMPORT_DESCRIPTOR)pNTHeader)
		{
			return;
		}

		DWORD oldProt = 0;
		VirtualProtect(pImportDesc, dwTableSize, PAGE_EXECUTE_READWRITE, &oldProt);

		while (pImportDesc->Name)
		{
			LPCSTR pszDllName = MakePtr<LPCSTR>(pDOSHeader, pImportDesc->Name);

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
				char* pszProcName = 0;

				if (IMAGE_SNAP_BY_ORDINAL(*thunkRef)) 
				{

				} 
				else 
				{
					PIMAGE_IMPORT_BY_NAME thunkData = (PIMAGE_IMPORT_BY_NAME)((DWORD)pDOSHeader + (*thunkRef));
					pszProcName = (LPSTR)&thunkData->Name;

					int len = strlen(pszProcName);
					memset(pszProcName, 0, len);
				}
			}

			pImportDesc++;
		}

		VirtualProtect(pImportDesc, dwTableSize, oldProt, &oldProt);
	}
};


#endif //__PEUtil_H