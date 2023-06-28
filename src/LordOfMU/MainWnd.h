#pragma once


#include "../_Shared/ProtocolSettings.h"


/**  
 * \brief 
 */
class CMainWnd
{
public:
	CMainWnd(CProtocolSettings& settings);
	virtual ~CMainWnd();

	bool Create();
	int GetResult(){ return m_iDlgResult; }


private:
	static LRESULT CALLBACK MyCtrlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleInit();
	LRESULT HandleCommand(WPARAM, LPARAM);
	LRESULT HandleDrawItem(WPARAM, LPARAM);
	LRESULT HandleErase(WPARAM, LPARAM);
	LRESULT HandleDestroy();

private:
	HBITMAP m_hBg;
	int m_iDlgResult;
	CProtocolSettings& m_cSettings;

public:
	HWND m_hWnd;
};