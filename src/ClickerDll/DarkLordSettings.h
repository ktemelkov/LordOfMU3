#ifndef __DarkLordSettings_H
#define __DarkLordSettings_H

#pragma once

#include "resource.h"       // main symbols
#include "Settings.h"
#include <atlhost.h>
#include "CreateDialogUtil.h"

class CDarkLordSettings :
	public CDialogImpl<CDarkLordSettings>
{
public:
	CDarkLordSettings(DLSettings& in)
		: m_tSettings(in) {}

	enum { IDD = IDD_DARKLORD };

	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

public:
	void Apply()
	{
		m_tSettings.dwAttackSlot = (DWORD)SendDlgItemMessage(IDC_DL_ATTACKSKILL, CB_GETCURSEL);
		m_tSettings.dwCriticalDmgSlot = (DWORD)SendDlgItemMessage(IDC_CRITDMGSKILL, CB_GETCURSEL);
		m_tSettings.dwDarkHorseSlot = (DWORD)SendDlgItemMessage(IDC_DARKHORSESKILL, CB_GETCURSEL);

		m_tSettings.fUseCritDmg = IsDlgButtonChecked(IDC_ENABLECRITDMG) == BST_CHECKED;
		m_tSettings.fUseDarkHorse = IsDlgButtonChecked(IDC_ENABLEDARKHORSE) == BST_CHECKED;

		m_tSettings.dwNoClickMargin = (DWORD)SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_GETPOS);
	}

	void InitValues()
	{
		SendDlgItemMessage(IDC_DL_ATTACKSKILL, CB_SETCURSEL, (WPARAM)m_tSettings.dwAttackSlot, 0);
		SendDlgItemMessage(IDC_CRITDMGSKILL, CB_SETCURSEL, (WPARAM)m_tSettings.dwCriticalDmgSlot, 0);
		SendDlgItemMessage(IDC_DARKHORSESKILL, CB_SETCURSEL, (WPARAM)m_tSettings.dwDarkHorseSlot, 0);
	
		CheckDlgButton(IDC_ENABLECRITDMG, m_tSettings.fUseCritDmg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ENABLEDARKHORSE, m_tSettings.fUseDarkHorse ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETPOS, 0, MAKELONG(m_tSettings.dwNoClickMargin, 0));

		TCHAR szTime[256] = {0};
		_stprintf(szTime, _T("%d"), m_tSettings.dwNoClickMargin);
		SetDlgItemText(IDC_NOCLICKMARGIN, szTime);

		BOOL fTemp = TRUE;
		ApplyState(0, 0, 0, fTemp);
	}

BEGIN_MSG_MAP(CDarkLordSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_ENABLECRITDMG, BN_CLICKED, ApplyState)
	COMMAND_HANDLER(IDC_ENABLEDARKHORSE, BN_CLICKED, ApplyState)
//	CHAIN_MSG_MAP(CDialogImpl<CDarkLordSettings>)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		for (int i=0; i < 10; ++i)
		{
			TCHAR szText[256] = {0};
			_stprintf(szText, _T("%d"), i);
			SendDlgItemMessage(IDC_DL_ATTACKSKILL, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_CRITDMGSKILL, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_DARKHORSESKILL, CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETRANGE, 0, MAKELONG(150, 0));

		UDACCEL udAcc[] = {{3, 1}, {7, 5}, {10, 10}};
		SendDlgItemMessage(IDC_NOCLICKSPIN, UDM_SETACCEL, (WPARAM)(sizeof(udAcc)/sizeof(udAcc[0])), (LPARAM)udAcc);

		return 1;
	}

	LRESULT ApplyState(WORD, WORD, HWND, BOOL&)
	{
		GetDlgItem(IDC_CRITDMGSKILL).EnableWindow(IsDlgButtonChecked(IDC_ENABLECRITDMG) == BST_CHECKED);
		GetDlgItem(IDC_DARKHORSESKILL).EnableWindow(IsDlgButtonChecked(IDC_ENABLEDARKHORSE) == BST_CHECKED);
		GetDlgItem(IDC_DL_ATTACKSKILL).EnableWindow(IsDlgButtonChecked(IDC_ENABLECRITDMG) == BST_CHECKED 
													|| IsDlgButtonChecked(IDC_ENABLEDARKHORSE) == BST_CHECKED);
		return 0;
	}

protected:
	DLSettings& m_tSettings;
};


#endif //__DarkLordSettings_H