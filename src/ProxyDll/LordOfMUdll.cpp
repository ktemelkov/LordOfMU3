// LordOfMUdll.cpp : Implementation of DLL Exports.


#include "stdafx.h"
#include "DebugOut.h"
#include "..\_Shared\LoaderInterface.h"
#include "ProxyBuilder.h"
#include "LordOfMUdll.h"
#include "..\_Shared\CommonPackets.h"
#include "..\_Shared\PEUtil.h"


CProxyClickerModule gModule;
TCHAR g_szRoot[_MAX_PATH + 1] = {0};



/**  
 * \brief 
 */
CProxyClickerModule::CProxyClickerModule()
	: m_pLoader(0), m_dwCookie(0), m_pGameProxy(0)
{
}

/**  
 * \brief 
 */
CProxyClickerModule* CProxyClickerModule::GetInstance()
{
	return &gModule;
}


/**  
 * \brief 
 */
bool CProxyClickerModule::Init(LoaderInitStruct* pInit)
{
	if (!pInit)
		return false;

	m_dwCookie = pInit->dwCookie;
	int iInstNo = 1;

	if (pInit->ppMoudle)
		*pInit->ppMoudle = this;
	
	if (pInit->pLoader)
	{
		m_pLoader = pInit->pLoader;

		char szRoot[_MAX_PATH+1] = {0};
		pInit->pLoader->GetRootDir(szRoot, _MAX_PATH);

		_tcscpy_s(g_szRoot, _MAX_PATH, CA2CT(szRoot));

		HWND hwnd = 0;
		pInit->pLoader->SendCommand(_MODULE_LOADER_COMMAND_GET_MUWND, _MODULE_LOADER_TARGET_SELF, (void*)&hwnd, (void**)&iInstNo);
	}

	CDebugOut::Init(iInstNo);
	CDebugOut::PrintDebug("InternalInit() OK.");

	return true;
}


/**  
 * \brief 
 */
bool CProxyClickerModule::Term()
{
	// do clean-up here
	CDebugOut::CleanUp();

	m_pLoader = 0;
	m_dwCookie = 0;
	return true;
}


/**  
 * \brief 
 */
bool CProxyClickerModule::SendCommand(int iCommand, void* pParam, void** pResult)
{
	if (_CLICKER_MODULE_COMMAND_CREATE_PROXY == iCommand)
	{
		if (!pResult)
			return false;

		return 0 != (*pResult = (void*)(IConnectionProxy*)CProxyBuilder::CreateProxy((IConnectionHandler*)pParam));
	}
	else if (_CLICKER_MODULE_COMMAND_SEND_PACKET == iCommand)
	{
		if (!m_pGameProxy || !pParam)
			return false;

		m_pGameProxy->send_packet(*((CPacket*)pParam));
		return true;
	}
	else if (_CLICKER_MODULE_COMMAND_RECV_PACKET == iCommand)
	{
		if (!m_pGameProxy || !pParam)
			return false;

		m_pGameProxy->recv_packet(*((CPacket*)pParam));
		return true;
	}
	else if (_CLICKER_MODULE_COMMAND_SEND_COMMAND == iCommand)
	{
		if (!m_pGameProxy || !pParam)
			return false;

		CPacketFilter* pFilter = m_pGameProxy->GetFilter("CharInfoFilter");

		if (!pFilter)
			return false;

		char* pszCharName = 0;
		pFilter->GetParam("CharName", &pszCharName);

		CCharacterSayPacket pkt(pszCharName, (const char*)pParam);
		m_pGameProxy->send_packet(pkt);
		return true;
	}


	return false;
}



/**
 * \brief DLL Entry Point
 */
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInstance);

		gModule.Init((LoaderInitStruct*)lpReserved);

		PeUtil::WipeImportTable((HMODULE)hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		CDebugOut::PrintDebug("DLL_PROCESS_DETACH ... ");
		gModule.Term();
	}

	return TRUE; 
}

