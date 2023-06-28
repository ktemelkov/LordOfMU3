#include "stdafx.h"
#include "AutosayParser.h"

#pragma warning(disable:4503)

/**  
 * \brief 
 */
bool CAutosayParser::LoadData()
{
	m_strLastError.Empty();
	m_cData.clear();
	m_iState = STATE_WAIT_CHAR_PATTERN;

	FILE* f = fopen(GetIniFilename(), "r");

	if (!f)
		return SetError("Cannot open autosay.ini file");

	char szLine[512] = {0};
	char ch = 0;
	int pos = 0;
	bool fResult = true;
	int iLineNo = 1;

	while(1)
	{
		bool fStop = (1 != fread(&ch, 1, 1, f));

		if (fStop || ch == '\n' || pos >= 511)
		{
			if (!ParseLine(szLine))
			{
				fResult = false;
				fStop = true;

				CStringA strMsg;
				strMsg.Format(" (error at line %d)", iLineNo);

				AppendError(strMsg);
			}

			iLineNo++;
			pos = 0;
			memset(szLine, 0, 512);
		}
		else
		{
			szLine[pos++] = ch;
		}

		if (fStop)
			break;
	}

	if (fResult)
		fResult = Finalize();

	fclose(f);
	return fResult;
}


/**  
 * \brief 
 */
bool CAutosayParser::ConfigExists()
{
	return PathFileExistsA(GetIniFilename()) == TRUE;
}


/**  
 * \brief 
 */
CStringA CAutosayParser::GetIniFilename()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "Autosay.ini");

	return CStringA(szPath);
}


/**  
 * \brief 
 */
bool CAutosayParser::ParseLine(char* pszLine)
{
	if (!pszLine)
		return SetError("Invalid function argument: ParseLine().");

	char* p = SkipWhitespace(pszLine);

	if (*p == 0)
		return true;

	if (IsCommented(p))
		return true;

	TrimWhitespace(p);

	switch (m_iState)
	{
	case STATE_WAIT_CHAR_PATTERN:
		{
			if (!CheckForCharPatternLine(&p))
				return SetError("Bad syntax, character name pattern expected");

			return ProcessCharPattern(p);
		}
		break;
	case STATE_WAIT_TEXT_PATTERN:
		{
			if (CheckForCharPatternLine(&p))
				return ProcessCharPattern(p);

			if (!CheckForTextPatternLine(&p))
				return SetError("Bad syntax, text pattern expected");

			return ProcessTextPattern(p);
		}
		break;
	case STATE_WAIT_REPLY_TEXT:
		{
			if (CheckForCharPatternLine(&p))
				return ProcessCharPattern(p);

			if (CheckForTextPatternLine(&p))
				return ProcessTextPattern(p);

			if (!CheckForReplyTextLine(&p))
				return SetError("Bad syntax, reply text expected");

			return ProcessReplyText(p);
		}
		break;
	}

	return SetError("Bad parser state");
}


/**
 * \brief 
 */
bool CAutosayParser::IsCommented(char* pszLine)
{
	return pszLine && (*pszLine == ';');
}


/**
 * \brief 
 */
char* CAutosayParser::SkipWhitespace(char* pszLine)
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
void CAutosayParser::TrimWhitespace(char* p)
{
	if (!p)
		return;

	for (int i=(int)strlen(p)-1; i >= 0 && (p[i] == ' ' || p[i] == '\t'); p[i--] = 0);
}


/**  
 * \brief 
 */
bool CAutosayParser::CheckForCharPatternLine(char** pp)
{
	char* p = *pp;
	int iLen = (int)strlen(p);

	if (iLen > 0 && p[0] == '{' && p[iLen-1] == '}')
	{
		p[iLen-1] = 0;
		p++;

		*pp = p;
		return true;
	}

	return false;
}


/**  
 * \brief 
 */
bool CAutosayParser::CheckForTextPatternLine(char** pp)
{
	char* p = *pp;

	if (strstr(p, ">>") == p || strstr(p, "!>") == p || strstr(p, ":>") == p || strstr(p, "|>") == p)
	{
		char* p1 = SkipWhitespace(p+2);
		return (*p1 != 0);
	}

	return false;
}


/**  
 * \brief 
 */
bool CAutosayParser::CheckForReplyTextLine(char** pp)
{
	char* p = *pp;

	if (p[0] == '-')
	{
		p = SkipWhitespace(++p);

		*pp = p;
		return true;
	}

	return false;
}


/**  
 * \brief 
 */
bool CAutosayParser::ProcessCharPattern(char* p)
{
	p = SkipWhitespace(p);
	TrimWhitespace(p);

	m_cData.push_back(std::pair<CStringA, CRepliesData>(p, CRepliesData()));

	m_iState = STATE_WAIT_TEXT_PATTERN;
	return true;
}


/**  
 * \brief 
 */
bool CAutosayParser::ProcessTextPattern(char* p)
{
	char hdr[3] = {p[0], p[1], 0};

	CStringA str(hdr);
	p = SkipWhitespace(p+2);

	str += p;

	int iPos1 = (int)m_cData.size()-1;
	if (iPos1 < 0)
		return SetError("Bad char pattern data");

	m_cData[iPos1].second.push_back(std::pair<CStringA, CReplyData>(str, CReplyData()));

	m_iState = STATE_WAIT_REPLY_TEXT;
	return true;
}


/**  
 * \brief 
 */
bool CAutosayParser::ProcessReplyText(char* p)
{
	int iPos1 = (int)m_cData.size()-1;
	if (iPos1 < 0)
		return SetError("Bad char pattern data");

	int iPos2 = (int)m_cData[iPos1].second.size()-1;
	if (iPos2 < 0)
		return SetError("Bad text pattern data");

	m_cData[iPos1].second[iPos2].second.push_back(p);
	return true;
}


/**  
 * \brief 
 */
bool CAutosayParser::Finalize()
{
	int count = (int)m_cData.size();
	int iPrevUnresolved = count;

	while (true)
	{
		int iUnresolved = count;

		for (int i=0; i < count; ++i)
		{
			char* p = (char*)(const char*)m_cData[i].first;
			char* p1 = strstr(p, "=>");

			if (p1 == 0)
			{
				iUnresolved--;
				continue;
			}

			if (*(p1+2) == 0)
				return SetError("Bad inheritance syntax");

			int idx = FindCharPattern(SkipWhitespace(p1+2));

			if (idx < 0)
				continue;
			
			iUnresolved--;

			int iPrCount = (int)m_cData[idx].second.size(); 

			for (int j=0; j < iPrCount; j++)
			{
				m_cData[i].second.push_back(m_cData[idx].second[j]);
			}

			*p1 = 0;
			TrimWhitespace(p);

			m_cData[i].first = CStringA(p);
		}

		if (iUnresolved == 0)
			return true;

		if (iPrevUnresolved == iUnresolved)
			return SetError("Unresolved inheritance");

		iPrevUnresolved = iUnresolved;
	}

	return SetError("Should never get here");
}


/**  
 * \brief 
 */
int CAutosayParser::FindCharPattern(char* p)
{
	for (int i=(int)m_cData.size()-1; i >= 0; --i)
	{
		if (m_cData[i].first == p)
			return i;
	}

	return -1;
}

