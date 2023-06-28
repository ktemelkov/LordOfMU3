#pragma once


#include "..\_Shared\LoaderInterface.h"

class CProxy;


/**  
 * \brief 
 */
class CProxyClickerModule : public IClickerModule
{
public:
	CProxyClickerModule();

	bool Init(LoaderInitStruct* pInit);
	bool Term();

	static CProxyClickerModule* GetInstance();

	virtual bool SendCommand(int iCommand, void* pParam, void** pResult);

public:
	IModuleLoader* m_pLoader;
	DWORD m_dwCookie;

	CProxy* m_pGameProxy;
};

