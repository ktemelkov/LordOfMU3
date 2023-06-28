#pragma once

#include <objbase.h>
#include "ApiTable.h"


/**  
 * \brief 
 */
class COleUtil
{
public:

	/**  
	 * \brief 
	 */
	static int StringFromGUID2(REFGUID guid, LPOLESTR strGuid, int cch)
	{
		int (__stdcall* proc)(REFGUID,LPOLESTR,int) = 
			(int(__stdcall*)(REFGUID,LPOLESTR,int)) CApiTable::GetProc(_OLE32_API_StringFromGUID2);

		return proc(guid, strGuid, cch);
	}

	/**  
	 * \brief 
	 */
	static long UuidCreateSequential(UUID* Uuid)
	{
		long (__stdcall* proc)(UUID*) = 
			(long(__stdcall*)(UUID*)) CApiTable::GetProc(_RPC_API_UuidCreateSequential);

		return proc(Uuid);
	}

};