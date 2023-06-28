//ResRead.H
#pragma once


/**  
 * \brief 
 */
class CCreateDialogUtil
{
protected:
	//Win32 RES file reader
	struct RESOURCEHEADER
	{ 
		DWORD  dwDataSize; 
		DWORD  dwHeaderSize;//Ignore
		LPCSTR pszType;            //These two can be <=0xFFFF then they
		LPCSTR pszName;            //are a numeric id (WORD) not string pointers
		DWORD  dwDataVersion; 
		WORD   wMemoryFlags;
		WORD   wLanguageId; 
		DWORD  dwVersion; 
		DWORD  dwCharacteristics; 
	}; 

	struct RESCALLBACKPARAM
	{
		UINT uiIdd;
		DLGTEMPLATE** ppDlgTemplate;
		BYTE** ppInitData;
	};

	typedef int (CALLBACK* READRESFILECALLBACK)(const RESOURCEHEADER*, LPVOID, LPARAM);

public:
	template <class T>
	CCreateDialogUtil(HWND hwndParent, T* pDlgClass)
	{
//		CreateDialogFromTemplate(hwndParent, pDlgClass);
		CreateDialogBase(hwndParent, pDlgClass);
	}

	operator HWND()
	{
		return m_hWnd;
	}

protected:
	template <class T>
	void CreateDialogFromTemplate(HWND hWndParent, T* pDlgClass)
	{
		extern TCHAR g_szRoot[_MAX_PATH + 1];

		TCHAR szFileName[_MAX_PATH+1] = {0};
		_tcscpy(szFileName, g_szRoot);
		_tcscat(szFileName, _T("AutoClicker.res"));

		pDlgClass->m_thunk.Init(NULL,NULL);

		// Add window data
		_AtlWinModule.AddCreateWndData(&pDlgClass->m_thunk.cd, (CDialogImplBaseT<CWindow>*)pDlgClass);

		DLGTEMPLATE* pTemplate = 0;
		BYTE* pInitData = 0;

		m_hWnd = 0;

		RESCALLBACKPARAM param;
		param.ppDlgTemplate = &pTemplate;
		param.ppInitData = &pInitData;
		param.uiIdd = pDlgClass->IDD;

		ReadResFile(szFileName, MyResReadCallback, (LPARAM)&param);

		if (pTemplate)
		{
			LPCDLGTEMPLATE lpDialogTemplate = _DialogSplitHelper::SplitDialogTemplate(pTemplate, pInitData);

			m_hWnd = ::CreateDialogIndirectParam(_AtlBaseModule.GetModuleInstance(), lpDialogTemplate, hWndParent, (DLGPROC)T::StartDialogProc, 0);

			if (lpDialogTemplate != pTemplate)
				GlobalFree(GlobalHandle(lpDialogTemplate));

			delete pTemplate;

			if (pInitData)
				delete pInitData;
		}
	}

	template <class T>
	void CreateDialogBase(HWND hWndParent, T* pDlgClass)
	{
		_AtlWinModule.AddCreateWndData(&pDlgClass->m_thunk.cd, (CDialogImplBaseT<CWindow>*)pDlgClass);
		m_hWnd = AtlAxCreateDialog(_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(pDlgClass->IDD), hWndParent, T::StartDialogProc, 0);
	}


protected:
	int WINAPI ReadResFile(LPCTSTR pszFilename, READRESFILECALLBACK ReadResFileCallback, LPARAM userParam);
	LPCSTR GetIdOrString(LPWORD* ppWord, LPSTR buffer, int bufsize);
	
	static int WINAPI MyResReadCallback(const RESOURCEHEADER* pResHdr, LPVOID lpData, LPARAM pParam);

private:
	HWND m_hWnd;
};
