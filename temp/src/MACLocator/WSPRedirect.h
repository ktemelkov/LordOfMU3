#pragma once

#include <WinSock2.h>
#include <Ws2spi.h>
#include <map>
class CConnectionHandler;


typedef int (__stdcall* sendPtr)(SOCKET s, const char FAR* buf, int len, int flags);
typedef int (__stdcall* WSASendPtr)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);


/**  
 * \brief 
 */
class CSocketInfo
{
public:
	CSocketInfo(SOCKET s);
	virtual ~CSocketInfo();

	SOCKET sock;      // lower provider socket handle
	HWND   hWnd;                // Window (if any) associated with socket
	UINT   uMsg;                // Message for socket events

	CConnectionHandler* pConnHandler;
};

typedef std::map<SOCKET,CSocketInfo*> CSockContextMap;


/**  
 * \brief 
 */
class CWSPRedirect
{
protected:
	CWSPRedirect();
	virtual ~CWSPRedirect();

public:
	static bool OnDllInit();
	static void OnDllTerm();

	static int WSPAPI WSPStartup(WORD wVersion, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpCallTable, LPWSPPROC_TABLE lpProcTable);

public:
	static CWSPRedirect* GetInstance();


protected:
	static int WSPAPI WSPAsyncSelect(SOCKET s, HWND hWnd, unsigned int wMsg, long lEvent, LPINT lpErrno);
	static int WSPAPI WSPConnect(SOCKET s, const struct sockaddr FAR* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, 
						  LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno);
	static int WSPAPI WSPRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, 
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno);
	static int WSPAPI WSPSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, 
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, LPWSATHREADID lpThreadId, LPINT lpErrno);
	static int WSPAPI WSPCloseSocket(SOCKET s, LPINT lpErrno);
	static int WSPAPI WSPCleanup(LPINT lpErrno);


	static int WSPAPI MySend(SOCKET s, const char FAR* buf, int len, int flags);

public:
	CSocketInfo* GetSockContext(SOCKET s);
	bool AddSockContext(SOCKET s, CSocketInfo* pContext);
	bool DelSockContext(SOCKET s);

	int CallSend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent);
	int CallRecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd);

protected:
	static CWSPRedirect* s_pInstance;

protected:
	LPWSPASYNCSELECT m_lpWSPAsyncSelect;
	LPWSPCLEANUP m_lpWSPCleanup;
	LPWSPCLOSESOCKET m_lpWSPCloseSocket;
	LPWSPCONNECT m_lpWSPConnect;
	LPWSPRECV m_lpWSPRecv;
	LPWSPSEND m_lpWSPSend;
	LPWSPSHUTDOWN m_lpWSPShutdown;
	LPWSPSOCKET m_lpWSPSocket;
	LPWSPGETPEERNAME m_lpWSPGetPeerName;

	sendPtr m_lpSend;

private:
	CSockContextMap m_vContexts;

private:
	CRITICAL_SECTION m_cs;
	int m_iEntryCount;

	bool m_fMuWindowRegistered;
};
