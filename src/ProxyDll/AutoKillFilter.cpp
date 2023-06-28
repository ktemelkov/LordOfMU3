#include "stdafx.h"
#include "AutoKillFilter.h"
#include "CommonPackets.h"
#include <math.h>

/**
 * \brief 
 */
CAutoKillFilter::CAutoKillFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy)
{
	m_fKillAllPl = false;
	m_fEnabled = FALSE;
	m_fSuspended = false;
	m_dwDelay = 100;
	m_dwSkill = 0;
	m_wPlayerId = 0;
	m_bX = 0;
	m_bY = 0;
	m_bX0 = 0;
	m_bY0 = 0;
	m_iDist = 5;
	m_fAfkIsOn = false;
	m_fSuspendMove = false;

	InitializeCriticalSection(&m_csQueue);

	m_hKillEvent = CreateEvent(0, 1, 0, 0);
	m_hStopEvent = CreateEvent(0, 1, 0, 0);
	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)KillThreadProc, this, 0, 0);
}


/**
 * \brief 
 */
CAutoKillFilter::~CAutoKillFilter()
{	
	if (m_hThread && m_hThread != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hStopEvent);

		Sleep(100);

		if (WaitForSingleObject(m_hThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 0);

		CloseHandle(m_hThread);
	}

	CloseHandle(m_hStopEvent);
	CloseHandle(m_hKillEvent);
	DeleteCriticalSection(&m_csQueue);
}


/**
 * \brief 
 */
int CAutoKillFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CMeetMonsterPacket::Type())
	{
		CMeetMonsterPacket& pktMMonster = (CMeetMonsterPacket&)pkt;

		for (int i = pktMMonster.GetCount()-1; i >= 0; --i)
		{
			BYTE x=0, y=0, x0=0, y0=0; 
			WORD wId = pktMMonster.GetId(i);
			WORD wType = pktMMonster.GetClass(i);

			if (wId != 0 && wType != 100 && wType != 101 && wType != 102 && wType != 103)
			{
				pktMMonster.GetPos(i, x, y, &x0, &y0);
				m_vMobList.insert(std::pair<WORD,DWORD>(wId, MAKELONG(MAKEWORD(x,y),MAKEWORD(x0,y0))));
			}
		}

		UpdateKillQueue();
	}
	else if (pkt == CMeetPlayerPacket::Type())
	{
		if (m_fKillAllPl || m_vKillPlayers.size() != 0)
		{
			CMeetPlayerPacket& pktMMonster = (CMeetPlayerPacket&)pkt;

			for (int i = pktMMonster.GetPlayerCount()-1; i >= 0; --i)
			{
				CStringA strName = pktMMonster.GetPlayerName(i);
				strName.MakeLower();

				if (m_fKillAllPl || m_vKillPlayers.find(strName) != m_vKillPlayers.end())
				{
					BYTE x=0, y=0; 
					WORD wId = pktMMonster.GetPlayerId(i);
					
					CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
					WORD myId = 0;

					if (pFilter)
						pFilter->GetParam("PlayerId", (void*)&myId);

					if (myId != wId && pktMMonster.GetPos(i, x, y))
						m_vMobList.insert(std::pair<WORD,DWORD>(wId, MAKELONG(MAKEWORD(x,y),MAKEWORD(x,y))));
				}
			}

			UpdateKillQueue();
		}
	}
	else if (pkt == CForgetObjectPacket::Type())
	{
		CForgetObjectPacket& pkt2 = (CForgetObjectPacket&)pkt;

		for (int i = pkt2.GetObjectCount()-1; i >= 0; --i)
		{
			m_vMobList.erase(pkt2.GetObjectId(i));
		}

		UpdateKillQueue();
	}
	else if (pkt == CObjectDeathPacket::Type())
	{
		CObjectDeathPacket& pkt2 = (CObjectDeathPacket&)pkt;

		if (pkt2.GetObjectId() == m_wPlayerId)
		{
//			m_fEnabled = FALSE;
			m_fSuspendMove = true;
		}
		else
		{
			m_vMobList.erase(pkt2.GetObjectId());
		}

		UpdateKillQueue();
	}
	else if (pkt == CWarpReplyPacket::Type())
	{
		CWarpReplyPacket& pkt2 = (CWarpReplyPacket&)pkt;

		m_fEnabled = FALSE;
		m_fAfkIsOn = false;

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();

		CAutoLockQueue autoCS(&m_csQueue);
		m_vKillQueue.clear();
		m_vMobList.clear();
	}
	else if (pkt == CGameServerHelloPacket::Type())
	{
		CGameServerHelloPacket& pktHelo = (CGameServerHelloPacket&)pkt;
		m_wPlayerId = pktHelo.GetPlayerId();
	}
	else if (pkt == CObjectMovedPacket::Type())
	{
		CObjectMovedPacket& pktMove = (CObjectMovedPacket&)pkt;

		WORD wId = pktMove.GetId();
		BYTE bX = pktMove.GetX();
		BYTE bY = pktMove.GetY();

		if (wId == m_wPlayerId)
		{
			m_bX = bX;
			m_bY = bY;

			UpdateKillQueue();
		}
		else if (wId != 0 && IS_MONSTER_ID(wId)) // is monster
		{
			std::map<WORD, DWORD>::iterator it = m_vMobList.find(wId);

			if (it != m_vMobList.end())
			{
				it->second = MAKELONG(MAKEWORD(bX, bY), MAKEWORD(bX, bY));
			}
			else
			{
				m_vMobList.insert(std::pair<WORD,DWORD>(wId, MAKELONG(MAKEWORD(bX, bY),MAKEWORD(bX, bY))));
			}

			UpdateKillQueue();
		}
	}
	else if (pkt == CObjectAnimationPacket::Type())
	{
		WORD wAtt = ((CObjectAnimationPacket&)pkt).GetActor();
		WORD wVic = ((CObjectAnimationPacket&)pkt).GetTarget();
		BYTE bType = ((CObjectAnimationPacket&)pkt).GetAnim();

		if (bType == 0x78 && wVic == m_wPlayerId && wAtt != 0 && IS_MONSTER_ID(wAtt))
		{
			std::map<WORD, DWORD>::iterator it = m_vMobList.find(wAtt);

			if (it == m_vMobList.end())
			{
				m_vMobList.insert(std::pair<WORD,DWORD>(wAtt, MAKELONG(MAKEWORD(m_bX, m_bY),MAKEWORD(m_bX, m_bY))));
				UpdateKillQueue();			
			}
		}
	}
	else if (pkt == CCharRespawnPacket::Type())
	{
		CCharRespawnPacket& pkt2 = (CCharRespawnPacket&)pkt;

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();

		if (abs((int)m_bX0 - (int)m_bX) > 2 || abs((int)m_bY0 - (int)m_bY) > 2)
		{
			m_fAfkIsOn = false;
			m_fEnabled = FALSE;
		}

		CAutoLockQueue autoCS(&m_csQueue);
		m_vKillQueue.clear();
		m_vMobList.clear();

		m_fSuspendMove = false;
	}
	else if (pkt == CCharStatsPacket::Type())
	{
		CCharStatsPacket pkt2(pkt);

		m_bX = pkt2.GetX();
		m_bY = pkt2.GetY();

		UpdateKillQueue();
	}
	else if (pkt == CUpdatePosSTCPacket::Type())
	{
		CUpdatePosSTCPacket pkt2(pkt);

		WORD wId = pkt2.GetId();
		BYTE bX = pkt2.GetX();
		BYTE bY = pkt2.GetY();

		if (wId == m_wPlayerId)
		{
			m_bX = bX;
			m_bY = bY;

			UpdateKillQueue();
		}
		else if (IS_MONSTER_ID(wId)) // is monster
		{
			std::map<WORD, DWORD>::iterator it = m_vMobList.find(wId);

			if (it != m_vMobList.end())
			{
				it->second = MAKELONG(MAKEWORD(bX, bY), MAKEWORD(bX, bY));
			}
			else
			{
				m_vMobList.insert(std::pair<WORD,DWORD>(wId, MAKELONG(MAKEWORD(bX, bY),MAKEWORD(bX, bY))));
			}

			UpdateKillQueue();
		}
	}

	return 0;
}


/**
 * \brief 
 */
int CAutoKillFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CCharSelectedPacket::Type())
	{
		CAutoLockQueue autoCS(&m_csQueue);
		m_vKillQueue.clear();
		m_vMobList.clear();
	}

	return 0;
}


/**
 * \brief 
 */
bool CAutoKillFilter::GetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "autokill") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);
		*((BOOL*)pData) = m_fEnabled;

		return true;
	}
	else if (_stricmp(pszParam, "afkstat") == 0)
	{
		*((BOOL*)pData) = m_fAfkIsOn;

		return true;
	}
	else if (_stricmp(pszParam, "targets_count") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);
		*((int*)pData) = (int)m_vKillQueue.size();

		return true;
	}

	return false;
}


/**
 * \brief 
 */
bool CAutoKillFilter::SetParam(const char* pszParam, void* pData)
{
	if (_stricmp(pszParam, "autokill") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);

		m_fEnabled = *((BOOL*)pData);
		CaptureOrigin();
		UpdateKillQueue();
	}
	else if (_stricmp(pszParam, "hdist") == 0)
	{
		m_iDist = *((int*)pData);
		UpdateKillQueue();
	}
	else if (_stricmp(pszParam, "toggle") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);

		m_fEnabled = !m_fEnabled;
		CaptureOrigin();
		UpdateKillQueue();
	}
	else if (_stricmp(pszParam, "suspend_move") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);
		m_fSuspendMove = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "setskill") == 0)
	{
		char* szSkill = (char*)pData;

		struct { const char* szSkillName; DWORD dwSkill; } _map[] =
		{
			{"Normal"		, 0x00000000},
			{"FireBurst"	, 0x0019003D},
			{"Force"		, 0x0019003C}, 
			{"TwistingSlash", 0x001E0029},
			{"DeathStab"	, 0x0019002B},
			{"FallingSlash"	, 0x00190013},
			{"Cyclone"		, 0x00190016},
			{"Lunge"		, 0x00190014},
			{"EnergyBall"	, 0x00190011},
			{"Poison"		, 0x00190001},
			{"Meteorite"	, 0x00190002},
			{"Lightning"	, 0x00190003},
			{"Fireball"		, 0x00190004},
			{"PowerWave"	, 0x0019000B},
			{"Ice"			, 0x00190007},
			{"IceArrow"		, 0x00190033},
			{"ChainLight"	, 0x001900D6},
			{"Sleep"		, 0x001900DB},
			{"DrainLife"	, 0x001E00D6},
			{"EvilSpirit"	, 0x001E0009},
			{"DarkHorse"	, 0x001E003E},
		};

		for (int i=sizeof(_map)/sizeof(_map[0])-1; i >=0; --i)
		{
			if (0 == _stricmp(szSkill, _map[i].szSkillName))
			{
				CAutoLockQueue autoCS(&m_csQueue);

				m_dwSkill = _map[i].dwSkill;
				return true;
			}
		}
		
		ULONG ulSkill = strtoul(szSkill, 0, 16);

		if ((HIWORD(ulSkill) != 0x0019 && HIWORD(ulSkill) != 0x001E) || LOWORD(ulSkill) == 0)
		{
			return false;
		}

		m_dwSkill = ulSkill;
		return true;
	}
	else if (_stricmp(pszParam, "suspended") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);
		m_fSuspended = *((bool*)pData);
	}
	else if (_stricmp(pszParam, "afkstat") == 0)
	{
		CAutoLockQueue autoCS(&m_csQueue);
		m_fAfkIsOn = *((bool*)pData);

		if (m_fAfkIsOn)
			CaptureOrigin();

		UpdateKillQueue();
	}
	else if (_stricmp(pszParam, "hitdelay") == 0)
	{
		CAutoLockQueue cs(&m_csQueue);
		m_dwDelay = *((DWORD*)pData);

		if (m_dwDelay > 2000)
			m_dwDelay = 2000;
	}
	else if (_stricmp(pszParam, "killpl") == 0)
	{
		CStringA plkill = (const char*)pData;
		plkill.MakeLower();

		m_vKillPlayers.insert(plkill);
		m_fKillAllPl = false;
	}
	else if (_stricmp(pszParam, "kill_clear") == 0)
	{
		m_vKillPlayers.clear();
		m_fKillAllPl = false;
	}
	else if (_stricmp(pszParam, "kill_all") == 0)
	{
		m_vKillPlayers.clear();
		m_fKillAllPl = true;
	}

	return true;
}


/**
 * \brief 
 */
DWORD CAutoKillFilter::KillThreadProc(CAutoKillFilter* pThis)
{
	WORD wLastTarget = 0;

	while (1)
	{
		HANDLE objs[] = {pThis->m_hStopEvent, pThis->m_hKillEvent};
		DWORD dwRes = WaitForMultipleObjects(2, objs, 0, 3000);

		if (dwRes == WAIT_OBJECT_0)
			break;
		
		wLastTarget = pThis->KillNext(wLastTarget);
	}

	return 0;
}


/**
 * \brief 
 */
WORD CAutoKillFilter::KillNext(WORD wLastTarget)
{
	WORD wObject = 0;
	bool fSuspended = true;
	DWORD dwDelay = 100;
	DWORD dwSkill = 0;
	WORD wPos = 0;

	EnterCriticalSection(&m_csQueue);

	bool fAfkIsOn = m_fAfkIsOn;
	bool fSuspendMove = m_fSuspendMove;

	if (m_vKillQueue.size() != 0)
	{
		std::map<WORD, DWORD>::iterator it = m_vKillQueue.find(wLastTarget);

		if (it == m_vKillQueue.end())
			it = m_vKillQueue.begin();

		wObject = it->first;
		wPos = HIWORD(it->second);
		fSuspended = m_fSuspended;
		dwDelay = m_dwDelay;
		dwSkill = m_dwSkill;
	}
	else
	{
		ResetEvent(m_hKillEvent);
	}

	LeaveCriticalSection(&m_csQueue);

	if (wObject != 0)
	{
		BYTE xm = LOBYTE(wPos);
		BYTE ym = HIBYTE(wPos);

		BYTE xc = xm+1;
		BYTE yc = ym;

		if (fAfkIsOn && (m_bX != xc || m_bY != yc) && !fSuspendMove)//wLastTarget != wObject)
		{
			TeleportTo(xc, yc);
		}
		else
		{
			xc = m_bX;
			yc = m_bY;
		}

		if (!fSuspended)
		{
			int dx = (int)xm - (int)xc;
			int dy = (int)ym - (int)yc;

			if (dx > 0) dx = 1; else if (dx < 0) dx = -1;
			if (dy > 0) dy = 1; else if (dy < 0) dy = -1;

			dx += 1;
			dy += 1;

			BYTE arrDir[3][3] = {{0, 7, 6},{1, 0, 5}, {2, 3, 4}}; 

			KillObject(wObject, dwSkill, xm, ym, arrDir[dx][dy]);
		}

		Sleep(dwDelay);
	}
	else if (fAfkIsOn)
	{
		int xx = (m_bY % 2 == 0 ? (m_bX % 2) : -(m_bX % 2)) + m_bX0;
		int yy = (m_bX % 2 == 0 ? (m_bY % 2) : -(m_bY % 2)) + m_bY0;

		if (!fSuspendMove)
			TeleportTo((BYTE)xx, (BYTE)yy);
	}

	return wObject;
}


/**
 * \brief 
 */
void CAutoKillFilter::KillObject(WORD wId, DWORD dwSkill, BYTE x, BYTE y, BYTE dir)
{
	if (dwSkill == 0)
	{
		GetProxy()->send_packet(CNormalAttackPacket(wId, dir));
	}
	else if (HIWORD(dwSkill) == 0x19)
	{
		GetProxy()->send_packet(CSingleSkillPacket(LOWORD(dwSkill), wId));
	}
	else if (HIWORD(dwSkill) == 0x1E)
	{
		GetProxy()->send_packet(CMassiveSkillPacket(LOBYTE(dwSkill), x, y, dir, wId));
	}
}


/**
 * \brief 
 */
void CAutoKillFilter::UpdateKillQueue()
{
	CAutoLockQueue autoCS(&m_csQueue);
	m_vKillQueue.clear();

	if (!m_fEnabled)
		return;

	for (std::map<WORD, DWORD>::iterator it = m_vMobList.begin();
			it != m_vMobList.end(); ++it)
	{
		BYTE x0 = m_fAfkIsOn ? m_bX0 : m_bX;
		BYTE y0 = m_fAfkIsOn ? m_bY0 : m_bY;

		int dx = abs((int)LOBYTE(it->second) - (int)x0);
		int dy = abs((int)HIBYTE(it->second) - (int)y0);

		float fdist = sqrtf((float)dx*dx + (float)dy*dy);

		if ((int)fdist <= m_iDist)
			m_vKillQueue.insert(std::pair<WORD,DWORD>(it->first, it->second));

//		int dx0 = abs((int)LOBYTE(HIWORD(it->second)) - (int)m_bX);
//		int dy0 = abs((int)HIBYTE(HIWORD(it->second)) - (int)m_bY);

//		if (dx <= m_iDist && dy <= m_iDist)
//			m_vKillQueue.insert(it->first);
	}

	if (m_vKillQueue.size() > 0)
		SetEvent(m_hKillEvent);
}


/**  
 * \brief 
 */
void CAutoKillFilter::CaptureOrigin()
{
	CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");

	if (pFilter)
	{
		pFilter->GetParam("X", &m_bX0);
		pFilter->GetParam("Y", &m_bY0);
	}	
}


/**
 * \brief 
 */
void CAutoKillFilter::TeleportTo(BYTE x, BYTE y)
{
	CUpdatePosCTSPacket pktMoveCTS(x, y);
	GetProxy()->send_packet(pktMoveCTS);

	CUpdatePosSTCPacket pktMoveSTC(m_wPlayerId, x, y);
	GetProxy()->recv_packet(pktMoveSTC);
}

/*
CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");

if (pFilter)
{
	std::map<WORD,CStringA>* vPlayers;
	if (pFilter->GetParam("Players", &vPlayers))
	{
		for (std::map<WORD,CStringA>::iterator it = vPlayers->begin(); it != vPlayers->end(); ++it)
		{
			CStringA strTmp = it->second;
			strTmp.MakeLower();

			if (plkill == strTmp)
			{
				BYTE x = 0, y = 0;
				pFilter->GetParam("X", &x);
				pFilter->GetParam("Y", &y);

				m_vMobList.insert(std::pair<WORD,DWORD>(it->first, MAKELONG(MAKEWORD(x,y),MAKEWORD(x,y))));
				UpdateKillQueue();
				break;
			}
		}
	}
}
*/