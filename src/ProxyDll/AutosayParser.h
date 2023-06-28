#ifndef __AutosayParser_H
#define __AutosayParser_H

#pragma once

#include <vector>

/**  
 * \brief 
 * array of char name patterns
 * array of text patterns
 * array of replies
 */
typedef std::vector<CStringA> CReplyData;
typedef std::vector< std::pair<CStringA, CReplyData> > CRepliesData;
typedef std::vector< std::pair<CStringA, CRepliesData> > CAutosayData;

/**  
 * \brief 
 */
class CAutosayParser
{
public:
	CAutosayParser(){ m_iState = 0; }

	bool LoadData();
	bool ConfigExists();

	CStringA GetLastError(){ CStringA res(m_strLastError); m_strLastError.Empty(); return res; }
	const CAutosayData& GetData(){ return m_cData; }

protected:
	CStringA GetIniFilename();
	bool SetError(const char* pszError){ m_strLastError = pszError; return false; }
	bool AppendError(const char* pszError){ m_strLastError += pszError; return false; }

	bool ParseLine(char* pszLine);
	bool IsCommented(char* pszLine);
	char* SkipWhitespace(char* pszLine);
	void TrimWhitespace(char* p);

	bool CheckForCharPatternLine(char** pp);
	bool ProcessCharPattern(char* p);

	bool CheckForTextPatternLine(char** pp);
	bool ProcessTextPattern(char* p);

	bool CheckForReplyTextLine(char** pp);
	bool ProcessReplyText(char* p);

	bool Finalize();
	int FindCharPattern(char* p);

protected:
	static const int STATE_INIT = 0;
	static const int STATE_WAIT_CHAR_PATTERN = 1;
	static const int STATE_WAIT_TEXT_PATTERN = 2;
	static const int STATE_WAIT_REPLY_TEXT = 3;

protected:
	CAutosayData m_cData;
	CStringA m_strLastError;
	int m_iState;
};


#endif //__AutosayParser_H