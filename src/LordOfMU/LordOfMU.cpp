// LordOfMU.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "LordOfMU.h"
#include "../_Shared/KernelUtil.h"
#include "InstanceLock.h"
#include "Loader.h"
#include "Application.h"
#include "Debugger.h"
#include "objbase.h"



// Global Variables
HINSTANCE ghInstance = 0;


/**  
 * \brief 
 */
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int)
{
	int ret = -1;
	ghInstance = hInstance;
	CoInitialize(0);

	CInstanceLock<_STRING_MuAutoClickerExecMutex> cInstLock;

	if (!cInstLock.IsFirst())
	{
		ret = CLoader::FakeLoad();
	}
	else
	{
		CApplication app;
		ret = app.Run();
	}

	CoUninitialize();
	return ret;
}
