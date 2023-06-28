#ifndef __StdCallThunk_H
#define __StdCallThunk_H

#pragma once


/**  
 * \brief
 */
#pragma pack(push)
#pragma pack(1)
struct TStdCallThunk
{
	BYTE mov_ecx;
	DWORD this_addr;
	BYTE jmp_opcode;
	DWORD rel_addr;
};
#pragma pack(pop)



/**  
 * \brief 
 */
#define STD_CALL_THUNK(ClassName,MethodName,FnType) \
struct ThunkFor_##ClassName##_##MethodName { \
	ThunkFor_##ClassName##_##MethodName(PVOID pThis) { \
		PVOID pfn = 0; \
		__asm { push eax }\
		__asm { mov eax, ClassName::MethodName }\
		__asm { mov pfn, eax }\
		__asm { pop eax }\
		pvThunk = CreateStdCallThunk(pThis,pfn); \
	} \
	virtual ~ThunkFor_##ClassName##_##MethodName(){ DeleteStdCallThunk(pvThunk); } \
	operator FnType() { return (FnType)pvThunk;} \
	PVOID pvThunk; \
	private: static void* operator new(size_t){}\
}


/**  
 * \brief 
 */
inline PVOID CreateStdCallThunk(PVOID pThis, PVOID pfn)
{
	DWORD dwOldProt;
	TStdCallThunk* pThunk = (TStdCallThunk*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(TStdCallThunk));

	if (!pThunk)
		return 0;

	if (!VirtualProtect(pThunk, sizeof(TStdCallThunk), PAGE_EXECUTE_READWRITE, &dwOldProt))
	{
		HeapFree(GetProcessHeap(), 0, pThunk);
		return 0;
	}

	pThunk->mov_ecx = 0xB9;
	pThunk->this_addr = (DWORD)pThis;
	pThunk->jmp_opcode = 0xE9;
	pThunk->rel_addr = (DWORD)pfn - (DWORD)&pThunk->jmp_opcode - 5;

	return pThunk;
}

/**  
 * \brief 
 */
inline void DeleteStdCallThunk(PVOID pThunk)
{
	if (pThunk)
		HeapFree(GetProcessHeap(), 0, pThunk);
}


#endif