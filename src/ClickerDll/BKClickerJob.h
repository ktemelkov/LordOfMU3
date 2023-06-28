#ifndef __BKClickerJob_H
#define __BKClickerJob_H

#pragma once
#include "ClickerJob.h"

class CBKClickerJob
	: public CClickerJob
{
public:
	CBKClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		m_dwGrFortTicks = 0;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();

		m_dwGrFortTicks = GetTickCount() - BK_GRFORT_SKILL_TIMEOUT - 10;
		srand(m_dwGrFortTicks);

		_left = m_cHudParams.x0 - (int)(m_tSettings.bk.dwAttackDistance*4.0f/3.0f);
		_top = m_cHudParams.y0 - m_tSettings.bk.dwAttackDistance;
		_right = m_cHudParams.x0 + (int)(m_tSettings.bk.dwAttackDistance*4.0f/3.0f);
		_bottom = m_cHudParams.y0 + m_tSettings.bk.dwAttackDistance;

		if (_left < 0)
			_left = 0;

		if (_right > m_cHudParams.play_width)
			_right = m_cHudParams.play_width;

		if (_top < 0)
			_top = 0;

		if (_bottom > m_cHudParams.play_height)
			_bottom = m_cHudParams.play_height;

		if (m_tSettings.bk.fUseGrFort)
			CKeySender::SendAsync((char)(m_tSettings.bk.dwAttackSkillSlot + 0x30), TRUE, 1000);
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		int x = (int)((float)_left + (float)(_right-_left)*((float)rand()/(float)RAND_MAX) + 0.5f);
		int y = (int)((float)_top + (float)(_bottom-_top)*((float)rand()/(float)RAND_MAX) + 0.5f);

		if (IsWindow(m_hWnd))
			SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)(MK_RBUTTON | MK_SHIFT), (LPARAM)MAKELONG(x, y));

		DWORD dwTicks = GetTickCount();

		if ((int)dwTicks - (int)m_dwGrFortTicks > BK_GRFORT_SKILL_TIMEOUT)
		{
			if (m_tSettings.bk.fUseGrFort)
			{
				CKeySender::SendAsync((char)(m_tSettings.bk.dwGrFortSkillSlot + 0x30), TRUE, 2000);
				Sleep(200);
				CKeySender::SendAsync((char)(m_tSettings.bk.dwAttackSkillSlot + 0x30), TRUE, 3000);
			}

			m_dwGrFortTicks = dwTicks;
		}

		Sleep(10);
	}

protected:
	static const int BK_GRFORT_SKILL_TIMEOUT = 30000;

protected:
	DWORD m_dwGrFortTicks;

	int _left;
	int _right;
	int _top;
	int _bottom;
};

#endif //__BKClickerJob_H