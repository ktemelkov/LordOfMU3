#ifndef __AElfSettings_H
#define __AElfSettings_H

#pragma once

#include "resource.h"       // main symbols
#include "Settings.h"
#include "CreateDialogUtil.h"

class CAElfSettings :
	public CDialogImpl<CAElfSettings>
{
public:
	CAElfSettings(AElfSettings& in) 
		: m_tSettings(in) {}

	enum { IDD = IDD_AELF };

	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

public:
	void Apply()
	{
		m_tSettings.dwTShotSkillSlot = (DWORD)SendDlgItemMessage(IDC_TSHOTSKILLSLOT, CB_GETCURSEL);
		m_tSettings.dwInfArrowSkillSlot = (DWORD)SendDlgItemMessage(IDC_INFARROWSLOT, CB_GETCURSEL);
		m_tSettings.dwChangeDirTime = (DWORD)SendDlgItemMessage(IDC_CHDIRSPIN, UDM_GETPOS);

		m_tSettings.fUseInfArrow = IsDlgButtonChecked(IDC_ENABLEINFARROW) == BST_CHECKED;
	}

	void InitValues()
	{
		SendDlgItemMessage(IDC_TSHOTSKILLSLOT, CB_SETCURSEL, (WPARAM)m_tSettings.dwTShotSkillSlot, 0);
		SendDlgItemMessage(IDC_INFARROWSLOT, CB_SETCURSEL, (WPARAM)m_tSettings.dwInfArrowSkillSlot, 0);
	
		CheckDlgButton(IDC_ENABLEINFARROW, m_tSettings.fUseInfArrow ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETPOS, 0, MAKELONG(m_tSettings.dwChangeDirTime, 0));

		TCHAR szTime[256] = {0};
		_stprintf(szTime, _T("%d"), m_tSettings.dwChangeDirTime);
		SetDlgItemText(IDC_CHANGEDIRTIME, szTime);

		BOOL fTemp = TRUE;
		ApplyState(0, 0, 0, fTemp);
	}

BEGIN_MSG_MAP(CAElfSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_ENABLEINFARROW, BN_CLICKED, ApplyState)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		for (int i=0; i < 10; ++i)
		{
			TCHAR szText[256] = {0};
			_stprintf(szText, _T("%d"), i);
			SendDlgItemMessage(IDC_TSHOTSKILLSLOT, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_INFARROWSLOT, CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETRANGE, 0, MAKELONG(10000, 100));

		UDACCEL udAcc[] = {{3, 100}, {7, 500}, {10, 1000}};
		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETACCEL, (WPARAM)(sizeof(udAcc)/sizeof(udAcc[0])), (LPARAM)udAcc);

		return 1;
	}

	LRESULT ApplyState(WORD, WORD, HWND, BOOL&)
	{
		GetDlgItem(IDC_TSHOTSKILLSLOT).EnableWindow(IsDlgButtonChecked(IDC_ENABLEINFARROW) == BST_CHECKED);
		GetDlgItem(IDC_INFARROWSLOT).EnableWindow(IsDlgButtonChecked(IDC_ENABLEINFARROW) == BST_CHECKED);

		return 0;
	}

protected:
	AElfSettings& m_tSettings;
};

#endif //__AElfSettings_H