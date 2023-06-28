#pragma once


#include <Windows.h>
#include "Winternl.h"

#ifdef StrCmpIA
#undef StrCmpIA
#undef StrCmpIW
#endif 

#ifdef StrCmpA
#undef StrCmpA
#undef StrCmpW
#endif 


/**  
 * \brief 
 */
namespace StringUtil
{
	bool StrCmpA(const char* str1, const char* str2);
	bool StrCmpW(const wchar_t* str1, const wchar_t* str2);
	bool StrCmp(const wchar_t* str1, const char* str2);
	bool StrCmpIA(const char* str1, const char* str2);
	bool StrCmpIW(const wchar_t* str1, const wchar_t* str2);
	bool StrCmpNIW(const wchar_t* str1, const wchar_t* str2, int cmp_len);
	bool StrCmpI(const char* str1, const wchar_t* str2);
	unsigned long StrLenW(const wchar_t* String);
	unsigned long StrLenA(const char* String);
	void InitUnicodeString(PUNICODE_STRING UnicodeString, PWCHAR String);
	void InitAnsiString(PANSI_STRING AnsiString, PCHAR String);
	void MemCopy(void* pDst, void* pSrc, long len);
	bool MemCmp(void* pDst, void* pSrc, long len);
	void StrCopyA(char* str1, const char* str2);
	void StrCopyW(wchar_t* str1, const wchar_t* str2);
	void StrConcatA(char* str1, char* str2);
	int StrFindIA(const char* str1, const char* str2);
	int StrFindIW(const wchar_t* str1, const wchar_t* str2);
	void MemSet(void* pDst, unsigned char set, long len);
	void AnsiToUnicode(wchar_t* pszDst, const char* pszSrc, int dstSize, int srcSize = -1);
	void UnicodeToAnsi(char* pszDst, const wchar_t* pszSrc, int dstSize, int srcSize = -1);
}