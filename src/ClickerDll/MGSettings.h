#ifndef __MGSettings_H
#define __MGSettings_H

#pragma once

#include "resource.h"       // main symbols
#include "Settings.h"
#include "CreateDialogUtil.h"

class CMGSettings :
	public CDialogImpl<CMGSettings>
{
public:
	CMGSettings(MGSettings& in)
		: m_tSettings(in) {}

	enum { IDD = IDD_MG };
	
	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

public:
	void Apply()
	{
		m_tSettings.dwChangeDirTime = (DWORD)SendDlgItemMessage(IDC_CHDIRSPIN, UDM_GETPOS);
	}

	void InitValues()
	{
		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETPOS, 0, MAKELONG(m_tSettings.dwChangeDirTime, 0));

		TCHAR szTime[256] = {0};
		_stprintf(szTime, _T("%d"), m_tSettings.dwChangeDirTime);
		SetDlgItemText(IDC_CHANGEDIRTIME, szTime);
	}

BEGIN_MSG_MAP(CMGSettings)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETRANGE, 0, MAKELONG(10000, 100));

		UDACCEL udAcc[] = {{3, 100}, {7, 500}, {10, 1000}};
		SendDlgItemMessage(IDC_CHDIRSPIN, UDM_SETACCEL, (WPARAM)(sizeof(udAcc)/sizeof(udAcc[0])), (LPARAM)udAcc);

		return 1;
	}

protected:
	MGSettings& m_tSettings;
};

#endif //__MGSettings_H