#include "stdafx.h"
#include "MuWindow.h"
#include "AutoClicker.h"
#include "ClickerJobFactory.h"
#include "AsyncKeyQueue.h"
#include <map>
#include "ProtocolSettings.h"
#include "AutoClicker.h"
#include "..\_Shared\PeUtil.h"

CMuWindow* CMuWindow::s_pInstance = NULL;

GetAsyncKeyStatePtr CMuWindow::GetAsyncKeyState = 0;
ChangeDisplaySettingsAPtr CMuWindow::ChangeDisplaySettingsA = 0;
GetForegroundWindowPtr CMuWindow::GetForegroundWindowTr = 0;

extern TCHAR g_szRoot[_MAX_PATH + 1];


/**
 * \brief 
 */
CMuWindow::CMuWindow()
	: m_cAdvSettingsDlg(m_cSettingsDlg.GetSettingsObj())
{
	m_fConnected = false;
	m_fKeyStateFilterSuspended = false;

	m_hwndPassword = 0;
	m_fWasLastActiveInstance = FALSE;

	m_fIsWndActive = TRUE;
	m_fBlockInput = FALSE;

	m_fGuiActive = FALSE;
	m_iInstanceNumber = 0;
	m_pClicker = NULL;

	memset(m_buffDevmode, 0, sizeof(m_buffDevmode));
	m_ulVersion = 0;
	m_fWindow = false;

	CProtocolSettings& sett = CProtocolSettings(CT2CA(g_szRoot));
	sett.Load();

	DWORD dwType = sett.data.dwClientType;

	if (dwType < 2)
		m_ulVersion = 104;
	else
		m_ulVersion = 105;


	memset(m_vFnKeys, 0, sizeof(m_vFnKeys));

	m_vFnKeys[0].vk = VK_F5;
	m_vFnKeys[1].vk = VK_F6;
	m_vFnKeys[2].vk = VK_F7;
	m_vFnKeys[3].vk = VK_F8;
	m_vFnKeys[4].vk = VK_F9;
	m_vFnKeys[5].vk = VK_F10;
	m_vFnKeys[6].vk = VK_F11;
	m_vFnKeys[7].vk = VK_F12;
//	m_vFnKeys[8].vk = VK_ESCAPE;

	CAsyncKeyQueue::Init();
}


/**
 * \brief 
 */
CMuWindow::~CMuWindow()
{
	if (IsWindow())
		UnsubclassWindow2();

	CAsyncKeyQueue::Term();
}


/**
 * \brief 
 */
BOOL CMuWindow::Init(HWND hwndMuWindow, int iInstanceNumber)
{
	if (s_pInstance != NULL)
		return FALSE; // Already initialized

	s_pInstance = new CMuWindow();

	if (!s_pInstance)
		return FALSE; // Out of memory

	if (!s_pInstance->SubclassWindow2(hwndMuWindow))
	{
		delete s_pInstance;
		s_pInstance = NULL;
		return FALSE;
	}

	s_pInstance->m_fIsWndActive = (::GetForegroundWindow() == hwndMuWindow);
	s_pInstance->m_iInstanceNumber = iInstanceNumber;

	s_pInstance->PostMessage(WM_MUWND_INITIALIZE, 0, 0);
	return TRUE;
}


void CMuWindow::Term()
{
	if (_AtlModule.m_pLoader)
	{
		InstallerServiceTable* pServices = 0;
		_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

		if (pServices)
		{
			pServices->ClearApiHook(_API_HOOK_GETASYNCKEYSTATE);
			pServices->ClearApiHook(_API_HOOK_GETASYNCKEYSTATE_FILTER);			
			pServices->ClearApiHook(_API_HOOK_CHANGEDISPLAYSETTINGSA);
			pServices->ClearApiHook(_API_HOOK_GETFOREGROUNDWINDOW);
		}
	}
	
	if (s_pInstance != NULL)
	{
		delete s_pInstance;
		s_pInstance = NULL;
	}
}


/**  
 * \brief 
 */
void CMuWindow::DoLogin(const char* pszPass)
{
	HWND hwndPass = GetInstance()->m_hwndPassword;

	if (!hwndPass || !pszPass)
		return;

	SetWindowTextA(hwndPass, pszPass);

	HWND hWnd = GetInstance()->m_hWnd;

	PostMessageA(hWnd, WM_SEND_RETURN, 0, 0);
}


/**  
 * \brief 
 */
LRESULT CMuWindow::OnSendKey(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	INPUT in = {0};
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = (BYTE)lParam;

//	SetForegroundWindow(m_hWnd);
//	SendInput(1, &in, sizeof(in));

	CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();

	if (pQueue)
		pQueue->SendKey((int)lParam);

	return 0;
}


/**  
 * \brief 
 */
LRESULT CMuWindow::OnSendReturn(UINT, WPARAM, LPARAM, BOOL& fHandled)
{
/*	BYTE pKeys[256] = {0};
	GetKeyboardState(pKeys);

	pKeys[VK_RETURN] = 0x81;

	SetKeyboardState(pKeys);
	CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();

	if (pQueue)
		pQueue->PressKey(VK_RETURN);
*/
	INPUT in = {0};
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = VK_RETURN;

	SetForegroundWindow(m_hWnd);
	SendInput(1, &in, sizeof(in));

	fHandled = TRUE;
	return 0;
}


/**  
 * \brief 
 */
BOOL CMuWindow::FindPasswordWnd(HWND hwnd, LPARAM lParam)
{
	DWORD dwStyle = GetWindowLongA(hwnd, GWL_STYLE);
	
	if ((dwStyle & ES_PASSWORD) != 0)
	{
		*((HWND*)lParam) = hwnd;
		return FALSE;
	}

	return TRUE;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnSelectChar(UINT uMsg, WPARAM, LPARAM iChar, BOOL& fHandled)
{
	iChar = iChar % 5;

	double CHAR_COORDS[5][2] = {{200.0, 550.0}, {350.0, 505.0}, {516.0, 514.0}, {350.0, 505.0}, {350.0, 505.0}};

	RECT rc = {0};
	GetClientRect(&rc);

	int resX = rc.right - rc.left;
	int resY = rc.bottom - rc.top;

	POINT pt = {(int)(resX*(CHAR_COORDS[iChar][0]/1024.0)), (int)(resY*(CHAR_COORDS[iChar][1]/768.0))};

	::ClientToScreen(m_hWnd, &pt);

	double fScreenWidth   = ::GetSystemMetrics(SM_CXSCREEN) - 1; 
	double fScreenHeight  = ::GetSystemMetrics(SM_CYSCREEN) - 1;

	double fx = pt.x*(65535.0/fScreenWidth);
	double fy = pt.y*(65535.0/fScreenHeight);

	SelCharThreadParam* pParam = new SelCharThreadParam;
	pParam->x = (int)fx;
	pParam->y = (int)fy;

	pParam->hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SelectCharThread, pParam, 0, 0);

	if (pParam->hThread == INVALID_HANDLE_VALUE || pParam->hThread == 0)
		delete pParam;

	fHandled = TRUE;
	return 0;
}


/**  
 * \brief 
 */
LRESULT CALLBACK CMuWindow::SelectCharThread(SelCharThreadParam* pParam)
{
	Sleep(1000);

	INPUT in = {0};
	in.type = INPUT_MOUSE;
	in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	in.mi.dx = (LONG)pParam->x;
	in.mi.dy = (LONG)pParam->y;

	SendInput(1, &in, sizeof(in));

	Sleep(200);

	in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, &in, sizeof(in));

	Sleep(20);

	in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &in, sizeof(in));

	Sleep(100);

	in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &in, sizeof(in));

	Sleep(100);

	in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &in, sizeof(in));

	Sleep(100);

	in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &in, sizeof(in));

	Sleep(20);

	in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	SendInput(1, &in, sizeof(in));

	CloseHandle(pParam->hThread);
	delete pParam;
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnInitMuWindow(UINT, WPARAM, LPARAM, BOOL&)
{
	if (_AtlModule.m_pLoader)
	{
		InstallerServiceTable* pServices = 0;
		_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

		if (pServices)
		{
//			pServices->SetApiHook(_API_HOOK_GETASYNCKEYSTATE_FILTER, (PVOID)&MyGetAsyncKeyStateFilter, (PVOID*)&GetAsyncKeyState);
			pServices->SetApiHook(_API_HOOK_GETASYNCKEYSTATE, (PVOID)&MyGetAsyncKeyState, (PVOID*)&GetAsyncKeyState);			
			pServices->SetApiHook(_API_HOOK_CHANGEDISPLAYSETTINGSA, (PVOID)&MyChangeDisplaySettingsA, (PVOID*)&ChangeDisplaySettingsA);
			pServices->SetApiHook(_API_HOOK_GETFOREGROUNDWINDOW, (PVOID)&MyGetForegroundWindow, (PVOID*)&GetForegroundWindowTr);
		}
	}

	memset(m_buffDevmode, 0, sizeof(m_buffDevmode));
	((LPDEVMODEA)m_buffDevmode)->dmSize = sizeof(DEVMODEA);
	((LPDEVMODEA)m_buffDevmode)->dmDriverExtra = sizeof(m_buffDevmode) - sizeof(DEVMODEA);

	EnumDisplaySettingsA(0, ENUM_CURRENT_SETTINGS, (LPDEVMODEA)m_buffDevmode);

	m_hwndPassword = 0;
	EnumChildWindows(m_hWnd, FindPasswordWnd, (LPARAM)&m_hwndPassword);


	TCHAR szFileName[_MAX_PATH+1] = {0};
	_tcscpy(szFileName, g_szRoot);


	TCHAR szSetFile[_MAX_PATH+1] = {0};
	_stprintf(szSetFile, _T("MUAutoClicker_%d.dat"), m_iInstanceNumber);
	_tcscat(szFileName, szSetFile);

	m_cSettingsDlg.LoadSettings(szFileName);
	

	if (::IsWindow(m_cSettingsDlg.m_hWnd))
	{
		m_cSettingsDlg.DestroyWindow();
	}

	m_cSettingsDlg.Create(m_hWnd);
	m_cLaunchMuDlg.Create(m_hWnd);
	m_cAdvSettingsDlg.Create(m_hWnd);

	CRegKey cRegKey;
	DWORD dwWndMode = 0;

	if (ERROR_SUCCESS == cRegKey.Open(HKEY_CURRENT_USER, _T("Software"))
				&& ERROR_SUCCESS == cRegKey.Open(cRegKey, _T("Webzen"))
				&& ERROR_SUCCESS == cRegKey.Open(cRegKey, _T("Mu"))
				&& ERROR_SUCCESS == cRegKey.Open(cRegKey, _T("Config"))
				&& ERROR_SUCCESS == cRegKey.QueryDWORDValue(_T("WindowMode"), dwWndMode)
				&& dwWndMode == 1)
	{
		m_fWindow = true;

		RECT rc, rcClient;
		GetWindowRect(&rc);
		GetClientRect(&rcClient);
		ClientToScreen(&rcClient);

		int cx = GetSystemMetrics(SM_CXSCREEN);
		int cy = GetSystemMetrics(SM_CYSCREEN);
		int offs = rcClient.top - rc.top;

		if (cy - GetSystemMetrics(SM_CYCAPTION) < rc.bottom - rc.top)
		{
			MoveWindow((cx - rc.right + rc.left)/2,
							-offs, rc.right - rc.left, rc.bottom - rc.top);
		}
	}


	PeUtil::WipeDosHeader((HMODULE)_AtlBaseModule.GetModuleInstance());
	PeUtil::WipeImportTable((HMODULE)_AtlBaseModule.GetModuleInstance());

	SetTimer(1011, 100, 0);
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnIsClickerInstalled(UINT, WPARAM, LPARAM, BOOL&)
{
	return (LRESULT)m_iInstanceNumber;
}


/**
 * \brief 
 */
BOOL CMuWindow::OnKeyboardEvent(UINT vkCode, UINT uMsg, BOOL fCheckFgWnd)
{
	if (!m_fConnected)
		return FALSE;

	if (vkCode == VK_F12 && uMsg == WM_KEYDOWN)
	{
		// Show/Hide all MU Windows
		if (CMuWindow::GetForegroundWindowTr() == m_hWnd)
		{
			m_fWasLastActiveInstance = TRUE;
			PostMessage(WM_HIDE_MU, 0, 0);
		}
		else if (m_fWasLastActiveInstance)
		{
			m_fWasLastActiveInstance = FALSE;
			PostMessage(WM_SHOW_MU, 0, 0);
		}
		else
		{
			m_fWasLastActiveInstance = FALSE;
			return FALSE;
		}

		return TRUE;
	}

	if (fCheckFgWnd && CMuWindow::GetForegroundWindowTr() != m_hWnd)
		return FALSE;


	if (vkCode == VK_F5 && uMsg == WM_KEYUP)
	{
		if ((CMuWindow::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
		{
			SayToServer("//script toggle");
		}
		else
		{
			SayToServer("//autokill toggle");
		}

		return TRUE;
	}


	if (vkCode == VK_F6 && uMsg == WM_KEYUP)
	{
		PostMessage(WM_START_CLICKER, 0, 0);
		return TRUE;
	}

	if (vkCode == VK_F7 && uMsg == WM_KEYUP)
	{
		PostMessage(WM_START_CLICKER, 0, TRUE);
		return TRUE;
	}

	if (vkCode == VK_F8 && uMsg == WM_KEYUP)
	{
		PostMessage(WM_STOP_CLICKER, 0, 0);
		return TRUE;
	}

	if (vkCode == VK_F9 && uMsg == WM_KEYUP)
	{
		PostMessage(WM_SHOW_SETTINGS_GUI, 0, 0);
		return TRUE;
	}

	if (vkCode == VK_F10 && uMsg == WM_KEYUP)
	{
		if ((CMuWindow::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
		{
			SayToServer("//moveitem 12 76");
		}
		else
		{
			// Launch another MU instance
			PostMessage(WM_LAUNCH_MU, 0, 0);
		}

		return TRUE;
	}

	if (vkCode == VK_F11 && uMsg == WM_KEYUP)
	{
		// Switch between MU windows
		PostMessage(WM_SWITCH_INSTANCE, 0, 0);
		return TRUE;
	}

	if (vkCode == VK_ESCAPE && uMsg == WM_KEYUP && m_fGuiActive)
	{
		m_cSettingsDlg.ShowWindow(SW_HIDE);
		m_cAdvSettingsDlg.ShowWindow(SW_HIDE);
		return TRUE;
	}

	return FALSE;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnMouseMessage(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	bHandled = m_fBlockInput;
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnActivateApp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_fIsWndActive = (BOOL)wParam;

	if (m_fIsWndActive)
	{
		bHandled = FALSE;
	}
	else
	{
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (LOWORD(wParam) != WA_INACTIVE)
	{
		bHandled = FALSE;
	}
	else
	{
		return ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsWindow())
		SendMessage(WM_SWITCH_INSTANCE, 0, 0);

	if (::IsWindow(m_cSettingsDlg.m_hWnd))
		m_cSettingsDlg.DestroyWindow();

	if (::IsWindow(m_cLaunchMuDlg.m_hWnd))
		m_cLaunchMuDlg.DestroyWindow();

	if (::IsWindow(m_cAdvSettingsDlg.m_hWnd))
		m_cAdvSettingsDlg.DestroyWindow();

	BOOL fHandled = FALSE;
	OnStopClicker(0, 0, 0, fHandled);

	DWORD dwStart = GetTickCount();

	while (m_pClicker != 0)
	{
		MSG msg = {0};
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if ((int)GetTickCount() - (int)dwStart > 5000)
		{
			break;
		}

		Sleep(10);
	}

	if (IsWindow())
		KillTimer(1011);

	HWND hWnd = m_hWnd;
	WNDPROC proc = m_pfnSuperWindowProc;

	if (IsWindow())
		UnsubclassWindow2();

	return CallWindowProc(proc, hWnd, uMsg, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnShowSettingsGUI(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	if (m_pClicker != NULL)
		return 0;

	CWindow dlg = m_cSettingsDlg;
	
	if (lParam == 1 || (CMuWindow::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
	{
		dlg = m_cAdvSettingsDlg;
	}


	if (m_fGuiActive || !dlg.IsWindow())
		return 0;

	m_fGuiActive = TRUE;

	dlg.ShowWindow(SW_SHOWNORMAL);

	InvalidateRect(0, TRUE);
	UpdateWindow();

	BOOL fOldBlockInput = m_fBlockInput;
	m_fBlockInput = FALSE;


	MSG msg = {0};
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) > 0)
	{
		if (msg.message != WM_COMMAND)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	PostMessage(WM_NULL, 0, 0);

	while (true)
	{
		if (GetMessage(&msg, 0, 0, 0) > 0)
		{
			if (msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE)
			{
				dlg.ShowWindow(SW_HIDE);
			}
			else if (!dlg.IsDialogMessage(&msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			dlg.ShowWindow(SW_HIDE);
		}

		if (!dlg.IsWindowVisible())
			break;
	}

	while (CMuWindow::GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Sleep(10);
	}

	m_fBlockInput = fOldBlockInput;
	m_fGuiActive = FALSE;

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnErasebkgnd(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
	if (m_fGuiActive)
	{
		HDC hDC = (HDC)wParam;

		RECT rc = {0};
		GetClientRect(&rc);
	
		SetBkColor(hDC, RGB(0,0,0));
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);
	}
	else
	{
		bHandled = FALSE;
	}

	return 1;
}


/**
 * \brief 
 */
SHORT WINAPI CMuWindow::MyGetAsyncKeyState(int vk)
{
	CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();
	if (pQueue && pQueue->GetKey(vk))
	{
		return 0x8000U;
	}

	CMuWindow* pInstance = CMuWindow::GetInstance();

	if (!pInstance || !pInstance->m_fIsWndActive)
		return 0;

	for (int i=0; pInstance->m_vFnKeys[i].vk != 0; ++i)
	{
		if (vk == pInstance->m_vFnKeys[i].vk)
			return 0;
	}

	if ((int)VkKeyScanA('/') == vk)
		return 0;

	return CMuWindow::GetAsyncKeyState(vk);
}



/**
 * \brief 
 */
SHORT WINAPI CMuWindow::MyGetAsyncKeyStateFilter(int vk, int origState)
{
	CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();
	if (pQueue && pQueue->GetKey(vk))
	{
		return 0x8000U;
	}

	CMuWindow* pInstance = CMuWindow::GetInstance();

	if (!pInstance || !pInstance->m_fIsWndActive)
		return 0;

	DWORD procId = 0;

	if (GetCurrentThreadId() == GetWindowThreadProcessId(pInstance->m_hWnd, &procId) && pInstance->m_fKeyStateFilterSuspended)
		return origState;

	for (int i=0; pInstance->m_vFnKeys[i].vk != 0; ++i)
	{
		if (vk == pInstance->m_vFnKeys[i].vk)
			return 0;
	}

	if ((int)VkKeyScanA('/') == vk)
		return 0;

	return origState;
}



/**
 * \brief 
 */
LRESULT CMuWindow::OnStartClicker(UINT, WPARAM, LPARAM fNoMouseMove, BOOL&)
{
	if (m_pClicker != NULL)
		return 0;

	BOOL fHealOnly = FALSE;

	if ((CMuWindow::GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0)
	{
		fHealOnly = TRUE;
	}

	m_pClicker = CClickerJobFactory::CreateClickerJob(m_hWnd, m_ulVersion, m_cSettingsDlg.GetSettings(), fNoMouseMove != 0, fHealOnly);

	if (m_pClicker)
	{	
		if (m_pClicker->Start())
		{
			m_fBlockInput = (fNoMouseMove == 0);
		}
		else
		{
			delete m_pClicker;
			m_pClicker = NULL;
		}
	}
	else // Failed to start clicker
	{
		m_fBlockInput = FALSE;
	}

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnStopClicker(UINT, WPARAM, LPARAM, BOOL&)
{
	if (m_pClicker != NULL)
	{
		m_pClicker->Stop();
	}

	m_fBlockInput = FALSE;
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerJobFinished(UINT, WPARAM, LPARAM, BOOL&)
{
	if (m_pClicker != NULL)
	{
		delete m_pClicker;
		m_pClicker = NULL;
	}

	m_fBlockInput = FALSE;
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_MOUSEMOVE, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerRButtonDown(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_RBUTTONDOWN, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerRButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_RBUTTONUP, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerLButtonDown(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_LBUTTONDOWN, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnClickerLButtonUp(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	return CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_LBUTTONUP, wParam, lParam);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnLaunchMu(UINT, WPARAM, LPARAM, BOOL&)
{
	if (m_pClicker != NULL)
		return 0;

	if (m_fGuiActive || !m_cLaunchMuDlg.IsWindow())
		return 0;

	m_fGuiActive = TRUE;

	m_cLaunchMuDlg.ShowWindow(SW_SHOWNORMAL);

	InvalidateRect(0, TRUE);
	UpdateWindow();

	PostMessage(WM_NULL, 0, 0);

	BOOL fOldBlockInput = m_fBlockInput;
	m_fBlockInput = FALSE;

	MSG msg = {0};
	while (true)
	{
		if (GetMessage(&msg, 0, 0, 0) > 0)
		{
			if (msg.message == WM_SYSCOMMAND && msg.wParam == SC_CLOSE)
			{
				m_cLaunchMuDlg.ShowWindow(SW_HIDE);
			}
			else if (!m_cLaunchMuDlg.IsDialogMessage(&msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			m_cLaunchMuDlg.ShowWindow(SW_HIDE);
		}

		if (!m_cLaunchMuDlg.IsWindowVisible())
			break;
	}

	while (CMuWindow::GetAsyncKeyState(VK_ESCAPE) & 0x8000)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Sleep(10);
	}

	m_fBlockInput = fOldBlockInput;
	m_fGuiActive = FALSE;

	if (m_cLaunchMuDlg.m_fResult)
	{
		LaunchMU();
	}

	return 0;
}


/**
 * \brief 
 */
void CMuWindow::LaunchMU()
{
	STARTUPINFO si = {0};
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi = {0};

	if (!CreateProcess(0, GetCommandLine(), 0, 0, FALSE, CREATE_SUSPENDED, 0, 0, &si, &pi))
		return;

	srand(GetTickCount());
	char ch = (char)((rand() % 254) + 1);

	PatchProcessMemory(pi.hProcess, ch, "MuOnline");

	ResumeThread(pi.hThread);
}


/**
 * \brief 
 */
void CMuWindow::PatchProcessMemory(HANDLE hProcess, char patchByte, const char* szMutexName)
{
	char buff[2048] = {0};
	SIZE_T cnt = 0;
	int addr = 0x00400000;
	bool fStop = false;

	while (!fStop)
	{
		ReadProcessMemory(hProcess, (LPCVOID)(DWORD_PTR)addr, buff, sizeof(buff), &cnt);
		
		if (cnt == 0)
			break;

		int pos = 0;

		for (int i=0; i < (int)cnt; i++)
		{
			char ch = buff[i];

			if (ch == szMutexName[pos])
			{
				pos++;

				if (szMutexName[pos] == 0)
				{
					// <--- Mutex name string is found
					int waddr = addr + i;
					ch = patchByte;
					WriteProcessMemory(hProcess, (LPVOID)(DWORD_PTR)waddr, &ch, 1, &cnt);
					
					fStop = true;
					break;
				}
			}
			else if (pos != 0)
			{
				pos = 0;
				--i;
			}
		}

		if (cnt < sizeof(buff) || addr > 0x00FFFFFF)
		{
			break;
		}
		else
		{
			addr = addr + sizeof(buff) -(int)sizeof(szMutexName) + 2;
		}
	}
}


/**
 * \brief 
 */
LONG WINAPI CMuWindow::MyChangeDisplaySettingsA(LPDEVMODEA pDevMode, DWORD dwFlags)
{
	CMuWindow* pInstance = CMuWindow::GetInstance();

	if (!pInstance)
		return DISP_CHANGE_SUCCESSFUL;

	int iCount = 1;

	if (_AtlModule.m_pLoader)
		_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INST_COUNT, _MODULE_LOADER_TARGET_SELF, 0, (void**)&iCount);

	if (iCount > 1)
		return DISP_CHANGE_SUCCESSFUL;

	return CMuWindow::ChangeDisplaySettingsA(pDevMode, dwFlags);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnSwitchMuInstance(UINT, WPARAM, LPARAM, BOOL&)
{
	if (!_AtlModule.m_pLoader)
		return 0;

	HWND hwndNext = 0;
	_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_NEXT_MUWND, _MODULE_LOADER_TARGET_SELF, 0, (void**)&hwndNext);

	if (hwndNext)
	{
		::BringWindowToTop(hwndNext);
		::SetForegroundWindow(hwndNext);
	}

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnShowMu(UINT, WPARAM, LPARAM, BOOL&)
{
	if (!m_fWindow)
		CMuWindow::ChangeDisplaySettingsA((LPDEVMODEA)m_buffDevmode, 0);

	if (_AtlModule.m_pLoader)
		_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_SHOW_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)1, 0);

	SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
	SetForegroundWindow(m_hWnd);

	Sleep(250);

	MSG msg = {0};

	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CHUDParams hud(m_hWnd, m_ulVersion);

	POINT ptClick = {hud.x0, hud.y0};
	ClientToScreen(&ptClick);

	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);

	ptClick.x = (LONG)(((float)ptClick.x/(float)cx)*65535);
	ptClick.y = (LONG)(((float)ptClick.y/(float)cy)*65535);

	INPUT ins[3] = {0};
	ins[0].type = INPUT_MOUSE;
	ins[0].mi.dx = ptClick.x;
	ins[0].mi.dy = ptClick.y;
	ins[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

	ins[1].type = INPUT_MOUSE;
	ins[1].mi.dx = ptClick.x;
	ins[1].mi.dy = ptClick.y;
	ins[1].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;

	ins[2].type = INPUT_MOUSE;
	ins[2].mi.dx = ptClick.x;
	ins[2].mi.dy = ptClick.y;
	ins[2].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;


	SendInput(1, ins, sizeof(INPUT));
	Sleep(100);
	SendInput(2, ins+1, sizeof(INPUT));

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnHideMu(UINT, WPARAM, LPARAM, BOOL&)
{
	if (_AtlModule.m_pLoader)
		_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_SHOW_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)0, 0);
	
	if (!m_fWindow)
		CMuWindow::ChangeDisplaySettingsA(0, 0);

	::SetForegroundWindow(GetDesktopWindow());
	return 0;
}


/**
 * \brief 
 */
HWND WINAPI CMuWindow::MyGetForegroundWindow(VOID)
{
	CMuWindow* pInstance = CMuWindow::GetInstance();

	if (pInstance && pInstance->m_fBlockInput)
		return pInstance->m_hWnd;

	return CMuWindow::GetForegroundWindowTr();

}



/**
 * \brief 
 */
LRESULT CMuWindow::OnNCActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_fIsWndActive = (BOOL)wParam;

	if ((BOOL)wParam)
		bHandled = FALSE;
	else
		::DefWindowProc(m_hWnd, uMsg, wParam, lParam);

	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnCaptureChanged(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	if (m_pClicker == 0)
		bHandled = FALSE;
	
	return 0;
}


/**
 * \brief 
 */
void CMuWindow::SayToServer(const char* buf)
{
	if (_AtlModule.m_pLoader)
		_AtlModule.m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SEND_COMMAND, _MODULE_LOADER_TARGET_PROXY, (void*)buf, 0);
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnSetVersion(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	m_ulVersion = (ULONG)lParam;
	return 0;
}


/**
 * \brief 
 */
LRESULT CMuWindow::OnTimer(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1011)
	{
		bHandled = TRUE;

		for (int i=0; m_vFnKeys[i].vk != 0; ++i)
		{
			bool fOldState = m_vFnKeys[i].fPressed;

			m_fKeyStateFilterSuspended = true;
			bool fNewState = (CMuWindow::GetAsyncKeyState(m_vFnKeys[i].vk) & 0x8000) != 0;
			m_fKeyStateFilterSuspended = false;

			if (fOldState != fNewState)
			{
				OnKeyboardEvent(m_vFnKeys[i].vk, fNewState ? WM_KEYDOWN : WM_KEYUP);
				m_vFnKeys[i].fPressed = fNewState;
			}
		}
	}
	else
	{
		bHandled = FALSE;
	}

	return 0;
}


/**  
 * \brief 
 */
BOOL CMuWindow::SubclassWindow2(HWND hWnd)
{
	ATLASSUME(m_hWnd == NULL);
	ATLASSERT(::IsWindow(hWnd));

	// Allocate the thunk structure here, where we can fail gracefully.
	if (!m_thunk.Init(GetWindowProc(), this)) 
		return FALSE;

	if (!_AtlModule.m_pLoader)
		return FALSE;


	InstallerServiceTable* pServices = 0;
	_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

	if (!pServices)
		return FALSE;


	m_hWnd = hWnd;

	WNDPROC pProc = m_thunk.GetWNDPROC();
	return pServices->SetApiHook(_API_HOOK_MAINWND_PROC, (PVOID)pProc, (PVOID*)&m_pfnSuperWindowProc) ? TRUE : FALSE;
}


/**  
 * \brief 
 */
HWND CMuWindow::UnsubclassWindow2()
{
	if (!_AtlModule.m_pLoader)
		return 0;

	InstallerServiceTable* pServices = 0;
	_AtlModule.m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_INSTALLER_SERVICES, _MODULE_LOADER_TARGET_SELF, 0, (void**)&pServices);

	if (!pServices)
		return 0;

	pServices->ClearApiHook(_API_HOOK_MAINWND_PROC);
	m_pfnSuperWindowProc = ::DefWindowProc;

	HWND hWnd = m_hWnd;
	m_hWnd = 0;
	return hWnd;
}



/**  
 * \brief 
 */
void CMuWindow::SetFeatures(DWORD dwFeatures)
{
	if (GetInstance())
	{
		GetInstance()->m_cAdvSettingsDlg.SetFeatures(dwFeatures);
		GetInstance()->m_cSettingsDlg.SetFeatures(dwFeatures);
	}
}


/**  
 * \brief 
 */
void CMuWindow::SetConnected()
{
	if (GetInstance())
		GetInstance()->m_fConnected = true;
}

