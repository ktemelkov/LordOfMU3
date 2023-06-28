#include "stdafx.h"
#include "WSPRedirect.h"
#include "ConnectionHandler.h"
#include "ModuleLoader.h"
#include "LoaderInterface.h"



/**  
 * \brief 
 */
CWSPRedirect* CWSPRedirect::s_pInstance = 0;


/**  
 * \brief 
 */
bool CWSPRedirect::OnDllInit()
{
	if (s_pInstance != 0)
		return false;

	s_pInstance = new CWSPRedirect();
	return true;
}


/**  
 * \brief 
 */
void CWSPRedirect::OnDllTerm()
{
	if (!s_pInstance)
		return;

	delete s_pInstance;
	s_pInstance = 0;
}


/**  
 * \brief 
 */
CSocketInfo::CSocketInfo(SOCKET s)
{
	sock = s;

	hWnd = 0;
	uMsg = 0;

	pConnHandler = new CConnectionHandler(s);
}


/**  
 * \brief 
 */
CSocketInfo::~CSocketInfo()
{
	delete pConnHandler;
}



/**  
 * \brief 
 */
CWSPRedirect::CWSPRedirect()
{
	m_iEntryCount = 0;
	InitializeCriticalSection(&m_cs);

	m_lpWSPAsyncSelect = 0;
	m_lpWSPCleanup = 0;
	m_lpWSPCloseSocket = 0;
	m_lpWSPConnect = 0;
	m_lpWSPRecv = 0;
	m_lpWSPSend = 0;
	m_lpWSPShutdown = 0;
	m_lpWSPSocket = 0;
	m_lpWSPGetPeerName = 0;

	m_lpSend = 0;
	m_lpRecv = 0;
	m_fIsMuWindowRegistered = false;
}



/**  
 * \brief 
 */
CWSPRedirect::~CWSPRedirect()
{
	DeleteCriticalSection(&m_cs);
}


/**  
 * \brief 
 */
CWSPRedirect* CWSPRedirect::GetInstance()
{
	if (!s_pInstance)
		OnDllInit();

	return s_pInstance;
}


/**  
 * \brief 
 */
CSocketInfo* CWSPRedirect::GetSockContext(SOCKET s)
{
	CSockContextMap::iterator it = m_vContexts.find(s);

	return (it != m_vContexts.end()) ? it->second : 0;
}


/**  
 * \brief 
 */
bool CWSPRedirect::AddSockContext(SOCKET s, CSocketInfo* pContext)
{
	return m_vContexts.insert(std::pair<SOCKET,CSocketInfo*>(s, pContext)).second;
}


/**  
 * \brief 
 */
bool CWSPRedirect::DelSockContext(SOCKET s)
{
	CSockContextMap::iterator it = m_vContexts.find(s);

	if (it != m_vContexts.end())
	{
		delete it->second;
		m_vContexts.erase(it);
	}

	return true;
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPStartup(WORD wVersion, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpCallTable, LPWSPPROC_TABLE lpProcTable)
{
	CWSPRedirect* pThis = GetInstance();

	EnterCriticalSection(&pThis->m_cs);

	if (0 == pThis->m_iEntryCount)
	{
		pThis->m_lpWSPAsyncSelect = lpProcTable->lpWSPAsyncSelect;
		pThis->m_lpWSPCleanup = lpProcTable->lpWSPCleanup;
		pThis->m_lpWSPCloseSocket = lpProcTable->lpWSPCloseSocket;
		pThis->m_lpWSPConnect = lpProcTable->lpWSPConnect;
		pThis->m_lpWSPRecv = lpProcTable->lpWSPRecv;
		pThis->m_lpWSPSend = lpProcTable->lpWSPSend;

		pThis->m_lpWSPGetPeerName = lpProcTable->lpWSPGetPeerName;

		lpProcTable->lpWSPAsyncSelect = CWSPRedirect::WSPAsyncSelect;
		lpProcTable->lpWSPCleanup = CWSPRedirect::WSPCleanup;
		lpProcTable->lpWSPCloseSocket = CWSPRedirect::WSPCloseSocket;
		lpProcTable->lpWSPConnect = CWSPRedirect::WSPConnect;
		lpProcTable->lpWSPRecv = CWSPRedirect::WSPRecv;
		lpProcTable->lpWSPSend = CWSPRedirect::WSPSend;
	}

	pThis->m_iEntryCount++;

	LeaveCriticalSection(&pThis->m_cs);
	return 0;
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPAsyncSelect(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent, LPINT lpErrno)
{
	if (!IsWindow(hWnd) || 0 != (lEvent & ~FD_ALL_EVENTS))
	{
		*lpErrno = WSAEINVAL;
		return SOCKET_ERROR;
	}


	CWSPRedirect* pThis = GetInstance();

	if (!gModuleLoader.GetInstallerService()->IsGameStarted() || hWnd != gModuleLoader.GetInstallerService()->GetGameWindow())
		return pThis->m_lpWSPAsyncSelect(s, hWnd, wMsg, lEvent, lpErrno);

	if (!pThis->m_fIsMuWindowRegistered)
	{
		pThis->m_fIsMuWindowRegistered = true;

		CModuleLoader::RegMuWindow(hWnd);
	}


	CSocketInfo* pContext = pThis->GetSockContext(s);

	if (pContext)
		pThis->DelSockContext(s);

	pThis->AddSockContext(s, (pContext = new CSocketInfo(s)));

	pContext->hWnd = hWnd;
	pContext->uMsg = wMsg;

	HWND hWorkerWindow = pContext->pConnHandler->GetWorkerWindow();

	if (!hWorkerWindow)
	{
		*lpErrno = WSAEINVAL;
		return SOCKET_ERROR;
	}
	
	gModuleLoader.GetInstallerService()->SetApiHook(_API_HOOK_WS2_SEND, (PVOID)&MySend, (PVOID*)&pThis->m_lpSend);
//	gModuleLoader.GetInstallerService()->SetApiHook(_API_HOOK_WS2_RECV, (PVOID)&MyRecv, (PVOID*)&pThis->m_lpRecv);

	return pThis->m_lpWSPAsyncSelect(s, hWorkerWindow, WM_SOCKET, lEvent, lpErrno);
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPConnect(SOCKET s, const struct sockaddr FAR* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, 
						LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno)
{
	CWSPRedirect* pThis = GetInstance();

	return pThis->m_lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, 
						LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	CWSPRedirect* pThis = GetInstance();
	CSocketInfo* pContext = pThis->GetSockContext(s);

	if (pThis->m_lpRecv || !pContext || !pContext->hWnd || lpOverlapped)
		return pThis->m_lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

	return pContext->pConnHandler->HandleRecv(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, 
					LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno)
{
	CWSPRedirect* pThis = GetInstance();
	CSocketInfo* pContext = pThis->GetSockContext(s);
	int res = 0;

//	gModuleLoader.GetInstallerService()->CheckWsHooks();

	if (pThis->m_lpSend || !pContext || !pContext->hWnd || lpOverlapped)
		res = pThis->m_lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	else
		res = pContext->pConnHandler->HandleSend(lpBuffers, dwBufferCount, lpNumberOfBytesSent);

	return res;
}


/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::MySend(SOCKET s, const char FAR* buf, int len, int flags)
{
	CWSPRedirect* pThis = GetInstance();
	CSocketInfo* pContext = pThis->GetSockContext(s);

	if (!pContext)
		return pThis->m_lpSend(s, buf, len, flags);

	WSABUF wsab;
	wsab.buf = (char*)buf;
	wsab.len = len;
	
	DWORD NumberOfBytesSent = 0;

	if (SOCKET_ERROR == pContext->pConnHandler->HandleSend(&wsab, 1, &NumberOfBytesSent))
		return SOCKET_ERROR;

	return (int)NumberOfBytesSent;
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::MyRecv(SOCKET s, char *buf, int len, int flags)
{
	CWSPRedirect* pThis = GetInstance();
	CSocketInfo* pContext = pThis->GetSockContext(s);

	if (!pContext)
		return pThis->m_lpRecv(s, buf, len, flags);


	WSABUF wsab;
	wsab.buf = (char*)buf;
	wsab.len = len;

	DWORD dwRecvd = 0;

	if (SOCKET_ERROR == pContext->pConnHandler->HandleRecv(&wsab, 1, &dwRecvd))
		return SOCKET_ERROR;

	return (int)dwRecvd;
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPCloseSocket(SOCKET s, LPINT lpErrno)
{
	CWSPRedirect* pThis = GetInstance();
	CSocketInfo* pContext = pThis->GetSockContext(s);

	if (pContext)
		pThis->DelSockContext(s);

	return pThis->m_lpWSPCloseSocket(s, lpErrno);
}



/**  
 * \brief 
 */
int WSPAPI CWSPRedirect::WSPCleanup(LPINT lpErrno)
{
	CWSPRedirect* pThis = GetInstance();

	EnterCriticalSection(&pThis->m_cs);

	//
	// Decrement the entry count
	//
	if (pThis->m_iEntryCount > 0)
	{
		pThis->m_iEntryCount--;

		if (0 == pThis->m_iEntryCount)
		{
			CSockContextMap::iterator it = pThis->m_vContexts.begin();

			for (it; it != pThis->m_vContexts.end(); ++it)
				delete it->second;

			pThis->m_vContexts.clear();

			gModuleLoader.GetInstallerService()->ClearApiHook(_API_HOOK_WS2_SEND);
			gModuleLoader.GetInstallerService()->ClearApiHook(_API_HOOK_WS2_RECV);

			CModuleLoader::Term();
		}
	}

	LeaveCriticalSection(&pThis->m_cs);

	return pThis->m_lpWSPCleanup(lpErrno);
}


/**  
 * \brief 
 */
int CWSPRedirect::CallSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent)
{
	if (m_lpSend)
	{
		int res = m_lpSend(s, lpBuffers->buf, lpBuffers->len, 0);

		if (res == SOCKET_ERROR)
		{
			if (lpNumberOfBytesSent != 0)
				*lpNumberOfBytesSent = 0;

			return SOCKET_ERROR;
		}

		if (lpNumberOfBytesSent != 0)
			*lpNumberOfBytesSent = (DWORD)res;

		return 0;
	}
	else if (m_lpWSPSend)
	{
		INT iErrno = 0;
		return m_lpWSPSend(s, lpBuffers, dwBufferCount, lpNumberOfBytesSent, 0, 0, 0, 0, &iErrno);
	}

	return SOCKET_ERROR;
}


/**  
 * \brief 
 */
int CWSPRedirect::CallRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd)
{
	if (m_lpRecv)
	{
		int recvd = m_lpRecv(s, lpBuffers->buf, lpBuffers->len, 0);

		if (recvd < 0)
			return recvd;

		if (lpNumberOfBytesRecvd)
			*lpNumberOfBytesRecvd = (DWORD)recvd;

		return 0;
	}

	DWORD dwFlags = 0;
	INT iErrno = 0;

	return m_lpWSPRecv(s, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd, &dwFlags, 0, 0, 0, &iErrno);
}

