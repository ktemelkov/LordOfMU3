#pragma once


#define BYTE_TYPE(x) __asm _emit x  
#define WORD_TYPE(x) BYTE_TYPE((x>>(0*8))&0xFF)    BYTE_TYPE((x>>(1*8))&0xFF) 
#define DWORD_TYPE(x) BYTE_TYPE((x>>(0*8))&0xFF) BYTE_TYPE((x>>(1*8))&0xFF) BYTE_TYPE((x>>(2*8))&0xFF) BYTE_TYPE((x>>(3*8))&0xFF) 

#define LDR_CODE_START 0xAFAFAFAF 
#define LDR_CODE_END 0xFAFAFAFA 
#define LDR_DATA_MARKER 0xDEADBEEF



/**  
 * \brief 
 */
class CInjectUtil
{
public:

	/**  
	 * \brief 
	 */
	static PVOID SearchDWORD(PVOID Start, DWORD dwSearch, int limit = -1) 
	{ 
		register PVOID pAddr = Start; 

		if (limit == -1)
		{
			while(*(PDWORD)pAddr != dwSearch) 
				((PBYTE&)pAddr)++;
		}
		else
		{
			while(*(PDWORD)pAddr != dwSearch && limit-- >= 4) 
				((PBYTE&)pAddr)++;

			if (*(PDWORD)pAddr != dwSearch)
				pAddr = 0;
		}

		return pAddr; 
	} 
};

