#ifndef __Settings_H
#define __Settings_H

#pragma once

#define CHAR_CLASS_DL 0
#define CHAR_CLASS_EE 1
#define CHAR_CLASS_AE 2
#define CHAR_CLASS_BK 3
#define CHAR_CLASS_SM 4
#define CHAR_CLASS_MG 5
#define CHAR_CLASS_LAST 5


#pragma pack(push)
#pragma pack(1)

struct GeneralSettings
{
	BOOL fAutoPick;
	BOOL fAutoRepair;
	BOOL fAutoLife;
	BOOL fAutoReOff;
	DWORD dwPickTime; //ms
	DWORD dwRepairTime; //ms
	DWORD dwHealTime; //ms
	DWORD dwClass;

	// --- Advanced Settings ---
	BOOL fAdvAutoPick;
	BOOL fAdvPickBless;
	BOOL fAdvPickSoul;
	BOOL fAdvPickLife;
	BOOL fAdvPickGuardian;
	BOOL fAdvPickCreation;
	BOOL fAdvPickChaos;
	BOOL fAdvPickExl;
	BOOL fAdvPickZen;

	// v1.4.4 options
	BOOL fExitAtLvl400;

	// v2.0 options
	int fAntiAFKProtect;

	// v2.1 options
	BOOL fAdvPickBlessMove;
	BOOL fAdvPickSoulMove;
	BOOL fAdvPickLifeMove;
	BOOL fAdvPickGuardianMove;
	BOOL fAdvPickCreationMove;
	BOOL fAdvPickChaosMove;
	BOOL fAdvPickExlMove;
	BOOL fAdvPickZenMove;

	BOOL fAdvPickCustom;
	BOOL fAdvPickCustomMove;
	WORD wPickCustomCode;

	BYTE  arrReserved[906];
};

struct DLSettings
{
	DWORD dwAttackSlot;
	DWORD dwCriticalDmgSlot;
	DWORD dwDarkHorseSlot;
	BOOL  fUseCritDmg;
	BOOL  fUseDarkHorse;
	DWORD dwNoClickMargin;
	BYTE  arrReserved[1000];
};

struct EElfSettings
{
	DWORD dwDmgSkillSlot;
	DWORD dwDefSkillSlot;
	DWORD dwHealSkillSlot;
	DWORD dwPartyMembers;
	BOOL  fUseDmgSkill;
	BOOL  fUseDefSkill;
	BOOL  fUseHealSkill;
	BYTE  arrReserved[996];
};

struct AElfSettings
{
	DWORD dwTShotSkillSlot;
	DWORD dwInfArrowSkillSlot;
	DWORD dwChangeDirTime;
	BOOL  fUseInfArrow;
	BYTE  arrReserved[1008];
};

struct BKSettings
{
	DWORD dwAttackSkillSlot;
	DWORD dwGrFortSkillSlot;
	BOOL  fUseGrFort;
	DWORD dwAttackDistance;
	BYTE  arrReserved[1008];
};

struct SMSettings
{
	DWORD dwAttackSkillSlot;
	DWORD dwManaShldSkillSlot;
	BOOL  fUseManaShld;
	DWORD dwAttackDistance;
	BYTE  arrReserved[1008];
};

struct MGSettings
{
	DWORD dwChangeDirTime;
	BYTE  arrReserved[1020];
};

struct ClickerSettings
{
	DWORD			dwVersion;
	GeneralSettings all;
	DLSettings		dl;
	EElfSettings	ee;
	AElfSettings	ae;
	BKSettings		bk;
	SMSettings		sm;
	MGSettings		mg;
};

#pragma pack(pop)


class CClickerSettings
{
public:
	CClickerSettings()
	{
		memset(m_szFile, 0, sizeof(m_szFile));
		memset(&m_cSettings, 0, sizeof(ClickerSettings));
		m_cSettings.dwVersion = 1;

		// TODO: Set default settings for all char classes
		m_cSettings.all.dwClass = CHAR_CLASS_EE;
		m_cSettings.all.dwHealTime = 5000;
		m_cSettings.all.dwPickTime = 1000;
		m_cSettings.all.dwRepairTime = 10000;
		m_cSettings.all.fAutoLife = FALSE;
		m_cSettings.all.fAutoPick = TRUE;
		m_cSettings.all.fAutoReOff = TRUE;
		m_cSettings.all.fAutoRepair = TRUE;

		m_cSettings.all.fAdvAutoPick = FALSE;
		m_cSettings.all.fAdvPickBless = TRUE;
		m_cSettings.all.fAdvPickSoul = TRUE;
		m_cSettings.all.fAdvPickLife = TRUE;
		m_cSettings.all.fAdvPickGuardian = TRUE;
		m_cSettings.all.fAdvPickCreation = TRUE;
		m_cSettings.all.fAdvPickChaos = TRUE;
		m_cSettings.all.fAdvPickExl = TRUE;
		m_cSettings.all.fAdvPickZen = TRUE;

		m_cSettings.all.fAdvPickBlessMove = TRUE;
		m_cSettings.all.fAdvPickSoulMove = TRUE;
		m_cSettings.all.fAdvPickLifeMove = TRUE;
		m_cSettings.all.fAdvPickGuardianMove = TRUE;
		m_cSettings.all.fAdvPickCreationMove = TRUE;
		m_cSettings.all.fAdvPickChaosMove = TRUE;
		m_cSettings.all.fAdvPickExlMove = TRUE;
		m_cSettings.all.fAdvPickZenMove = FALSE;

		m_cSettings.all.fAdvPickCustom = TRUE;
		m_cSettings.all.fAdvPickCustomMove = TRUE;
		m_cSettings.all.wPickCustomCode = 0x0E33;

		m_cSettings.ae.dwChangeDirTime = 3000;
		m_cSettings.ae.dwTShotSkillSlot = 4;
		m_cSettings.ae.dwInfArrowSkillSlot = 5;
		m_cSettings.ae.fUseInfArrow = TRUE;

		m_cSettings.bk.fUseGrFort = TRUE;
		m_cSettings.bk.dwAttackSkillSlot = 1;
		m_cSettings.bk.dwGrFortSkillSlot = 2;
		m_cSettings.bk.dwAttackDistance = 150;

		m_cSettings.dl.dwAttackSlot = 1;
		m_cSettings.dl.dwCriticalDmgSlot = 2;
		m_cSettings.dl.dwDarkHorseSlot = 3;
		m_cSettings.dl.fUseCritDmg = TRUE;
		m_cSettings.dl.fUseDarkHorse = TRUE;
		m_cSettings.dl.dwNoClickMargin = 70;

		m_cSettings.ee.dwDefSkillSlot = 2;
		m_cSettings.ee.dwDmgSkillSlot = 1;
		m_cSettings.ee.dwHealSkillSlot = 3;
		m_cSettings.ee.dwPartyMembers = 2;
		m_cSettings.ee.fUseDefSkill = TRUE;
		m_cSettings.ee.fUseDmgSkill = TRUE;
		m_cSettings.ee.fUseHealSkill = TRUE;
		
		m_cSettings.mg.dwChangeDirTime = 3000;

		m_cSettings.sm.dwAttackSkillSlot = 2;
		m_cSettings.sm.dwManaShldSkillSlot = 9;
		m_cSettings.sm.fUseManaShld = TRUE;
		m_cSettings.sm.dwAttackDistance = 150;
	}

	CClickerSettings(const CClickerSettings& in)
	{
		operator=(in);
	}

	CClickerSettings& operator=(const CClickerSettings& in)
	{
		memcpy(&m_cSettings, &in.m_cSettings, sizeof(ClickerSettings));
		memcpy(m_szFile, in.m_szFile, sizeof(m_szFile));
		return *this;
	}

	operator ClickerSettings*()
	{
		return &m_cSettings;
	}

	ClickerSettings* operator->()
	{
		return &m_cSettings;
	}

	operator ClickerSettings&()
	{
		return m_cSettings;
	}

	operator const ClickerSettings&()
	{
		return (const ClickerSettings&)m_cSettings;
	}

	BOOL Save(LPCTSTR pszFile = 0)
	{
		if (pszFile == 0 && m_szFile[0] == 0)
			return FALSE; // No filename specified

		if (pszFile != 0)
			_tcsncpy(m_szFile, pszFile, _MAX_PATH);

		BOOL fRes = FALSE;
		FILE* f = _tfopen(m_szFile, _T("wb"));
		
		if (f)
		{
			fRes = (fwrite(&m_cSettings, 1, sizeof(m_cSettings), f) == sizeof(m_cSettings));
			fclose(f);
		}

		return fRes;
	}

	BOOL Load(LPCTSTR pszFile)
	{
		if (pszFile == 0)
			return FALSE; // Invalid argument

		_tcsncpy(m_szFile, pszFile, _MAX_PATH);

		BOOL fRes = FALSE;
		FILE* f = _tfopen(pszFile, _T("rb"));
		
		if (f)
		{
			DWORD dwRepairTime = m_cSettings.all.dwRepairTime;
			fRes = (fread(&m_cSettings, 1, sizeof(m_cSettings), f) == sizeof(m_cSettings));

			m_cSettings.all.dwRepairTime = dwRepairTime;
			fclose(f);
		}

		return fRes;
	}

protected:
	ClickerSettings m_cSettings;
	TCHAR m_szFile[_MAX_PATH+1];
};

#endif //__Settings_H