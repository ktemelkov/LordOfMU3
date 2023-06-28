#include "stdafx.h"
#include "AutoLoginFilter.h"
#include "ProtocolSettings.h"
#include "LordOfMUdll.h"
#include "EncDec.h"
#include "DebugOut.h"
#include "CommonPackets.h"
#include "MuMessages.h"



/**
 * \brief 
 */
int CAutoLoginFilter::FilterRecvPacket(CPacket& pkt, CFilterContext& context)
{
/*	if (pkt == CServerListReplyPacket::Type())
	{
		GetProxy()->send_packet(CServerInfoReqPacket());
	}
	else if (pkt == CGameServerHelloPacket::Type())
	{
		CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_CLICKER_MODULE_COMMAND_LOGIN, _MODULE_LOADER_TARGET_GUI, (void*)"saferk0de", 0);
	}
	else if (pkt == CCharListReplyPacket::Type())
	{
		const char* pszCharName = "Kukata";

		CCharListReplyPacket pkt2(pkt);
		int iCount = pkt2.GetCharCount();
		
		for (int i=0; i < iCount; i++)
		{
			CStringA strName = pkt2.GetCharName(i);

			if (StrCmpA(pszCharName, (const char*)strName) == 0)
			{
				HWND hWnd = 0;
				CProxyClickerModule::GetInstance()->m_pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hWnd, 0);

				if (hWnd)
					PostMessage(hWnd, WM_SELECT_CHAR, 0, (LPARAM)i);

				break;
			}
		}
	}
*/
	return 0;
}



/**
 * \brief 
 */
int CAutoLoginFilter::FilterSendPacket(CPacket& pkt, CFilterContext&)
{
	return 0;
}
