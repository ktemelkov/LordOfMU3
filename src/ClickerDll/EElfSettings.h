#ifndef __EElfSettings_H
#define __EElfSettings_H

#pragma once

#include "resource.h"       // main symbols
#include "Settings.h"
#include "CreateDialogUtil.h"

class CEElfSettings :
	public CDialogImpl<CEElfSettings>
{
public:
	CEElfSettings(EElfSettings& in)
		: m_tSettings(in) {}

	enum { IDD = IDD_EELF };

	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

public:
	void Apply()
	{
		m_tSettings.dwDmgSkillSlot = (DWORD)SendDlgItemMessage(IDC_DMGSKILLSLOT, CB_GETCURSEL);
		m_tSettings.dwDefSkillSlot = (DWORD)SendDlgItemMessage(IDC_DEFSKILLSLOT, CB_GETCURSEL);
		m_tSettings.dwHealSkillSlot = (DWORD)SendDlgItemMessage(IDC_HEALSKILLSLOT, CB_GETCURSEL);

		m_tSettings.dwPartyMembers = (DWORD)SendDlgItemMessage(IDC_PARTYMEMBERS, CB_GETCURSEL) + 1;

		m_tSettings.fUseDmgSkill = IsDlgButtonChecked(IDC_ENABLEDMGSKILL) == BST_CHECKED;
		m_tSettings.fUseDefSkill = IsDlgButtonChecked(IDC_ENABLEDEFSKILL) == BST_CHECKED;
		m_tSettings.fUseHealSkill = IsDlgButtonChecked(IDC_ENABLEHEALSKILL) == BST_CHECKED;
	}

	void InitValues()
	{
		SendDlgItemMessage(IDC_DMGSKILLSLOT, CB_SETCURSEL, (WPARAM)m_tSettings.dwDmgSkillSlot, 0);
		SendDlgItemMessage(IDC_DEFSKILLSLOT, CB_SETCURSEL, (WPARAM)m_tSettings.dwDefSkillSlot, 0);
		SendDlgItemMessage(IDC_HEALSKILLSLOT, CB_SETCURSEL, (WPARAM)m_tSettings.dwHealSkillSlot, 0);

		SendDlgItemMessage(IDC_PARTYMEMBERS, CB_SETCURSEL, (WPARAM)((int)m_tSettings.dwPartyMembers-1), 0);

		CheckDlgButton(IDC_ENABLEDMGSKILL, m_tSettings.fUseDmgSkill ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ENABLEDEFSKILL, m_tSettings.fUseDefSkill ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ENABLEHEALSKILL, m_tSettings.fUseHealSkill ? BST_CHECKED : BST_UNCHECKED);

		BOOL fTemp = TRUE;
		ApplyState(0, 0, 0, fTemp);
	}

BEGIN_MSG_MAP(CEElfSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_ENABLEDMGSKILL, BN_CLICKED, ApplyState)
	COMMAND_HANDLER(IDC_ENABLEDEFSKILL, BN_CLICKED, ApplyState)
	COMMAND_HANDLER(IDC_ENABLEHEALSKILL, BN_CLICKED, ApplyState)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		for (int i=0; i < 10; ++i)
		{
			TCHAR szText[256] = {0};
			_stprintf(szText, _T("%d"), i);
			SendDlgItemMessage(IDC_DMGSKILLSLOT, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_DEFSKILLSLOT, CB_ADDSTRING, 0, (LPARAM)szText);
			SendDlgItemMessage(IDC_HEALSKILLSLOT, CB_ADDSTRING, 0, (LPARAM)szText);
		}

		SendDlgItemMessage(IDC_PARTYMEMBERS, CB_ADDSTRING, 0, (LPARAM)_T("none"));
		for (int i=2; i < 6; ++i)
		{
			TCHAR szText[256] = {0};
			_stprintf(szText, _T("%d"), i);
			SendDlgItemMessage(IDC_PARTYMEMBERS, CB_ADDSTRING, 0, (LPARAM)szText);
		}

		return 1;
	}

	LRESULT ApplyState(WORD, WORD, HWND, BOOL&)
	{
		GetDlgItem(IDC_DMGSKILLSLOT).EnableWindow(IsDlgButtonChecked(IDC_ENABLEDMGSKILL) == BST_CHECKED);
		GetDlgItem(IDC_DEFSKILLSLOT).EnableWindow(IsDlgButtonChecked(IDC_ENABLEDEFSKILL) == BST_CHECKED);
		GetDlgItem(IDC_HEALSKILLSLOT).EnableWindow(IsDlgButtonChecked(IDC_ENABLEHEALSKILL) == BST_CHECKED);
		return 0;
	}

protected:
	EElfSettings& m_tSettings;
};

#endif //__EElfSettings_H