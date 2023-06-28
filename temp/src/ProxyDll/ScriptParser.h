#ifndef __ScriptParser_H
#define __ScriptParser_H

#pragma once

#include <vector>
#include <map>


class CScriptCommand;

/**
 * \brief 
 */
class CScriptParser
{
public:
	CScriptParser(){ memset(m_szError, 0, sizeof(m_szError)); }
	virtual ~CScriptParser();

	bool ParseLine(char* pszLine);
	std::vector<CScriptCommand*> Finalize();

	const char* GetError(){ return m_szError; }

protected:
	bool ParseDefine(char* p);
	bool ParseCommand(char* p);
	char* ParseToken(char* p, char* pszToken, int iMaxLen);
	bool SubstDefines(char* p, CStringA& res);

	bool IsCommented(char* pszLine);
	char* SkipWhitespace(char* pszLine);
	void  TrimWhitespace(char* p);
	void  TrimComment(char* p);
	bool IsValidChar(char c) { return (c >= '0' && c <= '9') || (c == '_') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
	
	bool SetError(const char* pszError){ strcpy(m_szError, pszError); return false; }

protected:
	std::vector<CScriptCommand*> m_vCommands;
	std::map<CStringA,CStringA> m_vVariables;

	char m_szError[512];
};


#endif //__ScriptParser_H