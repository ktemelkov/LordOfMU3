#pragma once

#include "ApiTable.h"


/**  
 * \brief 
 */
class CUserUtil
{
public:
	/**  
	 * \brief 
	 */
	static HWND CreateDialogParamA(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
	{
		HWND(__stdcall* proc)(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM) = 
			(HWND(__stdcall*)(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM)) CApiTable::GetProc(_USER_API_CreateDialogParamA);

		return proc(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	}

	/**  
	 * \brief 
	 */
	static LRESULT DispatchMessageA(const MSG* lpmsg)
	{
		LRESULT(__stdcall* proc)(const MSG*) = 
			(LRESULT(__stdcall*)(const MSG*)) CApiTable::GetProc(_USER_API_DispatchMessageA);

		return proc(lpmsg);
	}

	/**  
	 * \brief 
	 */
	static BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
	{
		BOOL(__stdcall* proc)(LPMSG, HWND, UINT, UINT) = 
			(BOOL(__stdcall*)(LPMSG, HWND, UINT, UINT)) CApiTable::GetProc(_USER_API_GetMessageA);

		return proc(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	}

	/**  
	 * \brief 
	 */
	static BOOL IsDialogMessageA(HWND hDlg, LPMSG lpMsg)
	{
		BOOL(__stdcall* proc)(HWND, LPMSG) = 
			(BOOL(__stdcall*)(HWND, LPMSG)) CApiTable::GetProc(_USER_API_IsDialogMessageA);

		return proc(hDlg, lpMsg);
	}

	/**  
	 * \brief 
	 */
	static BOOL TranslateMessage(const MSG* lpMsg)
	{
		BOOL(__stdcall* proc)(const MSG*) = 
			(BOOL(__stdcall*)(const MSG*)) CApiTable::GetProc(_USER_API_TranslateMessage);

		return proc(lpMsg);
	}
};