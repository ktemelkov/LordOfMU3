#pragma once


#include "..\_Shared\ConnectionInterface.h"
#include "WSPRedirect.h"


/**  
 * \brief 
 */
class CConnectionHandler : public IConnectionHandler
{
public:
	CConnectionHandler(SOCKET s);
	virtual ~CConnectionHandler();

	HWND GetWorkerWindow();
	int  HandleSend(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent);
	int  HandleRecv(LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd);

protected:
	static LRESULT WINAPI AsyncMsgHandler(CConnectionHandler* pThis);
	static LRESULT WINAPI AsyncWndProc(HWND,UINT,WPARAM,LPARAM);

	int ProcessSendQueue(CSocketInfo* pSocketContext = 0);
	
	void ProcessRecvStream();
	void PostSocketAsyncMessage(LPARAM lParam = FD_READ);


private:
	SOCKET m_sock;
	HANDLE m_hThread;
	HANDLE m_hReadyEvent;
	HWND m_hWnd;


	CRITICAL_SECTION m_cs;
	static const int BUFFER_SIZE = 4096;
	char m_buffer[4096];

	bool m_fPostRecvMsg;

	IConnectionProxy* m_pProxy;

private:
	static const int MAX_ASYNC_RETRIES = 7;
	static const int ASYNC_TIMEOUT = 500;
};
