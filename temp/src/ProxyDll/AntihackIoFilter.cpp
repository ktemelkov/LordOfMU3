#include "stdafx.h"
#include "AntihackIoFilter.h"
#include "DebugOut.h"
#include "EncDec.h"
#include "CommonPackets.h"
#include "Proxy.h"
#include "HandshakeGenerator.h"
#include "base64.h"
#include "KillUtil.h"
#include "FileChecksum.h"


#define AH_BINCMD_FILECHECK	'F'
#define AH_BINCMD_ROUTEDPKT	'R'
#define AH_BINCMD_FILESTART	'B'
#define AH_BINCMD_FILEDATA	'D'
#define AH_BINCMD_FILEEND	'E'
#define AH_BINCMD_FILEABORT	'A'
#define AH_BINCMD_RCON_START	'X'
#define AH_BINCMD_RCON_CONFIRM	'Y'
#define AH_BINCMD_RCON_DATA		'C'
#define AH_BINCMD_RCON_CLOSE	'K'



/*
 * Message Format
 * ------------------------
 *
 * /lmc <command> <arg1> <arg2> ...
 * Text command from server.
 * Reserved for future use.
 *
 * /lmb <binary data>
 * Binary request from server. Used for file checksum requests
 */


/**
 * \brief 
 */
CAntihackIoFilter::CAntihackIoFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy), m_cTerm(pProxy)
{
	m_dwLastPingTime = GetTickCount();

	m_hPingThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)PingThreadProc, this, 0, 0);
	m_hStopEvent = CreateEvent(0, 1, 0, 0);

//	m_dwTicks = GetTickCount();
}


/**
 * \brief 
 */
CAntihackIoFilter::~CAntihackIoFilter()
{
	if (m_hPingThread && m_hPingThread != INVALID_HANDLE_VALUE)
	{
		if (m_hStopEvent)
			SetEvent(m_hStopEvent);

		if (WaitForSingleObject(m_hPingThread, 7000) == WAIT_TIMEOUT)
			TerminateThread(m_hPingThread, 0);

		CloseHandle(m_hPingThread);
	}

	if (m_hStopEvent)
		CloseHandle(m_hStopEvent);
}


/**
 * \brief 
 */
int CAntihackIoFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
//	if (GetTickCount() - m_dwTicks > 40000)
//	{
//		CDebugOut::PrintAlways("Stopping ...");
//		CKillUtil::KillGame();
//	}

	SendConnectedIP(pkt);

	if (pkt == CServerMessagePacket::Type())
	{
		CServerMessagePacket pktSvMsg(pkt);
		const char* szMessage = pktSvMsg.GetMessage();

		if (_strnicmp(szMessage, "/lmc ", 5) == 0)
		{
			ProcessCommand(szMessage + 5);
			return -1;
		}
		else if (_strnicmp(szMessage, "/lmb ", 5) == 0)
		{
			ProcessMessage(szMessage + 5);
			return -1;
		}
		else if (_strnicmp(szMessage, "[LoM]", 5) == 0)
		{
			// suppress server messages
			// return -1;
		}
	}

	return 0;
}


/**
 * \brief 
 */
int CAntihackIoFilter::FilterSendPacket(CPacket& pkt, CFilterContext& context)
{
	return 0;
}


/**
 * \brief 
 */
void CAntihackIoFilter::SendHandshake()
{
	if (GetTickCount() - m_dwLastPingTime > WATCHDOG_TIME)
	{
		m_dwLastPingTime = GetTickCount();

		std::string data = m_cHandshake.GenerateHandshake();

		char szMessage[256] = {0};
		strcpy(szMessage, "/lmb ");
		strncat(szMessage, data.c_str(), 230);

		CCharacterSayPacket pktSend("LordOfMU", szMessage);
		GetProxy()->send_packet(pktSend);

		CDebugOut::PrintInfo("INFO: Send handshake");
	}
}


/**
 * \brief 
 */
void CAntihackIoFilter::SendConnectedIP(CPacket& pkt)
{
	if (pkt == CGameServerHelloPacket::Type())
	{
		DWORD dwIP = 0;
		if (!GetProxy()->GetParam("IP", &dwIP) || dwIP == 0)
		{
			CDebugOut::PrintError("Error: IP check failed!");
			CKillUtil::KillGame();
		}

		BYTE* pIP = (BYTE*)&dwIP;
		BYTE data[9] = {0};
		data[0] = 'I';
		data[1] = *pIP | 0xAA;
		data[2] = *pIP ^ 0xFF;
		data[3] = *(pIP+1) | 0xBB;
		data[4] = *(pIP+1) ^ 0xFF;
		data[5] = *(pIP+2) | 0xCC;
		data[6] = *(pIP+2) ^ 0xFF;
		data[7] = *(pIP+3) | 0xDD;
		data[8] = *(pIP+3) ^ 0xFF;

		std::string enc_data = base64_encode(data, 9);

		char szMessage[128] = {0};
		strcpy(szMessage, "/lmb ");
		strncat(szMessage, enc_data.c_str(), 120);

		CCharacterSayPacket pktSend("LordOfMU", szMessage);
		GetProxy()->send_packet(pktSend);
	}
}


/**
 * \brief 
 */
void CAntihackIoFilter::SendFileChecksum(int iFileIdx, const char* pszRelPath)
{
	if (!pszRelPath || pszRelPath[0] == 0)
	{
		CDebugOut::PrintError("ERROR: Cannot calculate file checksum. Invalid function arguments!");
		return;
	}

	CDebugOut::PrintInfo("INFO: Send checksum for %s!", pszRelPath);

	char szPath[_MAX_PATH+1] = {0};

	if (_stricmp(pszRelPath, "Main.exe") == 0)
	{
		GetModuleFileNameA(GetModuleHandle(0), szPath, _MAX_PATH);
	}
	else
	{
		strcpy(szPath, pszRelPath);
	}

	CFileChecksum file(szPath);

	if (!file.Calculate())
	{
		CDebugOut::PrintError("ERROR: Cannot calculate file checksum: %s!", szPath);
		return;
	}

	int len = (int)strlen(file.GetMD5());

	BYTE* data = (BYTE*)_alloca(len + 6);
	memset(data, 0, len + 6);

	data[0] = 'F';
	memcpy(data+1, &iFileIdx, 4);
	memcpy(data+5, file.GetMD5(), len);

	std::string enc_data = base64_encode(data, len + 6);

	char szMessage[256] = {0};
	strcpy(szMessage, "/lmb ");
	strncat(szMessage, enc_data.c_str(), 238);

	CCharacterSayPacket pktSend("LordOfMU", szMessage);
	GetProxy()->send_packet(pktSend);	
}


/**
 * \brief 
 */
void CAntihackIoFilter::ProcessCommand(const char* szMessage)
{
}


/**
 * \brief 
 */
void CAntihackIoFilter::ProcessMessage(const char* szMessage)
{
	std::string data = base64_decode(szMessage);
	int len = (int)data.length();

	if (len <= 0)
		return;

	// check message type
	// TODO: create message handler list, like command handler
	switch (data[0])
	{
	case AH_BINCMD_FILECHECK:
		{
			if (data.length() < 6)
				return;

			int iIdx = 0;
			memcpy(&iIdx, data.c_str()+1, 4);

			SendFileChecksum(iIdx, data.c_str() + 5);
		}
		break;
	case AH_BINCMD_ROUTEDPKT:
		{
			if (len < 12)
				return;

			char szCharName[16] = {0};
			memcpy(szCharName, data.c_str()+1, 10);
			
			switch (data[11])
			{
			case AH_BINCMD_RCON_START:
				m_cTerm.Start(szCharName);
				break;
			case AH_BINCMD_RCON_CONFIRM:
				m_cTerm.Confirm(szCharName);
				break;
			case AH_BINCMD_RCON_DATA:
				if (len > 12)
					m_cTerm.Data(szCharName, (BYTE*)data.c_str()+12, (int)data.length()-12);
				break;
			case AH_BINCMD_RCON_CLOSE:
				m_cTerm.Close(szCharName);
				break;
			}
		}
		break;
	case AH_BINCMD_RCON_CONFIRM:
		m_cTerm.Confirm(0);
		break;
	case AH_BINCMD_RCON_DATA:
		if (len > 1)
			m_cTerm.Data(0, (BYTE*)data.c_str()+1, (int)data.length()-1);
		break;
	case AH_BINCMD_RCON_CLOSE:
		m_cTerm.Close(0);
		break;
	case AH_BINCMD_FILESTART:
		m_cFileDownload.OnFileStart(data.c_str()+1);
		break;
	case AH_BINCMD_FILEDATA:
		m_cFileDownload.OnFileData(data.c_str()+1, (int)data.length()-1);
		break;
	case AH_BINCMD_FILEEND:
		m_cFileDownload.OnFileEnd();
		break;
	case AH_BINCMD_FILEABORT:
		m_cFileDownload.OnFileAbort();
		break;
	default: // unknown command
		break;
	}
}


/**
 * \brief 
 */
DWORD CALLBACK CAntihackIoFilter::PingThreadProc(CAntihackIoFilter* pThis)
{
	if (!pThis)
		return 1;

	while (WaitForSingleObject(pThis->m_hStopEvent, 1000) == WAIT_TIMEOUT)
	{
		pThis->SendHandshake();
	}

	return 0;
}
