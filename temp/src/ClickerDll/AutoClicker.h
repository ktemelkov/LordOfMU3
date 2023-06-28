#ifndef __AutoClicker_H
#define __AutoClicker_H

#pragma once

#include <set>
#include "..\_Shared\LoaderInterface.h"


class CMuWindow;

/**
 * \brief Global ATL Module Class
 */
class CAutoClickerModule : public IClickerModule, public CAtlDllModuleT< CAutoClickerModule >
{
public:
	CAutoClickerModule() 
		: m_hMuWindow(0), m_pLoader(0), m_dwCookie(0), m_dwMainThreadId(0)
	{
	}

	BOOL InitClicker();
	void TermClicker();

	virtual bool SendCommand(int iCommand, void* pParam, void** pResult);

public:
	IModuleLoader* m_pLoader;
	DWORD m_dwCookie;
	HWND m_hMuWindow;
	DWORD m_dwMainThreadId;
};


extern CAutoClickerModule _AtlModule;


#endif //__AutoClicker_H