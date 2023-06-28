#ifndef __ClassClicker_H
#define __ClassClicker_H

#pragma once

#include "ClickerJob.h"
#include "SimpleClickerJob.h"
#include "DarkLordClickerJob.h"
#include "EElfClickerJob.h"
#include "AElfClickerJob.h"
#include "BKClickerJob.h"
#include "MGClickerJob.h"
#include "SMClickerJob.h"


class CClickerJobFactory
{
public:
	static CClickerJob* CreateClickerJob(HWND inHwnd, ULONG ulVersion, const ClickerSettings& inSettings, BOOL fNoMouseMove, BOOL fHealOnly)
	{
		if (fNoMouseMove)
		{
			return new CSimpleClickerJob(inHwnd, ulVersion, inSettings, fHealOnly);
		}
		
		switch (inSettings.all.dwClass)
		{
		default:
		case CHAR_CLASS_DL:
			return new CDarkLordClickerJob(inHwnd, ulVersion, inSettings);
		case CHAR_CLASS_EE:
			return new CEElfClickerJob(inHwnd, ulVersion, inSettings);
		case CHAR_CLASS_AE:
			return new CAElfClickerJob(inHwnd, ulVersion, inSettings);
		case CHAR_CLASS_BK:
			return new CBKClickerJob(inHwnd, ulVersion, inSettings);
		case CHAR_CLASS_SM:
			return new CSMClickerJob(inHwnd, ulVersion, inSettings);
		case CHAR_CLASS_MG:
			return new CMGClickerJob(inHwnd, ulVersion, inSettings);
		}

		return 0;
	}
};

#endif //__ClassClicker_H