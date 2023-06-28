#pragma once

#define _CRT_SECURE_NO_WARNINGS


#include "..\_Shared\MemoryModule.h"
#include "..\_Shared\LoaderInterface.h"

#define _MAX_MU_INSTANCES 256


/**  
 * \brief 
 */
struct MuProcInfo
{
	DWORD dwPid;
	HWND  hMainWnd;
	int iInstance;
};



/**  
 * \brief 
 */
class CModuleLoader : public IModuleLoader
{
public:
	CModuleLoader();

	virtual bool GetRootDir(char* szRootDir, int count);
	virtual bool SendCommand(int iCommand, int target, void* pParam, void** pResult);

	static bool OnDllInit(HMODULE hInstDll, InstallerInitStruct* pInit);
	static void OnDllTerm();

	static bool Init();
	static bool Term();

	static bool Load();

	static bool RegMuWindow(HWND hWnd);

	IClickerModule* GetProxy(){ return m_pProxyModule; }
	IClickerModule* GetGui(){ return m_pGuiModule; }

	bool IsMuWindow(HWND hWnd);

	const InstallerServiceTable* GetInstallerService() const { return &m_ServiceTable; }

protected:
	bool LoadMyCode();
	bool UnloadMyCode();

	static bool AddProcInfo(MuProcInfo& info);
	static int PackProcInfos(int* iFirstFreeInstanceNumber = 0);
	static int RemoveProcInfo();

	bool GetMuProcInfo(HWND* phWndMu, int* piInstanceNum);

	HWND GetNextMuWindow();
	void ShowMuWindows(bool fShow);

	void PatchFunctions(bool fPatch = true);

private:
	HMEMORYMODULE m_hProxy;
	HMEMORYMODULE m_hGui;

	IClickerModule* m_pProxyModule;
	IClickerModule* m_pGuiModule;

	HMODULE m_hModule;
	HANDLE m_hInstMutex;
	bool m_fCodeLoaded;

	HANDLE m_hFileMapping;
	MuProcInfo* m_vMuProcesses;

	char m_szRootDir[512];
	InstallerServiceTable m_ServiceTable;
};


extern CModuleLoader gModuleLoader;

