// MACLocator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ModuleLoader.h"
#include "..\_Shared\MuUtil.h"
#include "..\_Shared\StringTable.h"
#include "..\_Shared\LspUtil.h"
#include "..\_Shared\PackUtil.h"
#include <set>
#include "Winternl.h"



CModuleLoader gModuleLoader;

#define _MAX_LSP_IDLE_TIME 60000



#pragma comment(lib, "shlwapi.lib")

#define UPPERCASE(x) if((x) >= 'a' && (x) <= 'z') (x) -= 'a' - 'A'
#define UNLINK(x) (x).Blink->Flink = (x).Flink; \
	(x).Flink->Blink = (x).Blink;

#pragma pack(push, 1)


typedef struct _ModuleInfoNode
{
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
	HMODULE baseAddress;		//	Base address AKA module handle
	unsigned long entryPoint;
	unsigned int size;			//	Size of the modules image
	UNICODE_STRING fullPath;
	UNICODE_STRING name;
	unsigned long flags;
	unsigned short LoadCount;
	unsigned short TlsIndex;
	LIST_ENTRY HashTable;	//	A linked list of any other modules that have the same first letter
	unsigned long timestamp;
} ModuleInfoNode, *pModuleInfoNode;

typedef struct _ProcessModuleInfo
{
	unsigned int size;			//	Size of a ModuleInfo node?
	unsigned int initialized;
	HANDLE SsHandle;
	LIST_ENTRY LoadOrder;
	LIST_ENTRY InitOrder;
	LIST_ENTRY MemoryOrder;
} ProcessModuleInfo, *pProcessModuleInfo;


#pragma pack(pop)


/**  
 * \brief 
 */
CModuleLoader::CModuleLoader() 
	: m_hProxy(0), m_hGui(0), m_pProxyModule(0)
	, m_pGuiModule(0), m_fInsideMuProcess(false), m_fInsideMuRelatedProc(false)
	, m_hModule(0), m_fCodeLoaded(false)
{
	memset(m_szRootDir, 0, sizeof(m_szRootDir));
	memset(&m_ServiceTable, 0, sizeof(m_ServiceTable));
}


/**  
 * \brief 
 */
bool CModuleLoader::Init(HMODULE hInstDll, InstallerInitStruct* pInit)
{	
	gModuleLoader.m_hFileMapping = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(MuProcInfo)*_MAX_MU_INSTANCES, L"__Instances__");

	if (gModuleLoader.m_hFileMapping != NULL && gModuleLoader.m_hFileMapping != INVALID_HANDLE_VALUE)
	{
		gModuleLoader.m_vMuProcesses = (MuProcInfo*) MapViewOfFile(gModuleLoader.m_hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(MuProcInfo)*_MAX_MU_INSTANCES);

		if (!gModuleLoader.m_vMuProcesses)
		{
			CloseHandle(gModuleLoader.m_hFileMapping);
			gModuleLoader.m_hFileMapping = 0;
		}
	}
	else
	{
		gModuleLoader.m_hFileMapping = 0;
		gModuleLoader.m_vMuProcesses = 0;
	}


	gModuleLoader.m_hModule = hInstDll;
	gModuleLoader.m_fInsideMuProcess = true;//CMuUtil::IsMuProcess();
	gModuleLoader.m_fInsideMuRelatedProc = true;//CMuUtil::IsMuRelatedProcess();
	gModuleLoader.m_hInstMutex = CreateMutexA(0, FALSE, CStringTable::GetString(_STRING_LordOfMU_LspDllName).c_str());
	gModuleLoader.m_ServiceTable = *pInit->pServices;

	strcpy(gModuleLoader.m_szRootDir, pInit->pszRootDir);
	strcat(gModuleLoader.m_szRootDir, CStringTable::GetString(_STRING_LordOfMU_BootstrapperName).c_str());

	return true;
}


/**  
 * \brief 
 */
bool CModuleLoader::Load()
{
	if (gModuleLoader.m_fInsideMuProcess && !gModuleLoader.m_fCodeLoaded)
	{
		gModuleLoader.m_fCodeLoaded = true;

		MuProcInfo info = {0};
		info.dwPid = GetCurrentProcessId();

		CModuleLoader::AddProcInfo(info);

		gModuleLoader.LoadMyCode();
	}

	return true;
}


/**  
 * \brief 
 */
bool CModuleLoader::Term()
{
	if (gModuleLoader.m_fInsideMuProcess)
	{
		gModuleLoader.UnloadMyCode();
		gModuleLoader.m_fInsideMuProcess = false;

		CModuleLoader::RemoveProcInfo();
	}

	if (gModuleLoader.m_hInstMutex != 0 && gModuleLoader.m_hInstMutex != INVALID_HANDLE_VALUE)
	{
		CloseHandle(gModuleLoader.m_hInstMutex);
		gModuleLoader.m_hInstMutex = 0;
	}

	if (gModuleLoader.m_vMuProcesses != 0)
	{
		UnmapViewOfFile((LPCVOID)gModuleLoader.m_vMuProcesses);
		gModuleLoader.m_vMuProcesses = 0;

		CloseHandle(gModuleLoader.m_hFileMapping);
		gModuleLoader.m_hFileMapping = 0;
	}
	return true;
}



/**  
 * \brief 
 */
bool CModuleLoader::AddProcInfo(MuProcInfo& info)
{
	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return false;

	int iInstance = 0;
	int pos = PackProcInfos(&iInstance);
	gModuleLoader.m_vMuProcesses[pos] = info;
	gModuleLoader.m_vMuProcesses[pos].iInstance = iInstance;

	ReleaseMutex(gModuleLoader.m_hInstMutex);
	return true;
}


/**  
 * \brief 
 */
int CModuleLoader::PackProcInfos(int* iFirstFreeInstanceNumber)
{
	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return 0;

	int pos = 0;
	std::set<int> vInst;

	for (int i=0; i < _MAX_MU_INSTANCES; i++)
	{
		if (gModuleLoader.m_vMuProcesses[i].dwPid && GetProcessVersion(gModuleLoader.m_vMuProcesses[i].dwPid) != 0)
		{
			gModuleLoader.m_vMuProcesses[pos].dwPid = gModuleLoader.m_vMuProcesses[i].dwPid;
			gModuleLoader.m_vMuProcesses[pos++].hMainWnd = gModuleLoader.m_vMuProcesses[i].hMainWnd;

			vInst.insert(gModuleLoader.m_vMuProcesses[i].iInstance);
		}
		else
		{
			memset(&gModuleLoader.m_vMuProcesses[i], 0, sizeof(MuProcInfo));
		}
	}

	if (iFirstFreeInstanceNumber)
	{
		int iInst = 1;
		for (iInst; iInst < (_MAX_MU_INSTANCES + 1) && vInst.find(iInst) != vInst.end(); iInst++);

		*iFirstFreeInstanceNumber = iInst;
	}

	ReleaseMutex(gModuleLoader.m_hInstMutex);
	return pos;
}


/**  
 * \brief 
 */
int CModuleLoader::RemoveProcInfo()
{
	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return 0;

	DWORD dwPid = GetCurrentProcessId();

	for (int i=0; i < _MAX_MU_INSTANCES; i++)
	{
		if (gModuleLoader.m_vMuProcesses[i].dwPid == dwPid)
		{
			memset(&gModuleLoader.m_vMuProcesses[i], 0, sizeof(MuProcInfo));
			break;
		}
	}

	ReleaseMutex(gModuleLoader.m_hInstMutex);
	return CModuleLoader::PackProcInfos();
}


/**  
 * \brief 
 */
bool CModuleLoader::RegMuWindow(HWND hWnd)
{
	int iCount = PackProcInfos();

	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return false;

	DWORD dwPid = GetCurrentProcessId();
	bool fRet = false;

	for (int i=0; i < iCount; i++)
	{
		if (gModuleLoader.m_vMuProcesses[i].dwPid == dwPid)
		{
			gModuleLoader.m_vMuProcesses[i].hMainWnd = hWnd;

			fRet = true;
			break;
		}
	}

	ReleaseMutex(gModuleLoader.m_hInstMutex);

	if (fRet && gModuleLoader.m_pGuiModule)
		gModuleLoader.m_pGuiModule->SendCommand(_CLICKER_MODULE_COMMAND_REG_MU_WND, 0, 0);

	return fRet;
}


/**  
 * \brief 
 */
bool CModuleLoader::GetMuProcInfo(HWND* phWndMu, int* piInstanceNum)
{
	int iCount = gModuleLoader.PackProcInfos();

	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return false;


	DWORD dwPid = GetCurrentProcessId();
	bool fRet = false;

	for (int i=0; i < iCount; i++)
	{
		if (gModuleLoader.m_vMuProcesses[i].dwPid == dwPid)
		{
			if (phWndMu)
				*phWndMu = gModuleLoader.m_vMuProcesses[i].hMainWnd;
			
			if (piInstanceNum)
				*piInstanceNum = gModuleLoader.m_vMuProcesses[i].iInstance;

			fRet = true;
			break;
		}
	}


	ReleaseMutex(gModuleLoader.m_hInstMutex);
	return fRet;
}



/**  
 * \brief 
 */
HWND CModuleLoader::GetNextMuWindow()
{
	int iCount = PackProcInfos();

	if (iCount < 2 || !gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return 0;


	HWND hwndRet = 0;
	DWORD dwPid = GetCurrentProcessId();
	int idx = -1;

	for (int i=0; i < iCount; i++)
	{
		if (gModuleLoader.m_vMuProcesses[i].dwPid == dwPid)
		{
			idx = i;
			break;
		}
	}

	if (idx != -1)
		hwndRet = gModuleLoader.m_vMuProcesses[(idx+1) % iCount].hMainWnd;

	ReleaseMutex(gModuleLoader.m_hInstMutex);
	return hwndRet;
}


/**  
 * \brief 
 */
void CModuleLoader::ShowMuWindows(bool fShow)
{
	int iCount = PackProcInfos();

	if (!gModuleLoader.m_vMuProcesses || !gModuleLoader.m_hInstMutex || WaitForSingleObject(gModuleLoader.m_hInstMutex, 5000) != WAIT_OBJECT_0)
		return;

	DWORD dwPid = GetCurrentProcessId();

	for (int i=0; i < iCount; i++)
	{
		if (!fShow)
		{
			::ShowWindow(gModuleLoader.m_vMuProcesses[i].hMainWnd, SW_HIDE);
		}
		else if (gModuleLoader.m_vMuProcesses[i].dwPid != dwPid)
		{
			::ShowWindow(gModuleLoader.m_vMuProcesses[i].hMainWnd, SW_SHOWNOACTIVATE);
		}
	}

	ReleaseMutex(gModuleLoader.m_hInstMutex);
}


/**  
 * \brief 
 */
bool CModuleLoader::IsMuWindow(HWND hWnd)
{
	HWND hwndMu = 0;
	GetMuProcInfo(&hwndMu, 0);

	return hWnd != 0 && hWnd == hwndMu;
}


/**  
 * \brief 
 */
bool CModuleLoader::LoadMyCode()
{
	if (!gModuleLoader.IsInsideMuProcess())
		return false;

	CPackUtil p2;
	p2.Init(CPathUtil::ConstructModulePath(m_szRootDir, CStringTable::GetString(_STRING_LordOfMU_BootstrapperName).c_str()).c_str());

	int lSize = p2.GetPackedFileSize(_PACKUTIL_PROXY_FILE_ID);
	BYTE* data = new BYTE[lSize];
	
	p2.ExtractFileToMemory(data, lSize, _PACKUTIL_PROXY_FILE_ID);

	gModuleLoader.m_pProxyModule = 0;

	LoaderInitStruct ldr = {0};
	ldr.dwCookie = 1;
	ldr.pLoader = (IModuleLoader*)&gModuleLoader;
	ldr.ppMoudle = &gModuleLoader.m_pProxyModule;

	gModuleLoader.m_hProxy = MemoryLoadLibrary(data, &ldr);


	delete[] data;

	lSize = p2.GetPackedFileSize(_PACKUTIL_GUI_FILE_ID);
	data = new BYTE[lSize];

	p2.ExtractFileToMemory(data, lSize, _PACKUTIL_GUI_FILE_ID);

	gModuleLoader.m_pGuiModule = 0;

	ldr.dwCookie = 2;
	ldr.pLoader = (IModuleLoader*)&gModuleLoader;
	ldr.ppMoudle = &gModuleLoader.m_pGuiModule;

	gModuleLoader.m_hGui = MemoryLoadLibrary(data, &ldr);

	delete[] data;

	return true;
}


/**  
 * \brief 
 */
bool CModuleLoader::UnloadMyCode()
{
	gModuleLoader.m_pProxyModule = 0;
	gModuleLoader.m_pGuiModule = 0;

	if (gModuleLoader.m_hGui)
		MemoryFreeLibrary(gModuleLoader.m_hGui);

	gModuleLoader.m_hGui = 0;


	if (gModuleLoader.m_hProxy)
		MemoryFreeLibrary(gModuleLoader.m_hProxy);

	gModuleLoader.m_hProxy = 0;

	return true;
}



/**  
 * \brief 
 */
bool CModuleLoader::GetRootDir(char* szRootDir, int count)
{
	strcpy_s(szRootDir, count, m_szRootDir);
	for (int i=(int)strlen(szRootDir)-1; i >= 0 && szRootDir[i] != '\\'; szRootDir[i--] = 0);
	return true;
}


/**  
 * \brief 
 */
bool CModuleLoader::SendCommand(int iCommand, int target, void* pParam, void** pResult)
{
	if (_MODULE_LOADER_TARGET_PROXY == target)
	{
		if (m_pProxyModule)
			return m_pProxyModule->SendCommand(iCommand, pParam, pResult);
	}
	else if (_MODULE_LOADER_TARGET_GUI == target)
	{
		if (m_pGuiModule)
			return m_pGuiModule->SendCommand(iCommand, pParam, pResult);
	}
	else if (_MODULE_LOADER_TARGET_SELF == target)
	{
		if (_MODULE_LOADER_COMMAND_REG_MUWND == iCommand)
		{
			RegMuWindow((HWND)pParam);
			return true;
		}
		else if (_MODULE_LOADER_COMMAND_GET_MUWND == iCommand)
		{
			return GetMuProcInfo((HWND*)pParam, (int*)pResult);
		}
		else if (_MODULE_LOADER_COMMAND_NEXT_MUWND == iCommand)
		{
			if (!pResult)
				return false;

			*pResult = (void*)GetNextMuWindow();
			return true;
		}
		else if (_MODULE_LOADER_COMMAND_SHOW_MUWND == iCommand)
		{
			ShowMuWindows(pParam != 0);
			return true;
		}
		else if (_MODULE_LOADER_COMMAND_GET_INST_COUNT == iCommand)
		{
			if (!pResult)
				return false;

			int iCount = PackProcInfos();
			*pResult = (void*)iCount;
			return true;
		}
		else if (_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES == iCommand)
		{
			if (!pResult)
				return false;

			*pResult = (void*)&m_ServiceTable;
			return true;
		}
	}

	return false;
}
