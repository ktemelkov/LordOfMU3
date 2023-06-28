#include "stdafx.h"
#include "ProtocolDetectFilter.h"
#include "ProtocolSettings.h"
#include "LordOfMUdll.h"
#include "EncDec.h"
#include "DebugOut.h"
#include "CommonPackets.h"
#include "version.h"


#define P_DETECT_STATE_INIT 0
#define P_DETECT_STATE_PTYPE 1

#define P_DETECT_STATE_VERSION_DETECTED 100
#define P_DETECT_STATE_APPLY_VERSION 101
#define P_DETECT_STATE_READY 102


/**  
 * \brief 
 */
bool CProtocolDetectFilter::m_fVersionDetected = false;



/**
 * \brief 
 */
CProtocolDetectFilter::CProtocolDetectFilter(CProxy* pProxy) 
	: CPacketFilter(pProxy) 
{
	m_iState = CProtocolDetectFilter::m_fVersionDetected ? P_DETECT_STATE_APPLY_VERSION : P_DETECT_STATE_INIT;
	m_ulVersion = 0;
	m_dwGameStartTS = 0;
	m_dwTrialCheckTS = GetTickCount();

	m_dwTrialTime = 7*60*1000;
	m_dwCheckTime = 10*1000;


	if (!CProtocolDetectFilter::m_fVersionDetected)
	{
		DWORD dwType = PTYPE_ENG;
		DWORD dwGType = 1;

		extern TCHAR g_szRoot[_MAX_PATH + 1];

		CProtocolSettings& sett = CProtocolSettings(CT2CA(g_szRoot));

		if (sett.Load())
		{
			dwType = sett.data.dwProtocolType;
			dwGType = sett.data.dwClientType;
		}

		DWORD ulVersion = dwType | ((dwGType == 1) ? GTYPE_S3 : GTYPE_S4);

		CPacketType::SetVersion(ulVersion);
		CPacketType::SetProtocol(ulVersion);
		CPacketType::SetFeatures(sett.data.dwFlags);

		ULONG ulGameVersion = ((ulVersion & GTYPE_S3) != GTYPE_S3) ? 105 : 104;

		if (CProxyClickerModule::GetInstance()->m_pLoader)
		{
			CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SET_VERSION, _MODULE_LOADER_TARGET_GUI, &ulGameVersion, 0);
			CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SET_FEATURES, _MODULE_LOADER_TARGET_GUI, &sett.data.dwFlags, 0);
		}
	}
	else
	{
		m_ulVersion = CPacketType::GetVersion();
	}
}




/**
 * \brief 
 */
int CProtocolDetectFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
	if (pkt == CConnectServHelloPacket::Type())
	{
		m_dwGameStartTS = 0;
	}
	else if (pkt == CGameServerHelloPacket::Type())
	{
		if ((CPacketType::GetFeatures() & 0x80000000) == 0)
		{
			m_dwGameStartTS = GetTickCount();
			DWORD dwTempFt = 0x7FFFFFFF;

			CPacketType::SetFeatures(dwTempFt);

			if (CProxyClickerModule::GetInstance()->m_pLoader)
				CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SET_FEATURES, _MODULE_LOADER_TARGET_GUI, &dwTempFt, 0);
		}
		else
			m_dwGameStartTS = 0;

		if (CProxyClickerModule::GetInstance()->m_pLoader)
			CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_CONNECT, _MODULE_LOADER_TARGET_GUI, 0, 0);
	}

	CheckTrial();

	if (P_DETECT_STATE_READY == m_iState)
		return 0;

	if (P_DETECT_STATE_PTYPE == m_iState)
	{
		if (DetectProtocolVersion(pkt))
		{
			m_iState = P_DETECT_STATE_VERSION_DETECTED;
		}
	}

	FinalizeVersionDetect();
	return 0;
}



/**
 * \brief 
 */
int CProtocolDetectFilter::FilterSendPacket(CPacket& pkt, CFilterContext&)
{
	if (P_DETECT_STATE_READY == m_iState)
		return 0;

	
	if (P_DETECT_STATE_INIT == m_iState)
	{
		if (pkt.GetPktClass() == 0xC3)
		{
			CPacket pktCopy = pkt;
			CEncDec::DecryptC3asServer(pktCopy);

			int len = pktCopy.GetDecryptedLen();
			BYTE* buf = pktCopy.GetDecryptedPacket();

			if (buf && len > 3 && buf[3] == 0xF1) // Client auth packet
			{
				CEncDec::DecXor32(buf + 4, 3, len - 4);

				if (buf[4] != 0x01)
				{
					CEncDec::SetExtraCrypt();
					CDebugOut::PrintInfo("- Extra XOR encryption enabled.");
				}

				m_iState = P_DETECT_STATE_PTYPE;
			}
		}
	}
	
	FinalizeVersionDetect();
	return 0;
}


/**  
 * \brief 
 */
void CProtocolDetectFilter::FinalizeVersionDetect()
{
	if (P_DETECT_STATE_VERSION_DETECTED == m_iState)
	{
		CDebugOut::PrintInfo("- Game version detected.");

		CProtocolDetectFilter::m_fVersionDetected = true;
		m_iState = P_DETECT_STATE_APPLY_VERSION;
	}

	if (P_DETECT_STATE_APPLY_VERSION == m_iState)
	{
		ApplyVersion();
		m_iState = P_DETECT_STATE_READY;
	}
}



/**
 * \brief 
 */
bool CProtocolDetectFilter::DetectProtocolVersion(CPacket& pkt)
{
	BYTE* buf = pkt.AnyBuffer();

	if (buf[0] == 0xC1 && buf[1] == 0x08)
	{
		DWORD dwType = 0;

		switch (buf[2])
		{
		case 0xD4: // ENG
			dwType = PTYPE_ENG;
			break;
		case 0x1D: // JPN
			dwType = PTYPE_JPN;
			break;
		case 0xD3: // KOR
			dwType = PTYPE_KOR;
			break;
		case 0xD7: // CHS
			dwType = PTYPE_CHS;
			break;
		case 0xD9: // VTM
			dwType = PTYPE_VTM;
			break;
//		case 0xDD: // PHI
//			dwType = PTYPE_PHI;
//			break;
//		case 0xD7: // TAI
//			dwType = PTYPE_TAI;
//			break;
		default: // not a move packet
			break;
		}

		if (dwType != 0)
		{
			m_ulVersion = (m_ulVersion & 0xFF000000) | dwType;
			CPacketType::SetProtocol(m_ulVersion);
		}
	}
	else if (buf[0] == 0xC2 && buf[3] == 0x13)
	{
		int iCount = (int)buf[4];
		int iSize = pkt.GetPktLen();

		int iFldLen = (iSize - 5) / iCount;

		DWORD dwGType = (iFldLen == 16) ? GTYPE_S3 : GTYPE_S4;

		m_ulVersion = (m_ulVersion & 0x00FFFFFF) | dwGType;
		CPacketType::SetVersion(m_ulVersion);
	}

	return (m_ulVersion & 0xFF000000) != 0 && (m_ulVersion & 0x00FFFFFF) != 0;
}



/**
 * \brief 
 */
bool CProtocolDetectFilter::GetParam(const char* pszParam, void* pData)
{ 
	if (!pszParam || !pData)
		return false;

	if (_stricmp(pszParam, "version") == 0)
	{
		*((ULONG*)pData) = CPacketType::GetVersion();
		return true;
	}

	return false; 
}


/**  
 * \brief 
 */
bool CProtocolDetectFilter::SetParam(const char* pszParam, void* pData)
{ 
	if (_stricmp(pszParam, "ptype") == 0)
	{
		DWORD dwType = *((DWORD*)pData);
		DWORD dwGType = (m_ulVersion & 0xFF000000) >> 24;

		m_ulVersion = dwType | ((dwGType == 1) ? GTYPE_S3 : GTYPE_S4);

		ApplyVersion();
		return true;
	}
	else if (_stricmp(pszParam, "ctype") == 0)
	{
		DWORD dwType = m_ulVersion & 0x0000FFFF;
		DWORD dwGType = *((DWORD*)pData);

		m_ulVersion = dwType | ((dwGType == 1) ? GTYPE_S3 : GTYPE_S4);

		ApplyVersion();
		return true;
	}
	else if (_stricmp(pszParam, "testend") == 0)
	{
		DWORD dwTempFt = 0x00000000;
		CPacketType::SetFeatures(dwTempFt);	

		if (CProxyClickerModule::GetInstance()->m_pLoader)
			CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SET_FEATURES, _MODULE_LOADER_TARGET_GUI, &dwTempFt, 0);
	}

	return false; 
}



/**  
 * \brief 
 */
void CProtocolDetectFilter::ApplyVersion()
{
	ULONG ulGameVersion = ((m_ulVersion & GTYPE_S3) != GTYPE_S3) ? 105 : 104;

	if (CProxyClickerModule::GetInstance()->m_pLoader)
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_SET_VERSION, _MODULE_LOADER_TARGET_GUI, &ulGameVersion, 0);

	CPacketType::SetVersion(m_ulVersion);
	CPacketType::SetProtocol(m_ulVersion);

	CDebugOut::PrintInfo("- Apply client version: 0x%04X", m_ulVersion);


	CProtocolSettings& sett = CProtocolSettings(CT2CA(g_szRoot));

	if (sett.Load())
	{
		sett.data.dwClientType = (m_ulVersion & 0xFF000000) >> 24;
		sett.data.dwProtocolType = m_ulVersion & 0x00FFFFFF;

		sett.Save();
	}
}


/**  
 * \brief 
 */
void CProtocolDetectFilter::CheckTrial()
{
	if (m_dwGameStartTS)
	{
		DWORD dwTicks = GetTickCount();

		if (dwTicks - m_dwTrialCheckTS > m_dwCheckTime)
		{
			if (dwTicks - m_dwGameStartTS > m_dwTrialTime + m_dwCheckTime)
			{
				// trial is over
				EnforceFeaturesOff();

				m_dwCheckTime = 10*1000;
				m_dwGameStartTS = 0;
			}
			else if (dwTicks - m_dwGameStartTS > m_dwTrialTime)
			{
				CGMMessagePacket pktMsg("Thank you for testing"); 
				CGMMessagePacket pktMsg1("MU AutoClicker V" __SOFTWARE_VERSION_STR); 
				CGMMessagePacket pktMsg2("on this server!"); 
				 
				GetProxy()->recv_packet(pktMsg);
				GetProxy()->recv_packet(pktMsg1);
				GetProxy()->recv_packet(pktMsg2);

				CGMMessagePacket pktMsg3("Some bot functions will ");
				CGMMessagePacket pktMsg4("be suspended till next login.");
				GetProxy()->recv_packet(pktMsg3);
				GetProxy()->recv_packet(pktMsg4);

				m_dwCheckTime = 5*1000;
			}

			m_dwTrialCheckTS = dwTicks;
		}
	}
}


/**  
 * \brief 
 */
void CProtocolDetectFilter::EnforceFeaturesOff()
{
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--autokill off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--autospeak off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--reflect off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--multihit 0"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--itemcode off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--script off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--loglevel error"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--fmove off"));
	GetProxy()->send_packet(CCharacterSayPacket("LordOfMU", "--testend"));
}

