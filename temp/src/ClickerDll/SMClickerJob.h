#ifndef __SMClickerJob_H
#define __SMClickerJob_H

#pragma once

#include "ClickerJob.h"

class CSMClickerJob
	: public CClickerJob
{
public:
	CSMClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		m_dwManaShldTicks = 0;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();

		m_dwManaShldTicks = GetTickCount() - SM_MANASHLD_SKILL_TIMEOUT - 10;

		srand(m_dwManaShldTicks);

		_left = m_cHudParams.x0 - (int)(m_tSettings.sm.dwAttackDistance*4.0f/3.0f);
		_top = m_cHudParams.y0 - m_tSettings.sm.dwAttackDistance;
		_right = m_cHudParams.x0 + (int)(m_tSettings.sm.dwAttackDistance*4.0f/3.0f);
		_bottom = m_cHudParams.y0 + m_tSettings.sm.dwAttackDistance;

		if (_left < 0)
			_left = 0;

		if (_right > m_cHudParams.play_width)
			_right = m_cHudParams.play_width;

		if (_top < 0)
			_top = 0;

		if (_bottom > m_cHudParams.play_height)
			_bottom = m_cHudParams.play_height;

		if (m_tSettings.sm.fUseManaShld)
			CKeySender::SendAsync((char)(m_tSettings.sm.dwAttackSkillSlot + 0x30), TRUE, 1000);
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		int x = (int)((float)_left + (float)(_right-_left)*((float)rand()/(float)RAND_MAX) + 0.5f);
		int y = (int)((float)_top + (float)(_bottom-_top)*((float)rand()/(float)RAND_MAX) + 0.5f);

		if (IsWindow(m_hWnd))
			SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)(MK_RBUTTON | MK_SHIFT), (LPARAM)MAKELONG(x, y));

		DWORD dwTicks = GetTickCount();

		if ((int)dwTicks - (int)m_dwManaShldTicks > SM_MANASHLD_SKILL_TIMEOUT)
		{
			if (m_tSettings.sm.fUseManaShld)
			{
				CKeySender::SendAsync((char)(m_tSettings.sm.dwManaShldSkillSlot + 0x30), TRUE, 2000);
				Sleep(200);
				CKeySender::SendAsync((char)(m_tSettings.sm.dwAttackSkillSlot + 0x30), TRUE, 3000);
			}

			m_dwManaShldTicks = dwTicks;
		}

		Sleep(10);
	}


protected:
	static const int SM_MANASHLD_SKILL_TIMEOUT = 30000;

private:
	DWORD m_dwManaShldTicks;

	int _left;
	int _right;
	int _top;
	int _bottom;
};

#endif //__SMClickerJob_H