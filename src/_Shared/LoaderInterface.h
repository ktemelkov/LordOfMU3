#pragma once


#define _CLICKER_MODULE_COMMAND_CREATE_PROXY 1
#define _CLICKER_MODULE_COMMAND_SET_VERSION  2
#define _CLICKER_MODULE_COMMAND_SEND_PACKET  3
#define _CLICKER_MODULE_COMMAND_RECV_PACKET  4
#define _CLICKER_MODULE_COMMAND_SEND_COMMAND 5
#define _CLICKER_MODULE_COMMAND_REG_MU_WND   6
#define _CLICKER_MODULE_COMMAND_LOGIN		 7
#define _CLICKER_MODULE_COMMAND_SET_FEATURES 8
#define _CLICKER_MODULE_COMMAND_CONNECT		 9

#define _MODULE_LOADER_COMMAND_GET_MUWND	1
#define _MODULE_LOADER_COMMAND_NEXT_MUWND	2
#define _MODULE_LOADER_COMMAND_SHOW_MUWND	3
#define _MODULE_LOADER_COMMAND_GET_INST_COUNT 4
#define _MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES 5
#define _MODULE_LOADER_COMMAND_REG_MUWND	6

#define _MODULE_LOADER_TARGET_SELF	0
#define _MODULE_LOADER_TARGET_PROXY 1
#define _MODULE_LOADER_TARGET_GUI	2


// #define _API_HOOK_EXITPROCESS				1
// #define _API_HOOK_TERMINATEPROCESS			2
#define _API_HOOK_GETASYNCKEYSTATE			3
#define _API_HOOK_CHANGEDISPLAYSETTINGSA	4
#define _API_HOOK_GETFOREGROUNDWINDOW		5
// #define _API_HOOK_SHELLEXECUTEA				6
#define _API_HOOK_WGLSWAPBUFFERS			7
#define _API_HOOK_WS2_SEND					8
#define _API_HOOK_WS2_RECV					9
#define _API_HOOK_MAINWND_PROC				10
#define _API_HOOK_GETASYNCKEYSTATE_FILTER	11


/**  
 * \brief 
 */
struct IClickerModule
{
	virtual bool SendCommand(int iCommand, void* pParam, void** pResult) = 0;
};


/**  
 * \brief 
 */
struct IModuleLoader
{
	virtual bool GetRootDir(char* szRootDir, int count) = 0;
	virtual bool SendCommand(int iCommand, int target, void* pParam, void** pResult) = 0;
};


/**  
 * \brief 
 */
struct LoaderInitStruct
{
	DWORD				dwCookie;
	IModuleLoader*		pLoader;
	IClickerModule**	ppMoudle;
};


typedef SHORT (WINAPI *GetAsyncKeyStateHookPtr)(int vKey, int state);


/**  
 * \brief 
 */
struct InstallerServiceTable
{
	bool (*SetApiHook)(int fnIdx, PVOID pvAddr, PVOID* pvOrigAddr);
	bool (*ClearApiHook)(int fnIdx);
	bool (*IsGameStarted)();
	HWND (*GetGameWindow)();
	void (*CheckWsHooks)();
};



/**  
 * \brief 
 */
struct InstallerInitStruct
{
	const char* pszRootDir;
	InstallerServiceTable* pServices;
};