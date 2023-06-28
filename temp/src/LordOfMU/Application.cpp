#include "stdafx.h"
#include "Application.h"
#include "DebugMonitor.h"
#include "MainWnd.h"
#include "../_Shared/UserUtil.h"
#include "Debugger.h"
#include "Loader.h"


/**  
 * \brief 
 */
CApplication::CApplication()
{
	m_cSettings.Load();
}


/**  
 * \brief 
 */
CApplication::~CApplication()
{
}



/**  
 * \brief 
 */
int CApplication::Run()
{
	CMainWnd wnd(m_cSettings);

	if (!wnd.Create())
		return -1;

	MSG msg = {0};

	while (CUserUtil::GetMessageA(&msg, 0, 0, 0) > 0)
	{
		if (!CUserUtil::IsDialogMessageA(wnd.m_hWnd, &msg))
		{
			CUserUtil::TranslateMessage(&msg);
			CUserUtil::DispatchMessageA(&msg);
		}
	}

	m_cSettings.Save();

	if (wnd.GetResult() == IDOK)
	{
#ifndef _DEBUG
		CDebugger dbg;
		dbg.Run();
#else
		CLoader::Load();
#endif
	}

	return 0;
}


