#ifndef __SimpleClickerJob_H
#define __SimpleClickerJob_H

#pragma once

#include "ClickerJob.h"


class CSimpleClickerJob 
	: public CClickerJob
{
public:
	CSimpleClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings, BOOL fHealOnly = FALSE)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		m_fHealOnly = fHealOnly;
	}

protected:
	virtual void DoClicker()
	{
		DWORD dwTick = GetTickCount();

		if (!m_fHealOnly && ((int)dwTick - (int)m_dwClickTicks > RCLICK_TIMEOUT))
		{
			POINT ptClick = {0};

			if (GetForegroundWindow() == m_hWnd)
			{
				GetCursorPos(&ptClick);
				ScreenToClient(m_hWnd, &ptClick);

				RECT rc = {0};
				GetClientRect(m_hWnd, &rc);

				if (ptClick.x < 0 || ptClick.x >= rc.right)
					ptClick.x = rc.right/2;

				if (ptClick.y < 0 || ptClick.y >= rc.bottom)
					ptClick.y = rc.bottom/2;
			}
			else
			{
				ptClick = m_ptMouseStartPos;
			}

			if (IsWindow(m_hWnd))
			{
				SendMessage(m_hWnd, WM_CLICKER_JOB_RBUTTONUP, (WPARAM)0, (LPARAM)MAKELONG(ptClick.x, ptClick.y));
				SendMessage(m_hWnd, WM_CLICKER_JOB_RBUTTONDOWN, (WPARAM)MK_RBUTTON, (LPARAM)MAKELONG(ptClick.x, ptClick.y));
			}

			ReleaseCapture();
			ClipCursor(0);

			m_dwClickTicks = GetTickCount();
		}

		if ((int)dwTick - (int)m_dwHealTicks >= (int)m_tSettings.all.dwHealTime)
		{
			if (m_tSettings.all.fAutoLife)
			{
				CKeySender::SendAsync('Q', TRUE, 250);
			}

			m_dwHealTicks = dwTick;
		}

		if ((int)dwTick - (int)m_dwRepairTicks > (int)m_tSettings.all.dwRepairTime)
		{
			if (m_tSettings.all.fAutoRepair)
			{
				DoRepair();
			}

			m_dwRepairTicks = dwTick;
		}

		Sleep(10);
	}

private:
	BOOL m_fHealOnly;
};


#endif //__SimpleClickerJob_H