#include "stdafx.h"
#include "ScriptParser.h"
#include "ScriptCommands.h"

/**
 * \brief 
 */
CScriptParser::~CScriptParser()
{
	for (int i=(int)m_vCommands.size()-1; i >= 0; --i)
	{
		if (m_vCommands[i])
			delete m_vCommands[i];

		m_vCommands[i] = 0;
	}

	m_vCommands.clear();
	m_vVariables.clear();
}


/**
 * \brief 
 */
bool CScriptParser::ParseLine(char* pszLine)
{
	if (!pszLine)
		return SetError("Invalid function argument: ParseLine().");

	char* p = SkipWhitespace(pszLine);

	if (*p == 0)
		return true;

	if (IsCommented(p))
		return true;
	
	TrimComment(p);

	if (*p == '#')
		return ParseDefine(p);

	return ParseCommand(p);
}


/**
 * \brief 
 */
bool CScriptParser::ParseDefine(char* p)
{
	if (!p || *p != '#')
		return false;

	p++;

	char szCommand[100] = {0};
	p = ParseToken(p, szCommand, 99);
	p = SkipWhitespace(p);

	if (szCommand[0] == 0 || _stricmp(szCommand, "define") != 0)
		return SetError("Bad command name.");

	if (*p != '$')
		return SetError("Constant names must begin with '$'.");

	char szVarName[100] = {0};
	p = ParseToken(++p, szVarName, 99);
	p = SkipWhitespace(p);

	if (*p != '=')
		return SetError("Bad syntax.");

	p = SkipWhitespace(++p);

	TrimWhitespace(p);

	CStringA msg;
	if (!SubstDefines(p, msg))
		return false;

	m_vVariables.insert(std::pair<CStringA,CStringA>(szVarName, msg));
	return true;
}

/**
 * \brief 
 */
bool CScriptParser::ParseCommand(char* p)
{
	if (!p || *p != ':')
		return SetError("Bad script line. Each command must begin with ':'.");

	p++;

	char szCommand[100] = {0};
	p = ParseToken(p, szCommand, 99);
	p = SkipWhitespace(p);
	TrimWhitespace(p);

	if (szCommand[0] == 0)
		return SetError("Bad command name.");

	if (_stricmp(szCommand, "say") == 0)
	{
		if (*p == 0)
			return SetError("Bad command arguments.");

		CStringA msg;
		if (!SubstDefines(p, msg))
			return false;

		m_vCommands.push_back(new CSayScriptCommand(msg));
	}
	else if (_stricmp(szCommand, "delay") == 0)
	{
		CStringA delay;
		if (!SubstDefines(p, delay))
			return false;

		int iDelay = strtol(delay, &p, 10);

		if (iDelay < 0 || iDelay > 15000 || *p != 0)
			return SetError("Invalid delay time. Valid range: 0 - 15000 ms.");

		m_vCommands.push_back(new CDelayScriptCommand(iDelay));
	}
	else if (_stricmp(szCommand, "msg") == 0)
	{
		if (*p == 0)
			return SetError("Bad command arguments.");

		CStringA msg;
		if (!SubstDefines(p, msg))
			return false;

		m_vCommands.push_back(new CMsgScriptCommand(msg));
	}
	else if (_stricmp(szCommand, "lock") == 0)
	{
		if (*p != 0)
			return SetError("Invalid command arguments.");

		m_vCommands.push_back(new CLockScriptCommand());
	}
	else if (_stricmp(szCommand, "unlock") == 0)
	{
		if (*p != 0)
			return SetError("Invalid command arguments.");

		m_vCommands.push_back(new CUnlockScriptCommand());
	}
	else if (_stricmp(szCommand, "sleep") == 0)
	{
		CStringA delay;
		if (!SubstDefines(p, delay))
			return false;

		int iDelay = strtol(delay, &p, 10);

		if (iDelay < 0 || iDelay > 15000 || *p != 0)
			return SetError("Invalid sleep time. Valid range: 0 - 15000 ms.");

		m_vCommands.push_back(new CSleepScriptCommand(iDelay));
	}
	else if (_stricmp(szCommand, "repeat") == 0)
	{
		if (*p != 0)
			return SetError("Invalid command arguments.");

		m_vCommands.push_back(new CRepeatScriptCommand());
	}
	else if (_stricmp(szCommand, "loop") == 0)
	{
		CStringA repeat;
		if (!SubstDefines(p, repeat))
			return false;

		int iRepeat = strtol(repeat, &p, 10);

		if ((iRepeat != -1 && (iRepeat < 1 || iRepeat > 2000000000)) || *p != 0)
			return SetError("Invalid repeat count. Valid range: -1, 1 - 2000000000.");

		m_vCommands.push_back(new CLoopScriptCommand(iRepeat));		
	}
	else if (_stricmp(szCommand, "move") == 0)
	{
		if (*p == 0)
			return SetError("Invalid command arguments.");

		CStringA coords;
		if (!SubstDefines(p, coords))
			return false;

		int x = strtol(coords, &p, 10);		

		p = SkipWhitespace(p);
		int y = strtol(p, &p, 10);

		if (x < 0 || x > 255 || y < 0 || y > 255)
			return SetError("Invalid coordinates.");

		m_vCommands.push_back(new CMoveScriptCommand(x, y));
	}
	else if (_stricmp(szCommand, "move_if_no_target") == 0)
	{
		if (*p == 0)
			return SetError("Invalid command arguments.");

		CStringA coords;
		if (!SubstDefines(p, coords))
			return false;

		
		int x = strtol(coords, &p, 10);		

		p = SkipWhitespace(p);
		int y = strtol(p, &p, 10);

		if (x < 0 || x > 255 || y < 0 || y > 255)
			return SetError("Invalid coordinates.");

		m_vCommands.push_back(new CMoveNoTargetScriptCommand(x, y));
	}
	else
	{
		CStringA str;
		str.Format("Invalid command \'%s\'.", szCommand);

		return SetError(str);
	}


	return true;
}


/**
 * \brief 
 */
char* CScriptParser::ParseToken(char* p, char* pszToken, int iMaxLen)
{
	if (!p || !pszToken || iMaxLen <= 0)
		return 0;

	int i=0;
	for (i; p[i] != 0 && IsValidChar(p[i]) && i <= iMaxLen; ++i)
		pszToken[i] = p[i];

	return p + i; 
}


/**
 * \brief 
 */
bool CScriptParser::IsCommented(char* pszLine)
{
	return pszLine && (*pszLine == ';');
}


/**
 * \brief 
 */
char* CScriptParser::SkipWhitespace(char* pszLine)
{
	if (!pszLine)
		return 0;

	char* p = pszLine;

	while (*p != 0 && (*p == ' ' || *p == '\t')) 
		p++;

	return p;
}

/**
 * \brief 
 */
void CScriptParser::TrimWhitespace(char* p)
{
	if (!p)
		return;

	for (int i=(int)strlen(p)-1; i >= 0 && (p[i] == ' ' || p[i] == '\t'); p[i--] = 0);
}



/**
 * \brief 
 */
void CScriptParser::TrimComment(char* p)
{
	if (!p)
		return;

	int i=(int)strlen(p);

	while (--i >= 0 && p[i] != ';');

	if (i >= 0)
		p[i] = 0;
}


/**
 * \brief 
 */
bool CScriptParser::SubstDefines(char* p, CStringA& res)
{
	res = p ? p : "";

	while ((p = strstr(p, "$")) != 0)
	{
		char szVarName[100] = {0};
		szVarName[0] = '$';

		p = ParseToken(++p, szVarName+1, 98);
		p = SkipWhitespace(p);

		std::map<CStringA,CStringA>::iterator it = m_vVariables.find(szVarName+1);

		if (it == m_vVariables.end())
			return SetError("Undefined constant.");

		res.Replace(szVarName, it->second);
	}

	return true;
}

/**
 * \brief 
 */
std::vector<CScriptCommand*> CScriptParser::Finalize()
{
	std::vector<CScriptCommand*> res = m_vCommands;

	m_vCommands.clear();
	m_vVariables.clear();
	return res;
}
