#ifndef __MGClickerJob_H
#define __MGClickerJob_H

#pragma once

#include "ClickerJob.h"


class CMGClickerJob
	: public CClickerJob
{
public:
	CMGClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		_CurrentAngle = 0.0f;
		m_dwChDirTicks = 0;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();

		_CurrentAngle = 0.0f;
		m_dwChDirTicks = GetTickCount() - m_tSettings.mg.dwChangeDirTime - 10;
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		DWORD dwTicks = GetTickCount();

		if ((int)dwTicks - (int)m_dwChDirTicks > (int)m_tSettings.ae.dwChangeDirTime)
		{
			int x = (int)((float)m_cHudParams.x0 + (float)m_cHudParams.width*MG_ATTACK_RADIUS()*cosf(_CurrentAngle));
			int y = (int)((float)m_cHudParams.y0 + (float)m_cHudParams.width*MG_ATTACK_RADIUS()*sinf(_CurrentAngle));

			if (IsWindow(m_hWnd))
				SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)(MK_RBUTTON | MK_SHIFT), (LPARAM)MAKELONG(x, y));

			_CurrentAngle += MG_ROTATE_STEP();

			if (fabs(_CurrentAngle - PI2()) < 0.01)
			{
				_CurrentAngle = 0.0f;
			}

			m_dwChDirTicks = dwTicks;
		}
	}

protected:
	static float MG_ATTACK_RADIUS() { return 50.0f/1024.0f; }
	static float PI2(){ return 6.2831853f; }
	static float MG_ROTATE_STEP() { return 0.523598775598f; } // 30 deg.

protected:
	DWORD m_dwChDirTicks;
	float _CurrentAngle;
};


#endif //__MGClickerJob_H