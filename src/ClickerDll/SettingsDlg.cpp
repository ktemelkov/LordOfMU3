// SettingsDlg.cpp : Implementation of CSettingsDlg

#include "stdafx.h"
#include "SettingsDlg.h"
#include "PacketType.h"


static const DWORD s_arrHealTimes[] = {0, 1000, 3000, 5000, 7000, 10000, 15000};



// CSettingsDlg
LRESULT CSettingsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//	CDialogImpl<CSettingsDlg>::OnInitDialog(uMsg, wParam, lParam, bHandled);

	bHandled = TRUE;

	RECT rc = {0};
	RECT rcParent = {0};

	CWindow wndParent = GetParent();

	GetWindowRect(&rc);
	wndParent.GetClientRect(&rcParent);

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	int xPos = ((rcParent.right - rcParent.left) - width)/2;
	int yPos = ((rcParent.bottom - rcParent.top) - height)/2;

	MoveWindow(xPos, yPos, width, height, TRUE);

	RECT rcCont = {0};
	CWindow wndStatic = GetDlgItem(IDC_STATIC_CLASS);
	wndStatic.GetWindowRect(&rcCont);
	ScreenToClient(&rcCont);

	m_cDarkLordSettings.Create(m_hWnd);
	m_cEElfSettings.Create(m_hWnd);
	m_cAElfSettings.Create(m_hWnd);
	m_cBKSettings.Create(m_hWnd);
	m_cMGSettings.Create(m_hWnd);
	m_cSMSettings.Create(m_hWnd);

	CWindow* pDlgs[] = {&m_cDarkLordSettings, &m_cEElfSettings, &m_cAElfSettings, &m_cBKSettings, &m_cSMSettings, &m_cMGSettings, 0};
	const TCHAR* arrStrings[] = {_T("Dark Lord"), _T("Energy Elf"), _T("Agility Elf"), _T("Blade Knight"), _T("Dark Wizard"), _T("Magic Gladiator"), 0};

	HWND hwndAfter = GetDlgItem(IDC_CHARCLASS).m_hWnd;
	for (int i=0; pDlgs[i] != 0; ++i)
	{
		pDlgs[i]->SetWindowPos(hwndAfter, &rcCont, 0);
		SendDlgItemMessage(IDC_CHARCLASS, CB_ADDSTRING, 0, (LPARAM)arrStrings[i]);
	}

	for (int i=0; i < sizeof(s_arrHealTimes)/sizeof(s_arrHealTimes[0]); ++i)
	{
		TCHAR szText[256] = {0};
		_stprintf(szText, _T("%d"), s_arrHealTimes[i]/1000);

		SendDlgItemMessage(IDC_AUTOHEALTIME, CB_ADDSTRING, 0, (LPARAM)szText);
	}

	return 1;  // Let the system set the focus
}

void CSettingsDlg::InitValues()
{
	m_cDarkLordSettings.InitValues();
	m_cAElfSettings.InitValues();
	m_cEElfSettings.InitValues();
	m_cBKSettings.InitValues();
	m_cSMSettings.InitValues();
	m_cMGSettings.InitValues();

	SendDlgItemMessage(IDC_CHARCLASS, CB_SETCURSEL, (WPARAM)m_cSettings->all.dwClass, 0);

	BOOL fHandled = TRUE;
	OnSelectCharClass(0, 0, 0, fHandled);

	// Set settings values to controls
	CheckDlgButton(IDC_AUTOPICK, m_cSettings->all.fAutoPick ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_AUTOHEAL, m_cSettings->all.fAutoLife ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_AUTOREPAIR, m_cSettings->all.fAutoRepair ? BST_CHECKED : BST_UNCHECKED);


	CheckDlgButton(IDC_AUTOREOFF, m_cSettings->all.fAutoReOff ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_EXITAT400, m_cSettings->all.fExitAtLvl400 ? BST_CHECKED : BST_UNCHECKED);

	if ((m_dwFeatures & FEATURE_AFK_PROTECT) != 0)
	{
		CheckDlgButton(IDC_AUTOSPEAK, (m_cSettings->all.fAntiAFKProtect & 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_STOPZEN, (m_cSettings->all.fAntiAFKProtect & 2) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_STOPMOVE, (m_cSettings->all.fAntiAFKProtect & 4) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_STOPPICK, (m_cSettings->all.fAntiAFKProtect & 8) ? BST_CHECKED : BST_UNCHECKED);
	}


	BOOL fTemp = 0;
	OnCheckStopPick(0,0,0,fTemp);


	if ((m_dwFeatures & FEATURE_AFK_PROTECT) == 0)
	{
		CheckDlgButton(IDC_AUTOSPEAK, BST_UNCHECKED);
		CheckDlgButton(IDC_STOPZEN, BST_UNCHECKED);
		CheckDlgButton(IDC_STOPMOVE, BST_UNCHECKED);
		CheckDlgButton(IDC_STOPPICK, BST_UNCHECKED);

		GetDlgItem(IDC_AUTOSPEAK).EnableWindow(FALSE);
		GetDlgItem(IDC_STOPZEN).EnableWindow(FALSE);
		GetDlgItem(IDC_STOPMOVE).EnableWindow(FALSE);
		GetDlgItem(IDC_STOPPICK).EnableWindow(FALSE);
	}


	int nSel = 0;
	int nMinDelta = INT_MAX;

	for (int i=0; i < sizeof(s_arrHealTimes)/sizeof(s_arrHealTimes[0]); ++i)
	{
		int iEps = abs((int)s_arrHealTimes[i] - (int)m_cSettings->all.dwHealTime);
		if (iEps < nMinDelta)
		{
			nSel = i;
			nMinDelta = iEps;
		}
	}

	SendDlgItemMessage(IDC_AUTOHEALTIME, CB_SETCURSEL, (WPARAM)nSel, 0);
	GetDlgItem(IDC_AUTOHEALTIME).EnableWindow(m_cSettings->all.fAutoLife);
}

LRESULT CSettingsDlg::OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	if (wParam != 0)
	{
		InitValues();

		HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
		m_hOldCursor = SetCursor(hCursor);

		for (m_iShowCursor=0; ShowCursor(TRUE) < 1 && m_iShowCursor < 100; ++m_iShowCursor);

		GetDlgItem(IDC_AUTOPICK).SetFocus();
	}
	else
	{
		SetCursor(m_hOldCursor);
		for (m_iShowCursor; m_iShowCursor >= 0; m_iShowCursor--)
			ShowCursor(FALSE);
	}

	return 0;
}

LRESULT CSettingsDlg::OnClickedOK(WORD, WORD, HWND, BOOL&)
{
	Apply();
	m_cSettings.Save();

	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CSettingsDlg::OnClickedCancel(WORD, WORD, HWND, BOOL&)
{
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CSettingsDlg::OnSelectCharClass(WORD, WORD, HWND, BOOL&)
{
	int nSel = (int)SendDlgItemMessage(IDC_CHARCLASS, CB_GETCURSEL, 0, 0);
	
	CWindow* pDlgs[] = {&m_cDarkLordSettings, &m_cEElfSettings, &m_cAElfSettings, &m_cBKSettings, &m_cSMSettings, &m_cMGSettings, 0};

	for (int i=0; pDlgs[i] != 0; ++i)
	{
		pDlgs[i]->ShowWindow(i == nSel ? SW_SHOWNOACTIVATE : SW_HIDE);
	}

	return 0;
}

BOOL CSettingsDlg::LoadSettings(LPCTSTR pszFileName)
{
	return m_cSettings.Load(pszFileName);
}

void CSettingsDlg::Apply()
{
	m_cDarkLordSettings.Apply();
	m_cAElfSettings.Apply();
	m_cEElfSettings.Apply();
	m_cBKSettings.Apply();
	m_cSMSettings.Apply();
	m_cMGSettings.Apply();

	m_cSettings->all.fAutoPick = IsDlgButtonChecked(IDC_AUTOPICK) == BST_CHECKED;
	m_cSettings->all.fAutoLife = IsDlgButtonChecked(IDC_AUTOHEAL) == BST_CHECKED;
	m_cSettings->all.fAutoRepair = IsDlgButtonChecked(IDC_AUTOREPAIR) == BST_CHECKED;
	m_cSettings->all.fAutoReOff = IsDlgButtonChecked(IDC_AUTOREOFF) == BST_CHECKED;
	m_cSettings->all.fExitAtLvl400 = IsDlgButtonChecked(IDC_EXITAT400) == BST_CHECKED;
	
	
	m_cSettings->all.fAntiAFKProtect = 
			((IsDlgButtonChecked(IDC_AUTOSPEAK) == BST_CHECKED) ? 1 : 0)
			| ((IsDlgButtonChecked(IDC_STOPZEN) == BST_CHECKED) ? 2 : 0)
			| ((IsDlgButtonChecked(IDC_STOPMOVE) == BST_CHECKED) ? 4 : 0)
			| ((IsDlgButtonChecked(IDC_STOPPICK) == BST_CHECKED) ? 8 : 0);

	int nIdx = (int)SendDlgItemMessage(IDC_AUTOHEALTIME, CB_GETCURSEL);

	if (nIdx > sizeof(s_arrHealTimes)/sizeof(s_arrHealTimes[0]))
		nIdx = sizeof(s_arrHealTimes)/sizeof(s_arrHealTimes[0]);

	if (nIdx < 0) 
		nIdx = 0;

	m_cSettings->all.dwHealTime = s_arrHealTimes[nIdx];

	int nIdx2 = (int)SendDlgItemMessage(IDC_CHARCLASS, CB_GETCURSEL);

	if (nIdx2 > CHAR_CLASS_LAST)
		nIdx2 = 0;

	if (nIdx2 < 0)
		nIdx2 = 0;

	m_cSettings->all.dwClass = nIdx2;
}


/**
 * \brief 
 */
LRESULT CSettingsDlg::OnCheckAutoHeal(WORD, WORD, HWND, BOOL&)
{
	GetDlgItem(IDC_AUTOHEALTIME).EnableWindow(IsDlgButtonChecked(IDC_AUTOHEAL) == BST_CHECKED);
	return 0;
}


/**
 * \brief 
 */
LRESULT CSettingsDlg::OnCheckStopPick(WORD, WORD, HWND, BOOL&)
{
	BOOL fCheck = IsDlgButtonChecked(IDC_STOPPICK) != BST_CHECKED;
	GetDlgItem(IDC_STOPZEN).EnableWindow(fCheck);
	GetDlgItem(IDC_STOPMOVE).EnableWindow(fCheck);
	return 0;
}


/**
 * \brief 
 */
LRESULT CSettingsDlg::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	//::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	HDC hDC = GetWindowDC(); //GetDCEx((HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
    
	RECT rcWnd;
	RECT rcClient;
	GetClientRect(&rcClient);
	ClientToScreen(&rcClient);

	GetWindowRect(&rcWnd);

	int iXOffs = rcClient.left - rcWnd.left;
	int iCapSize = rcClient.top - rcWnd.top;

	GetClientRect(&rcClient);
	OffsetRect(&rcClient, iXOffs, iCapSize);
	ExcludeClipRect(hDC, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

	RECT rc = {0, 0, rcWnd.right-rcWnd.left, rcWnd.bottom - rcWnd.top};

	SetBkColor(hDC, RGB(10,35,150));
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, 0, 0, 0);

	rc.bottom = iCapSize;

//	TCHAR szCaption[256] = {0};
//	GetWindowText(szCaption, 255);

	TCHAR* szCaption = _T("MU AutoClicker Settings");

	SetTextColor(hDC, RGB(255, 255, 255));
	DrawText(hDC, szCaption, (int)_tcslen(szCaption), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	ReleaseDC(hDC);
	return 0;
}
