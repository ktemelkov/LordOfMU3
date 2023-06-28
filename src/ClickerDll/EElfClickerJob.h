#ifndef __EElfClickerJob_H
#define __EElfClickerJob_H

#pragma once

#include "ClickerJob.h"

class CEElfClickerJob
	: public CClickerJob
{
public:
	CEElfClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings)
		: CClickerJob(inHwnd, ulVersion, inSettings)
	{
		memset(_vBuffs, 0, sizeof(_vBuffs));
		_CurrentBuff = 0;
		_CurrentPlayer = 0;
	}

protected:
	virtual void InitClicker()
	{
		CClickerJob::InitClicker();

		_CurrentPlayer = 0;

		if (IsWindow(m_hWnd))
			SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)MK_RBUTTON, (LPARAM)MAKELONG(m_cHudParams.x0, m_cHudParams.y0));

		_CurrentBuff = 0;
		_NumberOfBuffs = 0;

		if (m_tSettings.ee.fUseDefSkill)
		{
			_vBuffs[_NumberOfBuffs++] = m_tSettings.ee.dwDefSkillSlot;
		}

		if (m_tSettings.ee.fUseDmgSkill)
		{
			_vBuffs[_NumberOfBuffs++] = m_tSettings.ee.dwDmgSkillSlot;
		}

		if (m_tSettings.ee.fUseHealSkill)
		{
			_vBuffs[_NumberOfBuffs++] = m_tSettings.ee.dwHealSkillSlot;
		}

		if (_NumberOfBuffs != 0)
		{
			CKeySender::SendAsync((char)(_vBuffs[_CurrentBuff]+0x30), TRUE, 1000);
		}

		m_dwChPlayerTicks = GetTickCount();
	}

	virtual void DoClicker()
	{
		CClickerJob::DoClicker();

		DWORD dwTicks = GetTickCount();

		if (m_tSettings.ee.dwPartyMembers > 1)
		{
			int x = (int)((955.0f/1024.0f)*(float)m_cHudParams.width);
			int y = (int)(((23.0f/768.0f) + (31.0f/768.0f)*(float)_CurrentPlayer)*(float)m_cHudParams.height);

			if (m_ulVersion >= 105)
			{
				x = (int)((955.0f/1024.0f)*(float)m_cHudParams.width);
				y = (int)(((38.0f/768.0f) + (39.0f/768.0f)*(float)_CurrentPlayer)*(float)m_cHudParams.height);
			}

			if (IsWindow(m_hWnd))
				SendMessage(m_hWnd, WM_CLICKER_JOB_MOUSEMOVE, (WPARAM)MK_RBUTTON, (LPARAM)MAKELONG(x, y));
		}

		if ((int)dwTicks - (int)m_dwChPlayerTicks > EELF_BUFF_PLAYER_TIMEOUT)
		{
			// Next player
			_CurrentPlayer = (_CurrentPlayer+1) % m_tSettings.ee.dwPartyMembers;

			if (_CurrentPlayer == 0 && _NumberOfBuffs > 0)
			{
				 // Next buff
				_CurrentBuff = (_CurrentBuff+1) % _NumberOfBuffs;
				CKeySender::SendAsync((char)(_vBuffs[_CurrentBuff]+0x30), TRUE, 1000);
			}

			m_dwChPlayerTicks = dwTicks;
		}

		Sleep(10);
	}

protected:
	static const int EELF_BUFF_PLAYER_TIMEOUT = 1700;

protected:
	DWORD _vBuffs[3];
	int   _CurrentBuff;
	int   _NumberOfBuffs;
	int _CurrentPlayer;
	DWORD m_dwChPlayerTicks;
};

#endif //__EElfClickerJob_H