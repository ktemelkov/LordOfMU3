/*
						CSLS - Controlled Special Layer Service
	This is a Layered Service Provider which gives access only to those ports that are 
	specified. All the other ports are blocked. This LSP is ment only for security and
	doesn't deal with any I/O models. The I/O requests are passed directly to the lower
	Service Provider.

*/

#include <fstream.h>
#include <ws2spi.h>
#include <mswsock.h>
#include <stdio.h>

bool IsAllow(SOCKET s,const sockaddr *name, int func);
bool IsInAllowRange(long port);
void GetPorts();

HINSTANCE dllInstance=NULL;
WSPUPCALLTABLE MainUpCallTable;
CRITICAL_SECTION CS1;

long gPorts [100][2];
short N;

union SockAddr_In
{
	sockaddr_in ws2;
	sockaddr ws1;
};

DWORD gLayerCatId = 0;
DWORD gChainId = 0;
DWORD gEntryCount = 0;
LPWSPDATA gWSPData = NULL;
WSPPROC_TABLE NextProcTable;
LPWSPPROC_TABLE gProcTable = NULL;
LPWSAPROTOCOL_INFOW gBaseInfo = NULL;
HINSTANCE HDllInstance = NULL;
HINSTANCE hProvider = NULL;
INT gLayerCount=0;                    // Number of base providers we're layered over

ofstream outfile("c:\\lsp.log",ios::trunc);
ifstream infile("c:\\instlsp.cfg",ios::binary);
char dbgstr[255];

BOOL WINAPI DllMain(HINSTANCE hInstDll,DWORD dwReason, LPVOID lpvReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		dllInstance=hInstDll;
		InitializeCriticalSection(&CS1);
		GetPorts();
	break;
	case DLL_THREAD_ATTACH:
	break;
	case DLL_THREAD_DETACH:
	break;
	case DLL_PROCESS_DETACH:
	break;
	}

	return TRUE;
}



GUID ProviderGuid = {  // {CC051FC0-74A6-11d7-8ADE-008048134FE4}
	0xcc051fc0, 
	0x74a6, 
	0x11d7, 
	{ 0x8a, 0xde, 0x0, 0x80, 0x48, 0x13, 0x4f, 0xe4 } 
};

/*{ //c5fabbd0-9736-11d1-937f-00c04fad860d
	0xc5fabbd0,
	0x9736,
	0x11d1,
	{0x93, 0x7f, 0x00, 0xc0, 0x4f, 0xad, 0x86, 0x0d}
};*/

GUID ProviderChainGuid = {// {3674BE61-74A7-11d7-8ADE-008048134FE4}
	0x3674be61, 
	0x74a7, 
	0x11d7, 
	{ 0x8a, 0xde, 0x0, 0x80, 0x48, 0x13, 0x4f, 0xe4 } 
};

	
/*	//f9065320-9e90-11d1-9381-00c04fad860d
	0xf9065320,
	0x9e90,
	0x11d1,
	{0x93, 0x81, 0x00, 0xc0, 0x4f, 0xad, 0x86, 0x0d}
};*/

LPWSAPROTOCOL_INFOW GetProviders(LPINT TotalProtocols)
{
	INT ErrorCode;

	LPWSAPROTOCOL_INFOW ProtocolInfo = NULL;
	DWORD ProtocolInfoSize = 0;
	*TotalProtocols = 0;

	// Find out how many entries we need to enumerate
	if (WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode) == SOCKET_ERROR)
	{
		if (ErrorCode != WSAENOBUFS)
		{
			return(NULL);
		}
	}

	if ((ProtocolInfo = (LPWSAPROTOCOL_INFOW) GlobalAlloc(GPTR, ProtocolInfoSize)) == NULL)
	{
		return(NULL);
	}

	if ((*TotalProtocols = WSCEnumProtocols(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode)) == SOCKET_ERROR)
	{

		return(NULL);
	}

	return(ProtocolInfo);
}

void FreeProviders(LPWSAPROTOCOL_INFOW ProtocolInfo)
{
	GlobalFree(ProtocolInfo); 
}


SOCKET WSPAPI WSPAccept (
    SOCKET s,                      
    struct sockaddr FAR * addr,  
    LPINT addrlen,                 
    LPCONDITIONPROC lpfnCondition,  
    DWORD dwCallbackData,          
    LPINT lpErrno)
{

	SOCKET NewProviderSocket;
	SOCKET NewSocket;
	SockAddr_In Addr;
	int addlen=sizeof Addr;

	outfile<<" In Accept ";

	NewProviderSocket = NextProcTable.lpWSPAccept(s, addr, addrlen, lpfnCondition, dwCallbackData, lpErrno);

	if(NewProviderSocket != INVALID_SOCKET)
		if((NewSocket = MainUpCallTable.lpWPUModifyIFSHandle(gChainId,NewProviderSocket,lpErrno))!=INVALID_SOCKET)
		{
			IsAllow(NewSocket,NULL,2);
			/*if(!IsAllow(NewSocket,NULL))
			{
				*lpErrno=WSAEACCES;
				closesocket(NewSocket);
				return SOCKET_ERROR;
			}*/

			return NewSocket;
		}
	return INVALID_SOCKET;

#ifdef COMMENT
    SOCKET NewProviderSocket;
    SOCKET NewSocket;
    SOCK_INFO *NewSocketContext;

    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return INVALID_SOCKET;

    NewProviderSocket = NextProcTable.lpWSPAccept(SocketContext->ProviderSocket, addr, addrlen,
                                                    lpfnCondition, dwCallbackData, lpErrno);
    
    
    if (NewProviderSocket != INVALID_SOCKET)
    {
        if ((NewSocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        NewSocketContext->ProviderSocket = NewProviderSocket;
        NewSocketContext->bClosing  = FALSE;
        NewSocketContext->dwOutstandingAsync = 0;
        NewSocketContext->BytesRecv = 0;
        NewSocketContext->BytesSent = 0;

        if ((NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) NewSocketContext, lpErrno)) != INVALID_SOCKET)
            DuplicateAsyncSocket(SocketContext->ProviderSocket, NewProviderSocket, NewSocket);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            //OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;

#endif
}

int WSPAPI WSPAddressToString(
    LPSOCKADDR lpsaAddress,            
    DWORD dwAddressLength,               
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPWSTR lpszAddressString,            
    LPDWORD lpdwAddressStringLength,   
    LPINT lpErrno)
{
	outfile<<" In Addrtostring ";
    return NextProcTable.lpWSPAddressToString(lpsaAddress, dwAddressLength,               
        &gBaseInfo[0], lpszAddressString, lpdwAddressStringLength, lpErrno);
}

int WSPAPI WSPAsyncSelect (
    SOCKET s,              
    HWND hWnd,           
    unsigned int wMsg,     
    long lEvent,           
    LPINT lpErrno)
{
#ifdef COMMENT
    SOCK_INFO *SocketContext;
    HWND hWorkerWindow;


    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s,(LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if ((hWorkerWindow = SetWorkerWindow(SocketContext->ProviderSocket, s, hWnd, wMsg)) == NULL)
        return SOCKET_ERROR;
#endif
	outfile<<" In Asyncselect ";
    return NextProcTable.lpWSPAsyncSelect(s, hWnd, wMsg, lEvent, lpErrno);
}

int WSPAPI WSPBind(
    SOCKET s,                           
    const struct sockaddr FAR * name,
    int namelen,                        
    LPINT lpErrno)
{

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;
#endif
	
	outfile<<" In Bind ";
	int ret;


	IsAllow(-1,name,1);
	/*if(!IsAllow(-1,name))
	{
		*lpErrno=WSAEACCES;
		return SOCKET_ERROR;
	}*/
	
	//outfile<<"In wspbind: "<<Addr.ws2.sin_port;

    ret = NextProcTable.lpWSPBind(s, name, namelen, lpErrno);


	/*SockAddr_In add,add2;
	int addlen=sizeof (add);

	if(getsockname(s,&add.ws1,&addlen)!=-1)
	{
		if(getpeername(s,&add2.ws1,&addlen)==-1)
		{
			sprintf(dbgstr,"Error: %d ",GetLastError());
			MessageBox(NULL,dbgstr,"Lsp",MB_OK);
		}
		
		sprintf(dbgstr,"Current:\n Addr: %s \nPort: %d \n\nPeer: \nAddr: %s \nPort: %d \n\n",
			inet_ntoa(add.ws2.sin_addr), add.ws2.sin_port, inet_ntoa(add2.ws2.sin_addr), add2.ws2.sin_port);
		MessageBox(NULL,dbgstr,"Bind",MB_OK);
	}
	else
	{
		sprintf(dbgstr,"Error: %d ",GetLastError());
		MessageBox(NULL,dbgstr,"Lsp",MB_OK);
	}*/


	return ret;
}

int WSPAPI WSPCancelBlockingCall(
    LPINT lpErrno)
{
	outfile<<" In Cancelblockingcall ";
    return NextProcTable.lpWSPCancelBlockingCall(lpErrno);
}

int WSPAPI WSPCleanup (
    LPINT lpErrno  
    )
{
    int Ret;

	outfile<<" In cleanup ";

    if (!gEntryCount)
    {
        *lpErrno = WSANOTINITIALISED;
        return SOCKET_ERROR;
    }

    Ret = NextProcTable.lpWSPCleanup(lpErrno);

    EnterCriticalSection(&CS1);

    gEntryCount--;

    if (gEntryCount == 0)
    {
        FreeLibrary(hProvider);
        hProvider = NULL;
    }

    LeaveCriticalSection(&CS1);

    return Ret;
}

int WSPAPI WSPCloseSocket (  
    SOCKET s,        
    LPINT lpErrno
)
{

	outfile<<" In closesocket ";

	if (NextProcTable.lpWSPCloseSocket(s, lpErrno) == SOCKET_ERROR) 
		return SOCKET_ERROR;

    return 0;

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if (SocketContext->dwOutstandingAsync != 0)
    {
        SocketContext->bClosing = TRUE;

        if (NextProcTable.lpWSPCloseSocket(SocketContext->ProviderSocket, lpErrno) == SOCKET_ERROR) 
            return SOCKET_ERROR;

        return 0;
    }

    if (NextProcTable.lpWSPCloseSocket(SocketContext->ProviderSocket, lpErrno) == SOCKET_ERROR) 
        return SOCKET_ERROR;

    RemoveSockInfo(SocketContext->ProviderSocket);

    if (MainUpCallTable.lpWPUCloseSocketHandle(s, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    {
        TCHAR buffer[128];
        wsprintf(buffer, L"Closing socket %d Bytes Sent [%lu] Bytes Recv [%lu]\n", s,
            SocketContext->BytesSent, SocketContext->BytesRecv);
        OutputDebugString(buffer);
    }

    GlobalFree(SocketContext);

    return 0;
#endif
}

int WSPAPI WSPConnect (
    SOCKET s,                           
    const struct sockaddr FAR * name,
    int namelen,                        
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,              
    LPQOS lpSQOS,
    LPQOS lpGQOS,                       
    LPINT lpErrno
)
{
	INT ret;

	outfile<<" In connect ";

	IsAllow(-1,name,3);

	/*if(!IsAllow(-1,name))
	{
		//IsAllow(-1,name);
		*lpErrno=WSAEACCES;
		return SOCKET_ERROR;
	}*/
	
	ret =  NextProcTable.lpWSPConnect(s, name, namelen, lpCallerData, lpCalleeData,
        lpSQOS, lpGQOS, lpErrno);

	return ret;

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    INT ret;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
    {
        return SOCKET_ERROR;
    }

    ret =  NextProcTable.lpWSPConnect(SocketContext->ProviderSocket, name, namelen, lpCallerData, lpCalleeData,
        lpSQOS, lpGQOS, lpErrno);

    return ret;
#endif
}

int WSPAPI WSPDuplicateSocket(
    SOCKET s,                             
    DWORD dwProcessId,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPINT lpErrno)
{
	outfile<<" In DuplicateSocket ";
	return NextProcTable.lpWSPDuplicateSocket(s, dwProcessId, lpProtocolInfo, lpErrno);
#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPDuplicateSocket(SocketContext->ProviderSocket,                             
        dwProcessId, lpProtocolInfo, lpErrno);
#endif
}

int WSPAPI WSPEnumNetworkEvents(  
    SOCKET s,                             
    WSAEVENT hEventObject,                  
    LPWSANETWORKEVENTS lpNetworkEvents,   
    LPINT lpErrno)
{
	outfile<<" In enumnetwork ";
	return NextProcTable.lpWSPEnumNetworkEvents(s, hEventObject, lpNetworkEvents, lpErrno);
#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPEnumNetworkEvents(SocketContext->ProviderSocket,                             
        hEventObject, lpNetworkEvents, lpErrno);
#endif
}

int WSPAPI WSPEventSelect(
    SOCKET s,                
    WSAEVENT hEventObject,   
    long lNetworkEvents,     
    LPINT lpErrno)
{
	outfile<<" In eventselect ";
	return NextProcTable.lpWSPEventSelect(s, hEventObject, lNetworkEvents, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;
    
    return NextProcTable.lpWSPEventSelect(SocketContext->ProviderSocket, hEventObject,
        lNetworkEvents, lpErrno);
#endif
}

BOOL WSPAPI WSPGetOverlappedResult (
    SOCKET s,
    LPWSAOVERLAPPED lpOverlapped,
    LPDWORD lpcbTransfer,
    BOOL fWait,
    LPDWORD lpdwFlags,
    LPINT lpErrno)
{
    DWORD Ret;
	outfile<<" In getoverlappedres ";

    if (lpOverlapped->Internal!=WSS_OPERATION_IN_PROGRESS) 
    {
        *lpcbTransfer = lpOverlapped->InternalHigh;
        *lpdwFlags = lpOverlapped->Offset;
        *lpErrno = lpOverlapped->OffsetHigh;

        return(lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
    }
    else
        if (fWait) 
        {
            Ret = WaitForSingleObject(lpOverlapped->hEvent, INFINITE);
            if ((Ret == WAIT_OBJECT_0)
                && (lpOverlapped->Internal != WSS_OPERATION_IN_PROGRESS))
            {
                *lpcbTransfer = lpOverlapped->InternalHigh;
                *lpdwFlags = lpOverlapped->Offset;
                *lpErrno = lpOverlapped->OffsetHigh;
                    
                return(lpOverlapped->OffsetHigh == 0 ? TRUE : FALSE);
            }
            else 
                *lpErrno = WSASYSCALLFAILURE;
        }
        else 
            *lpErrno = WSA_IO_INCOMPLETE;

    return FALSE;
}

int WSPAPI WSPGetPeerName(  
    SOCKET s,                     
    struct sockaddr FAR * name,     
    LPINT namelen,                
    LPINT lpErrno)
{
	outfile<<" In getpeername ";
	return NextProcTable.lpWSPGetPeerName(s, name, namelen, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetPeerName(SocketContext->ProviderSocket, name,
        namelen, lpErrno);
#endif
}

int WSPAPI WSPGetSockName(
    SOCKET s,                     
    struct sockaddr FAR * name,
    LPINT namelen,                
    LPINT lpErrno)
{
	outfile<<" In getsockname ";
	return NextProcTable.lpWSPGetSockName(s, name, namelen, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetSockName(SocketContext->ProviderSocket, name,
        namelen, lpErrno);
#endif
}

int WSPAPI WSPGetSockOpt(
    SOCKET s,              
    int level,           
    int optname,           
    char FAR * optval,     
    LPINT optlen,        
    LPINT lpErrno)
{
	outfile<<" In getsockopt ";
	return NextProcTable.lpWSPGetSockOpt(s, level, optname, optval, optlen, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetSockOpt(SocketContext->ProviderSocket, level,           
        optname, optval, optlen, lpErrno);
#endif
}

BOOL WSPAPI WSPGetQOSByName(
    SOCKET s,               
    LPWSABUF lpQOSName,   
    LPQOS lpQOS,            
    LPINT lpErrno)
{
	outfile<<" In getqosbyname ";
	return NextProcTable.lpWSPGetQOSByName(s, lpQOSName, lpQOS, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPGetQOSByName(SocketContext->ProviderSocket, lpQOSName,
        lpQOS, lpErrno);
#endif
}


int WSPAPI WSPIoctl(
    SOCKET s,
    DWORD dwIoControlCode,
    LPVOID lpvInBuffer,
    DWORD cbInBuffer,
    LPVOID lpvOutBuffer,
    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	int Ret;

	outfile<<" In ioctl ";

    if (lpOverlapped)
    {
        Ret = NextProcTable.lpWSPIoctl(s, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, NULL, NULL, lpErrno);
    }
    else
    {
        Ret = NextProcTable.lpWSPIoctl(s, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }

    return Ret;

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    GUID AcceptExGuid = WSAID_ACCEPTEX;
    GUID TransmitFileGuid = WSAID_TRANSMITFILE;
    GUID GetAcceptExSockAddrsGuid = WSAID_GETACCEPTEXSOCKADDRS;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext,
        lpErrno) == SOCKET_ERROR)
    {
//        MessageBox(NULL, L"wham", L"wham", MB_OK);
        return SOCKET_ERROR;
    }

    if (dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER)
    {
        if (memcmp (lpvInBuffer, &TransmitFileGuid, sizeof (GUID)) == 0)
        {
            *((LPFN_TRANSMITFILE *)lpvOutBuffer) = ExtTransmitFile;
            *lpErrno = 0;
            return 0;
        }
        else
            if (memcmp(lpvInBuffer, &AcceptExGuid, sizeof(GUID)) == 0)
            {
                *((LPFN_ACCEPTEX *)lpvOutBuffer) = ExtAcceptEx;
                *lpErrno = 0;
                return 0;
            }
            else 
                if (memcmp (lpvInBuffer, &GetAcceptExSockAddrsGuid, sizeof (GUID)) == 0)
                {
                    // No socket handle translation needed, let the call pass through below
                }
                else 
                {
                    *lpErrno = WSAEOPNOTSUPP;
                    return SOCKET_ERROR;
                }
    }

    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, NULL);

        Ret = NextProcTable.lpWSPIoctl(SocketContext->ProviderSocket, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, ProviderOverlapped, NULL, NULL, lpErrno);
    }
    else
    {
        Ret = NextProcTable.lpWSPIoctl(SocketContext->ProviderSocket, dwIoControlCode, lpvInBuffer,
            cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
    }

	//outfile<<Ret;

    return Ret;
#endif
}

SOCKET WSPAPI WSPJoinLeaf(
    SOCKET s,                           
    const struct sockaddr FAR * name,     
    int namelen,                        
    LPWSABUF lpCallerData,                
    LPWSABUF lpCalleeData,              
    LPQOS lpSQOS,                         
    LPQOS lpGQOS,                       
    DWORD dwFlags,                        
    LPINT lpErrno)
{

    SOCKET NextProviderSocket;
    SOCKET NewSocket;

	outfile<<" In joinleaf ";

    NextProviderSocket = NextProcTable.lpWSPJoinLeaf(s,                           
        name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, dwFlags,                        
        lpErrno);
        
    if (NextProviderSocket != INVALID_SOCKET)
    {

        NewSocket = MainUpCallTable.lpWPUModifyIFSHandle(gChainId,NextProviderSocket,lpErrno);

        return NewSocket;
    }

    return INVALID_SOCKET;

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    SOCKET NextProviderSocket;
    SOCKET NewSocket;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    NextProviderSocket = NextProcTable.lpWSPJoinLeaf(SocketContext->ProviderSocket,                           
        name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, dwFlags,                        
        lpErrno);
        
    if (NextProviderSocket != INVALID_SOCKET)
    {
        if ((SocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        SocketContext->ProviderSocket = NextProviderSocket;
        SocketContext->bClosing  = FALSE;
        SocketContext->dwOutstandingAsync = 0;
        SocketContext->BytesRecv = 0;
        SocketContext->BytesSent = 0;

        NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) SocketContext, lpErrno);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;
#endif
}

int WSPAPI WSPListen(
    SOCKET s,        
    int backlog,     
    LPINT lpErrno)
{

	outfile<<" In listen ";
	return NextProcTable.lpWSPListen (s, backlog, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPListen (SocketContext->ProviderSocket, backlog, lpErrno);
#endif
}

int WSPAPI WSPRecv(
    SOCKET s,                                                 
    LPWSABUF lpBuffers,                                       
    DWORD dwBufferCount,                                      
    LPDWORD lpNumberOfBytesRecvd,                             
    LPDWORD lpFlags,                                          
    LPWSAOVERLAPPED lpOverlapped,                             
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
    LPWSATHREADID lpThreadId,                                 
    LPINT lpErrno)
{
	int Ret;

	outfile<<" In recv ";
		
	Ret = NextProcTable.lpWSPRecv(s, lpBuffers, dwBufferCount,
           lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId,
            lpErrno);
	
    return Ret;

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

	outfile<<"1"<<endl;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

	outfile<<"2"<<endl;


    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesRecv);

        Ret = NextProcTable.lpWSPRecv(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, ProviderOverlapped, NULL, NULL, lpErrno);
		outfile<<"3 "<<*lpErrno<<endl;

    }
    else
    {
        // Process ordinary blocking call

        Ret = NextProcTable.lpWSPRecv(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpOverlapped, lpCompletionRoutine, lpThreadId,
            lpErrno);
		outfile<<"4"<<endl;

        if (Ret != SOCKET_ERROR)
        {
			outfile<<"5"<<endl;
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
		//Ret=SOCKET_ERROR;
    }
	outfile<<"6"<<endl;

    return Ret;
#endif
}

int WSPAPI WSPRecvDisconnect(
    SOCKET s,                           
    LPWSABUF lpInboundDisconnectData,     
    LPINT lpErrno)
{

	outfile<<" In recvdisconnect ";

	return NextProcTable.lpWSPRecvDisconnect(s, lpInboundDisconnectData, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPRecvDisconnect(SocketContext->ProviderSocket,                           
        lpInboundDisconnectData, lpErrno);
#endif
}

int WSPAPI WSPRecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
    int Ret;

	outfile<<" In recvfrom ";

	Ret = NextProcTable.lpWSPRecvFrom(s, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

    return Ret;

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesRecv);

        if (ProviderOverlapped == NULL)
        {
            OutputDebugString(L"WSPRecvFrom got a NULL overlapp structure\n");
        }

        Ret = NextProcTable.lpWSPRecvFrom(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        Ret = NextProcTable.lpWSPRecvFrom(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesRecvd, lpFlags, lpFrom, lpFromlen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesRecv += *lpNumberOfBytesRecvd;
        }
    }

    return Ret;
#endif
}

int WSPAPI WSPSelect(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR * exceptfds,
    const struct timeval FAR * timeout,
    LPINT lpErrno)
{

	outfile<<" In select ";

	return NextProcTable.lpWSPSelect(nfds, readfds, writefds, exceptfds, timeout, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    u_int i;
    u_int count;
    int Ret;
    int HandleCount;

    // Convert handles
    struct
    {
        SOCKET ClientSocket;
        SOCKET ProvSocket;

    } Read[FD_SETSIZE], Write[FD_SETSIZE], Except[FD_SETSIZE];

    fd_set ReadFds, WriteFds, ExceptFds;

    if (readfds)
    {
        FD_ZERO(&ReadFds);

        for (i = 0; i < readfds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Read[i].ClientSocket = readfds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Read[i].ProvSocket = SocketContext->ProviderSocket), &ReadFds);
        }
    }

    if (writefds)
    {
        FD_ZERO(&WriteFds);

        for (i = 0; i < writefds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Write[i].ClientSocket = writefds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Write[i].ProvSocket = SocketContext->ProviderSocket), &WriteFds);
        }
    }

    if (exceptfds)
    {
        FD_ZERO(&ExceptFds);

        for (i = 0; i < exceptfds->fd_count; i++)
        {
            if (MainUpCallTable.lpWPUQuerySocketHandleContext(
                (Except[i].ClientSocket = exceptfds->fd_array[i]), 
                (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
                return SOCKET_ERROR;
            FD_SET((Except[i].ProvSocket = SocketContext->ProviderSocket), &ExceptFds);
        }
    }

    Ret = NextProcTable.lpWSPSelect(nfds, 
        (readfds ? &ReadFds : NULL), (writefds ? &WriteFds : NULL), 
        (exceptfds ? &ExceptFds : NULL), timeout, lpErrno);

    if (Ret != SOCKET_ERROR)
    {
        HandleCount = Ret;

        if (readfds)
        {
            count = readfds->fd_count;
            FD_ZERO(readfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Read[i].ProvSocket, &ReadFds))
                {
                    FD_SET(Read[i].ClientSocket, readfds);
                    HandleCount--;
                }
            }
        }

        if (writefds)
        {
            count = writefds->fd_count;
            FD_ZERO(writefds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Write[i].ProvSocket, &WriteFds))
                {
                    FD_SET(Write[i].ClientSocket, writefds);
                    HandleCount--;
                }
            }
        }

        if (exceptfds)
        {
            count = exceptfds->fd_count;
            FD_ZERO(exceptfds);

            for(i = 0; (i < count) && HandleCount; i++)
            {
                if (MainUpCallTable.lpWPUFDIsSet(Except[i].ProvSocket, &ExceptFds))
                {
                    FD_SET(Except[i].ClientSocket, exceptfds);
                    HandleCount--;
                }
            }
        }
    }

    return Ret;

#endif
}

int WSPAPI WSPSend (
    SOCKET s,                                                 
    LPWSABUF lpBuffers,                                       
    DWORD dwBufferCount,                                      
    LPDWORD lpNumberOfBytesSent,                              
    DWORD dwFlags,                                            
    LPWSAOVERLAPPED lpOverlapped,                             
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,   
    LPWSATHREADID lpThreadId,                                 
    LPINT lpErrno                                             
    )
{
	outfile<<"In wspsend ";

	return NextProcTable.lpWSPSend(s, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

#ifdef COMMENT
    INT Ret;
    SOCK_INFO *SocketContext;
    LPWSAOVERLAPPED ProviderOverlapped;

	if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    // Check for overlapped I/O
    
    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesSent);
		
        Ret = NextProcTable.lpWSPSend(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, ProviderOverlapped, NULL, NULL, lpErrno);
		
    }
    else
    {
        Ret = NextProcTable.lpWSPSend(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);
	    if (Ret != SOCKET_ERROR)
        {
	        SocketContext->BytesSent += *lpNumberOfBytesSent;
        }
    }

	return Ret;
#endif
}

int WSPAPI WSPSendDisconnect(
    SOCKET s,                            
    LPWSABUF lpOutboundDisconnectData,
    LPINT lpErrno)
{

	outfile<<" In senddisconnect ";
	return NextProcTable.lpWSPSendDisconnect(s, lpOutboundDisconnectData, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPSendDisconnect(SocketContext->ProviderSocket,
        lpOutboundDisconnectData, lpErrno);
#endif
}

int WSPAPI WSPSendTo(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
    LPWSATHREADID lpThreadId,
    LPINT lpErrno)
{
	outfile<<" In sendto ";

	return NextProcTable.lpWSPSendTo(s, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;
    int Ret;
    LPWSAOVERLAPPED ProviderOverlapped;

    // Check for overlapped I/O
    
    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    if (lpOverlapped)
    {
        ProviderOverlapped = GetOverlappedStructure(s, SocketContext->ProviderSocket, lpOverlapped, lpCompletionRoutine,
            lpThreadId, &SocketContext->BytesSent);

        Ret = NextProcTable.lpWSPSendTo(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpTo, iTolen, ProviderOverlapped, NULL, NULL, lpErrno);

    }
    else
    {
        Ret = NextProcTable.lpWSPSendTo(SocketContext->ProviderSocket, lpBuffers, dwBufferCount,
            lpNumberOfBytesSent, dwFlags, lpTo, iTolen, lpOverlapped, lpCompletionRoutine, lpThreadId, lpErrno);

        if (Ret != SOCKET_ERROR)
        {
            SocketContext->BytesSent += *lpNumberOfBytesSent;
        }

    }

    return Ret;
#endif
}

int WSPAPI WSPSetSockOpt(
    SOCKET s,
    int level,                 
    int optname,                 
    const char FAR * optval,   
    int optlen,                  
    LPINT lpErrno)
{
	
	outfile<<" In setsockopt ";
	return NextProcTable.lpWSPSetSockOpt(s, level, optname, optval, optlen, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPSetSockOpt(SocketContext->ProviderSocket, level,                 
        optname, optval, optlen, lpErrno);
#endif
}

int WSPAPI WSPShutdown (
    SOCKET s,        
    int how,         
    LPINT lpErrno)
{
	
	outfile<<" In shutdown ";
	return NextProcTable.lpWSPShutdown(s, how, lpErrno);

#ifdef COMMENT
    SOCK_INFO *SocketContext;

    if (MainUpCallTable.lpWPUQuerySocketHandleContext(s, (LPDWORD) &SocketContext, lpErrno) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return NextProcTable.lpWSPShutdown(SocketContext->ProviderSocket, how, lpErrno);
#endif
}


int WSPAPI WSPStringToAddress(
    LPWSTR AddressString,                 
    INT AddressFamily,                      
    LPWSAPROTOCOL_INFOW lpProtocolInfo,   
    LPSOCKADDR lpAddress,                   
    LPINT lpAddressLength,                
    LPINT lpErrno)
{
	outfile<<" In stringtoaddress ";
    return NextProcTable.lpWSPStringToAddress (AddressString, AddressFamily,
        &gBaseInfo[0], lpAddress, lpAddressLength, lpErrno);
}


SOCKET WSPAPI WSPSocket(
    int af,                               
    int type,
    int protocol,                         
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    GROUP g,                              
    DWORD dwFlags,
    LPINT lpErrno                         
    )
{

	SOCKET NextProviderSocket;
    SOCKET NewSocket;

	outfile<<" In socket ";

	NextProviderSocket = NextProcTable.lpWSPSocket(af, type, protocol, lpProtocolInfo,
                                g, dwFlags, lpErrno);

	if(NextProviderSocket != INVALID_SOCKET)
	{
		NewSocket = MainUpCallTable.lpWPUModifyIFSHandle(gChainId,NextProviderSocket,lpErrno);

		return NewSocket;
	}

	return INVALID_SOCKET;

#ifdef COMMENT
    SOCKET NextProviderSocket;
    SOCKET NewSocket;
    SOCK_INFO *SocketContext;
    LPWSAPROTOCOL_INFOW pInfo=NULL;
    INT        iProtocol, iSockType, i;

    iProtocol = (!lpProtocolInfo ? lpProtocolInfo->iProtocol   : protocol);
    iSockType = (!lpProtocolInfo ? lpProtocolInfo->iSocketType : type);
    for(i=0; i < gLayerCount ;i++)
    {
        if ((gBaseInfo[i].iSocketType == iSockType) && 
            (gBaseInfo[i].iProtocol   == iProtocol))
        {
            pInfo = &gBaseInfo[i];
            break;
        }
    }

    NextProviderSocket = NextProcTable.lpWSPSocket(af, type, protocol, (pInfo ? pInfo : lpProtocolInfo),
                                g, dwFlags, lpErrno);

    if (NextProviderSocket != INVALID_SOCKET)
    {
        if ((SocketContext = (SOCK_INFO *) GlobalAlloc(GPTR, sizeof SOCK_INFO)) == NULL)
        {
            *lpErrno = WSAENOBUFS;
            return INVALID_SOCKET;
        }
        SocketContext->ProviderSocket = NextProviderSocket;
        SocketContext->bClosing  = FALSE;
        SocketContext->dwOutstandingAsync = 0;
        SocketContext->BytesRecv = 0;
        SocketContext->BytesSent = 0;

        NewSocket = MainUpCallTable.lpWPUCreateSocketHandle(gChainId, (DWORD) SocketContext, lpErrno);

        {
            TCHAR buffer[128];
            wsprintf(buffer, L"Creating socket %d\n", NewSocket);
            OutputDebugString(buffer);
        }

        return NewSocket;
    }

    return INVALID_SOCKET;

#endif
}

int WSPAPI WSPStartup(
    WORD wVersion,
    LPWSPDATA lpWSPData,
    LPWSAPROTOCOL_INFOW lpProtocolInfo,
    WSPUPCALLTABLE UpCallTable,
    LPWSPPROC_TABLE lpProcTable)
{

    INT      ReturnCode = 0;
    WCHAR    ProviderPath[MAX_PATH];
    INT      ProviderPathLen = MAX_PATH;
    //WCHAR    LibraryPath[MAX_PATH];
    INT      i, j, x, y, z;
    INT      TotalProtocols, idx;
    INT      Error;
    DWORD    NextProviderCatId;
    UINT     iBaseId;

    LPWSAPROTOCOL_INFOW ProtocolInfo;
    LPWSAPROTOCOL_INFOW ProtoInfo = lpProtocolInfo;
    LPWSPSTARTUP    WSPStartupFunc = NULL;

	char providerpath[MAX_PATH], librarypath[MAX_PATH];

    EnterCriticalSection(&CS1);

    MainUpCallTable = UpCallTable;

	outfile<<" In startup ";
	//int n = GetLastError();

	//outfile<<n<<endl;

    // Load Next Provider in chain if this is the first time called
    if (!gEntryCount)
    {
//        OutputDebugString(L"Layered Service Provider\n");

        //  Get all protocol information in database
        if ((ProtocolInfo = GetProviders(&TotalProtocols)) == NULL)
        {
			//outfile<<"1032"<<endl;
            return  WSAEPROVIDERFAILEDINIT;
        }

        // Find out what our layered protocol catalog ID entry is
        for (i = 0; i < TotalProtocols; i++)
            if (memcmp (&ProtocolInfo[i].ProviderId, &ProviderGuid, sizeof (GUID))==0)
            {
                gLayerCatId = ProtocolInfo[i].dwCatalogEntryId;
                break;
            }

        // Save our protocol chains catalog ID entry
        gChainId = lpProtocolInfo->dwCatalogEntryId;

        gLayerCount=0;
        for(x=0; x < TotalProtocols ;x++)
        {
            for(y=0; y < ProtocolInfo[x].ProtocolChain.ChainLen ;y++)
            {
                if (gLayerCatId == ProtocolInfo[x].ProtocolChain.ChainEntries[y])
                {
                    gLayerCount++;
                    break;
                }
            }
        }
        gBaseInfo = (LPWSAPROTOCOL_INFOW)GlobalAlloc(GPTR, sizeof(WSAPROTOCOL_INFOW)*gLayerCount);
        if (!gBaseInfo)
        {
            return WSAENOBUFS;
        }
        idx=0;
        for(x=0; x < TotalProtocols ;x++)
        {
            for(y=0; y < ProtocolInfo[x].ProtocolChain.ChainLen ;y++)
            {
                if (gLayerCatId == ProtocolInfo[x].ProtocolChain.ChainEntries[y])
                {
                    // Our LSP exists in this entries chain
                    //
                    iBaseId = ProtocolInfo[x].ProtocolChain.ChainEntries[ProtocolInfo[x].ProtocolChain.ChainLen-1];
                    for(z=0; z < TotalProtocols ;z++)
                    {
                        if (ProtocolInfo[z].dwCatalogEntryId == iBaseId)
                        {
                            memcpy(&gBaseInfo[idx++], &ProtocolInfo[z], sizeof(WSAPROTOCOL_INFOW));
                           // OutputDebugString(gBaseInfo[idx-1].szProtocol);
                           // OutputDebugString(L"\n");
                        }
                    }
                }
            }
        }

        // Find our layered catalog ID entry in the protocol chain
        for(j = 0; j < lpProtocolInfo->ProtocolChain.ChainLen; j++)
        {
            if (lpProtocolInfo->ProtocolChain.ChainEntries[j] == gLayerCatId)
            {

                NextProviderCatId = lpProtocolInfo->ProtocolChain.ChainEntries[j + 1];
                break;
            }
        }


        // Find next provider path to load
        for (i = 0; i < TotalProtocols; i++)
            if (NextProviderCatId == ProtocolInfo[i].dwCatalogEntryId)
            {
                if (WSCGetProviderPath(&ProtocolInfo[i].ProviderId, ProviderPath, &ProviderPathLen, &Error) == SOCKET_ERROR)
                {
					//outfile<<"1105"<<endl;
                    return  WSAEPROVIDERFAILEDINIT;
                }
                break;
            }

		wcstombs(providerpath,ProviderPath,MAX_PATH);

        if (!ExpandEnvironmentStringsA(providerpath,librarypath, MAX_PATH))
        {
			return  WSAEPROVIDERFAILEDINIT;
        }

		//mbstowcs(LibraryPath,librarypath,MAX_PATH);

        if ((hProvider = LoadLibraryA(librarypath)) == NULL)
        {
			//outfile<<"1120 "<<GetLastError()<<endl;
            return  WSAEPROVIDERFAILEDINIT;
        }


        if((WSPStartupFunc = (LPWSPSTARTUP) GetProcAddress(hProvider, "WSPStartup")) == NULL)
        {
			//outfile<<"1125 "<<GetLastError()<<endl;
            return  WSAEPROVIDERFAILEDINIT;
        }

        ReturnCode = (*WSPStartupFunc)(wVersion, lpWSPData, ProtoInfo, UpCallTable, lpProcTable);

        // Save the next providers procedure table
        memcpy(&NextProcTable, lpProcTable, sizeof WSPPROC_TABLE);

        // Remap service provider functions here

        lpProcTable->lpWSPAccept = WSPAccept;
        lpProcTable->lpWSPAddressToString = WSPAddressToString;
        lpProcTable->lpWSPAsyncSelect = WSPAsyncSelect;
        lpProcTable->lpWSPBind = WSPBind;
        lpProcTable->lpWSPCancelBlockingCall = WSPCancelBlockingCall;
        lpProcTable->lpWSPCleanup = WSPCleanup;
        lpProcTable->lpWSPCloseSocket = WSPCloseSocket;
        lpProcTable->lpWSPConnect = WSPConnect;
        lpProcTable->lpWSPDuplicateSocket = WSPDuplicateSocket;
        lpProcTable->lpWSPEnumNetworkEvents = WSPEnumNetworkEvents;
        lpProcTable->lpWSPEventSelect = WSPEventSelect;
        lpProcTable->lpWSPGetOverlappedResult = WSPGetOverlappedResult;
        lpProcTable->lpWSPGetPeerName = WSPGetPeerName;
        lpProcTable->lpWSPGetSockOpt = WSPGetSockOpt;
        lpProcTable->lpWSPGetSockName = WSPGetSockName;
        lpProcTable->lpWSPGetQOSByName = WSPGetQOSByName;
        lpProcTable->lpWSPIoctl = WSPIoctl;
        lpProcTable->lpWSPJoinLeaf = WSPJoinLeaf;
        lpProcTable->lpWSPListen = WSPListen;
        lpProcTable->lpWSPRecv = WSPRecv;
        lpProcTable->lpWSPRecvDisconnect = WSPRecvDisconnect;
        lpProcTable->lpWSPRecvFrom = WSPRecvFrom;
        lpProcTable->lpWSPSelect = WSPSelect;
        lpProcTable->lpWSPSend = WSPSend;
        lpProcTable->lpWSPSendDisconnect = WSPSendDisconnect;
        lpProcTable->lpWSPSendTo = WSPSendTo;
        lpProcTable->lpWSPSetSockOpt = WSPSetSockOpt;
        lpProcTable->lpWSPShutdown = WSPShutdown;
        lpProcTable->lpWSPSocket = WSPSocket;
        lpProcTable->lpWSPStringToAddress = WSPStringToAddress;

        gWSPData = lpWSPData;
        gProcTable = lpProcTable;
    } else
    {
        lpWSPData = gWSPData;
        lpProcTable = gProcTable;
        ReturnCode = 0;
    }

    gEntryCount++;

    LeaveCriticalSection(&CS1);

    return(ReturnCode);
}

bool IsAllow(SOCKET s, const sockaddr *name, int func)
{
	int addrLen;
//	int port;
	SockAddr_In Addr;

	addrLen= sizeof Addr;

	EnterCriticalSection(&CS1);

	if(s!=SOCKET_ERROR)
		getsockname(s,&Addr.ws1,&addrLen);
	else
		memcpy(&Addr,name,sizeof(Addr));

	sprintf(dbgstr,"Addr: %s \nPort: %d ",inet_ntoa(Addr.ws2.sin_addr), Addr.ws2.sin_port);
	switch(func){
	case 1:	MessageBox(NULL,dbgstr,"Bind",MB_OK); break;
	case 2:	MessageBox(NULL,dbgstr,"Accept",MB_OK); break;
	case 3:	MessageBox(NULL,dbgstr,"Connect",MB_OK); break;
	}

	outfile<<Addr.ws2.sin_port<<" ";
	if(IsInAllowRange(Addr.ws2.sin_port))
		outfile<<" Allow"<<endl;
	else
	{
		/*getpeername(s,&Addr.ws1,&addrLen);
		outfile<<Addr.ws2.sin_port;
		if(IsInAllowRange(Addr.ws2.sin_port))
		 outfile<<" Allow"<<endl;
		else*/
			outfile<<" Block"<<endl;
			LeaveCriticalSection(&CS1);
			return false;
	}
	//if(Addr.ws2.sin_port!=6000)
		
	//memcpy(&Addr,name,sizeof(Addr));
	//port=(Addr.ws2.sin_port);
	/*if(port==6000)
		outfile<<"In wsprecv: "<<"Allow"<<endl;
	else
		outfile<<"In wsprecv: "<<"Blocked: "<<port<<endl;*/
	LeaveCriticalSection(&CS1);

	return true;
}

bool IsInAllowRange(long port)
{
	int i;

	/*sprintf(dbgstr,"%u",port);
	MessageBox(NULL,dbgstr,"Count",MB_OK);*/

	for(i=0; i<N;i++)
	{
		if(port==gPorts[i][0] || port==gPorts[i][1])
			return true;
		if(port>gPorts[i][0] && port<gPorts[i][1])
			return true;
	}

	/*for(i=0;gPorts[i][0]!=-1;i++)
		outfile<<gPorts[i][0]<<" "<<gPorts[i][1]<<endl;*/

	return false;
}

void GetPorts()
{
	
	int i;

	infile.read((char *)&N,sizeof N);

	EnterCriticalSection(&CS1);

	outfile<<N<<endl;

	for(i=0;i<N;i++)
	{
		infile.read((char *)&gPorts[i][0],sizeof (long));
		infile.read((char *)&gPorts[i][1],sizeof (long));
		//outfile<<gPorts[i][0]<<" "<<gPorts[i][1]<<endl;
	}
	LeaveCriticalSection(&CS1);
}