#ifndef __DarkLordClickerJob_H
#define __DarkLordClickerJob_H

#pragma once

#include "ClickerJob.h"

class CDarkLordClickerJob
	: public CClickerJob
{
public:
	CDarkLordClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		m_dwHorseTicks = 0;
		m_dwCritDmgTicks = 0;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();
		
		m_dwHorseTicks = GetTickCount() - DARK_LORD_HORSE_SKILL_TIMEOUT - 10;
		m_dwCritDmgTicks = GetTickCount() - DARK_LORD_CDAMAGE_SKILL_TIMEOUT - 10;

		srand(m_dwCritDmgTicks);

		_left = m_tSettings.dl.dwNoClickMargin;
		_top = m_tSettings.dl.dwNoClickMargin;
		_right = m_cHudParams.play_width - _left;
		_bottom = m_cHudParams.play_height;

		if (m_tSettings.dl.fUseCritDmg || m_tSettings.dl.fUseDarkHorse)
			CKeySender::SendAsync((char)(m_tSettings.dl.dwAttackSlot + 0x30), TRUE, 1000);
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		int x = (int)((float)_left + (float)(_right-_left)*((float)rand()/(float)RAND_MAX) + 0.5f);
		int y = (int)((float)_top + (float)(_bottom-_top)*((float)rand()/(float)RAND_MAX) + 0.5f);

		if (IsWindow(m_hWnd))
			SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)(MK_RBUTTON | MK_SHIFT), (LPARAM)MAKELONG(x, y));

		DWORD dwTicks = GetTickCount();

		if ((int)dwTicks - (int)m_dwCritDmgTicks > DARK_LORD_CDAMAGE_SKILL_TIMEOUT)
		{
			if (m_tSettings.dl.fUseCritDmg)
			{
				CKeySender::SendAsync((char)(m_tSettings.dl.dwCriticalDmgSlot + 0x30), TRUE, 2000);
				Sleep(200);
				CKeySender::SendAsync((char)(m_tSettings.dl.dwAttackSlot + 0x30), TRUE, 3000);
			}

			m_dwCritDmgTicks = dwTicks;
		}

		if ((int)dwTicks - (int)m_dwHorseTicks > DARK_LORD_HORSE_SKILL_TIMEOUT)
		{
			if (m_tSettings.dl.fUseDarkHorse)
			{
				CKeySender::SendAsync((char)(m_tSettings.dl.dwDarkHorseSlot + 0x30), TRUE, 2000);
				Sleep(200);
				CKeySender::SendAsync((char)(m_tSettings.dl.dwAttackSlot + 0x30), TRUE, 3000);
			}

			m_dwHorseTicks = dwTicks;
		}

		Sleep(10);
	}

protected:
	static const int DARK_LORD_HORSE_SKILL_TIMEOUT = 15000;
	static const int DARK_LORD_CDAMAGE_SKILL_TIMEOUT = 30000;

private:
	DWORD m_dwHorseTicks;
	DWORD m_dwCritDmgTicks;

	int _left;
	int _right;
	int _top;
	int _bottom;
};

#endif //__DarkLordClickerJob_H