#pragma once


/**  
 * \brief 
 */
struct IConnectionHandler
{

};


/**  
 * \brief 
 */
struct IConnectionProxy
{
	virtual bool ProcessSendStream(char* lpBuffer, int iLen) = 0;
	virtual bool ProcessRecvStream(char* lpBuffer, int iLen) = 0;

	virtual bool GetRecvBuffer(char* lpBuffer, int& iLen, bool& fHasMore) = 0;
	virtual bool GetSendBuffer(char* lpBuffer, int& iLen, bool& fHasMore) = 0;

	virtual bool HasIngoingPackets() = 0;
	virtual bool HasOutgoingPackets() = 0;

	virtual bool ShouldCloseConnection() = 0;

	virtual void Destroy() = 0;
};

