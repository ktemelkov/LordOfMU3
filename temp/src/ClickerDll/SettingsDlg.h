// SettingsDlg.h : Declaration of the CSettingsDlg

#pragma once

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "Settings.h"
#include "DarkLordSettings.h"
#include "EElfSettings.h"
#include "AElfSettings.h"
#include "BKSettings.h"
#include "MGSettings.h"
#include "SMSettings.h"

// CSettingsDlg

class CSettingsDlg : 
	public CAxDialogImpl<CSettingsDlg>
{
public:
	CSettingsDlg()
		: m_cSettings()
		, m_cDarkLordSettings(m_cSettings->dl)
		, m_cEElfSettings(m_cSettings->ee)
		, m_cAElfSettings(m_cSettings->ae)
		, m_cBKSettings(m_cSettings->bk)
		, m_cMGSettings(m_cSettings->mg)
		, m_cSMSettings(m_cSettings->sm)
	{
		m_hOldCursor = 0;
		m_iShowCursor = 0;

		m_dwFeatures = 0xFFFFFFFF;
	}

	virtual ~CSettingsDlg()
	{
	}

	enum { IDD = IDD_SETTINGSDLG };

public:
	BOOL LoadSettings(LPCTSTR pszFileName);
	const ClickerSettings& GetSettings() const { return (const ClickerSettings&)m_cSettings; }
	
	CClickerSettings& GetSettingsObj() { return m_cSettings; }

	void SetFeatures(DWORD dwFeatures){ m_dwFeatures = dwFeatures; }

BEGIN_MSG_MAP(CSettingsDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	COMMAND_HANDLER(IDC_CHARCLASS, CBN_SELENDOK, OnSelectCharClass)
	COMMAND_HANDLER(IDC_AUTOHEAL, BN_CLICKED, OnCheckAutoHeal)
	COMMAND_HANDLER(IDC_STOPPICK, BN_CLICKED, OnCheckStopPick)
	MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
	MESSAGE_HANDLER(WM_NCPAINT, OnNCPaint)
	CHAIN_MSG_MAP(CAxDialogImpl<CSettingsDlg>)
END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL&);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnSelectCharClass(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCheckAutoHeal(WORD, WORD, HWND, BOOL&);
	LRESULT OnCheckStopPick(WORD, WORD, HWND, BOOL&);
	LRESULT OnNCPaint(UINT, WPARAM, LPARAM, BOOL&);

protected:
	void InitValues();
	void Apply();

protected:
	HCURSOR m_hOldCursor;
	int m_iShowCursor;
	DWORD m_dwFeatures;

	CClickerSettings    m_cSettings;

	CDarkLordSettings   m_cDarkLordSettings;
	CEElfSettings		m_cEElfSettings;
	CAElfSettings		m_cAElfSettings;
	CBKSettings			m_cBKSettings;
	CMGSettings			m_cMGSettings;
	CSMSettings			m_cSMSettings;
};


