#include "stdafx.h"
#include "GameCommands.h"
#include "CommonPackets.h"
#include "DebugOut.h"
#include "base64.h"
#include "EncDec.h"
#include "LordOfMUdll.h"
#include "..\_Shared\MuMessages.h"


/**
 * \brief 
 */
CGameCommands::CGameCommands(CProxy* pProxy)
	: CCommandInterface(pProxy)
{
	AddCommand("say", new CSayCommandHandler(pProxy));
	AddCommand("whisp", new CWhisperCommandHandler(pProxy));

	AddCommand("multihit", new CMultihitCommandHandler(pProxy));
	AddCommand("hd", new CHitDelayCommandHandler(pProxy));
	AddCommand("hda", new CHitDelayAKCommandHandler(pProxy));
	AddCommand("fmove", new CFastMoveCommandHandler(pProxy));
	AddCommand("stealth", new CStealthCommandHandler(pProxy));
	AddCommand("autokill", new CAutokillCommandHandler(pProxy));
	AddCommand("setskill", new CSkillCommandHandler(pProxy));
	AddCommand("hdist", new CHitDistCommandHandler(pProxy));
	AddCommand("mixb", new CLahapCommandHandler(pProxy));
	AddCommand("mixs", new CLahapCommandHandler(pProxy));
	AddCommand("dsb", new CLahapCommandHandler(pProxy));
	AddCommand("dss", new CLahapCommandHandler(pProxy));
	AddCommand("reflect", new CReflectCommandHandler(pProxy));
	AddCommand("route", new CRouteCommandHandler(pProxy));
	AddCommand("moveitem", new CMoveItemCommandHandler(pProxy));
	AddCommand("loglevel", new CLogLevelCommandHandler(pProxy));
	AddCommand("suicide", new CSuicideCommandHandler(pProxy));
	AddCommand("repairall", new CRepairAllCommandHandler(pProxy));
	AddCommand("afkstat", new CAfkStatCommandHandler(pProxy));
	AddCommand("lockag", new CSetFilterFlagCommandHandler(pProxy, "CharInfoFilter", "lockag"));
	AddCommand("autopot", new CSetFilterFlagCommandHandler(pProxy, "AutoPotFilter", "autopot"));
	AddCommand("fixagi", new CFixStatCommandHandler(pProxy));
	AddCommand("killpl", new CKillPlCommandHandler(pProxy));

#ifdef DEBUG
	AddCommand("test", new CTestCommandHandler(pProxy));
#endif

	AddCommand("set_pick_opt", new CPickOptCommandHandler(pProxy));
	AddCommand("set_stealth_opt", new CStealthOptCommandHandler(pProxy));
	AddCommand("autopick", new CAutopickCommandHandler(pProxy));
	AddCommand("autosay", new CAutosayCommandHandler(pProxy));
	AddCommand("exit400", new CAutoexitCommandHandler(pProxy));
	AddCommand("pick", new CPickCommandHandler(pProxy));
	AddCommand("gt", new CGotoCommandHandler(pProxy));
	AddCommand("pdist", new CPickDistCommandHandler(pProxy));
	AddCommand("script", new CScriptCommandHandler(pProxy));
	AddCommand("run", new CRunCommandHandler(pProxy));
	AddCommand("drop", new CDropCommandHandler(pProxy));
	AddCommand("itemcode", new CItemCodeCommandHandler(pProxy));
	AddCommand("throw", new CThrowCommandHandler(pProxy));
	AddCommand("use", new CUseCommandHandler(pProxy));
	AddCommand("sellall", new CInvokeWithNoArgsCommandHandler(pProxy, "InventoryManagerFilter", "sellall"));
	AddCommand("sellany", new CInvokeWithNoArgsCommandHandler(pProxy, "InventoryManagerFilter", "sellany"));
	AddCommand("throwall", new CInvokeWithNoArgsCommandHandler(pProxy, "InventoryManagerFilter", "throwall"));
	AddCommand("throwany", new CInvokeWithNoArgsCommandHandler(pProxy, "InventoryManagerFilter", "throwany"));
	AddCommand("testend", new CInvokeWithNoArgsCommandHandler(pProxy, "ProtocolDetectFilter", "testend", 0, true));
	AddCommand("meetpl", new CMeetPlCommandHandler(pProxy));
	AddCommand("config", new CConfigCommandHandler(pProxy));
	AddCommand("advcfg", new CAdvcfgCommandHandler(pProxy));
	AddCommand("afk", new CAfkCommandHandler(pProxy));
	AddCommand("exit", new CExitCommandHandler(pProxy));
	AddCommand("ptype", new CPtypeCommandHandler(pProxy));
	AddCommand("ctype", new CCtypeCommandHandler(pProxy));
	AddCommand("die", new CDieCommandHandler(pProxy));
	AddCommand("sendkey", new CSendKeyCommandHandler(pProxy));

	AddCommand("send", new CSendCommandHandler(pProxy));
	AddCommand("recv", new CRecvCommandHandler(pProxy));

	AddCommand("logout", new CLogoutCommandHandler(pProxy));
	
	AddCommand("help", new CHelpCommandHandler(pProxy, m_cCommands));
}


/**
 * \brief 
 */
bool CSayCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szNick[128] = {0};
	char* szMessage = strchr((char*)args, ' ');
	
	if (szMessage != 0)
	{
		int len = (int)(szMessage - args);

		if (len > 127)
			len = 127;

		strncpy(szNick, args, len);

		GetProxy()->send_lop_packet(CCharacterSayPacket(szNick, szMessage+1));
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s [OK].", cmd));
	return true;
}

/**
 * \brief 
 */
bool CWhisperCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szNick[128] = {0};
	char* szMessage = strchr((char*)args, ' ');
	
	if (szMessage != 0)
	{
		int len = (int)(szMessage - args);

		if (len > 127)
			len = 127;

		strncpy(szNick, args, len);

		GetProxy()->send_lop_packet(CCharacterWhisperPacket(szNick, szMessage+1));
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s [OK].", cmd));
	return true;
}




/**
 * \brief 
 */
bool CPickOptCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};
	char szArg2[256] = {0};

	const char* p = args;
	
	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg2[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0 || szArg1[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("AutoPickupFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter object not found."));
		return false;
	}

	unsigned long ulFlags = 0;

	if (_stricmp(szArg1, "on") == 0)
		ulFlags |= 1;

	if (_stricmp(szArg2, "on") == 0)
		ulFlags |= 2;

	pPickFilter->SetParam(szArg0, &ulFlags);
	return true;
}


/**
 * \brief 
 */
bool CStealthOptCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};

	const char* p = args;
	
	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;


	if (szArg0[0] == 0 || szArg1[0] == 0 || *p != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("AutoPickupFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter object not found."));
		return false;
	}

	bool fOpt = 0;

	if (_stricmp(szArg1, "on") == 0)
		fOpt = true;
	else if (_stricmp(szArg1, "off") == 0)
		fOpt = false;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	pPickFilter->SetParam(szArg0, &fOpt);
	return true;
}

/**
 * \brief 
 */
bool CAutopickCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("AutoPickupFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter object not found."));
		return false;
	}

	pPickFilter->SetParam(cmd, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CAutokillCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;
	BOOL fToggle = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else if (_stricmp(args, "toggle") == 0)
	{
		fToggle = TRUE;
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_AUTOKILL) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pKillFilter = GetProxy()->GetFilter("AutoKillFilter");
	if (!pKillFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoKill filter object not found."));
		return false;
	}

	if (fToggle)
	{
		pKillFilter->GetParam("autokill", &fCmd);
		fCmd = !fCmd;
	}

	pKillFilter->SetParam(cmd, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, fCmd ? "on" : "off"));
	return true;
}


/**
 * \brief 
 */
bool CAutosayCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;
	BOOL fReload = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else if (_stricmp(args, "reload") == 0)
	{
		fReload = TRUE;
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_AUTOSPEAK) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("AutosayFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Autosay filter object not found."));
		return false;
	}

	if (fReload)
		pFilter->SetParam("reload", &fCmd);
	else
		pFilter->SetParam(cmd, &fCmd);

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CAutoexitCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. CharInfoFilter filter object not found."));
		return false;
	}

	pFilter->SetParam(cmd, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CPickCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};
	char szArg2[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg2[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("AutoPickupFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter object not found."));
		return false;
	}

	if (_stricmp(szArg0, "clear") == 0 && szArg1[0] == 0)
	{
		pPickFilter->SetParam("pick_clear", 0);
	}
	else if (_stricmp(szArg0, "all") == 0 && szArg1[0] == 0)
	{
		pPickFilter->SetParam("pick_all", 0);
	}
	else if (szArg1[0] != 0)
	{
		WORD wCode = ((atoi(szArg0) & 0xFF) << 8) | (atoi(szArg1) & 0xFF);
		WORD wMask = 0xFFFF;

		if (szArg2[0] != 0)
			wMask = 1 << atoi(szArg2);

		DWORD dwData = ((DWORD)wMask << 16) | wCode;
		pPickFilter->SetParam(cmd, &dwData);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}



/**
 * \brief 
 */
bool CGotoCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	const char* p = args;

	while (*p != 0 && *p != ' ') p++;
	while (*p != 0 && *p == ' ') p++;

	if (*p == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_MOVETO) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pCharNfoFilter = GetProxy()->GetFilter("CharInfoFilter");

	if (!pCharNfoFilter)
	{
		CDebugOut::PrintError(">> Internal error! CharInfoFilter object not found.");
		return false;
	}

	BYTE x = (BYTE)atoi(args);
	BYTE y = (BYTE)atoi(p);
	WORD wPlayerId = 0;

	pCharNfoFilter->GetParam("PlayerId", &wPlayerId);

	CUpdatePosCTSPacket pktMoveCTS(x, y);
	GetProxy()->send_packet(pktMoveCTS);

	CUpdatePosSTCPacket pktMoveSTC(wPlayerId, x, y);
	GetProxy()->recv_packet(pktMoveSTC);

	CObjectAnimationPacket pktAnim(wPlayerId, wPlayerId, 2, 1);
	GetProxy()->recv_packet(pktAnim);

	return true;
}


/**
 * \brief 
 */
bool CMultihitCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_MULTIHIT) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}


	ULONG ulHitCount = strtoul(args, 0, 10);
	CPacketFilter* pFilter = GetProxy()->GetFilter("MultihitFilter");

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Multi-hit filter object not found."));
		return false;
	}

	pFilter->SetParam("hitcount", &ulHitCount);

	GetProxy()->recv_direct(CServerMessagePacket(">> multi-hit count set to %d [OK].", ulHitCount));
	return true;
}



/**
 * \brief 
 */
bool CHitDelayCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	DWORD dwDelay = strtoul(args, 0, 10);
	CPacketFilter* pFilter = GetProxy()->GetFilter("MultihitFilter");

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Multi-hit filter object not found."));
		return false;
	}

	pFilter->SetParam("hitdelay", &dwDelay);

	GetProxy()->recv_direct(CServerMessagePacket(">> hit delay set to %d ms [OK].", dwDelay));
	return true;
}


/**
 * \brief 
 */
bool CHitDelayAKCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	DWORD dwDelay = strtoul(args, 0, 10);
	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoKillFilter");

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoKillFilter filter object not found."));
		return false;
	}

	pFilter->SetParam("hitdelay", &dwDelay);

	GetProxy()->recv_direct(CServerMessagePacket(">> auto-kill delay set to %d ms [OK].", dwDelay));
	return true;
}


/**
 * \brief 
 */
bool CFastMoveCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] == 0 || (_stricmp(args, "on") != 0 && _stricmp(args, "off") != 0))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_FASTMOVE) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	BOOL fEnabled = _stricmp(args, "on") == 0;
	CPacketFilter* pFilter = GetProxy()->GetFilter("FastMoveFilter");

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. FastMove filter object not found."));
		return false;
	}

	pFilter->SetParam("enabled", &fEnabled);

	GetProxy()->recv_direct(CServerMessagePacket(">> Fast move %s [OK].", fEnabled ? "ENABLED" : "DISABLED"));
	return true;
}




/**
 * \brief 
 */
bool CDieCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	CPacketFilter* pCharNfoFilter = GetProxy()->GetFilter("CharInfoFilter");

	if (!pCharNfoFilter)
	{
		CDebugOut::PrintError(">> Internal error! CharInfoFilter object not found.");
		return false;
	}

	WORD wPlayerId = 0;
	pCharNfoFilter->GetParam("PlayerId", &wPlayerId);
		
	GetProxy()->recv_packet(CObjectDeathPacket(wPlayerId));
	return true;
}


/**
 * \brief 
 */
bool CStealthCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pCharNfoFilter = GetProxy()->GetFilter("CharInfoFilter");

	if (!pCharNfoFilter)
	{
		CDebugOut::PrintError(">> Internal error! CharInfoFilter object not found.");
		return false;
	}

	bool fMode = false;

	if (_stricmp(args, "on") == 0)
	{
		fMode = true;
	}
	else if (_stricmp(args, "off") != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	pCharNfoFilter->SetParam("stealth", &fMode);
	GetProxy()->recv_direct(CServerMessagePacket(">> Stealth mode %s [OK].", fMode ? "ON" : "OFF"));

	return true;
}



/**
 * \brief 
 */
bool CSkillCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoKillFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! AutoKillFilter object not found.");
		return false;
	}

	if (!pFilter->SetParam(cmd, (void*)args))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}


	pFilter = GetProxy()->GetFilter("ReflectFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! ReflectFilter object not found.");
		return false;
	}

	if (!pFilter->SetParam(cmd, (void*)args))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> Active skill set to %s [OK].", args));

	return true;
}


/**
 * \brief 
 */
bool CHitDistCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoKillFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! AutoKillFilter object not found.");
		return false;
	}

	int dist = atoi(args);

	if (!pFilter->SetParam(cmd, (void*)&dist))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> Hit distance set to %d [OK].", dist));
	return true;
}


/**
 * \brief 
 */
bool CPickDistCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoPickupFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! AutoPickupFilter object not found.");
		return false;
	}

	int dist = atoi(args);

	if (!pFilter->SetParam(cmd, (void*)&dist))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> Pick-up distance set to %d [OK].", dist));
	return true;
}


/**
 * \brief 
 */
bool CScriptCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_SCRIPT) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("ScriptProcessorFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! ScriptProcessorFilter object not found.");
		return false;
	}

	char szArg0[256] = {0};
	char szArg1[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;


	if (_stricmp(szArg0, "load") == 0 && szArg1[0] != 0)
	{
		if (!pFilter->SetParam(szArg0, szArg1))
			return false;
	}
	else if (_stricmp(szArg0, "on") == 0 && szArg1[0] == 0)
	{
		bool fEnable = true;

		if (!pFilter->SetParam("enable", (void*)&fEnable))
			return false;
	}
	else if (_stricmp(szArg0, "off") == 0 && szArg1[0] == 0)
	{
		bool fEnable = false;

		if (!pFilter->SetParam("enable", (void*)&fEnable))
			return false;
	}
	else if (_stricmp(szArg0, "toggle") == 0 && szArg1[0] == 0)
	{
		bool fEnable = false;

		if (!pFilter->GetParam("enable", (void*)&fEnable))
			return false;

		fEnable = !fEnable;

		if (!pFilter->SetParam("enable", (void*)&fEnable))
			return false;

		strcpy(szArg0, fEnable ? "on" : "off");
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, szArg0));
	return true;
}


/**
 * \brief 
 */
bool CHelpCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	for (CCommandList::iterator it = m_vCommands.begin(); it != m_vCommands.end(); ++it)
	{
		if (it->second)
		{
			const char* pszUsage = it->second->PrintUsage();

			if (pszUsage)
				GetProxy()->recv_direct(CServerMessagePacket(">> %s", pszUsage));
		}
	}

	return true;
}


/**
 * \brief 
 */
bool CDropCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};
	char szArg2[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg2[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("AutoPickupFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter object not found."));
		return false;
	}

	if (_stricmp(szArg0, "clear") == 0 && szArg1[0] == 0)
	{
		pPickFilter->SetParam("drop_clear", 0);
	}
	else if (szArg1[0] != 0)
	{
		WORD wCode = ((atoi(szArg0) & 0xFF) << 8) | (atoi(szArg1) & 0xFF);
		WORD wMask = 0xFFFF;

		if (szArg2[0] != 0)
			wMask = 1 << atoi(szArg2);

		DWORD dwData = ((DWORD)wMask << 16) | wCode;
		pPickFilter->SetParam(cmd, &dwData);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}



/**
 * \brief 
 */
bool CRunCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_SCRIPT) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("ScriptProcessorFilter");

	if (!pFilter)
	{
		CDebugOut::PrintError(">> Internal error! ScriptProcessorFilter object not found.");
		return false;
	}

	if (!pFilter->SetParam("load", (void*)args))
		return false;

	bool fEnable = true;

	if (!pFilter->SetParam("run", (void*)&fEnable))
		return false;

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CLahapCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0 || (_stricmp(args, "10") != 0 && _stricmp(args, "30") != 0))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}


	if ((CPacketType::GetFeatures() & FEATURE_LAHAP) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}


	CPacket pkt;

	if (_stricmp(cmd, "mixb") == 0)
	{
		if (_stricmp(args, "10") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x06, 0xBC, 0x35, 0x89, 0x3B, 0xC1, 0x03, 0x31};
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
		else if (_stricmp(args, "30") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x06, 0xBC, 0x35, 0x89, 0x39, 0xC1, 0x03, 0x31};
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
	}
	else if (_stricmp(cmd, "mixs") == 0)
	{
		if (_stricmp(args, "10") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x06, 0xBC, 0x35, 0x88, 0x3A, 0xC1, 0x03, 0x31};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
		else if (_stricmp(args, "30") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x06, 0xBC, 0x35, 0x88, 0x38, 0xC1, 0x03, 0x31};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
	}
	else if (_stricmp(cmd, "dsb") == 0)
	{
		if (_stricmp(args, "10") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x07, 0xBC, 0x34, 0x88, 0x3A, 0xA9};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
		else if (_stricmp(args, "30") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x07, 0xBC, 0x34, 0x88, 0x38, 0xAB};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
	}
	else if (_stricmp(cmd, "dss") == 0)
	{
		if (_stricmp(args, "10") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x07, 0xBC, 0x34, 0x89, 0x3B, 0xA8};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
		else if (_stricmp(args, "30") == 0)
		{
			unsigned char buf1[] = {0xC1, 0x07, 0xBC, 0x34, 0x89, 0x39, 0xAA};			
			pkt.ConstructFromBuffer(buf1, sizeof(buf1));
		}
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command name."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->send_direct(pkt);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CItemCodeCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] == 0 || (_stricmp(args, "on") != 0 && _stricmp(args, "off") != 0))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_ITEMCODE) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	BOOL fEnabled = _stricmp(args, "on") == 0;
	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoPickupFilter");

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoPickupFilter filter object not found."));
		return false;
	}

	pFilter->SetParam("itemcode", &fEnabled);

	GetProxy()->recv_direct(CServerMessagePacket(">> Item code %s [OK].", fEnabled ? "ENABLED" : "DISABLED"));
	return true;
}


/**
 * \brief 
 */
bool CReflectCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;
	BOOL fToggle = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else if (_stricmp(args, "toggle") == 0)
		fToggle = TRUE;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_REFLECT) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("ReflectFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. ReflectFilter filter object not found."));
		return false;
	}

	if (fToggle)
	{
		pFilter->GetParam("enable", &fCmd);
		fCmd = !fCmd;
	}

	pFilter->SetParam("enable", &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CRouteCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("RecordRouteFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. RecordRouteFilter object not found."));
		return false;
	}

	if ((_stricmp(szArg0, "clear") == 0 || _stricmp(szArg0, "start") == 0 || _stricmp(szArg0, "stop") == 0) && szArg1[0] == 0)
	{
		pFilter->SetParam(szArg0, 0);
	}
	else if (_stricmp(szArg0, "save") == 0 && szArg1[0] != 0)
	{
		pFilter->SetParam(szArg0, (void*)szArg1);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}



/**
 * \brief 
 */
bool CThrowCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};
	char szArg2[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg2[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}


	if ((CPacketType::GetFeatures() & FEATURE_INVENTORY) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pPickFilter = GetProxy()->GetFilter("InventoryManagerFilter");
	if (!pPickFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. InventoryManagerFilter object not found."));
		return false;
	}

	if (szArg1[0] != 0)
	{
		WORD wCode = ((atoi(szArg0) & 0xFF) << 8) | (atoi(szArg1) & 0xFF);
		WORD wMask = 0xFFFF;

		if (szArg2[0] != 0)
			wMask = 1 << atoi(szArg2);

		DWORD dwData = ((DWORD)wMask << 16) | wCode;
		pPickFilter->SetParam(cmd, &dwData);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}



/**
 * \brief 
 */
bool CMoveItemCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;


	if (szArg0[0] == 0 || szArg1[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}


	if ((CPacketType::GetFeatures() & FEATURE_INVENTORY) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}


	CPacketFilter* pFilter = GetProxy()->GetFilter("InventoryManagerFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. InventoryManagerFilter object not found."));
		return false;
	}

	BYTE src = (BYTE)strtoul(szArg0, 0, 16);
	BYTE dst = (BYTE)strtoul(szArg1, 0, 16);
	WORD wData = MAKEWORD(src, dst);

	if (!pFilter->SetParam(cmd, &wData))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [FAILED].", cmd, args));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CLogLevelCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	char szArg0[256] = {0};
	char szArg1[256] = {0};

	const char* p = args;

	int pos = 0;
	while (*p != 0 && *p != ' '){ szArg0[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	pos = 0;
	while (*p != 0 && *p != ' '){ szArg1[pos++] = *p; p++; }
	while (*p != 0 && *p == ' ') p++;

	if (szArg0[0] == 0 || szArg1[0] != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_LOGLEVEL) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}


	int iLogLevel = -1;

	if (_stricmp(szArg0, "error") == 0)
		iLogLevel = 0;
	else if (_stricmp(szArg0, "warn") == 0)
		iLogLevel = 1;
	else if (_stricmp(szArg0, "info") == 0)
		iLogLevel = 2;
	else if (_stricmp(szArg0, "debug") == 0)
		iLogLevel = 3;
	else if (_stricmp(szArg0, "silent") != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CDebugOut::SetLogLevel(iLogLevel);

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CTestCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BYTE buf[] = {0xC3, 0x07, 0x00, 0x30, 0xB9, 0x4A, 0xEE};
	
	CPacket pkt;
	pkt.SetDecryptedPacket(buf, sizeof(buf));
	pkt.SetInjected();

	GetProxy()->send_packet(pkt);

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CSuicideCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	CPacketFilter* pFilter = GetProxy()->GetFilter("ReflectFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. ReflectFilter object not found."));
		return false;
	}

	pFilter->SetParam("suicide", 0);

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**
 * \brief 
 */
bool CMeetPlCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BOOL fCmd = FALSE;

	if (_stricmp(args, "on") == 0)
		fCmd = TRUE;
	else if (_stricmp(args, "off") == 0)
		fCmd = FALSE;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. CharInfoFilter filter object not found."));
		return false;
	}

	pFilter->SetParam(cmd, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}


/**  
 * \brief 
 */
bool CConfigCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	LPARAM lParam = (_stricmp(cmd, "config") == 0) ? 0 : 1;

	if (CProxyClickerModule::GetInstance()->m_pLoader)
	{
		HWND hWnd = 0;
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

		if (hWnd)
			PostMessage(hWnd, WM_SHOW_SETTINGS_GUI, 0, lParam);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Object not found."));
	}

	return true;
}


/**  
 * \brief 
 */
bool CAfkCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (_stricmp(args, "on") != 0 && _stricmp(args, "off") != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}


	UINT uMsg = (_stricmp(args, "on") == 0) ? WM_START_CLICKER : WM_STOP_CLICKER;

	if (CProxyClickerModule::GetInstance()->m_pLoader)
	{
		HWND hWnd = 0;
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

		if (hWnd)
			PostMessage(hWnd, uMsg, 0, 0);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Object not found."));
	}

	return true;
}



/**  
 * \brief 
 */
bool CExitCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (CProxyClickerModule::GetInstance()->m_pLoader)
	{
		HWND hWnd = 0;
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

		if (hWnd)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Object not found."));
	}

	return true;
}



/**  
 * \brief 
 */
bool CPtypeCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	DWORD dwType = 0;

	if (_stricmp(args, "kor") == 0)
	{
		dwType = 1;
	}
	else if (_stricmp(args, "jpn") == 0)
	{
		dwType = 4;
	}
	else if (_stricmp(args, "phi") == 0)
	{
		dwType = 5;
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("ProtocolDetectFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. ProtocolDetectFilter filter object not found."));
		return false;
	}

	pFilter->SetParam(cmd, &dwType);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));

	return true;
}



/**  
 * \brief 
 */
bool CCtypeCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	DWORD dwType = 0;

	if (_stricmp(args, "s3") == 0)
	{
		dwType = 1;
	}
	else if (_stricmp(args, "s4") == 0)
	{
		dwType = 2;
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("ProtocolDetectFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. ProtocolDetectFilter filter object not found."));
		return false;
	}

	pFilter->SetParam(cmd, &dwType);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));

	return true;
}



/**  
 * \brief 
 */
bool CSendCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if ((CPacketType::GetFeatures() & FEATURE_PACKETS) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	int len = 0;
	bool fRet = false;

	if (args && 0 < (len = CBufferUtil::HexToBufferLen(args)))
	{
		BYTE* buf = new BYTE[len+1];
		
		if (CBufferUtil::HexToBuffer(args, buf, len))
		{
			CPacket pkt;
			pkt.SetDecryptedPacket(buf, len);
			pkt.SetInjected();

			if (buf[0] == 0xC1)
				pkt.SetType(CUnkSendC1Packet::Type());
			else if (buf[0] == 0xC3)
				pkt.SetType(CUnkSendC3Packet::Type());

			fRet = GetProxy()->send_packet(pkt);
		}

		delete[] buf;
	}

	if (!fRet)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	}

	return fRet;
}


/**  
 * \brief 
 */
bool CRecvCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if ((CPacketType::GetFeatures() & FEATURE_PACKETS) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	int len = 0;
	bool fRet = false;

	if (args && 0 < (len = CBufferUtil::HexToBufferLen(args)))
	{
		BYTE* buf = new BYTE[len+1];
		
		if (CBufferUtil::HexToBuffer(args, buf, len))
		{
			CPacket pkt;
			pkt.SetDecryptedPacket(buf, len);
			pkt.SetInjected();

			fRet = GetProxy()->recv_packet(pkt);
		}

		delete[] buf;
	}

	if (!fRet)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	}

	return fRet;
}


/**  
 * \brief 
 */
bool CRepairAllCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (m_hThread)
		return true;

	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, 0);
	return true;
}


/**  
 * \brief 
 */
LRESULT CALLBACK CRepairAllCommandHandler::ThreadProc(CRepairAllCommandHandler* pThis)
{
	for (BYTE i=0; i < 0x0C; i++)
	{
		CRepairInvPacket pkt(i);
		pThis->GetProxy()->send_packet(pkt);

		Sleep(550);
	}

	HANDLE hThread = pThis->m_hThread;
	pThis->m_hThread = 0;
	CloseHandle(hThread);
	return 0;
}


/**
 * \brief 
 */
bool CAfkStatCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	bool fCmd = false;

	if (_stricmp(args, "on") == 0)
		fCmd = true;
	else if (_stricmp(args, "off") == 0)
		fCmd = false;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_AUTOKILL) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pKillFilter = GetProxy()->GetFilter("AutoKillFilter");
	if (!pKillFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoKill filter object not found."));
		return false;
	}

	pKillFilter->SetParam(cmd, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, fCmd ? "on" : "off"));
	return true;
}


/**  
 * \brief 
 */
bool CSetFilterFlagCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	bool fCmd = false;

	if (_stricmp(args, "on") == 0)
		fCmd = true;
	else if (_stricmp(args, "off") == 0)
		fCmd = false;
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s <on|off>", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter(m_szFilter);

	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. %s filter object not found.", m_szFilter));
		return false;
	}

	pFilter->SetParam(m_szParam, &fCmd);
	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, fCmd ? "on" : "off"));
	return true;
}


/**  
 * \brief 
 */
bool CInvokeWithNoArgsCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args[0] != 0)
	{
		if (!m_fSilent)
		{
			GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
			GetProxy()->recv_direct(CServerMessagePacket(">> %s", cmd));
		}
		return false;
	}

	if (_stricmp(m_szFilter, "InventoryManagerFilter") == 0 && (CPacketType::GetFeatures() & FEATURE_INVENTORY) == 0)
	{
		if (!m_fSilent)
		{
			GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		}
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter(m_szFilter);

	if (!pFilter)
	{
		if (!m_fSilent)
		{
			GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. %s filter object not found.", m_szFilter));
		}
		return false;
	}

	pFilter->SetParam(m_szParam, 0);

	if (!m_fSilent)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> %s [OK].", cmd));
	}
	return true;
}



/**  
 * \brief 
 */
bool CSendKeyCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	BYTE key = 0;

	if (!args || args[0] == 0 || !CBufferUtil::HexToBuffer(args, &key, 1))
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if (CProxyClickerModule::GetInstance()->m_pLoader)
	{
		HWND hWnd = 0;
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

		if (hWnd)
			PostMessage(hWnd, WM_SEND_KEY, 0, (LPARAM)key);
	}
	else
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. Object not found."));
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}




/**  
 * \brief 
 */
bool CLogoutCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (args && args[0] != 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s [OK].", cmd));

	CClientDCReplyPacket pktDC(0x02);
	GetProxy()->recv_packet(pktDC);

	GetProxy()->SetParam("foce_close", 0);
	return true;
}


/**  
 * \brief 
 */
bool CFixStatCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_AGILITY_HACK) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. CharInfoFilter filter object not found."));
		return false;
	}

	WORD wAgi = (WORD)strtoul(args, 0, 10);
	pFilter->SetParam(cmd, &wAgi);

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	GetProxy()->recv_direct(CServerMessagePacket(">> Please switch character now."));
	return true;
}


/**
 * \brief 
 */
bool CKillPlCommandHandler::ProcessCommand(const char* cmd, const char* args)
{
	if (!args || args[0] == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command arguments."));
		GetProxy()->recv_direct(CServerMessagePacket(">> %s", PrintUsage()));
		return false;
	}

	CPacketFilter* pFilter = GetProxy()->GetFilter("AutoKillFilter");
	if (!pFilter)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad software configuration. AutoKillFilter object not found."));
		return false;
	}

	if ((CPacketType::GetFeatures() & FEATURE_KILL_PLAYERS) == 0)
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Command \'%s\' not available.", cmd));
		return false;
	}

	if (_stricmp(args, "clear") == 0)
	{
		pFilter->SetParam("kill_clear", 0);
	}
	else if (_stricmp(args, "*") == 0)
	{
		pFilter->SetParam("kill_all", 0);
	}
	else
	{
		pFilter->SetParam("killpl", (void*)args);
	}

	GetProxy()->recv_direct(CServerMessagePacket(">> %s %s [OK].", cmd, args));
	return true;
}
