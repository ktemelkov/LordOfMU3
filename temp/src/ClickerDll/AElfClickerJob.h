#ifndef __AElfClickerJob_H
#define __AElfClickerJob_H

#pragma once

#include "ClickerJob.h"
#include <math.h>

class CAElfClickerJob
	: public CClickerJob
{
public:
	CAElfClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		_CurrentAngle = 0.0f;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();

		_CurrentAngle = 0.0f;

		m_dwInfArrowTicks = GetTickCount() - AELF_INF_ARROW_TIMEOUT - 10;
		m_dwChDirTicks = GetTickCount() - m_tSettings.ae.dwChangeDirTime - 10;
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		DWORD dwTicks = GetTickCount();

		if ((int)dwTicks - (int)m_dwInfArrowTicks > AELF_INF_ARROW_TIMEOUT)
		{
			if (m_tSettings.ae.fUseInfArrow)
			{
				CKeySender::SendAsync((char)(m_tSettings.ae.dwInfArrowSkillSlot + 0x30), TRUE, 2000);
				Sleep(200);
				CKeySender::SendAsync((char)(m_tSettings.ae.dwTShotSkillSlot + 0x30), TRUE, 3000);
			}

			m_dwInfArrowTicks = dwTicks;
		}

		if ((int)dwTicks - (int)m_dwChDirTicks > (int)m_tSettings.ae.dwChangeDirTime)
		{
			int x = (int)((float)m_cHudParams.x0 + (float)m_cHudParams.width*AELF_ATTACK_RADIUS()*cosf(_CurrentAngle));
			int y = (int)((float)m_cHudParams.y0 + (float)m_cHudParams.width*AELF_ATTACK_RADIUS()*sinf(_CurrentAngle));

			if (IsWindow(m_hWnd))
				SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)(MK_RBUTTON | MK_SHIFT), (LPARAM)MAKELONG(x, y));

			_CurrentAngle += AELF_ROTATE_STEP();

			if (fabs(_CurrentAngle - PI2()) < 0.01)
			{
				_CurrentAngle = 0.0f;
			}

			m_dwChDirTicks = dwTicks;
		}
	}

protected:
	static const int AELF_INF_ARROW_TIMEOUT = 30000;
	static float AELF_ATTACK_RADIUS() { return 50.0f/1024.0f; }
	static float PI2(){ return 6.2831853f; }
	static float AELF_ROTATE_STEP() { return 0.523598775598f; } // 30 deg.

protected:
	DWORD m_dwInfArrowTicks;
	DWORD m_dwChDirTicks;
	float _CurrentAngle;
};

#endif //__AElfClickerJob_H