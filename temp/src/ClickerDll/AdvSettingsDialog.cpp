#include "stdafx.h"
#include "AdvSettingsDialog.h"
#include "PacketType.h"


void CAdvSettingsDialog::InitValues()
{
	CheckDlgButton(IDC_ADV_PICKUP, m_cSettings->all.fAdvAutoPick ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_BLESS, m_cSettings->all.fAdvPickBless ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SOUL, m_cSettings->all.fAdvPickSoul ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_LIFE, m_cSettings->all.fAdvPickLife ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_JOG, m_cSettings->all.fAdvPickGuardian ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_JOC, m_cSettings->all.fAdvPickCreation ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CHAOS, m_cSettings->all.fAdvPickChaos ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_EXL, m_cSettings->all.fAdvPickExl ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ZEN, m_cSettings->all.fAdvPickZen ? BST_CHECKED : BST_UNCHECKED);


	CheckDlgButton(IDC_BLESS1, m_cSettings->all.fAdvPickBlessMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_SOUL1, m_cSettings->all.fAdvPickSoulMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_LIFE1, m_cSettings->all.fAdvPickLifeMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_JOG1, m_cSettings->all.fAdvPickGuardianMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_JOC1, m_cSettings->all.fAdvPickCreationMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CHAOS1, m_cSettings->all.fAdvPickChaosMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_EXL1, m_cSettings->all.fAdvPickExlMove ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ZEN1, m_cSettings->all.fAdvPickZenMove ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(IDC_CUSTOM1, m_cSettings->all.fAdvPickCustom ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CUSTOM2, m_cSettings->all.fAdvPickCustomMove ? BST_CHECKED : BST_UNCHECKED);

	TCHAR szCustom[256] = {0};
	_sntprintf(szCustom, 255, _T("%d %d"), HIBYTE(m_cSettings->all.wPickCustomCode), LOBYTE(m_cSettings->all.wPickCustomCode));
	SetDlgItemText(IDC_EDIT_CUSTOM, szCustom);

	BOOL fTemp = TRUE;
	ApplyState(0, 0, 0, fTemp);
}

void CAdvSettingsDialog::Apply()
{
	m_cSettings->all.fAdvAutoPick = IsDlgButtonChecked(IDC_ADV_PICKUP) == BST_CHECKED;
	m_cSettings->all.fAdvPickBless = IsDlgButtonChecked(IDC_BLESS) == BST_CHECKED;
	m_cSettings->all.fAdvPickSoul = IsDlgButtonChecked(IDC_SOUL) == BST_CHECKED;
	m_cSettings->all.fAdvPickLife = IsDlgButtonChecked(IDC_LIFE) == BST_CHECKED;
	m_cSettings->all.fAdvPickGuardian = IsDlgButtonChecked(IDC_JOG) == BST_CHECKED;
	m_cSettings->all.fAdvPickCreation = IsDlgButtonChecked(IDC_JOC) == BST_CHECKED;
	m_cSettings->all.fAdvPickChaos = IsDlgButtonChecked(IDC_CHAOS) == BST_CHECKED;
	m_cSettings->all.fAdvPickExl = IsDlgButtonChecked(IDC_EXL) == BST_CHECKED;
	m_cSettings->all.fAdvPickZen = IsDlgButtonChecked(IDC_ZEN) == BST_CHECKED;

	m_cSettings->all.fAdvPickBlessMove = IsDlgButtonChecked(IDC_BLESS1) == BST_CHECKED;
	m_cSettings->all.fAdvPickSoulMove = IsDlgButtonChecked(IDC_SOUL1) == BST_CHECKED;
	m_cSettings->all.fAdvPickLifeMove = IsDlgButtonChecked(IDC_LIFE1) == BST_CHECKED;
	m_cSettings->all.fAdvPickGuardianMove = IsDlgButtonChecked(IDC_JOG1) == BST_CHECKED;
	m_cSettings->all.fAdvPickCreationMove = IsDlgButtonChecked(IDC_JOC1) == BST_CHECKED;
	m_cSettings->all.fAdvPickChaosMove = IsDlgButtonChecked(IDC_CHAOS1) == BST_CHECKED;
	m_cSettings->all.fAdvPickExlMove = IsDlgButtonChecked(IDC_EXL1) == BST_CHECKED;
	m_cSettings->all.fAdvPickZenMove = IsDlgButtonChecked(IDC_ZEN1) == BST_CHECKED;

	m_cSettings->all.fAdvPickCustom = IsDlgButtonChecked(IDC_CUSTOM1) == BST_CHECKED;
	m_cSettings->all.fAdvPickCustomMove = IsDlgButtonChecked(IDC_CUSTOM2) == BST_CHECKED;

	TCHAR szCustom[256] = {0};
	GetDlgItemText(IDC_EDIT_CUSTOM, szCustom, 255);

	TCHAR* p1 = szCustom;
	TCHAR* p2 = 0;

	while (*p1 != 0 && *p1 == ' ') p1++;
	p2 = p1;

	while (*p2 != 0 && *p2 != ' ') p2++;
	while (*p2 != 0 && *p2 == ' ') p2++;

	m_cSettings->all.wPickCustomCode = ((_ttoi(p1) & 0xFF) << 8) | (_ttoi(p2) & 0xFF);
}

LRESULT CAdvSettingsDialog::ApplyState(WORD, WORD, HWND, BOOL&)
{
	BOOL fEnable = IsDlgButtonChecked(IDC_ADV_PICKUP) == BST_CHECKED;
	GetDlgItem(IDC_BLESS).EnableWindow(fEnable);
	GetDlgItem(IDC_SOUL).EnableWindow(fEnable);
	GetDlgItem(IDC_LIFE).EnableWindow(fEnable);
	GetDlgItem(IDC_JOG).EnableWindow(fEnable);
	GetDlgItem(IDC_JOC).EnableWindow(fEnable);
	GetDlgItem(IDC_CHAOS).EnableWindow(fEnable);
	GetDlgItem(IDC_EXL).EnableWindow(fEnable);
	GetDlgItem(IDC_ZEN).EnableWindow(fEnable);
	GetDlgItem(IDC_CUSTOM1).EnableWindow(fEnable);

	GetDlgItem(IDC_BLESS1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_BLESS) == BST_CHECKED));
	GetDlgItem(IDC_SOUL1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_SOUL) == BST_CHECKED));
	GetDlgItem(IDC_LIFE1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_LIFE) == BST_CHECKED));
	GetDlgItem(IDC_JOG1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_JOG) == BST_CHECKED));
	GetDlgItem(IDC_JOC1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_JOC) == BST_CHECKED));
	GetDlgItem(IDC_CHAOS1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_CHAOS) == BST_CHECKED));
	GetDlgItem(IDC_EXL1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_EXL) == BST_CHECKED));
	GetDlgItem(IDC_ZEN1).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_ZEN) == BST_CHECKED));

	GetDlgItem(IDC_CUSTOM2).EnableWindow(fEnable && (IsDlgButtonChecked(IDC_CUSTOM1) == BST_CHECKED));

	GetDlgItem(IDC_EDIT_CUSTOM).EnableWindow(FALSE);

	ApplyFeatures();
	return 0;
}

LRESULT CAdvSettingsDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CAxDialogImpl<CAdvSettingsDialog>::OnInitDialog(uMsg, wParam, lParam, bHandled);
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

	return 1;  // Let the system set the focus
}

LRESULT CAdvSettingsDialog::OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	if (wParam != 0)
	{
		InitValues();
		m_fResult = FALSE;

		HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
		m_hOldCursor = SetCursor(hCursor);

		for (m_iShowCursor=0; ShowCursor(TRUE) < 1 && m_iShowCursor < 100; ++m_iShowCursor);

		GetDlgItem(IDC_ADV_PICKUP).SetFocus();
	}
	else
	{
		SetCursor(m_hOldCursor);
		for (m_iShowCursor; m_iShowCursor >= 0; m_iShowCursor--)
			ShowCursor(FALSE);
	}

	return 0;
}

LRESULT CAdvSettingsDialog::OnClickedOK(WORD, WORD, HWND, BOOL&)
{
	Apply();
	m_cSettings.Save();

	m_fResult = TRUE;
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CAdvSettingsDialog::OnClickedCancel(WORD, WORD, HWND, BOOL&)
{
	m_fResult = FALSE;
	ShowWindow(SW_HIDE);
	return 0;
}

LRESULT CAdvSettingsDialog::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
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

	TCHAR* szCaption = _T("MU AutoClicker Advanced Settings");

	SetTextColor(hDC, RGB(255, 255, 255));
	DrawText(hDC, szCaption, (int)_tcslen(szCaption), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	ReleaseDC(hDC);
	return 0;
}

/**  
 * \brief 
 */
void CAdvSettingsDialog::ApplyFeatures()
{
	if ((m_dwFeatures & FEATURE_MOVE_TO_PICK) != 0)
		return;

	BOOL fEnable = FALSE;

	GetDlgItem(IDC_BLESS1).EnableWindow(fEnable);
	GetDlgItem(IDC_SOUL1).EnableWindow(fEnable);
	GetDlgItem(IDC_LIFE1).EnableWindow(fEnable);
	GetDlgItem(IDC_JOG1).EnableWindow(fEnable);
	GetDlgItem(IDC_JOC1).EnableWindow(fEnable);
	GetDlgItem(IDC_CHAOS1).EnableWindow(fEnable);
	GetDlgItem(IDC_EXL1).EnableWindow(fEnable);
	GetDlgItem(IDC_ZEN1).EnableWindow(fEnable);
	GetDlgItem(IDC_CUSTOM2).EnableWindow(fEnable);

	CheckDlgButton(IDC_BLESS1, BST_UNCHECKED);
	CheckDlgButton(IDC_BLESS1, BST_UNCHECKED);
	CheckDlgButton(IDC_SOUL1, BST_UNCHECKED);
	CheckDlgButton(IDC_LIFE1, BST_UNCHECKED);
	CheckDlgButton(IDC_JOG1, BST_UNCHECKED);
	CheckDlgButton(IDC_JOC1, BST_UNCHECKED);
	CheckDlgButton(IDC_CHAOS1, BST_UNCHECKED);
	CheckDlgButton(IDC_EXL1, BST_UNCHECKED);
	CheckDlgButton(IDC_ZEN1, BST_UNCHECKED);

	CheckDlgButton(IDC_CUSTOM2, BST_UNCHECKED);
}
