#include "stdafx.h"
#include "EncDec.h"
#include "CommonPackets.h"
#include "AutoBuffer.h"
#include "BufferUtil.h"


/**
 * \brief 
 */
CGMMessagePacket::CGMMessagePacket(const char* format, ...)
{    
	va_list args;
	va_start(args, format);

	int len = _vscprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
	
	CCharAutoBuffer szMessage(len);
	vsprintf(szMessage, format, args);

	int iMsgLen = (int)strlen(szMessage);

	if (iMsgLen <= 0)
		return;

	if (iMsgLen > 1)
	{
		if (((char*)szMessage)[0] == '>' && ((char*)szMessage)[1] == '>')
		{
			((char*)szMessage)[0] = ' ';
		}
	}

	int dataLen = 11 + iMsgLen + 4;
	CAutoBuffer buff_msg(dataLen);
	memset(buff_msg, 0, dataLen);

	buff_msg[0] = 0x0D;
	buff_msg[1] = 0x00;
	memcpy(buff_msg+11, szMessage, iMsgLen);

	CPacket::operator=(CPacket(0xC1, dataLen, buff_msg));
	SetType(Type());
	SetInjected();
}


/**  
 * \brief 
 */
CStringA CGMMessagePacket::DescribePacket(CPacket& pkt)
{
	BYTE* buf = pkt.GetDecryptedPacket();
	int nLen = pkt.GetDecryptedLen();

	char szMessage[260] = {0};

	memset(szMessage, 0x20, 7);
	memcpy(szMessage + 7, buf+13, nLen-13);
	return CStringA(szMessage);
}



/**
 * \brief 
 */
CServerMessagePacket::CServerMessagePacket(const char* format, ...)
{   
	memset(m_szMessage, 0, sizeof(m_szMessage));

	va_list args;
	va_start(args, format);

	int len = _vscprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
	
	CCharAutoBuffer szMessage(len);
	vsprintf(szMessage, format, args);

	int iMsgLen = (int)strlen(szMessage);

	if (iMsgLen <= 0 || iMsgLen > 238)
		return;

	if (iMsgLen > 1)
	{
		if (((char*)szMessage)[0] == '>' && ((char*)szMessage)[1] == '>')
		{
			((char*)szMessage)[0] = ' ';
		}
	}

	strncpy(m_szMessage, szMessage, 255);

	int dataLen = 11 + iMsgLen + 4;
	CAutoBuffer buff_msg(dataLen);
	memset(buff_msg, 0, dataLen);

	buff_msg[0] = 0x0D;
	buff_msg[1] = 0x01;
	memcpy(buff_msg+11, szMessage, iMsgLen);

	CPacket::operator=(CPacket(0xC1, dataLen, buff_msg));
	SetType(Type());
	SetInjected();
}


/**
 * \brief 
 */
CServerMessagePacket::CServerMessagePacket(CPacket& in)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));

	BYTE* buf = in.GetDecryptedPacket();
	int nLen = in.GetDecryptedLen();

	if (!buf || nLen < 0)
	{
		buf = in.GetRawPacket();
		nLen = in.GetPktLen();
	}

	if (buf && nLen > 11)
	{
		memcpy(m_szMessage, (char*)buf + 13, nLen - 13);
	}

	CPacket::operator=(in);
}




/**
 * \brief 
 */
CCharacterSayPacket::CCharacterSayPacket(const char* pszChar, const char* szMessage)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	const char* szCharName = (pszChar != 0) ? pszChar : "LordOfMU";

	if (szMessage != 0 && szMessage[0] != 0)
	{
		strncpy(m_szMessage, szMessage, 255);
		strncpy(m_szCharName, szCharName, 15);

		CPacket pkt;

		int nLen = (BYTE)(3 + 10 + (int)strlen(szMessage) + 1);
		CAutoBuffer buf(nLen);
		memset(buf, 0, nLen);

		buf[0] = 0xC1;
		buf[1] = (BYTE)nLen;
		buf[2] = 0x00;

		strncpy((char*)(BYTE*)buf + 3, szCharName, 10);
		
		if (nLen - 14 > 0)
			strncpy((char*)(BYTE*)buf + 13, szMessage, nLen - 14);

		SetDecryptedPacket(buf, nLen);
		SetType(Type());
		SetInjected();
	}
}


/**
 * \brief 
 */
CCharacterSayPacket::CCharacterSayPacket(CPacket& in)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	BYTE* buf = in.GetDecryptedPacket();
	int nLen = in.GetDecryptedLen();

	if (!buf || nLen < 0)
	{
		buf = in.GetRawPacket();
		nLen = in.GetPktLen();
	}

	if (buf && nLen > 13)
	{
		if (nLen > 256)
			nLen = 256;

		memcpy(m_szCharName, (char*)buf + 3, 10);
		memcpy(m_szMessage, (char*)buf + 13, nLen - 13);
	}

	CPacket::operator=(in);
}



/**  
 * \brief 
 */
CStringA CCharacterSayPacket::DescribePacket(CPacket& pkt)
{ 
	CCharacterSayPacket pkt2(pkt);

	const char* pszName = pkt2.GetCharName();
	const char* pszMsg = pkt2.GetMessage();

	CStringA str;
	str.Format("       [%s] %s", pszName, pszMsg);
	return str; 
}




/**
 * \brief 
 */
CGlobalSpeakPacket::CGlobalSpeakPacket(const char* pszChar, const char* szMessage)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	const char* szCharName = (pszChar != 0) ? pszChar : "LordOfMU";

	if (szMessage != 0 && szMessage[0] != 0)
	{
		strncpy(m_szMessage, szMessage, 255);
		strncpy(m_szCharName, szCharName, 15);

		CPacket pkt;

		int nLen = (BYTE)(3 + 10 + (int)strlen(szMessage) + 1);
		CAutoBuffer buf(nLen);
		memset(buf, 0, nLen);

		buf[0] = 0xC1;
		buf[1] = (BYTE)nLen;
		buf[2] = 0x00;

		strncpy((char*)(BYTE*)buf + 3, szCharName, 10);
		
		if (nLen - 14 > 0)
			strncpy((char*)(BYTE*)buf + 13, szMessage, nLen - 14);

		SetDecryptedPacket(buf, nLen);
		SetType(Type());
		SetInjected();
	}
}


/**
 * \brief 
 */
CGlobalSpeakPacket::CGlobalSpeakPacket(CPacket& in)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	BYTE* buf = in.GetDecryptedPacket();
	int nLen = in.GetDecryptedLen();

	if (!buf || nLen < 0)
	{
		buf = in.GetRawPacket();
		nLen = in.GetPktLen();
	}

	if (buf && nLen > 13)
	{
		memcpy(m_szCharName, (char*)buf + 3, 10);
		memcpy(m_szMessage, (char*)buf + 13, nLen - 13);
	}

	CPacket::operator=(in);
}


/**  
 * \brief 
 */
CStringA CGlobalSpeakPacket::DescribePacket(CPacket& pkt)
{ 
	CGlobalSpeakPacket pkt2(pkt);

	const char* pszName = pkt2.GetCharName();
	const char* pszMsg = pkt2.GetMessage();

	CStringA str;
	str.Format("       [%s] %s", pszName, pszMsg);
	return str; 
}




/**
 * \brief 
 */
CCharacterWhisperPacket::CCharacterWhisperPacket(const char* szCharName, const char* szMessage)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	if (szCharName != 0 && szMessage != 0 && szMessage[0] != 0)
	{
		strncpy(m_szMessage, szMessage, 255);
		strncpy(m_szCharName, szCharName, 15);

		CPacket pkt;

		int nLen = (BYTE)(3 + 10 + (int)strlen(szMessage) + 1);
		CAutoBuffer buf(nLen);
		memset(buf, 0, nLen);

		buf[0] = 0xC1;
		buf[1] = (BYTE)nLen;
		buf[2] = 0x02;

		strncpy((char*)(BYTE*)buf + 3, szCharName, 10);
		strncpy((char*)(BYTE*)buf + 13, szMessage, nLen - 14);

		SetDecryptedPacket(buf, nLen);
		SetType(Type());
		SetInjected();
	}
}


/**
 * \brief 
 */
CCharacterWhisperPacket::CCharacterWhisperPacket(CPacket& in)
{
	memset(m_szMessage, 0, sizeof(m_szMessage));
	memset(m_szCharName, 0, sizeof(m_szCharName));

	BYTE* buf = in.GetDecryptedPacket();
	int nLen = in.GetDecryptedLen();

	if (!buf || nLen < 0)
	{
		buf = in.GetRawPacket();
		nLen = in.GetPktLen();
	}

	if (buf && nLen > 13)
	{
		BYTE* buf2 = 0;

		if (buf[nLen - 1] != 0)
		{
			buf2 = new BYTE[nLen];
			memcpy(buf2, buf, nLen);

			CEncDec::DecXor32(buf2 + 3, 3, nLen - 3);
			buf = buf2;
		}

		memcpy(m_szCharName, (char*)buf + 3, 10);
		memcpy(m_szMessage, (char*)buf + 13, nLen - 13);

		if (buf2 != 0)
			delete[] buf2;
	}

	CPacket::operator=(in);
}



/**  
 * \brief 
 */
CStringA CCharacterWhisperPacket::DescribePacket(CPacket& pkt)
{ 
	CCharacterWhisperPacket pkt2(pkt);

	CStringA str("       "); 
	str += pkt2.GetCharName(); 
	str+=": "; 
	str += pkt2.GetMessage(); 
	return str; 
}



/**
 * \brief 
 */
int CMeetItemPacket::GetItemCount()
{
	BYTE* buf = GetDecryptedPacket();
	
	if (!buf)
		buf = GetRawPacket();

	if (!GetDecryptedPacket())
		return 0;

	return (int)(buf[4]);
}


/**
 * \brief 
 */
BYTE* CMeetItemPacket::GetItemData(int idx)
{
	BYTE* buf = AnyBuffer();

	if (!buf)
		return 0;

	int iCount = (int)buf[4];
	int iLen = (int)((WORD)buf[1] << 8 | buf[2]);

	if (idx >= iCount || iCount <= 0)
		return 0;

	int itemSize = (iLen - 5)/iCount;

	return buf + 5 + itemSize*idx;
}


/**
 * \brief 
 */
WORD CMeetItemPacket::GetItemType(int idx)
{
	BYTE* buf = GetItemData(idx);

	if (!buf)
		return 0;

	BYTE* pItemCode = buf + 4;

	return ((WORD)pItemCode[5] << 4) | ((WORD)(pItemCode[1] & 0x78) << 9) | pItemCode[0];
}

/**
 * \brief 
 */
WORD CMeetItemPacket::GetItemId(int idx)
{
	BYTE* buf = GetItemData(idx);

	if (!buf)
		return 0;

	return (WORD)(buf[0] & 0x7F) << 8 | buf[1];
}


/**
 * \brief 
 */
bool CMeetItemPacket::GetItemPos(int idx, BYTE& x, BYTE& y)
{
	BYTE* buf = GetItemData(idx);

	if (!buf)
		return false;

	x = buf[2];
	y = buf[3];
	return true;
}



/**
 * \brief 
 */
CPickItemPacket::CPickItemPacket(WORD wID)
{
	unsigned char pickBuf[] = {0xC3, 0x06, 0x00, 0x22, 0x00, 0x00};

	pickBuf[4] = (BYTE)((wID >> 8) & 0x007F);
	pickBuf[5] = (BYTE)(wID & 0x00FF);

	SetDecryptedPacket(pickBuf, sizeof(pickBuf));	
	SetType(Type());
	SetInjected();
}


/**
 * \brief 
 */
int CCharListReplyPacket::GetCharCount()
{
	BYTE* buf = AnyBuffer();

	if (!buf)
		return 0;

	return (int)(BYTE)buf[6] % 5;
}


/**
 * \brief 
 */
CStringA CCharListReplyPacket::GetCharName(int idx)
{
	BYTE* buf = AnyBuffer();
	int iCount = GetCharCount();

	if (!buf || iCount <= idx)
		return CStringA("");

	int iSize = 34;
	int char_data_offs = ((int)buf[1] - 7) - iCount*iSize;
	
	if (char_data_offs < 0)
	{
		iSize = ((int)buf[1] - 7)/iCount;
		char_data_offs = 0;
	}
	
	int offs = 7 + idx*iSize + char_data_offs;

	char szName[16] = {0};
	memcpy(szName, buf + offs + 1, 10);
	
	return CStringA(szName);
}


/**
 * \brief 
 */
WORD CCharListReplyPacket::GetCharLevel(int idx)
{
	BYTE* buf = AnyBuffer();
	int iCount = GetCharCount();

	if (!buf || iCount <= idx)
		return 0;

	int iSize = 34;
	int char_data_offs = ((int)buf[1] - 7) - iCount*iSize;
	
	if (char_data_offs < 0)
	{
		iSize = ((int)buf[1] - 7)/iCount;
		char_data_offs = 0;
	}

	int offs = 7 + idx*iSize + char_data_offs;

	WORD wLvl = 0;
	memcpy(&wLvl, buf + offs + 12, 2);

	return wLvl;
}


/**
 * \brief 
 */
void CCharListReplyPacket::SetCharLevel(int idx, WORD wLevel)
{
	BYTE* buf = AnyBuffer();
	int iCount = GetCharCount();

	if (!buf || iCount <= idx)
		return;

	int iSize = 34;
	int char_data_offs = ((int)buf[1] - 7) - iCount*iSize;
	
	if (char_data_offs < 0)
	{
		iSize = ((int)buf[1] - 7)/iCount;
		char_data_offs = 0;
	}

	int offs = 7 + idx*iSize + char_data_offs;
	memcpy(buf + offs + 12, &wLevel, 2);
}


/**
 * \brief 
 */
int CCharListReplyPacket::FindChar(const char* pszName)
{
	BYTE* buf = AnyBuffer();
	int iCount = GetCharCount();

	if (!buf || !pszName)
		return -1;

	int iSize = 34;
	int char_data_offs = ((int)buf[1] - 7) - iCount*iSize;

	if (char_data_offs < 0)
	{
		iSize = ((int)buf[1] - 7)/iCount;
		char_data_offs = 0;
	}

	for (int i = GetCharCount() - 1; i >= 0; --i)
	{
		int offs = 7 + i*iSize + char_data_offs;

		char szName[16] = {0};
		memcpy(szName, buf + offs + 1, 10);

		if (_stricmp(szName, pszName) == 0)
			return i;
	}

	return -1;
}


/**
 * \brief 
 */
CStringA CCharSelectedPacket::GetCharName()
{
	BYTE* buf = AnyBuffer();

	if (!buf)
		return CStringA("");

	char szCharName[16] = {0};
	memcpy(szCharName, buf + 4, 10);

	return CStringA(szCharName);
}



/**  
 * \brief 
 */
CObjectMovedPacket::CObjectMovedPacket(WORD wId, BYTE x, BYTE y, BYTE dir)
{
	const BYTE* patt = Type().GetPattern();

	BYTE buf[9] = {patt[1], patt[2], patt[3], HIBYTE(wId), LOBYTE(wId), x, y, (dir % 8) << 4, 0};
	int len = patt[2];

	SetDecryptedPacket(buf, len);
	SetType(Type());
	SetInjected();
}



/**  
 * \brief 
 */
CStringA CObjectMovedPacket::DescribePacket(CPacket& pkt)
{
	CObjectMovedPacket pkt2(pkt);

	const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

	CStringA str;
	str.Format("       Id: 0x%04X\n       x: %d, y: %d, dir: %s", 
		pkt2.GetId(), pkt2.GetX(), pkt2.GetY(), dirs[pkt2.GetDir() % 8]);

	return str;
}



/**
 * \brief 
 */
WORD CObjectMovedPacket::GetId()
{
	BYTE* buf = AnyBuffer();
	return (buf && (buf[1] >= 5)) ?  (((WORD)(buf[3] & 0x7F) << 8) | buf[4]) : 0;
}


/**
 * \brief 
 */
BYTE CObjectMovedPacket::GetX()
{
	BYTE* buf = AnyBuffer();
	return (buf && (buf[1] > 5)) ?  buf[5] : 0;
}


/**
 * \brief 
 */
BYTE CObjectMovedPacket::GetY()
{
	BYTE* buf = AnyBuffer();
	return (buf && (buf[1] > 6)) ?  buf[6] : 0;
}


/**  
 * \brief 
 */
BYTE CObjectMovedPacket::GetDir()
{
	BYTE* buf = AnyBuffer();
	return buf ?  (buf[7] >> 4) : 0;
}


/**
 * \brief 
 */
CUpdatePosSTCPacket::CUpdatePosSTCPacket(WORD wId, BYTE x, BYTE y)
{
	BYTE buf[7] = {0xC1, 0x07, 0x15, HIBYTE(wId), LOBYTE(wId), x, y};
	
	const BYTE* pat = Type().GetPattern();

	if (pat && pat[0] >= 2)
		buf[2] = pat[3];

	SetDecryptedPacket(buf, 7);
	SetType(Type());
	SetInjected();
}


/**
 * \brief 
 */
CUpdatePosCTSPacket::CUpdatePosCTSPacket(BYTE x, BYTE y, BYTE dir)
{	
	const BYTE* patt = Type().GetPattern();

	BYTE buf[7] = {patt[1], patt[2], patt[3], x, y, (dir % 8) << 4, 0};
	int len = patt[2];

	SetDecryptedPacket(buf, len);
	SetType(Type());
	SetInjected();
}


/**  
 * \brief 
 */
CDropItemPacket::CDropItemPacket(BYTE x, BYTE y, BYTE pos)
{
	unsigned char pPacket[] = {0xC3, 0x07, 0x00, 0x23, x, y, pos};
									// xor, hdr=3,     ^  ^   ^ 
									//                 X  Y  Inventory Pos

	SetDecryptedPacket(pPacket, sizeof(pPacket));
	SetType(Type());
	SetInjected();
}



/**  
 * \brief 
 */
CStringA CMeetDisguisedPlayerPacket::GetPlayerName(int idx)
{
	BYTE* pPacket = AnyBuffer();
	int iCount = GetPlayerCount();
	int iLen = (int)(((WORD)pPacket[1] << 8) | pPacket[2]);

	if (!pPacket || idx < 0 || idx >= iCount || iLen < 5)
		return CStringA("");

	char szName[16] = {0};
	int iItemSize = (iLen - 5)/iCount;

	int pos = 5 + idx*iItemSize;
	memcpy(szName, pPacket + pos + 6, 10);

	return CStringA(szName);
}


/**  
 * \brief 
 */
WORD CMeetDisguisedPlayerPacket::GetPlayerId(int idx)
{
	BYTE* pPacket = AnyBuffer();
	int iCount = GetPlayerCount();
	int iLen = (int)(((WORD)pPacket[1] << 8) | pPacket[2]);

	if (!pPacket || idx < 0 || idx >= iCount || iLen < 5)
		return false;

	int iItemSize = (iLen - 5)/iCount;
	int offs = 0;

	int pos = 5 + idx*iItemSize;

	return ((WORD)((BYTE)pPacket[pos] & 0x7F) << 8) | (BYTE)pPacket[pos+1];
}


/**  
 * \brief 
 */
bool CMeetDisguisedPlayerPacket::GetPos(int idx, BYTE& x, BYTE& y)
{
	BYTE* pPacket = AnyBuffer();
	int iCount = GetPlayerCount();
	int iLen = (int)(((WORD)pPacket[1] << 8) | pPacket[2]);

	if (!pPacket || idx < 0 || idx >= iCount || iLen < 5)
		return false;

	int iItemSize = (iLen - 5)/iCount;
	int offs = 0;

	int pos = 5 + idx*iItemSize;
	offs = pos + 2;

	x = *(pPacket + offs);
	y = *(pPacket + offs + 1);
	return true;
}


/**  
 * \brief 
 */
CStringA CMeetDisguisedPlayerPacket::DescribePacket(CPacket& pkt)
{
	CMeetDisguisedPlayerPacket pkt2(pkt);

	CStringA str;
	int iCount = pkt2.GetPlayerCount();

	str.Format("       Count of players: %d", iCount);

	for (int i=0; i < iCount; i++)
	{
		CStringA str2;
		str2.Format("\r\n         [%d] -> ID: 0x%04X, Name: %s", i, pkt2.GetPlayerId(i), pkt2.GetPlayerName(i));

		str += str2;
	}

	return str;
}



/**  
 * \brief 
 */
CStringA CMeetPlayerPacket::DescribePacket(CPacket& pkt)
{
	CMeetPlayerPacket pkt2(pkt);

	CStringA str;
	int iCount = pkt2.GetPlayerCount();

	str.Format("       Count of players: %d", iCount);

	for (int i=0; i < iCount; i++)
	{
		CStringA str2;
		str2.Format("\r\n         [%d] -> ID: 0x%04X, Name: %s", i, pkt2.GetPlayerId(i), pkt2.GetPlayerName(i));

		str += str2;
	}

	return str;
}


/**
 * \brief 
 */
int CMeetPlayerPacket::GetPlayerOffset(BYTE* pPacket, int idx, int exbPos, int iSize)
{
	int pos = 5;

	for (int i=0; i < idx; ++i)
	{
		int extraBytes = (int)pPacket[pos + exbPos];
		pos += iSize + extraBytes;
	}

	return pos;
}


/**
 * \brief 
 */
CStringA CMeetPlayerPacket::GetPlayerName(int idx)
{
	BYTE* pPacket = GetDecryptedPacket();

	if (!pPacket)
		return CStringA("!error!");

	int iCount = GetPlayerCount();
	int iLen = GetDecryptedLen();

	if (idx < 0 || idx >= iCount || iLen < 5)
		return CStringA("!error!");

	char szName[16] = {0};
	int iItemSize = (iLen - 5)/iCount;

	if (iItemSize <= 0)
		return CStringA("!error!");

	if (iItemSize < 40)
	{
		int pos = GetPlayerOffset(pPacket, idx, 35, 36);
		memcpy(szName, pPacket + pos + 22, 10);
	}
	else if (iItemSize < 44)
	{
		int pos = GetPlayerOffset(pPacket, idx, 37, 40);
		memcpy(szName, pPacket + pos + 24, 10);
	}
	else if (iItemSize > 55)
	{
		int pos = GetPlayerOffset(pPacket, idx, 35, 56);
		memcpy(szName, pPacket + pos + 22, 10);
	}
	else
	{
		int pos = GetPlayerOffset(pPacket, idx, 43, 44);
		memcpy(szName, pPacket + pos + 28, 10);
	}

	return CStringA(szName);
}


/**
 * \brief 
 */
bool CMeetPlayerPacket::GetPos(int idx, BYTE& x, BYTE& y)
{
	BYTE* pPacket = AnyBuffer();
	int iCount = GetPlayerCount();
	int iLen = (int)(((WORD)pPacket[1] << 8) | pPacket[2]);

	if (!pPacket || idx < 0 || idx >= iCount || iLen < 5)
		return false;

	int iItemSize = (iLen - 5)/iCount;
	int offs = 0;

	if (iItemSize < 40)
	{
		int pos = GetPlayerOffset(pPacket, idx, 35, 36);
		offs = pos + 22 + 10;
	}
	else if (iItemSize < 44)
	{
		int pos = GetPlayerOffset(pPacket, idx, 37, 40);
		offs = pos + 24 + 10;
	}
	else if (iItemSize > 55)
	{
		int pos = GetPlayerOffset(pPacket, idx, 35, 56);
		offs = pos + 22 + 10;
	}
	else
	{
		int pos = GetPlayerOffset(pPacket, idx, 43, 44);
		offs = pos + 28 + 10;
	}

	x = *(pPacket + offs);
	y = *(pPacket + offs + 1);
	return true;
}


/**
 * \brief 
 */
WORD CMeetPlayerPacket::GetPlayerId(int idx)
{
	BYTE* pPacket = AnyBuffer();
	int iCount = GetPlayerCount();
	int iLen = (int)((WORD)pPacket[1] << 8 | pPacket[2]);

	if (!pPacket || idx < 0 || idx >= iCount || iLen < 5)
		return 0;

	int iSize = (iLen - 5)/iCount;
	int offs = 0;

	if (iSize < 40)
		offs = GetPlayerOffset(pPacket, idx, 35, 36);
	else if (iSize < 44)
		offs = GetPlayerOffset(pPacket, idx, 37, 40);
	else if (iSize > 55)
		offs = GetPlayerOffset(pPacket, idx, 35, 56);
	else
		offs = GetPlayerOffset(pPacket, idx, 43, 44);

	return ((WORD)((BYTE)pPacket[offs] & 0x7F) << 8) | (BYTE)pPacket[offs+1];
}



/**
 * \brief 
 */
WORD CForgetObjectPacket::GetObjectId(int idx)
{
	BYTE* pPacket = AnyBuffer();

	if (!pPacket || idx < 0 || idx >= GetObjectCount())
		return 0;

	int offs = 4 + idx*2;
	return ((WORD)(pPacket[offs] & 0x7F) << 8) | (BYTE)pPacket[offs+1];
}


/**
 * \brief 
 */
WORD CForgetItemPacket::GetItemId(int idx)
{
	BYTE* pPacket = AnyBuffer();

	if (!pPacket || idx < 0 || idx >= GetItemCount())
		return 0;

	int offs = 5 + idx*2;
	return ((WORD)pPacket[offs] << 8) | (BYTE)pPacket[offs+1];
}




/**  
 * \brief 
 */
BYTE CNormalAttackPacket::m_sAttackPkt[16] = {0};
int CNormalAttackPacket::m_sOffsetAnim = 5;
int CNormalAttackPacket::m_sOffsetID = 3;
bool CNormalAttackPacket::m_fInitStatic = true;


/**  
 * \brief 
 */
void CNormalAttackPacket::Load()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "attack.pkt");

	FILE* f = fopen(szPath, "rb");

	if (f)
	{
		fread(m_sAttackPkt, 1, sizeof(m_sAttackPkt), f);

		FindOffsets();
		fclose(f);
	}
}



/**  
 * \brief 
 */
void CNormalAttackPacket::Save()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "attack.pkt");

	FILE* f = fopen(szPath, "wb");

	if (f)
	{
		fwrite(m_sAttackPkt, 1, sizeof(m_sAttackPkt), f);
		fclose(f);
	}
}


/**  
 * \brief 
 */
void CNormalAttackPacket::FindOffsets()
{
	if (m_sAttackPkt[0] == 0)
		return;

	for (int i=3; i < (int)m_sAttackPkt[1]; i++)
	{
		if (m_sAttackPkt[i] == 0x78 && (m_sAttackPkt[i+1] & 0xF0) == 0 && m_sAttackPkt[i+1] < 8)
		{
			m_sOffsetAnim = i;
			break;
		}
	}

	if (m_sOffsetAnim - 2 >= 3)
		m_sOffsetID = m_sOffsetAnim - 2;
	else
		m_sOffsetID = m_sOffsetID + 2;
}



/**  
 * \brief 
 */
void CNormalAttackPacket::InitStatic(CPacket& pkt)
{
	if (pkt.GetInjected())
		return;

	if (!m_fInitStatic)
		return;

	m_fInitStatic = false;

	int len = pkt.GetDecryptedLen();
	BYTE* buf = pkt.GetDecryptedPacket();
	const BYTE* xorP = pkt.GetType().GetXorParams();

	if (len > 0 && len <= 16)
	{
		memcpy(m_sAttackPkt, buf, len);

		if (xorP && (xorP[0] != 0 || xorP[1] != 0))
			CEncDec::DecXor32(m_sAttackPkt + xorP[0], xorP[1], len - xorP[0], true);

		FindOffsets();
		Save();
	}
}


/**
 * \brief 
 */
CNormalAttackPacket::CNormalAttackPacket(WORD wId, BYTE rot)
{
	SetType(Type());
	SetInjected();

	if (m_sAttackPkt[0] != 0)
	{
		BYTE buf[16] = {0};
		memcpy(buf, m_sAttackPkt, m_sAttackPkt[1]);

		buf[m_sOffsetID] = HIBYTE(wId);
		buf[m_sOffsetID + 1] = LOBYTE(wId);

		buf[m_sOffsetAnim + 1] = rot;

		SetDecryptedPacket(buf, (int)buf[1]);
	}
	else if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
	{
		BYTE buf[] = {0xC1, 0x07, Type().GetPattern()[3], (BYTE)((wId >> 8) & 0x00FF), (BYTE)(wId & 0x00FF), 0x78, rot};
		SetDecryptedPacket(buf, (int)buf[1]);
	}
	else if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		BYTE buf[] = {0xC1, 0x07, Type().GetPattern()[3], (BYTE)((wId >> 8) & 0x00FF), (BYTE)(wId & 0x00FF), 0x78, rot};
		SetDecryptedPacket(buf, (int)buf[1]);
	}
	else
	{
		BYTE buf[] = {0xC1, 0x08, Type().GetPattern()[3], (BYTE)((wId >> 8) & 0x00FF), (BYTE)(wId & 0x00FF), 0x78, rot, 0xAB };
		SetDecryptedPacket(buf, (int)buf[1]);
	}
}


/**
 * \brief 
 */
WORD CMeetMonsterPacket::GetId(int idx)
{
	BYTE* pPacket = AnyBuffer();

	if (!pPacket || idx < 0 || idx >= GetCount())
		return 0;

	int iSize = ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3) ? 16 : 10;
	int offs = 5 + idx*iSize;

	return ((WORD)(pPacket[offs] & 0x7F) << 8) | (BYTE)pPacket[offs+1];
}


/**  
 * \brief 
 */
WORD CMeetMonsterPacket::GetClass(int idx)
{
	BYTE* pPacket = AnyBuffer();

	if (!pPacket || idx < 0 || idx >= GetCount())
		return 0;

	int iSize = ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3) ? 16 : 10;
	int offs = 5 + idx*iSize + 2;

	return ((WORD)pPacket[offs] << 8) | (BYTE)pPacket[offs+1];
}


/**
 * \brief 
 */
bool CMeetMonsterPacket::GetPos(int idx, BYTE& x, BYTE& y, BYTE* x0, BYTE* y0)
{
	BYTE* pPacket = AnyBuffer();

	if (!pPacket || idx < 0 || idx >= GetCount())
		return false;

	int iSize = ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3) ? 16 : 10;
	int offs = 5 + idx*iSize 
				+ (((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3) ? 8 : 4) ;

	x = pPacket[offs+2];
	y = pPacket[offs+3];

	if (x0)
		*x0 = pPacket[offs];

	if (y0)
		*y0 = pPacket[offs+1];

	return true;
}




/**  
 * \brief 
 */
BYTE CSingleSkillPacket::m_sAttackPkt[16] = {0};
bool CSingleSkillPacket::m_fInitStatic = true;


/**  
 * \brief 
 */
void CSingleSkillPacket::Load()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "skill_s.pkt");

	FILE* f = fopen(szPath, "rb");

	if (f)
	{
		fread(m_sAttackPkt, 1, sizeof(m_sAttackPkt), f);
		fclose(f);
	}
}




/**  
 * \brief 
 */
void CSingleSkillPacket::Save()
{
	char szPath[_MAX_PATH+1] = {0};
	extern TCHAR g_szRoot[_MAX_PATH + 1];

	strcpy(szPath, CT2A(g_szRoot));
	strcat(szPath, "skill_s.pkt");

	FILE* f = fopen(szPath, "wb");

	if (f)
	{
		fwrite(m_sAttackPkt, 1, sizeof(m_sAttackPkt), f);
		fclose(f);
	}
}



/**  
 * \brief 
 */
void CSingleSkillPacket::InitStatic(CPacket& pkt)
{
	if (pkt.GetInjected())
		return;

	if (!m_fInitStatic)
		return;

	m_fInitStatic = false;

	int len = pkt.GetDecryptedLen();
	BYTE* buf = pkt.GetDecryptedPacket();
	const BYTE* xorP = pkt.GetType().GetXorParams();

	if (len > 0 && len <= 16)
	{
		memcpy(m_sAttackPkt, buf, len);

		if (xorP && (xorP[0] != 0 || xorP[1] != 0))
			CEncDec::DecXor32(m_sAttackPkt + xorP[0], xorP[1], len - xorP[0], true);

		Save();
	}
}



/**
 * \brief 
 */
CSingleSkillPacket::CSingleSkillPacket(WORD wSkill, WORD wTarget, int fInjected)
{
	DWORD dwVer = GetVersion();

	if (dwVer == GTYPE_S3)
	{
		unsigned char buff[] = {0xC3, 0x09, 0x00, 0x19, LOBYTE(wSkill), 
			(BYTE)((wTarget >> 8) & 0x00FF), (BYTE)(wTarget & 0x00FF), 0x00, 0x00};

		SetDecryptedPacket(buff, sizeof(buff));	
	}
	else if (dwVer == GTYPE_S4)
	{
		unsigned char buff[] = {0xC3, 0x08, 0x00, 0x19, HIBYTE(wSkill), LOBYTE(wSkill), 
			(BYTE)((wTarget >> 8) & 0x00FF), (BYTE)(wTarget & 0x00FF)};

		SetDecryptedPacket(buff, sizeof(buff));	
	}
	else if (dwVer == GTYPE_S62)
	{
		unsigned char buff[] = {0xC3, 0x09, 0x00, 0x19, HIBYTE(wSkill), LOBYTE(wSkill),
			0x00, (BYTE)((wTarget >> 8) & 0x00FF), (BYTE)(wTarget & 0x00FF)};

		SetDecryptedPacket(buff, sizeof(buff));	
	}

	SetType(Type());
	SetInjected(fInjected);
}



/**  
 * \brief 
 */
WORD CSingleSkillPacket::GetSkill()
{
	WORD wSkill = 0;
	DWORD dwVer = GetVersion();

	BYTE* buf = GetDecryptedPacket();

	if (dwVer == GTYPE_S3)
	{
		wSkill = (WORD)buf[4];
	}
	else if (dwVer == GTYPE_S4)
	{
		wSkill = MAKEWORD(buf[5], buf[4]);

	}
	else if (dwVer == GTYPE_S62)
	{
		wSkill = MAKEWORD(buf[5], buf[4]);
	}

	return wSkill;
}



/**  
 * \brief 
 */
DWORD CSingleSkillPacket::GetVersion()
{
	DWORD dwVer = CPacketType::GetVersion() & 0xFF000000;

	if (m_sAttackPkt[0] != 0)
	{
		if (m_sAttackPkt[1] == 0x08)
		{
			dwVer = GTYPE_S4;
		}
		else if (m_sAttackPkt[1] == 0x09)
		{
			if (dwVer == GTYPE_S4)
				dwVer = GTYPE_S62;
			else
				dwVer = GTYPE_S3;
		}
	}

	return dwVer;
}



/**  
 * \brief 
 */
WORD CSkillUsedPacket::GetTarget()
{ 
	WORD wRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		wRes = MAKEWORD(pPacket[8], (pPacket[7] & 0x7F));
	}
	else
	{
		wRes = MAKEWORD(pPacket[9], (pPacket[8] & 0x7F));
	}

	return wRes;
}


/**  
 * \brief 
 */
WORD CSkillUsedPacket::GetAttacker()
{ 
	WORD wRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;


	if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		wRes = MAKEWORD(pPacket[6], (pPacket[5] & 0x7F));
	}
	else
	{
		wRes = MAKEWORD(pPacket[7], (pPacket[6] & 0x7F));
	}

	return wRes;
}



/**
 * \brief 
 */
CMassiveSkillPacket::CMassiveSkillPacket(WORD wSkill, BYTE x, BYTE y, BYTE rot, WORD wTarget, int fInjected)
{
	DWORD dwVers = CSingleSkillPacket::GetVersion();

	if (dwVers == GTYPE_S3)
	{
		unsigned char buff[] = {0xC3, 0x0E, 0x00, 0x1E, LOBYTE(wSkill), 
			(BYTE)x, (BYTE)y, 
			(BYTE)rot, 
			0x00, 0x00, 
			HIBYTE(wTarget), LOBYTE(wTarget), 
			0x00, 0x05};

		SetDecryptedPacket(buff, sizeof(buff));	
	}
	else if (dwVers == GTYPE_S4)
	{
		unsigned char buff[] = {0xC3, 0x0E, 0x00, 0x1E, HIBYTE(wSkill), LOBYTE(wSkill), 
			(BYTE)x, (BYTE)y, 
			(BYTE)rot, 
			0x00, 0x00, 
			HIBYTE(wTarget), LOBYTE(wTarget), 
			0x00};

		SetDecryptedPacket(buff, sizeof(buff));	
	}
	else
	{
		unsigned char buff[] = {0xC3, 0x0E, 0x00, 0x1E, 
			(BYTE)x, (BYTE)y, 
			HIBYTE(wSkill), LOBYTE(wSkill), 
			(BYTE)rot, 
			0x00, 0x00, 
			0x00,
			HIBYTE(wTarget), LOBYTE(wTarget)
			};

		SetDecryptedPacket(buff, sizeof(buff));	
	}

	SetType(Type());
	SetInjected(fInjected);
}


/**  
 * \brief 
 */
WORD CMassiveSkillPacket::GetTarget()
{ 
	WORD wRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	DWORD dwVers = CSingleSkillPacket::GetVersion();

	if (dwVers == GTYPE_S3)
	{
		wRes = MAKEWORD(pPacket[11], pPacket[10]);
	}
	else if (dwVers == GTYPE_S4)
	{
		wRes = MAKEWORD(pPacket[12], pPacket[11]);
	}
	else
	{
		wRes = MAKEWORD(pPacket[13], pPacket[12]);
	}

	return wRes;
}



/**  
 * \brief 
 */
WORD CMassiveSkillPacket::GetSkill()
{ 
	WORD wRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	DWORD dwVers = CSingleSkillPacket::GetVersion();

	if (dwVers == GTYPE_S3)
	{
		wRes = MAKEWORD(pPacket[4], 0);
	}
	else if (dwVers == GTYPE_S4)
	{
		wRes = MAKEWORD(pPacket[5], pPacket[4]);
	}
	else
	{
		wRes = MAKEWORD(pPacket[7], pPacket[6]);
	}

	return wRes;
}



/**  
 * \brief 
 */
BYTE CMassiveSkillUsedPacket::GetTargetX()
{ 
	BYTE bRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		bRes = pPacket[7];
	}
	else
	{
		bRes = pPacket[8];
	}

	return bRes;
}


/**  
 * \brief 
 */
BYTE CMassiveSkillUsedPacket::GetTargetY()
{ 
	BYTE bRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		bRes = pPacket[8];
	}
	else
	{
		bRes = pPacket[9];
	}

	return bRes;
}




/**  
 * \brief 
 */
WORD CMassiveSkillUsedPacket::GetAttacker()
{ 
	WORD wRes = 0;
	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;


	if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S3)
	{
		wRes = MAKEWORD(pPacket[6], (pPacket[5] & 0x7F));
	}
	else
	{
		wRes = MAKEWORD(pPacket[7], (pPacket[6] & 0x7F));
	}

	return wRes;
}




/**
 * \brief 
 */
CObjectDeathPacket::CObjectDeathPacket(WORD wId)
{
	BYTE buf[] = {0xC1, 0x08, 0x17, HIBYTE(wId), LOBYTE(wId), 0x00, 0x00, 0x00};

	SetDecryptedPacket(buf, sizeof(buf));
	SetType(Type());
	SetInjected();
}


/**  
 * \brief 
 */
CStringA CClientAuthPacket::DescribePacket(CPacket& pkt)
{ 
	CClientAuthPacket pkt2(pkt);

	char szUser[12] = {0};
	char szPass[24] = {0};
	char szSerial[24] = {0};
	char szVersion[8] = {0};

	pkt2.GetUserName(szUser);
	pkt2.GetPassword(szPass); 
	pkt2.GetSerial(szSerial);
	pkt2.GetVersion(szVersion);

	CStringA str; 
	str.Format("       User: %s\n       Password: %s\n       Serial: %s\n       Version: %c.%c%c.%c%c", 
					szUser, szPass, szSerial, szVersion[0], szVersion[1], szVersion[2], szVersion[3], szVersion[4]);
	return str; 
}


/**
 * \brief 
 */
void CClientAuthPacket::GetUserName(char* pszUser)
{
	if (!pszUser)
		return;

	pszUser[0] = 0;

	BYTE* buf = AnyBuffer();
	if (!buf || (int)buf[1] < 15)
		return;

	memcpy(pszUser, buf + 5, 10);
	CEncDec::EncDecLogin((BYTE*)pszUser, 10); //Decrypt Login
}


/**
 * \brief 
 */
void CClientAuthPacket::GetPassword(char* pszPass)
{
	if (!pszPass)
		return;

	pszPass[0] = 0;

	BYTE* buf = AnyBuffer();
	if (!buf || (int)buf[1] < 50)
		return;

	if ((int)buf[1] > 50)
	{
		memcpy(pszPass, buf + 15, 20);
		CEncDec::EncDecLogin((BYTE*)pszPass, 20); //Decrypt Login
	}
	else
	{
		memcpy(pszPass, buf + 15, 10);
		CEncDec::EncDecLogin((BYTE*)pszPass, 10); //Decrypt Login
	}
}


/**
 * \brief 
 */
void CClientAuthPacket::GetSerial(char* pszSerial)
{
	if (!pszSerial)
		return;

	pszSerial[0] = 0;

	BYTE* buf = AnyBuffer();

	if (!buf || (int)buf[1] < 50)
		return;

	if ((int)buf[1] > 52)
		memcpy(pszSerial, buf + 44, 16);
	else if (buf[1] == 0x34)
		memcpy(pszSerial, buf + 36, 16);
	else
		memcpy(pszSerial, buf + 34, 16);
}


/**  
 * \brief 
 */
void CClientAuthPacket::GetVersion(char* pszVersion)
{
	if (!pszVersion)
		return;

	pszVersion[0] = 0;

	BYTE* buf = AnyBuffer();

	if (!buf || (int)buf[1] < 50)
		return;

	if ((int)buf[1] > 52)
		memcpy(pszVersion, buf + 39, 5);
	else if (buf[1] == 0x34)
		memcpy(pszVersion, buf + 31, 5);
	else
		memcpy(pszVersion, buf + 29, 5);
}



/**  
 * \brief 
 */
BYTE CInitInventoryPacket::GetPosition(int idx)
{
	if (idx < 0 || idx >= GetCount())
		return 0;

	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	int iSize = GetDataSize();
	int offs = 7 + idx*iSize;
	return pPacket[offs];
}


/**  
 * \brief 
 */
BYTE* CInitInventoryPacket::GetItemData(int idx)
{
	if (idx < 0 || idx >= GetCount())
		return 0;

	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	int iSize = GetDataSize();
	int offs = 7 + idx*iSize + 1;
	return pPacket + offs;
}


/**  
 * \brief 
 */
int CRemoveFromInventoryPacket::GetPosition(int idx)
{
	if (idx < 0 || idx >= GetCount())
		return 0;

	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	int offs = 4 + idx;
	return pPacket[offs];
}



/**  
 * \brief 
 */
BYTE CVaultContentsPacket::GetPosition(int idx)
{
	if (idx < 0 || idx >= GetCount())
		return 0;

	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	int iSize = GetDataSize();
	int offs = DATA_OFFS + idx*iSize;
	return pPacket[offs];
}


/**  
 * \brief 
 */
BYTE* CVaultContentsPacket::GetItemData(int idx)
{
	if (idx < 0 || idx >= GetCount())
		return 0;

	BYTE* pPacket = AnyBuffer();

	if (!pPacket)
		return 0;

	int iSize = GetDataSize();
	int offs = DATA_OFFS + idx*iSize + 1;
	return pPacket + offs;
}

