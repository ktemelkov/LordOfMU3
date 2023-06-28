#include "stdafx.h"
//#include "lsp\lspdef.h"
#include "ConnectionHandler.h"
#include "ModuleLoader.h"
#include <stdlib.h>
#include <stdio.h>


/**  
 * \brief 
 */
CConnectionHandler::CConnectionHandler(SOCKET s) 
	: m_sock(s), m_hThread(0), m_hReadyEvent(0), m_hWnd(0), m_fPostRecvMsg(true)
{
	IClickerModule* pProxy = gModuleLoader.GetProxy();

	m_pProxy = 0;

	InitializeCriticalSection(&m_cs);
	if (pProxy)
		pProxy->SendCommand(_CLICKER_MODULE_COMMAND_CREATE_PROXY, (void*)this, (void**)&m_pProxy);
}


/**  
 * \brief 
 */
CConnectionHandler::~CConnectionHandler()
{
	if (m_pProxy)
		m_pProxy->Destroy();

	m_pProxy = 0;


	if (m_hThread != 0)
	{
		if (m_hWnd != 0)
			PostMessage(m_hWnd, WM_DESTROY, 0, 0);

		// Wait for the thread to cleanup and exit
		DWORD rc = WaitForSingleObject(m_hThread, 10000);

		if (WAIT_TIMEOUT == rc)
		{
			dbgprint("StopAsyncWindowManager: Timed out waiting for async thread!");
		}

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	DeleteCriticalSection(&m_cs);
}



/**  
 * \brief 
 */
HWND CConnectionHandler::GetWorkerWindow()
{
	if (0 == m_hThread) 
	{
		// Create an event which the worker thread will signal when its ready
		m_hReadyEvent = CreateEvent(0, TRUE, FALSE, 0);

		// Create the async window message thread
		m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AsyncMsgHandler, (LPVOID)this, 0, 0);

		if (0 == m_hThread || m_hThread == INVALID_HANDLE_VALUE)
		{
			dbgprint("CreateThread failed: %d", GetLastError());
		}
		else
		{
			// Wait for the window to become initialized
			DWORD rc = WaitForSingleObject(m_hReadyEvent, INFINITE);

			if ((WAIT_FAILED == rc) || (WAIT_TIMEOUT == rc))
			{
				dbgprint( "GetWorkerWindow: WaitForSingleObject failed: %d! (error = %d)", rc, GetLastError());
			}
		}

		CloseHandle(m_hReadyEvent);
		m_hReadyEvent = 0;
	}

	return m_hWnd;
}


/**  
 * \brief 
 */
LRESULT WINAPI CConnectionHandler::AsyncMsgHandler(CConnectionHandler* pThis)
{
	if (!pThis)
		return 0;

	char szAsyncProviderClassName[_MAX_PATH+1] = {0};
	sprintf_s(szAsyncProviderClassName, _MAX_PATH, "Layered WS2 Provider 0x%08x", pThis->m_sock);

	dbgprint("AsyncMsgHandler: Class name is '%s'", szAsyncProviderClassName);

	WNDCLASSA wndclass = {0};
	wndclass.lpfnWndProc = (WNDPROC)AsyncWndProc;
	wndclass.hInstance = gDllInstance;
	wndclass.lpszClassName = szAsyncProviderClassName;

	if (0 == RegisterClassA(&wndclass))
	{
		dbgprint("AsyncMsgHandle: RegisterClass failed: %d", GetLastError());
		SetEvent(pThis->m_hReadyEvent);
		return 0;
	}


	// Create a window.
	pThis->m_hWnd = CreateWindowA(szAsyncProviderClassName, 0, WS_OVERLAPPEDWINDOW, 
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, gDllInstance, NULL);

	SetWindowLongPtr(pThis->m_hWnd, GWL_USERDATA, (LONG_PTR)pThis);

	SetEvent(pThis->m_hReadyEvent);


	if (0 != pThis->m_hWnd)
	{

		while (1)
		{
			MSG msg = {0};
			int count = 0;

			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0 && count++ < 25)
			{
				if (msg.message == WM_QUIT)
					break;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (msg.message == WM_QUIT)
				break;

			if (pThis->m_pProxy)
			{
				if (pThis->m_fPostRecvMsg && pThis->m_pProxy->HasIngoingPackets())
					pThis->PostSocketAsyncMessage();

				if (pThis->m_pProxy->HasOutgoingPackets())
					pThis->ProcessSendQueue();

//				if (pThis->m_pProxy->ShouldCloseConnection())
//					pThis->PostCloseMessage();
			}

			Sleep(10);
		}


		// Clean up the window and window class which were created in this thread
		if (pThis->m_hWnd != 0)
		{
			DestroyWindow(pThis->m_hWnd);
			pThis->m_hWnd = 0;
		}
	}

	UnregisterClassA(szAsyncProviderClassName, gDllInstance);
	return 0;
}



/**  
 * \brief 
 */
LRESULT WINAPI CConnectionHandler::AsyncWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_SOCKET == uMsg)
	{
		CConnectionHandler* pThis = (CConnectionHandler*)GetWindowLongPtr(hWnd, GWL_USERDATA);

		if (!pThis)
			return 0;

		if (pThis->m_pProxy)
			pThis->ProcessRecvStream();
		else
			pThis->PostSocketAsyncMessage(lParam);
	}
	else if (WM_DESTROY == uMsg)
	{
		// Post a quit message to exit our async message pump thread
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/**  
 * \brief 
 */
int CConnectionHandler::HandleSend(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent)
{
	CWSPRedirect* pRedirector = CWSPRedirect::GetInstance();
	CSocketInfo* pSocketContext = pRedirector->GetSockContext(m_sock);

	if (!pSocketContext)
		return SOCKET_ERROR;


	int ret = SOCKET_ERROR;

	if (m_pProxy && (int)dwBufferCount != 0)
	{
		for (int i = 0; i < (int)dwBufferCount; i++)
		{
			if (m_pProxy->ProcessSendStream(lpBuffers[i].buf, lpBuffers[i].len))
				*lpNumberOfBytesSent += lpBuffers[i].len;
		}

		ret = ProcessSendQueue(pSocketContext);
	}
	else
	{
		// Make a blocking send call
		ret = pRedirector->CallSend(pSocketContext->sock, lpBuffers, dwBufferCount, lpNumberOfBytesSent);
	}

	return ret;
}


/**  
 * \brief 
 */
int CConnectionHandler::ProcessSendQueue(CSocketInfo* pSocketContext)
{
	CSocketInfo* pContext = pSocketContext;

	if (!pSocketContext)
		pContext = CWSPRedirect::GetInstance()->GetSockContext(m_sock);

	if (!pContext)
		return SOCKET_ERROR;


	bool fHasMore = true;
	int count = 0;
	int ret = SOCKET_ERROR;

	EnterCriticalSection(&m_cs);

	while (fHasMore && count++ < 5)
	{
		int len = BUFFER_SIZE;
		m_pProxy->GetSendBuffer(m_buffer, len, fHasMore);

		DWORD dwSent = 0;
		WSABUF wsabuf = {0};
		wsabuf.len = len;
		wsabuf.buf = m_buffer;

		ret = CWSPRedirect::GetInstance()->CallSend(pContext->sock, &wsabuf, 1, &dwSent);

		if (ret == SOCKET_ERROR)
			break;
	}

	LeaveCriticalSection(&m_cs);

	return ret;
}


/**  
 * \brief 
 */
int  CConnectionHandler::HandleRecv(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd)
{
	CSocketInfo* pSocketContext = CWSPRedirect::GetInstance()->GetSockContext(m_sock);
	int ret = 0;

	if (!pSocketContext)
		return SOCKET_ERROR;

	if (m_pProxy && dwBufferCount != 0)
	{		
		int len = (int)lpBuffers[0].len;
		char* buff = lpBuffers[0].buf; 
		bool fHasMore = false;

		if (m_pProxy->GetRecvBuffer(buff, len, fHasMore))
		{
			*lpNumberOfBytesRecvd = (DWORD)len;

			if (!fHasMore)
				m_fPostRecvMsg = true;
			else
				PostSocketAsyncMessage();
		}
	}
	else
	{		
		ret = CWSPRedirect::GetInstance()->CallRecv(pSocketContext->sock, lpBuffers, dwBufferCount, lpNumberOfBytesRecvd);
	}

	return ret;
}


/**  
 * \brief 
 */
void CConnectionHandler::PostSocketAsyncMessage(LPARAM lParam)
{
	INT iErrno = 0;
	CSocketInfo* pContext = CWSPRedirect::GetInstance()->GetSockContext(m_sock);

	if (!pContext)
		return;

	m_fPostRecvMsg = false;
	PostMessage(pContext->hWnd, pContext->uMsg, (WPARAM)m_sock, lParam);
}


/**  
 * \brief 
 */
void CConnectionHandler::ProcessRecvStream()
{
	INT iErrno = 0;
	CSocketInfo* pSocketContext = CWSPRedirect::GetInstance()->GetSockContext(m_sock);

	if (!pSocketContext)
		return;

	EnterCriticalSection(&m_cs);

	WSABUF wsabuf = {0};
	wsabuf.buf = m_buffer;
	wsabuf.len = BUFFER_SIZE;

	DWORD dwRecvd = 0;
	int ret = CWSPRedirect::GetInstance()->CallRecv(pSocketContext->sock, &wsabuf, 1, &dwRecvd);

	if (ret == 0)
		m_pProxy->ProcessRecvStream(m_buffer, (int)dwRecvd);
	
	LeaveCriticalSection(&m_cs);
}

