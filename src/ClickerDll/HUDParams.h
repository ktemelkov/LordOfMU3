#ifndef __HUDParams_H
#define __HUDParams_H

#pragma once

class CHUDParams
{
public:
	CHUDParams() 
	{
		m_hWndMu = 0;
		memset(&rcClient, 0, sizeof(RECT));
		x0 = 0;
		y0 = 0;
		play_width = 0;
		play_height = 0;
	}

	CHUDParams(HWND hWndMu, ULONG ulVersion)
	{
		Init(hWndMu, ulVersion);
	}

	void Init(HWND hWndMu, ULONG ulVersion)
	{
		m_hWndMu = hWndMu;
		m_ulVersion = ulVersion;

		GetClientRect(m_hWndMu, &rcClient);

		width = rcClient.right - rcClient.left;
		height = rcClient.bottom - rcClient.top;

		play_width = width;
		play_height = height - (int)((float)height*MU_BOTTOM_PANEL_HEIGTH());

		x0 = (int)(MU_CHARACTER_X0()*(float)width);
		y0 = (int)(MU_CHARACTER_Y0()*(float)height);
	}

public:
	int x0;
	int y0;

	int play_width;
	int play_height;

	int width;
	int height;

	RECT rcClient;

	float MU_BOTTOM_PANEL_HEIGTH() { return 0.098f; }
	float MU_CHARACTER_X0(){ return 0.5f; }
	float MU_CHARACTER_Y0(){ return (m_ulVersion >= 105) ? 0.41927f : 0.427083f; }

protected:
	HWND m_hWndMu;
	ULONG m_ulVersion;
};

#endif //__HUDParams_H