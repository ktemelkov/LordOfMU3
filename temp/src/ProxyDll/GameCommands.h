#ifndef __GameCommands_H
#define __GameCommands_H

#include "CommandInterface.h"


/**
 * \brief 
 */
class CGameCommands
	: public CCommandInterface
{
public:
	CGameCommands(CProxy* pProxy);
	virtual ~CGameCommands() {}
};


/**
 * \brief 
 */
class CSayCommandHandler
	: public CCommandHandler
{
public:
	CSayCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CSayCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//say character_name message"; }
};


/**
 * \brief 
 */
class CWhisperCommandHandler
	: public CCommandHandler
{
public:
	CWhisperCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CWhisperCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//whisp character_name message"; }
};



/**
 * \brief 
 */
class CPickOptCommandHandler
	: public CCommandHandler
{
public:
	CPickOptCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CPickOptCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//set_pick_opt <bless|soul|chaos|joc|jog|jol|exl|zen> <on|off> <on|off>"; }
};



/**
 * \brief 
 */
class CKillPlCommandHandler
	: public CCommandHandler
{
public:
	CKillPlCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CKillPlCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//killpl [<player name>|*|clear]"; }
};


/**
 * \brief 
 */
class CStealthOptCommandHandler
	: public CCommandHandler
{
public:
	CStealthOptCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CStealthOptCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//set_stealth_opt <susp_zen_pick|susp_move_pick|susp_pick> <on|off>"; }
};


/**
 * \brief 
 */
class CPickCommandHandler
	: public CCommandHandler
{
public:
	CPickCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CPickCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//pick [<item code high> <item code low>|clear]"; }
};


/**
 * \brief 
 */
class CItemCodeCommandHandler
	: public CCommandHandler
{
public:
	CItemCodeCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CItemCodeCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//itemcode <on|off>"; }
};


/**
 * \brief 
 */
class CAutopickCommandHandler
	: public CCommandHandler
{
public:
	CAutopickCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAutopickCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//autopick <on|off>"; }
};

/**
 * \brief 
 */
class CAutokillCommandHandler
	: public CCommandHandler
{
public:
	CAutokillCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAutokillCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//autokill <on|off>"; }
};


/**
 * \brief 
 */
class CAutosayCommandHandler
	: public CCommandHandler
{
public:
	CAutosayCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAutosayCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//autosay <on|off>"; }
};



/**
 * \brief 
 */
class CAutoexitCommandHandler
	: public CCommandHandler
{
public:
	CAutoexitCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAutoexitCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//exit400 <on|off>"; }
};


/**
 * \brief 
 */
class CGotoCommandHandler
	: public CCommandHandler
{
public:
	CGotoCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CGotoCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//gt <x> <y>"; }
};


/**
 * \brief 
 */
class CMultihitCommandHandler
	: public CCommandHandler
{
public:
	CMultihitCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CMultihitCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//multihit <hit_count>"; }
};


/**
 * \brief 
 */
class CHitDelayCommandHandler
	: public CCommandHandler
{
public:
	CHitDelayCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CHitDelayCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//hd <hit_delay_in_milliseconds>"; }
};


/**
 * \brief 
 */
class CHitDelayAKCommandHandler
	: public CCommandHandler
{
public:
	CHitDelayAKCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CHitDelayAKCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//hda <auto_kill_delay_in_ms>"; }
};


/**
 * \brief 
 */
class CFastMoveCommandHandler
	: public CCommandHandler
{
public:
	CFastMoveCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CFastMoveCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//fmove <on|off>"; }
};



/**
 * \brief 
 */
class CDieCommandHandler
	: public CCommandHandler
{
public:
	CDieCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CDieCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//die"; }
};


/**
 * \brief 
 */
class CStealthCommandHandler
	: public CCommandHandler
{
public:
	CStealthCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CStealthCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//stealth <off|on>"; }
};

/**
 * \brief 
 */
class CSkillCommandHandler
	: public CCommandHandler
{
public:
	CSkillCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CSkillCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//setskill <skill name>"; }
};

/**
 * \brief 
 */
class CHitDistCommandHandler
	: public CCommandHandler
{
public:
	CHitDistCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CHitDistCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//hdist <distance>"; }
};

/**
 * \brief 
 */
class CPickDistCommandHandler
	: public CCommandHandler
{
public:
	CPickDistCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CPickDistCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//pdist <distance>"; }
};


/**
 * \brief 
 */
class CScriptCommandHandler
	: public CCommandHandler
{
public:
	CScriptCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CScriptCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//script <load [filename]|on|off|toggle>"; }
};


/**
 * \brief 
 */
class CHelpCommandHandler
	: public CCommandHandler
{
public:
	CHelpCommandHandler(CProxy* pProxy, CCommandList& vCommands) 
			: CCommandHandler(pProxy), m_vCommands(vCommands) {}
	virtual ~CHelpCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//help"; }

private:
	CCommandList& m_vCommands;
};


/**
 * \brief 
 */
class CDropCommandHandler
	: public CCommandHandler
{
public:
	CDropCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CDropCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//drop [<item code high> <item code low>|clear]"; }
};


/**
 * \brief 
 */
class CRunCommandHandler
	: public CCommandHandler
{
public:
	CRunCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CRunCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//run <script filename>"; }
};


/**
 * \brief 
 */
class CLahapCommandHandler
	: public CCommandHandler
{
public:
	CLahapCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CLahapCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//[mixb|mixs|dsb|dss] <10|30>"; }
};


/**
 * \brief 
 */
class CReflectCommandHandler
	: public CCommandHandler
{
public:
	CReflectCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CReflectCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//reflect <on|off>"; }
};


/**
 * \brief 
 */
class CRouteCommandHandler
	: public CCommandHandler
{
public:
	CRouteCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CRouteCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//route <start|stop|clear|[save filename]>"; }
};

/**
 * \brief 
 */
class CThrowCommandHandler
	: public CCommandHandler
{
public:
	CThrowCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CThrowCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//throw xx yy [zz]"; }
};

/**
 * \brief 
 */
class CUseCommandHandler 
	: public CThrowCommandHandler
{
public:
	CUseCommandHandler(CProxy* pProxy) : CThrowCommandHandler(pProxy) {}
	virtual ~CUseCommandHandler(){}

public:
	virtual const char* PrintUsage(){ return "//use xx yy [zz]"; }
};



/**
 * \brief 
 */
class CMoveItemCommandHandler
	: public CCommandHandler
{
public:
	CMoveItemCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CMoveItemCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//moveitem src dst"; }
};


/**
 * \brief 
 */
class CLogLevelCommandHandler
	: public CCommandHandler
{
public:
	CLogLevelCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CLogLevelCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//loglevel <silent|error|warn|info|debug>"; }
};


/**
 * \brief 
 */
class CTestCommandHandler
	: public CCommandHandler
{
public:
	CTestCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CTestCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//test"; }
};


/**
 * \brief 
 */
class CSuicideCommandHandler
	: public CCommandHandler
{
public:
	CSuicideCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CSuicideCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//suicide"; }
};



/**
 * \brief 
 */
class CMeetPlCommandHandler
	: public CCommandHandler
{
public:
	CMeetPlCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CMeetPlCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//meetpl <on|off>"; }
};


/**
 * \brief 
 */
class CConfigCommandHandler
	: public CCommandHandler
{
public:
	CConfigCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CConfigCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//config"; }
};


/**
 * \brief 
 */
class CAdvcfgCommandHandler
	: public CConfigCommandHandler
{
public:
	CAdvcfgCommandHandler(CProxy* pProxy) : CConfigCommandHandler(pProxy) {}
	virtual ~CAdvcfgCommandHandler(){}

public:
	virtual const char* PrintUsage(){ return "//advcfg"; }
};


/**
 * \brief 
 */
class CAfkCommandHandler
	: public CCommandHandler
{
public:
	CAfkCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAfkCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//afk <on|off>"; }
};


/**
 * \brief 
 */
class CExitCommandHandler
	: public CCommandHandler
{
public:
	CExitCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CExitCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//exit"; }
};


/**
 * \brief 
 */
class CPtypeCommandHandler
	: public CCommandHandler
{
public:
	CPtypeCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CPtypeCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//ptype <kor|jpn|phi>"; }
};



/**
 * \brief 
 */
class CCtypeCommandHandler
	: public CCommandHandler
{
public:
	CCtypeCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CCtypeCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//ctype <s3|s4>"; }
};



/**
 * \brief 
 */
class CSendCommandHandler
	: public CCommandHandler
{
public:
	CSendCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CSendCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//send <packet in hex eg. C1 03 31>"; }
};


/**
 * \brief 
 */
class CRecvCommandHandler
	: public CCommandHandler
{
public:
	CRecvCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CRecvCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//recv <packet in hex eg. C1 03 31>"; }
};


/**
 * \brief 
 */
class CRepairAllCommandHandler
	: public CCommandHandler
{
public:
	CRepairAllCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy), m_hThread(0) {}
	virtual ~CRepairAllCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//repairall"; }

private:
	static LRESULT CALLBACK ThreadProc(CRepairAllCommandHandler*);
	HANDLE m_hThread;
};



/**
 * \brief 
 */
class CAfkStatCommandHandler
	: public CCommandHandler
{
public:
	CAfkStatCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CAfkStatCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//afkstat <on|off>"; }
};


/**
 * \brief 
 */
class CLogoutCommandHandler
	: public CCommandHandler
{
public:
	CLogoutCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CLogoutCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//logout"; }
};




/**
 * \brief 
 */
class CSetFilterFlagCommandHandler
	: public CCommandHandler
{
public:
	CSetFilterFlagCommandHandler(CProxy* pProxy, const char* pszFilter, const char* pszParam, const char* pszUsage = 0) 
		: CCommandHandler(pProxy) 
	{
		m_szFilter[sizeof(m_szFilter)-1] = 0;
		strncpy(m_szFilter, pszFilter, sizeof(m_szFilter)-1);

		m_szParam[sizeof(m_szParam)-1] = 0;
		strncpy(m_szParam, pszParam, sizeof(m_szParam)-1);
		
		m_szUsage[sizeof(m_szUsage)-1] = 0;
		m_szUsage[0] = 0;

		if (pszUsage)
			strncpy(m_szUsage, pszUsage, sizeof(m_szUsage)-1);
			
	}

	virtual ~CSetFilterFlagCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return m_szUsage[0] ? m_szUsage : 0; }

private:
	char m_szFilter[128];
	char m_szParam[128];
	char m_szUsage[256];
};



/**
 * \brief 
 */
class CInvokeWithNoArgsCommandHandler
	: public CCommandHandler
{
public:
	CInvokeWithNoArgsCommandHandler(CProxy* pProxy, const char* pszFilter, const char* pszParam, const char* pszUsage = 0, bool fSilent = false) 
		: CCommandHandler(pProxy) 
	{
		m_fSilent = fSilent;

		m_szFilter[sizeof(m_szFilter)-1] = 0;
		strncpy(m_szFilter, pszFilter, sizeof(m_szFilter)-1);

		m_szParam[sizeof(m_szParam)-1] = 0;
		strncpy(m_szParam, pszParam, sizeof(m_szParam)-1);

		m_szUsage[sizeof(m_szUsage)-1] = 0;
		m_szUsage[0] = 0;

		if (pszUsage)
			strncpy(m_szUsage, pszUsage, sizeof(m_szUsage)-1);
	}

	virtual ~CInvokeWithNoArgsCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return m_szUsage[0] ? m_szUsage : 0; }

private:
	char m_szFilter[128];
	char m_szParam[128];
	char m_szUsage[256];
	bool m_fSilent;
};



/**
 * \brief 
 */
class CSendKeyCommandHandler
	: public CCommandHandler
{
public:
	CSendKeyCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CSendKeyCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//sendkey <hex>"; }
};


/**
 * \brief 
 */
class CFixStatCommandHandler
	: public CCommandHandler
{
public:
	CFixStatCommandHandler(CProxy* pProxy) : CCommandHandler(pProxy) {}
	virtual ~CFixStatCommandHandler(){}

public:
	virtual bool ProcessCommand(const char* cmd, const char* args);
	virtual const char* PrintUsage(){ return "//fixagi <number>"; }
};


#endif //__GameCommands_H
