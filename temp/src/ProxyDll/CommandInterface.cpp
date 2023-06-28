#include "stdafx.h"
#include "CommandInterface.h"
#include "CommonPackets.h"


/**
 * \brief 
 */
CCommandInterface::CCommandInterface(CProxy* pProxy)
	: CPacketFilter(pProxy)
{
}


/**
 * \brief 
 */
CCommandInterface::~CCommandInterface()
{
	ClearCommands();
}

/**
 * \brief 
 */
int CCommandInterface::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	return 0;
}

/**
 * \brief 
 */
int CCommandInterface::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CCharacterSayPacket::Type() 
			|| pkt == CCharacterSayPacket2::Type()
			|| pkt == CCharacterSayPacket3::Type()
			|| pkt == CCharacterWhisperPacket::Type())
	{
		CCharacterSayPacket pktCopy(pkt);
		const char* szText = pktCopy.GetMessage();

		if (strlen(szText) > 1 && ((szText[0] == '/' && szText[1] == '/') || (szText[0] == '-' && szText[1] == '-')
									|| (szText[0] == '+' && szText[1] == '+') || (szText[0] == '=' && szText[1] == '=')))
		{
			ProcessCommand(szText+2);
			return -1;
		}
	}

	return 0;
}


/**
 * \brief 
 */
bool CCommandInterface::AddCommand(const char* szCommand, CCommandHandler* pHandler)
{
	CCommandList::iterator it = m_cCommands.find(szCommand);

	if (it != m_cCommands.end())
		return false;

	m_cCommands.insert(CCommandList::value_type(szCommand, pHandler));
	return true;
}

/**
 * \brief 
 */
void CCommandInterface::ClearCommands()
{
	for (CCommandList::iterator it = m_cCommands.begin(); 
			it != m_cCommands.end(); ++it)
	{
		if (it->second)
		{
			delete it->second;
			it->second = 0;
		}
	}

	m_cCommands.clear();
}


/**
 * \brief 
 */
void CCommandInterface::ProcessCommand(const char* szCommand)
{
	char szCmd[256] = {0};
	const char* szArgs = szCommand;
	
	for (int i=0; szCommand[i] != 0 && szCommand[i] != ' '; ++i)
	{
		szCmd[i] = szCommand[i];
		szArgs = szCommand + i + 1;
	}

	while (*szArgs != 0 && *szArgs == ' ')
		szArgs++;


	CCommandList::iterator it = m_cCommands.find(szCmd);

	if (it == m_cCommands.end())
	{
		GetProxy()->recv_direct(CServerMessagePacket(">> Bad command name: %s", szCmd));
		return;
	}

	it->second->ProcessCommand(szCmd, szArgs);
}
