#ifndef __BkSettings_H
#define __BkSettings_H

#pragma once

#include "resource.h"       // main symbols
#include "Settings.h"
#include "CreateDialogUtil.h"

class CBKSettings :
	public CDialogImpl<CBKSettings>
{
public:
	CBKSettings(BKSettings& in)
		: m_tSettings(in) {}

	enum { IDD = IDD_BLADEKNIGHT };

	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

public:
	void Apply()
	{
		m_tSettings.dwAttackSkillSlot = (DWORD)SendDlgItemMessage(IDC_ATTACKSKILL, CB_GETCURSEL);
		m_tSettings.dwGrFortSkillSlot = (DWORD)SendDlgItemMessage(IDC_GFSKILL, CB_GETCURSEL);

		m_tSettings.fUseGrFort = IsDlgButtonChecked(IDC_ENABLEGF) == BST_CHECKED;

		m_tSettings.dwAttackDistance = (DWORD)SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_GETPOS);
	}

	void InitValues()
	{
		SendDlgItemMessage(IDC_ATTACKSKILL, CB_SETCURSEL, (WPARAM)m_tSettings.dwAttackSkillSlot, 0);
		SendDlgItemMessage(IDC_GFSKILL, CB_SETCURSEL, (WPARAM)m_tSettings.dwGrFortSkillSlot, 0);

		CheckDlgButton(IDC_ENABLEGF, m_tSettings.fUseGrFort ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETPOS, 0, MAKELONG(m_tSettings.dwAttackDistance, 0));

		TCHAR szTime[256] = {0};
		_stprintf(szTime, _T("%d"), m_tSettings.dwAttackDistance);
		SetDlgItemText(IDC_NOCLICKMARGIN, szTime);

		BOOL fTemp = TRUE;
		ApplyState(0, 0, 0, fTemp);
	}

BEGIN_MSG_MAP(CBKSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_ENABLEGF, BN_CLICKED, ApplyState)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		for (int i=0; i < 10; ++i)
		{
			TCHAR szText[256] = {0};
			_stprintf(szText, _T("%d"), i);
			SendDlgItemMessage(IDC_ATTACKSKILL, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_GFSKILL, CB_ADDSTRING, 0, (LPARAM)szText);
		}

	
		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETRANGE, 0, MAKELONG(500, 0));

		UDACCEL udAcc[] = {{3, 1}, {7, 5}, {10, 10}};
		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETACCEL, (WPARAM)(sizeof(udAcc)/sizeof(udAcc[0])), (LPARAM)udAcc);

		return 1;
	}

	LRESULT ApplyState(WORD, WORD, HWND, BOOL&)
	{
		GetDlgItem(IDC_ATTACKSKILL).EnableWindow(IsDlgButtonChecked(IDC_ENABLEGF) == BST_CHECKED);
		GetDlgItem(IDC_GFSKILL).EnableWindow(IsDlgButtonChecked(IDC_ENABLEGF) == BST_CHECKED);
		return 0;
	}

protected:
	BKSettings& m_tSettings;
};

#endif //__BkSettings_H