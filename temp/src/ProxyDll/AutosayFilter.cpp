#include "stdafx.h"
#include "AutosayFilter.h"
#include "CommonPackets.h"
#include <atlrx.h>

/**
 * \brief 
 */
CAutosayFilter::CAutosayFilter(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
	m_iNonsenseCounter = 0;
	m_iRepeatCounter = 0;
	m_fEnabled = FALSE;
	m_dwTicks = GetTickCount();

	m_fNewConfig = m_cAutosayParser.ConfigExists();

	if (m_fNewConfig)
		m_cAutosayParser.LoadData();
	else
		LoadData();
}


/**
 * \brief 
 */
CAutosayFilter::~CAutosayFilter()
{

}


/**
 * \brief 
 */
int CAutosayFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_fEnabled)
	{
		if (pkt == CCharacterSayPacket::Type())
		{
			CCharacterSayPacket pkt2(pkt);

			CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
			const char* pszCharName = 0;

			if (pFilter)
			{
				pFilter->GetParam("CharName", &pszCharName);

				if (pszCharName 
						&& _stricmp(pkt2.GetCharName(), pszCharName) != 0
						&& pkt2.GetMessage()
						&& pkt2.GetMessage()[0] != '@'
						&& pkt2.GetMessage()[0] != '~')
				{
					m_cLastSayPkt = pkt;
					m_dwTicks = GetTickCount();
					m_fDoReply = TRUE;
				}
			}
		}
		else if (m_fDoReply && (GetTickCount() - m_dwTicks) > AUTOREPLY_DELAY)
		{
			m_fDoReply = FALSE;
			DoReply();
		}
	}
	else if (pkt == CWarpReplyPacket::Type())
	{
		m_fEnabled = FALSE;
		m_iNonsenseCounter = 0;
		m_iRepeatCounter = 0;
	}

	return 0;
}


/**
 * \brief 
 */
int CAutosayFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (m_fEnabled)
	{
		if (m_fDoReply && (GetTickCount() - m_dwTicks) > AUTOREPLY_DELAY)
		{
			DoReply();
			m_fDoReply = FALSE;
		}
	}

	return 0;
}


/**
 * \brief 
 */
bool CAutosayFilter::GetParam(const char* pszParam, void* pData)
{
	return false;
}


/**
 * \brief 
 */
bool CAutosayFilter::SetParam(const char* pszParam, void* pData)
{
	if (!pszParam || !pData)
		return 0;

	if (_stricmp(pszParam, "autosay") == 0)
	{
		m_fEnabled = *((BOOL*)pData);
		m_iNonsenseCounter = 0;
		m_iRepeatCounter = 0;
	}
	else if (_stricmp(pszParam, "reload") == 0)
	{
		m_fNewConfig = m_cAutosayParser.ConfigExists();

		if (m_fNewConfig)
		{
			if (!m_cAutosayParser.LoadData())
			{
				GetProxy()->recv_direct(CServerMessagePacket(">> Error loading auto-say script: %s", m_cAutosayParser.GetLastError()));
			}
		}
		else
			LoadData();		
	}

	m_fDoReply = FALSE;
	return true;
}


/**
 * \brief 
 */
void CAutosayFilter::DoReply()
{
	srand(GetTickCount());

	if (m_fNewConfig)
	{
		CCharacterSayPacket pkt2(m_cLastSayPkt);
		const CAutosayData& data = m_cAutosayParser.GetData();

		int iCharIdx = MatchChar(pkt2.GetCharName());

		if (iCharIdx >= 0)
		{
			if (m_strLastMessage == pkt2.GetMessage())
			{
				int iMsgIdx = FindRepeatMsg(iCharIdx);

				if (iMsgIdx >= 0)
				{
					int iRepCount = atoi((const char*)data[iCharIdx].second[iMsgIdx].first + 2);
					m_iRepeatCounter++;

					if (m_iRepeatCounter >= iRepCount)
					{
						int iPos = rand() % (int)data[iCharIdx].second[iMsgIdx].second.size();

						CStringA strMessage(data[iCharIdx].second[iMsgIdx].second[iPos]);
						strMessage.Replace("$", CStringA(pkt2.GetCharName()).MakeLower());

						SendReplyMessage(strMessage);
					}
				}
			}
			else if (IsNonsense(pkt2.GetMessage()))
			{
				m_iRepeatCounter = 0;

				int iMsgIdx = FindNonsenseMsg(iCharIdx);

				if (iMsgIdx >= 0)
				{
					int iNonsCount = atoi((const char*)data[iCharIdx].second[iMsgIdx].first + 2);
					m_iNonsenseCounter++;

					if (m_iNonsenseCounter >= iNonsCount)
					{
						m_iNonsenseCounter = 0;

						int iPos = rand() % (int)data[iCharIdx].second[iMsgIdx].second.size();

						CStringA strMessage(data[iCharIdx].second[iMsgIdx].second[iPos]);
						strMessage.Replace("$", CStringA(pkt2.GetCharName()).MakeLower());

						SendReplyMessage(strMessage);
					}
				}
			}
			else
			{
				m_iNonsenseCounter = 0;
				m_iRepeatCounter = 0;

				int iMsgIdx = MatchMessage(iCharIdx, pkt2.GetMessage());

				if (iMsgIdx >= 0)
				{
					int iPos = rand() % (int)data[iCharIdx].second[iMsgIdx].second.size();

					CStringA strMessage(data[iCharIdx].second[iMsgIdx].second[iPos]);
					strMessage.Replace("$", CStringA(pkt2.GetCharName()).MakeLower());

					SendReplyMessage(strMessage);
				}
			}
		}

		m_strLastMessage = pkt2.GetMessage();
	}
	else
	{
		int iSize = (int)m_vMessages.size();
		int iPos = rand() % iSize;

		CStringA strMessage(m_vMessages[iPos]);

		CCharacterSayPacket pkt2(m_cLastSayPkt);
		strMessage.Replace("$", CStringA(pkt2.GetCharName()).MakeLower());

		CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
		const char* pszCharName = 0;

		if (!pFilter || !pFilter->GetParam("CharName", &pszCharName))
		{
			CCharacterSayPacket pkt("LordOfMU", strMessage);
			GetProxy()->send_packet(pkt);
		}
		else
		{
			CCharacterSayPacket pkt(pszCharName, strMessage);
			GetProxy()->send_packet(pkt);
		}
	}
}


/**
 * \brief 
 */
void CAutosayFilter::LoadData()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "autosay.txt");

	m_vMessages.clear();

	FILE* f = fopen(szPath, "r");

	if (!f)
		return;

	char szLine[256] = {0};
	char ch = 0;
	int pos = 0;

	while(1)
	{
		bool fStop = (1 != fread(&ch, 1, 1, f));

		if (fStop || ch == '\n' || pos >= 256)
		{
			if (szLine[0] != 0)
			{
				m_vMessages.push_back(CStringA(szLine));
			}

			pos = 0;
			memset(szLine, 0, 256);
		}
		else
		{
			szLine[pos++] = ch;
		}

		if (fStop)
			break;
	}

	fclose(f);
}


/**  
 * \brief 
 */
int CAutosayFilter::MatchChar(const char* pszCharName)
{
	const CAutosayData& data = m_cAutosayParser.GetData();
	int count = (int)data.size();

	for (int i=0; i < count; i++)
	{
		CAtlREMatchContext<CAtlRECharTraitsA> ctx;
		CAtlRegExp<CAtlRECharTraitsA> re;
		re.Parse(data[i].first, false);

		if (re.Match(pszCharName, &ctx))
			return i;
	}

	return -1;
}


/**  
 * \brief 
 */
int CAutosayFilter::MatchMessage(int iCharIdx, const char* pszMessage)
{
	const CAutosayData& data = m_cAutosayParser.GetData();
	int count2 = (int)data[iCharIdx].second.size();

	for (int j=0; j < count2; j++)
	{
		if (data[iCharIdx].second[j].second.size() > 0)
		{
			const CStringA& strTxtPattern = data[iCharIdx].second[j].first;

			if (strTxtPattern[0] == ':' || strTxtPattern[0] == '!' || strTxtPattern[0] == '|')
				continue;

			CAtlRegExp<CAtlRECharTraitsA> re2;
			re2.Parse(((const char*)strTxtPattern) + 2, false);

			CAtlREMatchContext<CAtlRECharTraitsA> ctx2;
			if (re2.Match(pszMessage, &ctx2))
				return j;
		}
	}

	return -1;
}


/**  
 * \brief 
 */
bool CAutosayFilter::IsNonsense(const char* pszMessage)
{
	char hist[256] = {0};
	int len = (int)strlen(pszMessage);

	const char* p = pszMessage;

	for (int i=0; i < len; i++)
		hist[pszMessage[i]]++;

	int iNonRepCnt = 0;

	for (int i=0; i < 256; i++)
		if (hist[i] != 0)
			iNonRepCnt++;

	float fCoef = (float)iNonRepCnt/(float)len;
	return fCoef < 0.3f;
}


/**  
 * \brief 
 */
int CAutosayFilter::FindNonsenseMsg(int iCharIdx)
{
	const CAutosayData& data = m_cAutosayParser.GetData();
	int count2 = (int)data[iCharIdx].second.size();

	for (int j=0; j < count2; j++)
	{
		if (data[iCharIdx].second[j].second.size() > 0)
		{
			const CStringA& strTxtPattern = data[iCharIdx].second[j].first;

			if (strTxtPattern[0] == '!')
				return j;
		}
	}

	return -1;
}


/**  
 * \brief 
 */
int CAutosayFilter::FindRepeatMsg(int iCharIdx)
{
	const CAutosayData& data = m_cAutosayParser.GetData();
	int count2 = (int)data[iCharIdx].second.size();

	for (int j=0; j < count2; j++)
	{
		if (data[iCharIdx].second[j].second.size() > 0)
		{
			const CStringA& strTxtPattern = data[iCharIdx].second[j].first;

			if (strTxtPattern[0] == '|')
				return j;
		}
	}

	return -1;
}


/**  
 * \brief 
 */
void CAutosayFilter::SendReplyMessage(const char* pszMessage)
{
	CPacketFilter* pFilter = GetProxy()->GetFilter("CharInfoFilter");
	const char* pszCharName = 0;

	if (!pFilter || !pFilter->GetParam("CharName", &pszCharName))
	{
		CCharacterSayPacket pkt("LordOfMU", pszMessage);
		GetProxy()->send_packet(pkt);
	}
	else
	{
		CCharacterSayPacket pkt(pszCharName, pszMessage);
		GetProxy()->send_packet(pkt);
	}
}

