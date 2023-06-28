#pragma once

#include "../_Shared/ProtocolSettings.h"
#include "../_Shared/StdString.h"


/**  
 * \brief 
 */
class CApplication
{
public:
	CApplication();
	virtual ~CApplication();

	int Run();

private:
	CStdString GetUrlForUI();
	void AnsiToUnicode(wchar_t* pszDst, const char* pszSrc, int dstSize, int srcSize = -1);

	CProtocolSettings m_cSettings;
};