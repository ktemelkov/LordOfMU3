#ifndef __LaunchMuDialog_H
#define __LaunchMuDialog_H

#pragma once

#include "resource.h"       // main symbols
#include <atlhost.h>
#include "CreateDialogUtil.h"


class CLaunchMuDialog
	: public CAxDialogImpl<CLaunchMuDialog>
{
public:
	CLaunchMuDialog() : m_hOldCursor(0), m_iShowCursor(0), m_fResult(FALSE) {}

	enum { IDD = IDD_LAUNCH_MESSAGE };

	HWND Create(HWND hWndParent){ return CCreateDialogUtil(hWndParent, this); }

BEGIN_MSG_MAP(CLaunchMuDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
	MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
	MESSAGE_HANDLER(WM_NCPAINT, OnNCPaint)
	CHAIN_MSG_MAP(CAxDialogImpl<CLaunchMuDialog>)
END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL&);
	LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnNCPaint(UINT, WPARAM, LPARAM, BOOL&);

public:
	BOOL m_fResult;

protected:
	HCURSOR m_hOldCursor;
	int m_iShowCursor;
};

inline
LRESULT CLaunchMuDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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

inline
LRESULT CLaunchMuDialog::OnShowWindow(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	if (wParam != 0)
	{
		m_fResult = FALSE;

		HCURSOR hCursor = LoadCursor(0, IDC_ARROW);
		m_hOldCursor = SetCursor(hCursor);

		for (m_iShowCursor=0; ShowCursor(TRUE) < 1 && m_iShowCursor < 100; ++m_iShowCursor);

		GetDlgItem(IDOK).SetFocus();
	}
	else
	{
		SetCursor(m_hOldCursor);
		for (m_iShowCursor; m_iShowCursor >= 0; m_iShowCursor--)
			ShowCursor(FALSE);
	}

	return 0;
}

inline
LRESULT CLaunchMuDialog::OnClickedOK(WORD, WORD, HWND, BOOL&)
{
	m_fResult = TRUE;
	ShowWindow(SW_HIDE);
	return 0;
}

inline
LRESULT CLaunchMuDialog::OnClickedCancel(WORD, WORD, HWND, BOOL&)
{
	m_fResult = FALSE;
	ShowWindow(SW_HIDE);
	return 0;
}

inline
LRESULT CLaunchMuDialog::OnNCPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
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

	TCHAR szCaption[256] = {0};
	GetWindowText(szCaption, 255);

	SetTextColor(hDC, RGB(255, 255, 255));
	DrawText(hDC, szCaption, (int)_tcslen(szCaption), &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	ReleaseDC(hDC);
	return 0;
}

#endif //__LaunchMuDialog_H