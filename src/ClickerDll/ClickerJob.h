#ifndef __ClickerJob_H
#define __ClickerJob_H

#pragma once

#include "Settings.h"
#include "HUDParams.h"
#include "KeySender.h"
#include "Packet.h"
#include "MuMessages.h"



/**
 * \brief 
 */
class CClickerJob
{
	struct POINTF
	{
		float x;
		float y;
		DWORD dwWait;
	};

public:
	CClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings);
	virtual ~CClickerJob();

	BOOL Start(); 
	void Stop();


protected:
	virtual void InitClicker();
	virtual void TermClicker();
	virtual void DoClicker();

	virtual void DoHeal();
	virtual void DoPickUp();
	virtual void DoRepair();

	void SetAdvancedAutopickOptions();
	void SetAutopickOption(const char* opt, BOOL fPick, BOOL fMove);

protected:
	static POINTF* GetRepairCoords()
	{
		static POINTF coords[] =
		{
//			{829.0f/1024.0f, 647.0f/768.0f, 1000}, // Repair button
			{875.0f/1024.0f, 105.0f/768.0f, 100},  // helm
			{950.0f/1024.0f, 105.0f/768.0f, 100},  // wings
			{775.0f/1024.0f, 190.0f/768.0f, 100},  // weapon
			{828.0f/1024.0f, 157.0f/768.0f, 100},  // necklace
			{875.0f/1024.0f, 190.0f/768.0f, 100},  // armor
			{970.0f/1024.0f, 190.0f/768.0f, 100},  // shield
			{775.0f/1024.0f, 275.0f/768.0f, 100},  // gloves
			{828.0f/1024.0f, 260.0f/768.0f, 100},  // left ring
			{875.0f/1024.0f, 275.0f/768.0f, 100},  // pants
			{923.0f/1024.0f, 261.0f/768.0f, 100},  // right ring
			{971.0f/1024.0f, 275.0f/768.0f, 100},  // boots
//			{676.0f/1024.0f, 743.0f/768.0f, 100},
			{0.0f, 0.0f}
		};

		return coords;
	}

private:
	static DWORD WINAPI ThreadProc(void*);

protected:
	HWND m_hWnd;
	POINT m_ptMouseStartPos;

	ClickerSettings m_tSettings;
	CHUDParams m_cHudParams;

	static const int RCLICK_TIMEOUT = 1000;

protected:
	DWORD m_dwHealTicks;
	DWORD m_dwPickUpTicks;
	DWORD m_dwRepairTicks;
	DWORD m_dwClickTicks;
	
	ULONG m_ulVersion;

private:
	HANDLE m_hThread;
	HANDLE m_hStopEvent;
};


#endif //__ClickerJob_H