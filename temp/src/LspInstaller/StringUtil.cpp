#include "StringUtil.h"


/**  
 * \brief 
 */
namespace StringUtil
{
	/**  
	 * \brief 
	 */
	bool StrCmpA(const char* str1, const char* str2)
	{
		bool fEqual;
		char temp;
	
		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else
		{
			fEqual = false;

			for (int j=0; str1[j] != 0; j++)
			{
				fEqual = true;
				temp = str1[j];

				if (str2[j] == 0 || temp != str2[j] || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief 
	 */
	bool StrCmpW(const wchar_t* str1, const wchar_t* str2)
	{
		bool fEqual;
		wchar_t temp;
	
		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else
		{
			fEqual = false;

			for (int j=0; str1[j] != 0; j++)
			{
				fEqual = true;
				temp = str1[j];

				if (str2[j] == 0 || temp != str2[j] || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief 
	 */
	bool StrCmp(const wchar_t* str1, const char* str2)
	{
		bool fEqual;
		char temp;
	
		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && (char)str2[0] == 0)
		{
			fEqual = true;
		}
		else
		{
			fEqual = false;

			for (int j=0; str1[j] != 0; j++)
			{
				fEqual = true;
				temp = (char)str1[j];

				if (str2[j] == 0 || temp != str2[j] || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}

	/**  
	 * \brief 
	 */
	bool StrCmpIA(const char* str1, const char* str2)
	{
		bool fEqual; 
		char temp; 
		char temp2; 

		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else 
		{ 
			fEqual = false;

			for (int j=0; str1[j] != 0; j++) 
			{ 
				fEqual = true; 

				temp = str1[j]; 
				temp2 = str2[j]; 

				if (temp >= 0x61 && temp <= 0x7A)
					temp = temp - 0x20;

				if (temp2 >= 0x61 && temp2 <= 0x7A)
					temp2 = temp2 - 0x20;

				if (str2[j] == 0 || temp != temp2 || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief 
	 */
	bool StrCmpIW(const wchar_t* str1, const wchar_t* str2)
	{
		bool fEqual; 
		wchar_t temp; 
		wchar_t temp2; 

		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else 
		{ 
			fEqual = false;

			for (int j=0; str1[j] != 0; j++) 
			{ 
				fEqual = true; 

				temp = str1[j]; 
				temp2 = str2[j]; 

				if (temp >= 0x61 && temp <= 0x7A)
					temp = temp - 0x20;

				if (temp2 >= 0x61 && temp2 <= 0x7A)
					temp2 = temp2 - 0x20;

				if (str2[j] == 0 || temp != temp2 || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}



	/**  
	 * \brief 
	 */
	bool StrCmpNIW(const wchar_t* str1, const wchar_t* str2, int cmp_len)
	{
		bool fEqual; 
		wchar_t temp; 
		wchar_t temp2; 

		if (!str1 || !str2 || cmp_len <= 0)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else 
		{ 
			fEqual = false;

			for (int j=0; str1[j] != 0 && j < cmp_len; j++) 
			{ 
				fEqual = true; 

				temp = str1[j]; 
				temp2 = str2[j]; 

				if (temp >= 0x61 && temp <= 0x7A)
					temp = temp - 0x20;

				if (temp2 >= 0x61 && temp2 <= 0x7A)
					temp2 = temp2 - 0x20;

				if (str2[j] == 0 || temp != temp2 || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief 
	 */
	bool StrCmpNIA(const char* str1, const char* str2, int cmp_len)
	{
		bool fEqual; 
		char temp; 
		char temp2; 

		if (!str1 || !str2 || cmp_len <= 0)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && str2[0] == 0)
		{
			fEqual = true;
		}
		else 
		{ 
			fEqual = false;

			for (int j=0; str1[j] != 0 && j < cmp_len; j++) 
			{ 
				fEqual = true; 

				temp = str1[j]; 
				temp2 = str2[j]; 

				if (temp >= 0x61 && temp <= 0x7A)
					temp = temp - 0x20;

				if (temp2 >= 0x61 && temp2 <= 0x7A)
					temp2 = temp2 - 0x20;

				if (str2[j] == 0 || temp != temp2 || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief 
	 */
	bool StrCmpI(const char* str1, const wchar_t* str2)
	{
		bool fEqual; 
		char temp; 
		char temp2; 

		if (!str1 || !str2)
		{
			fEqual = false;
		}
		else if (str1[0] == 0 && (char)str2[0] == 0)
		{
			fEqual = true;
		}
		else 
		{ 
			fEqual = false;

			for (int j=0; str1[j] != 0; j++) 
			{ 
				fEqual = true; 

				temp = str1[j]; 
				temp2 = (char)str2[j]; 

				if (temp >= 0x61 && temp <= 0x7A)
					temp = temp - 0x20;

				if (temp2 >= 0x61 && temp2 <= 0x7A)
					temp2 = temp2 - 0x20;

				if (str2[j] == 0 || temp != temp2 || (str1[j+1] == 0 && str2[j+1] != 0))
				{
					fEqual = false;
					break;
				}
			}
		}

		return fEqual;
	}


	/**  
	 * \brief Unicode string length calculator
	 */
	unsigned long StrLenW(const wchar_t* String)
	{
		unsigned long Length = 0;

		while (String && String[Length]) Length++;

		return Length;
	}


	/**  
	 * \brief ANSI string length calculator
	 */
	unsigned long StrLenA(const char* String)
	{
		unsigned long Length = 0;

		while (String && String[Length]) Length++;

		return Length;
	}


	/**  
	 * \brief Initialize a UNICODE string
	 */
	void InitUnicodeString(PUNICODE_STRING UnicodeString, PWCHAR String)
	{
		UnicodeString->Buffer        = String;
		UnicodeString->Length        = (USHORT)(StringUtil::StrLenW(String) * 2);
		UnicodeString->MaximumLength = UnicodeString->Length;
	}


	/**  
	 * \brief Initializes an ANSI string
	 */
	void InitAnsiString(PANSI_STRING AnsiString, PCHAR String)
	{
		AnsiString->Buffer        = String;
		AnsiString->Length        = (USHORT)StringUtil::StrLenA(String);
		AnsiString->MaximumLength = AnsiString->Length;
	}


	/**  
	 * \brief 
	 */
	void MemCopy(void* pDst, void* pSrc, long len)
	{
		long rem = (len % 4);
		long len1 = len - rem;
		DWORD* dst = (DWORD*)pDst;
		DWORD* src = (DWORD*)pSrc;

		while ((len1-=4) >= 0)
			*(dst++) = *(src++);

		len1 = len - rem;

		while (rem-- > 0)
		{
			*((char*)pDst + len1) = *((char*)pSrc + len1++);
		}
	}


	/**  
	 * \brief 
	 */
	bool MemCmp(void* pDst, void* pSrc, long len)
	{
		BYTE* dst = (BYTE*)pDst;
		BYTE* src = (BYTE*)pSrc;

		for (int i=0; i < len; i++)
		{
			if (dst[i] != src[i])
				return false;
		}

		return true;
	}


	/**  
	 * \brief 
	 */
	void StrCopyA(char* str1, const char* str2)
	{
		if (!str1 || !str2)
			return;

		StringUtil::MemCopy(str1, (void*)str2, StringUtil::StrLenA(str2)+1);
	}

	/**  
	 * \brief 
	 */
	void StrCopyW(wchar_t* str1, const wchar_t* str2)
	{
		if (!str1 || !str2)
			return;

		StringUtil::MemCopy(str1, (void*)str2, StringUtil::StrLenW(str2)*2+2);
	}


	/**  
	 * \brief 
	 */
	void StrConcatA(char* str1, char* str2)
	{
		if (!str1 || !str2)
			return;

		StringUtil::MemCopy(str1 + StringUtil::StrLenA(str1), str2, StringUtil::StrLenA(str2)+1);
	}


	/**  
	 * \brief 
	 */
	int StrFindIA(const char* str1, const char* str2)
	{
		int len1 = StringUtil::StrLenA(str1);
		int len2 = StringUtil::StrLenA(str2);

		for (int i=0; i < len1 - len2 + 1; i++)
		{
			if (StringUtil::StrCmpNIA(str1+i, str2, len2))
				return i;
		}

		return -1;
	}


		/**  
	 * \brief 
	 */
	int StrFindIW(const wchar_t* str1, const wchar_t* str2)
	{
		int len1 = StringUtil::StrLenW(str1);
		int len2 = StringUtil::StrLenW(str2);

		for (int i=0; i < len1 - len2 + 1; i++)
		{
			if (StringUtil::StrCmpNIW(str1+i, str2, len2))
				return i;
		}

		return -1;
	}

	/**  
	 * \brief 
	 */
	void MemSet(void* pDst, unsigned char set, long len)
	{
		unsigned char* dst;
		dst = (unsigned char*)pDst;

		while (len-- > 0)
		{
			*(dst++) = set;
		}
	}

	/**  
	 * \brief 
	 */
	void AnsiToUnicode(wchar_t* pszDst, const char* pszSrc, int dstSize, int srcSize)
	{
		int len = (srcSize > -1) ? srcSize : (StringUtil::StrLenA(pszSrc) + 1);

		if (len > dstSize)
		{
			len = dstSize-1;

			if (dstSize > 0)
				pszDst[len] = 0;
		}

		for (int i=0; i < len; i++)
			pszDst[i] = (wchar_t)pszSrc[i];
	}


	/**  
	 * \brief 
	 */
	void UnicodeToAnsi(char* pszDst, const wchar_t* pszSrc, int dstSize, int srcSize)
	{
		int len = (srcSize > -1) ? srcSize : StringUtil::StrLenW(pszSrc) + 1;

		if (len > dstSize)
		{
			len = dstSize-1;

			if (dstSize > 0)
				pszDst[len] = 0;
		}

		for (int i=0; i < len; i++)
			pszDst[i] = (char)pszSrc[i];
	}

}
