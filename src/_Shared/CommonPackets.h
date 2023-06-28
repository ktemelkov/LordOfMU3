#ifndef __CommonPackets_H
#define __CommonPackets_H

#pragma once

#include "Packet.h"
#include "CommonPacketsMacros.h"
#include "BufferUtil.h"

#include <map>

#define IS_MONSTER_ID(wId) ((wId) < 0x1F00)

#define ITEM_CODE(pPacket,offs) (((WORD)(pPacket)[(offs)+5] << 4) | ((WORD)((pPacket)[(offs)+1] & 0x78) << 9) | (pPacket)[(offs)])

#define ITEM_GROUP(wType) (((wType) >> 8) & 0x0F)
#define ITEM_SUBGR(wType) ((wType) & 0xFF)
#define ITEM_LEVEL(wType) ((wType) >> 12)


/**
 * \brief 
 */
class CGMMessagePacket
	: public CPacket
{
public:
	CGMMessagePacket(const char* szFormat, ...);

	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CGMMessagePacket)
		PACKET_PATT4(0xC1, 0x00, 0x0D, 0x00)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Server anouncement / GM message")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CServerMessagePacket
	: public CPacket
{
public:
	CServerMessagePacket(CPacket& in);
	CServerMessagePacket(const char* szFormat, ...);

	const char* GetMessage(){ return m_szMessage; }

	static CStringA DescribePacket(CPacket& pkt){ CStringA str("       "); str += CStringA(CServerMessagePacket(pkt).GetMessage()); return str; }

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CServerMessagePacket)
		PACKET_PATT4(0xC1, 0x00, 0x0D, 0x01)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Server message")
	END_COMMON_PACKET_DECL()

private:
	char m_szMessage[256];
};


/**
 * \brief 
 */
class CCharacterSayPacket
	: public CPacket
{
public:
	CCharacterSayPacket(CPacket& in);
	CCharacterSayPacket(const char* szCharName, const char* szMessage);

	const char* GetMessage(){ return m_szMessage; }
	const char* GetCharName(){ return m_szCharName; }

	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CCharacterSayPacket)
		PACKET_PATT3(0xC1, 0x00, 0x00)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character speak")
	END_COMMON_PACKET_DECL()

private:
	char m_szMessage[256];
	char m_szCharName[16];
};



/**
 * \brief Global speak
 *
 */
class CGlobalSpeakPacket
	: public CPacket
{
public:
	CGlobalSpeakPacket(CPacket& in);
	CGlobalSpeakPacket(const char* szCharName, const char* szMessage);

	const char* GetMessage(){ return m_szMessage; }
	const char* GetCharName(){ return m_szCharName; }

	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CGlobalSpeakPacket)
		PACKET_PATT3(0xC1, 0x00, 0x00)
		PACKET_MASK3(0xFF, 0x00, 0xFE)
		PACKET_DESCR("Global speak")
	END_COMMON_PACKET_DECL()

private:
	char m_szMessage[256];
	char m_szCharName[16];
};



/**
 * \brief 
 */
class CCharacterSayPacket2
	: public CCharacterSayPacket
{
public:
	CCharacterSayPacket2(CPacket& in) : CCharacterSayPacket(in) {}

	static CStringA DescribePacket(CPacket& pkt){ return CCharacterSayPacket::DescribePacket(pkt); }

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CCharacterSayPacket2)
		PACKET_PATT3(0xC1, 0x00, 0x45)
		PACKET_MASK3(0xFF, 0x00, 0xFC)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character speak")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CCharacterSayPacket3
	: public CCharacterSayPacket
{
public:
	CCharacterSayPacket3(CPacket& in) : CCharacterSayPacket(in) {}

	static CStringA DescribePacket(CPacket& pkt){ return CCharacterSayPacket::DescribePacket(pkt); }

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CCharacterSayPacket3)
		PACKET_PATT3(0xC1, 0x00, 0x65)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character speak")
	END_COMMON_PACKET_DECL()
};

/**
 * \brief 
 */
class CCharacterWhisperPacket
	: public CPacket
{
public:
	CCharacterWhisperPacket(CPacket& in);
	CCharacterWhisperPacket(const char* szCharName, const char* szMessage);

	const char* GetMessage(){ return m_szMessage; }
	const char* GetCharName(){ return m_szCharName; }

	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CCharacterWhisperPacket)
		PACKET_PATT3(0xC1, 0x00, 0x02)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character whisper")
	END_COMMON_PACKET_DECL()

private:
	char m_szMessage[256];
	char m_szCharName[16];
};



/**
 * \brief 
 */
class CCharacterWhisperSTCPacket
	: public CCharacterWhisperPacket
{
public:
	CCharacterWhisperSTCPacket(CPacket& in) : CCharacterWhisperPacket(in){}
	CCharacterWhisperSTCPacket(const char* szCharName, const char* szMessage) : CCharacterWhisperPacket(szCharName, szMessage){}

	static CStringA DescribePacket(CPacket& pkt) { return CCharacterWhisperPacket::DescribePacket(pkt); }

	BEGIN_COMMON_PACKET_DECL_NOCCONSTR(CCharacterWhisperSTCPacket)
		PACKET_PATT3(0xC1, 0x00, 0x02)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Character whisper STC")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CClientF1Packet
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CClientF1Packet)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0xF1)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Client to server 0xF1 command")
		NEED_POST_DETECT()
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CServerF1Packet
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CServerF1Packet)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0xF1)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Server to client 0xF1 command")
		NEED_POST_DETECT()
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CClientAuthPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CClientAuthPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0xF1, 0x01)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Client login")
	END_COMMON_PACKET_DECL()

	void GetUserName(char* pszUser);
	void GetPassword(char* pszPass);
	void GetSerial(char* pszSerial);
	void GetVersion(char* pszVersion);
};


/**
 * \brief 
 */
class CClientAuthReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CClientAuthReplyPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF1, 0x01)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Client login reply")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 *
 * C3 06 03 F1 02 02
 *                 ^
 *                 0 - close socket immediately
 *                 1 - go to character selection screen
 *                 2 - go to server selection screen
 */
class CClientDCReqPacket
	: public CPacket
{
public:
	/**  
	 * \brief 
	 *
	 * \param dcType - 0 = close socket, 1 - exit to characters screen, 2 - exit to server select screen
	 */
	CClientDCReqPacket(BYTE dcType)
	{
		BYTE buf[] = {0xC3, 0x06, 0x00, 0xF1, 0x02, dcType};
		SetDecryptedPacket(buf, sizeof(buf));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL(CClientDCReqPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0xF1, 0x02)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Client disconnect request")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CClientDCReplyPacket
	: public CPacket
{
public:
	/**  
	 * \brief 
	 *
	 * \param dcType - 0 - close socket, 1 - exit to characters screen, 2 - exit to server select screen
	 */
	CClientDCReplyPacket(BYTE dcType)
	{
		SetType(Type());
		SetInjected();

		BYTE buf[] = {0xC3, 0x06, 0x00, 0xF1, 0x02, dcType};
		SetDecryptedPacket(buf, sizeof(buf));
	}

	BEGIN_COMMON_PACKET_DECL(CClientDCReplyPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0xF1, 0x02)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Client disconnect reply")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CClientAuthCSPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{ 
		char szName[12] = {0};
		CClientAuthCSPacket(pkt).GetCharName(szName);

		CStringA str("       User: ");
		str+= szName; 
		return str; 
	}

	BEGIN_COMMON_PACKET_DECL(CClientAuthCSPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0xB1)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Client login CS")
	END_COMMON_PACKET_DECL()

	void GetCharName(char* pszName){ BYTE* pPacket = AnyBuffer(); if (pPacket) memcpy(pszName, pPacket + 17, 10); }
};


/**
 * \brief Object moved server to client packet
 *
 * 0xC1 0x08 0xD3 ID_HI ID_LO X_TO Y_TO FACE_DIR
 *
 */
class CObjectMovedPacket
	: public CPacket
{
public:
	CObjectMovedPacket(WORD wId, BYTE x, BYTE y, BYTE dir = 0);

	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CObjectMovedPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x08, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x08, 0x1D)			
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x08, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x08, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x08, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x08, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Object moved")
	END_COMMON_PACKET_DECL()

	WORD GetId();

	BYTE GetX();
	BYTE GetY();
	BYTE GetDir();
};


/**
 * \brief 
 */
class CObjectAnimationPacket
	: public CPacket
{
// Object attacks object
// C1 09 18 04 85 02 78 1C B0
//           ^ ^   ^  ^  ^ ^
//            |    |  |   |
//		  Attacker |  | Victim
//                Dir |
//                   Anim2
public:
	CObjectAnimationPacket(WORD src, WORD dst, BYTE dir, BYTE anim)
	{
		const BYTE* patt = Type().GetPattern();

		BYTE buf[10] = {patt[1], patt[2], patt[3], HIBYTE(src), LOBYTE(src), dir, anim, HIBYTE(dst), LOBYTE(dst), 0};
		int len = patt[2];

		SetDecryptedPacket(buf, len);
		SetType(Type());
		SetInjected();
	}

	static CStringA DescribePacket(CPacket& pkt)
	{
		CObjectAnimationPacket& pkt2 = (CObjectAnimationPacket&)pkt;

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       Actor:   0x%04X\n       Target: 0x%04X\n       Type: 0x%02X\n       Dir: %s", pkt2.GetActor(), 
				pkt2.GetTarget(), pkt2.GetAnim(), dirs[pkt2.GetDir()]);
		
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CObjectAnimationPacket)
		PACKET_PATT3(0xC1, 0x09, 0x18)
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Object animation")
	END_COMMON_PACKET_DECL()

	WORD GetTarget(){ BYTE* buff = AnyBuffer(); return !buff ? 0 : MAKEWORD(buff[8], (buff[7] & 0x7F)); }
	WORD GetActor(){ BYTE* buff = AnyBuffer(); return !buff ? 0 : MAKEWORD(buff[4], (buff[3] & 0x7F)); }
	BYTE GetDir() { BYTE* buff = AnyBuffer(); return !buff ? 0 : (buff[5] & 0x7); }
	BYTE GetAnim() { BYTE* buff = AnyBuffer(); return !buff ? 0 : buff[6]; }
};


/**
 * \brief 
 */
class CHitInfoPacket
	: public CPacket
{
// hit display
// C1 0A 11 1C B0 00 01 00 00 00
//           ^ ^   ^  ^--^  ^--^ SD Damage
//         Victim  |   Life Damage
//               Hit Type (normal, reflect, ignore, critical)
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CHitInfoPacket pkt2(pkt);
		CStringA str;
		str.Format("       Victim: 0x%04X\n       Type:   %d\n       Damage: %d\n       SD Damage: %d", pkt2.GetId(), pkt2.GetType(), pkt2.GetDamage(), pkt2.GetSDDamage());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CHitInfoPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x00, 0x11)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x00, 0xDC)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x00, 0xD7)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x00, 0xD9)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x00, 0x15)

			PACKET_PATT_V3(0        , 0xC1, 0x00, 0x11)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Hit info")
	END_COMMON_PACKET_DECL()

	WORD GetId(){ BYTE* buf = AnyBuffer(); return (((WORD)(buf[3] & 0x7F) << 8) | buf[4]); }
	BYTE GetType() { BYTE* buf = AnyBuffer(); return buf[5]; }
	WORD GetDamage() { BYTE* buf = AnyBuffer(); return *((WORD*)(buf + 6)); }
	WORD GetSDDamage() { BYTE* buf = AnyBuffer(); if (!buf || buf[1] < 9) return 0; return (((WORD)buf[8] << 8) | buf[9]); }
};


/**
 * \brief 
 */
class CClientPingPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CClientPingPacket)
		PACKET_PATT4(0xC3, 0x0D, 0x00, 0x0E)
		PACKET_MASK4(0xFF, 0xFF, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Client Ping")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CPingReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CPingReplyPacket)
		PACKET_PATT3(0xC1, 0x04, 0x0F)
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Ping Reply")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief Meet monster packet
 *
 * meet monster (protocol version <= 104)
 * C2 00 15 13
 * 	  01 04 45 00 1A 00 00 00 00 93 70 92 71 60 00 00 00
 *    ^   ^ ^   ^-^   ^--------^  ^-^   ^-^   ^
 * Count   ID  Type   Some info  Meet At | Facing Dir.
 * of Monsters                       Stopped At
 * -------------------------------------------------------
 * meet monster (protocol version > 104)
 * C2 00 0f 13  
 *    01 0c 51 01 a5 78 bf 79 bf 40 00
 *    ^   ^ ^   ^-^   ^-^   ^-^   ^
 * Count   ID  Type  Meet At | Facing Dir.
 * of Monsters           Stopped At
 */
class CMeetMonsterPacket
	: public CPacket
{
public:

	static CStringA DescribePacket(CPacket& pkt)
	{
		CMeetMonsterPacket pkt2(pkt);

		int iCount = pkt2.GetCount();

		CStringA str;
		str.Format("       Count of Monsters: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			BYTE x =0, y=0, x0=0, y0=0;
			pkt2.GetPos(i, x, y, &x0, &y0);

			CStringA str2;
			str2.Format("\n       [%d] Id: 0x%04X, x: %d, y: %d, x0: %d, y0: %d, type: %d", i, pkt2.GetId(i), x, y, x0, y0, pkt2.GetClass(i));
			str += str2;
		}

		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CMeetMonsterPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x13)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Meet monster(s)")
	END_COMMON_PACKET_DECL()

	int GetCount(){ BYTE* buf = AnyBuffer(); return buf ? buf[4] : 0; }
	WORD GetId(int idx);
	WORD GetClass(int idx);
	bool GetPos(int idx, BYTE& x, BYTE& y, BYTE* x0 = 0, BYTE* y0 = 0);
};


/**
 * \brief Meet item packet
 *
 * C2 00 1B 20 02 80 67 35 86 0D 0A A7 00 00 0D 00 80 68 35 88 20 08 60 00 00 B0 00
 *     ^ ^      ^  ^ ^   ^ ^  <---- item code --->| ^ ^   ^ ^  <----- item code -->
 *     Len      ^   ^    x,y position                ^     x,y position
 *              ^   Item ID                         Item ID
 *              number of items
 *
 *
 * V0.99
 * C2 00 0E 20 01 80 4F C9 ED CF 00 5E 80 5D
 * C2 00 0E 20 01 80 8B 36 18 C7 90 2A 00 00
 * C2 00 17 20 02 81 0E C8 F0 CF 00 62 80 77  81 0F C3 EE CF 00 61 80 19
 *     ^ ^      ^  ^ ^   ^ ^   ^ item code ^
 *     Len      ^   ^    x,y position     
 *              ^   Item ID               
 *              number of items
 *
 */
class CMeetItemPacket
	: public CPacket
{
public:
	int GetItemCount();
	BYTE* GetItemData(int idx);
	WORD GetItemType(int idx);
	WORD GetItemId(int idx);
	bool GetItemPos(int idx, BYTE& x, BYTE& y);

public:
	BEGIN_COMMON_PACKET_DECL(CMeetItemPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x20)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Meet item(s)")
	END_COMMON_PACKET_DECL()
};


/**  
 * \brief 
 * C3 11 20 22 12 20 00 FF 00 00 E0 00 FF FF FF FF FF
 * C3 15 0C 22 0C 03 00 FF 00 00 E0 00 FF FF FF FF FF 00 FD FD FD 
 *              ^  ^----------------^   ---- Some junk here ----
 *              |      Item code
 *        Inventory Place
 */
class CPutInventoryPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CPutInventoryPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0x22, 0x00)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0x80)
		PACKET_DESCR("Put inventory")
	END_COMMON_PACKET_DECL()

	WORD GetItemType()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 5);
	}

	BYTE GetInvPos()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[4];
	}

	BYTE* GetItemData()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket + 5;
	}
};


/**  
 * \brief 
 * C3 16 1A 24 00 0C 22 13 4A 00 00 A0 00 FF FF FF FF FF 00 00 00 00 
 *                 ^  ^----------------^   ---- Some junk here ----
 *                 |      Item code
 *           Inventory Place
 *
 * C3 11 73 24 00 40 09 38 3C 00 00 B0 00 A8 C7 77 26 <-- Season 3
 *                 ^  ^----------------^   
 *                 |      Item code
 *           Inventory Place
 *
 */
class CMoveToInventoryPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CMoveToInventoryPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0x24, 0x00)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Move to inventory")
	END_COMMON_PACKET_DECL()

	WORD GetItemType()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 6);
	}

	BYTE GetInvPos()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[5];
	}

	BYTE* GetItemData()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket + 6;
	}
};


/**  
 * \brief 
 * C3 16 1A 24 02 0C 22 13 4A 00 00 A0 00 FF FF FF FF FF 00 00 00 00 
 *                 ^  ^----------------^   ---- Some junk here ----
 *                 |      Item code
 *           Inventory Place
 *
 * C3 11 55 24 02 58 03 3F 46 00 00 50 00 A8 C7 47 26 <- Season 3
 *                 ^  ^----------------^   
 *                 |      Item code
 *           Inventory Place
 *
 */
class CMoveToVaultPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CMoveToVaultPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0x24, 0x02)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Move to vault")
	END_COMMON_PACKET_DECL()

	WORD GetItemType()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 6);
	}

	BYTE GetInvPos()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[5];
	}
};


/**  
 * \brief 
 * C3 14 06 24 00 14 1C 20 78 00 00 E0 00 FF FF FF FF FF 02 17 
 *              ^  ^  ^----------------^   ^-- junk --^   ^  ^ 
 *              |  |      Item code                       | Inventory place
 *              | Inventory Place                       Destination: 0 - inventory, 2 - vault, 4 - store
 *             Source: 0 - inventory, 2 - vault 
 *
 * C3 0F 9D 24 00 06 09 38 3C 00 00 B0 00 00 40  <- season 3
 *              ^  ^  ^----------------^   ^  ^ 
 *              |  |      Item code        | Inventory place
 *              | Inventory Place      Destination: 0 - inventory, 2 - vault, 4 - store
 *             Source: 0 - inventory, 2 - vault 
 *
 */
class CMoveItemReqPacket
	: public CPacket
{
public:
	CMoveItemReqPacket(WORD src, WORD dst, BYTE* pData)
	{
		if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
		{
			BYTE buff[] = {0xC3, 0x14, 0x00, 0x24, HIBYTE(src), LOBYTE(src), pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], 
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, HIBYTE(dst), LOBYTE(dst)};

			SetDecryptedPacket(buff, sizeof(buff));
		}
		else
		{
			BYTE buff[] = {0xC3, 0x0F, 0x00, 0x24, HIBYTE(src), LOBYTE(src), pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], 
							HIBYTE(dst), LOBYTE(dst)};

			SetDecryptedPacket(buff, sizeof(buff));
		}

		SetType(Type());
		SetInjected();
	}

	static const BYTE PLACE_INVENTORY = 0;
	static const BYTE PLACE_VAULT = 2;
	static const BYTE PLACE_STORE = 4;

	BEGIN_COMMON_PACKET_DECL(CMoveItemReqPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x24)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Move item request")
	END_COMMON_PACKET_DECL()

	WORD GetItemType()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 6);
	}

	BYTE GetInvPos()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[5];
	}

	BYTE GetSource()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[4];
	}

	BYTE GetDest()
	{
		BYTE* pPacket = AnyBuffer();	
		int offs = ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4) ? 18 : 13;

		return (pPacket == 0) ? 0 : pPacket[offs];
	}

	BYTE GetDestPos()
	{
		BYTE* pPacket = AnyBuffer();
		int offs = ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4) ? 19 : 14;

		return (pPacket == 0) ? 0 : pPacket[offs];
	}
};





/**  
 * \brief 
 *
 * C3 07 10 23 39 0C B2
 *              ^ ^   ^ 
 *              X Y  Inventory Pos
 */
class CDropItemPacket
	: public CPacket
{
public:
	CDropItemPacket(BYTE x, BYTE y, BYTE pos);

	BEGIN_COMMON_PACKET_DECL(CDropItemPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x23)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Drop item")
		PACKET_XORP2(4, 3)
	END_COMMON_PACKET_DECL()

};


/**
 * \brief 
 */
class CGameServerHelloPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{ 
		CStringA str;
		str.Format("       Player ID: 0x%04X", CGameServerHelloPacket(pkt).GetPlayerId());
		
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CGameServerHelloPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF1, 0x00)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Game server hello")
	END_COMMON_PACKET_DECL()

	WORD GetPlayerId()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket && (pPacket[1] > 6)) ? (((WORD)pPacket[5] << 8) | pPacket[6]) : 0;
	}
};


/**
 * \brief 
 */
class CCharCommandsPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharCommandsPacket)
		PACKET_PATT3(0xC1, 0x00, 0xF3)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character command")
		NEED_POST_DETECT()
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CCharCommandReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharCommandReplyPacket)
		PACKET_PATT3(0xC1, 0x00, 0xF3)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Character command reply")
		NEED_POST_DETECT()
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CCharSelectedPacket
	: public CPacket
{
public:
	CCharSelectedPacket(const char* pszName)
	{
		SetType(Type());
		SetInjected();

		BYTE buf[] = {0xC1, 0x0E, 0xF3, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		int len = sizeof(buf);

		strncpy((char*)buf + 4, pszName, 10);
		SetDecryptedPacket(buf, len);
	}

	static CStringA DescribePacket(CPacket& pkt)
	{
		CStringA str;
		str.Format("       Character: %s", CCharSelectedPacket(pkt).GetCharName());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CCharSelectedPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x03)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character selected")
	END_COMMON_PACKET_DECL()

	CStringA GetCharName();
};



/**
 * \brief 
 *
 * C1 0F F3 01 4A 65 72 69 63 6F 4F 6E 65 00 30
 *              ^-------------------------^  ^
 *                       Name              Class
 */
class CCharCreateReqPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharCreateReqPacket pkt2(pkt);

		CStringA str;
		str.Format("       Character: %s\n       Type: %d", pkt2.GetCharName(), pkt2.GetCharType());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CCharCreateReqPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x01)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character create request")
	END_COMMON_PACKET_DECL()

	BYTE GetCharType() { BYTE* buf = AnyBuffer(); if (!buf || (int)buf[1] < 14) return 0; return buf[14]; }

	CStringA GetCharName()
	{
		BYTE* buf = AnyBuffer();

		if (!buf)
			return CStringA("");

		char szCharName[16] = {0};
		memcpy(szCharName, buf + 4, 10);

		return CStringA(szCharName);
	}
};


/**
 * \brief 
 *
 * C1 2C F3 01 01 4A 65 72 69 63 6F 4F 6E 65 00 01 01 00 60 01 00 00 00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF 71
 *           ^     ^-------------------------^
 *           |            Name
 *         success
 *
 */
class CCharCreateReplyPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharCreateReqPacket pkt2(pkt);

		CStringA str;
		str.Format("       Character: %s\n", pkt2.GetCharName());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CCharCreateReplyPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x01)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character create reply")
	END_COMMON_PACKET_DECL()

	CStringA GetCharName()
	{
		BYTE* buf = AnyBuffer();

		if (!buf)
			return CStringA("");

		char szCharName[16] = {0};
		memcpy(szCharName, buf + 5, 10);

		return CStringA(szCharName);
	}
};



/**
 * \brief 
 */
class CCharListReqPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharListReqPacket)
		PACKET_PATT4(0xC1, 0x04, 0xF3, 0x00)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Character list request")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CCharSetupPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharSetupPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x30)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character setup")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief
 *
 * C1 29 F3 00 03 00 01 00 4B 75 6B 61 74 61 00 00 00 00 00 E5 00 00 20 0D 0D 99 99 9F 0D B6 DB 00 00 00 00 00 00 00 00 00 FF
 *                    ^     ^_________________________^      ^ ^
 *                  Count            Name                   Level
 *
 * C1 3A F3 00 05 00 01 00 01 00 00 00 01 00 00 00 58 84 94 1E C8 57 09 0D 00 4B 75 6B 61 74 61 00 00 00 00 00 01 00 00 20 00 FF FF FF F3 00 00 00 F8 00 00 20 FF FF FF 00 00 FF
 *                    ^                                                        ^_________________________^      ^ ^
 *                  Count                                                               Name                   Level
 *
 * C1 5C F3 00 06 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 62 61 74 42 6F 69 6B 6F 00 00 00 04 01 00 20 05 FF 11 11 17 12 A9 04 03 00 00 40 F0 00 00 00 00 FF 
                                                                           01 4A 65 72 69 63 6F 4F 6E 65 00 00 B2 00 00 80 0B 02 BB BB B3 12 52 61 F8 00 00 40 C0 00 00 00 00 FF
 *                    ^                                                        ^_________________________^      ^ ^
 *                  Count                                                               Name                   Level
 *
 * Version 0.99
 * C1 25 F3 00 02 00 01 00 4B 75 6B 61 74 61 00 00 00 00 00 B7 00 00 00 A1 FF F7 FF 2F 00 04 02 B0 00 00 00 FF 00
 *                    ^     ^_________________________^      ^ ^
 *                  Count            Name                   Level
 */
class CCharListReplyPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharListReplyPacket pkt2(pkt);

		int iCount = pkt2.GetCharCount();

		CStringA str;
		str.Format("       Characters: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			CStringA str2;
			str2.Format("\n       [%d] Name: %s, Level: %d", i, pkt2.GetCharName(i), pkt2.GetCharLevel(i));
			str += str2;
		}

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CCharListReplyPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x00)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character list reply")
	END_COMMON_PACKET_DECL()

	int GetCharCount();
	CStringA GetCharName(int idx);
	WORD GetCharLevel(int idx);
	void SetCharLevel(int idx, WORD wLevel);
	int FindChar(const char* pszName);
};



/**
 * \brief 
 *
 * C3 06 24 22 00 A9
 *              ^ ^
 *             Item ID
 */
class CPickItemPacket
	: public CPacket
{
public:
	CPickItemPacket(WORD wId);

	BEGIN_COMMON_PACKET_DECL(CPickItemPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x22)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Pick item")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 */
class CWarpReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CWarpReplyPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x1C)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Warp reply")
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 6) ? pPacket[6] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 7) ? pPacket[7] : 0; }
	BYTE GetMap(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 5) ? pPacket[5] : 0; }
};


/**
 * \brief 
 *
 * C3 19 25 F3 04 8F 7B 00 00 67 00 8E 00 FC 00 2A 00 00 00 00 00 00 0C D3 34
 *                 ^ ^
 *                 X Y
 *
 */
class CCharRespawnPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharRespawnPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0xF3, 0x04)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character respawn")
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 5) ? pPacket[5] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 6) ? pPacket[6] : 0; }
};



/**
 * \brief 
 *
 *  Season 3 -> 
 *  C3 3D 03 F3 03 0F 0F 07 00 9A 24 3D 02 14 A7 42 02 00 00 38 02 F0 00 19 00 0A 00 9E 01 9E 01 60 00 60 00 6B 07 6B 07 4B 00 96 00 12 00 6D 97 4B 00 03 00 00 00 20 00 00 00 00 00 20 00
 *                  ^  ^        ^--exp---^ ^_exp nxt-^  ^_^   ^_^   ^_^   ^_^   ^_^
 *                  X  Y                                 |  Strength |     |   Energy
 *                                              Points to distribute |  Vitality
 *                                                                Agility
 *
 *  Season 4+ -> 
 *  C3 45 04 F3 03 C0 40 08 00 00 00 00 00 4C E4 C1 7A 00 00 00 00 4E 5A 47 10 03 00 FA 00 91 01 65 00 09 0C 43 02 43 02 2E 01 C8 1A 75 22 75 22 AC 01 58 03 18 00 01 BD 91 00 03 00 D0 07 65 00 00 00 00 00 65 00
 *                  ^  ^        ^---  experience ---^   ^- exp to next lvl -^   ^_^   ^_^   ^_^   ^_^   ^_^
 *                  X  Y                                                         |  Strength |     |   Energy
 *                                                                      Points to distribute |  Vitality
 *                                                                                        Agility
 */
class CCharStatsPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharStatsPacket)
		PACKET_PATT5(0xC3, 0x00, 0x00, 0xF3, 0x03)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character stats")
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 5) ? pPacket[5] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return (pPacket && pPacket[1] > 6) ? pPacket[6] : 0; }

	void SetAgility(WORD wAgility)
	{
		BYTE* buf = GetDecryptedPacket(); 
		int offs = 21;

		if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
			offs +=	8;

		if (buf && buf[1] > 30) 
			*((WORD*)(buf+offs)) = wAgility; 
	}

	void SetEnergy(WORD wEnergy)
	{
		BYTE* buf = GetDecryptedPacket(); 
		int offs = 25;

		if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
			offs +=	8;

		if (buf && buf[1] > 30) 
			*((WORD*)(buf+offs)) = wEnergy; 
	}

	void SetVitality(WORD wStat)
	{
		BYTE* buf = GetDecryptedPacket(); 
		int offs = 23;

		if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
			offs +=	8;

		if (buf && buf[1] > 30) 
			*((WORD*)(buf+offs)) = wStat; 
	}

	void SetStrength(WORD wStat)
	{
		BYTE* buf = GetDecryptedPacket(); 
		int offs = 19;

		if ((CPacketType::GetVersion() & 0xFF000000) == GTYPE_S4)
			offs +=	8;

		if (buf && buf[1] > 30) 
			*((WORD*)(buf+offs)) = wStat; 
	}
};




/**
 * \brief Update position server-to-client packet
 * Update Position STC ->>>> versions prior to 104
 * C1 07 15 1C 32 28 E4 
 *            ^ ^  ^  ^
 *            ^ ^  ^  Y position
 *             ^   X position
 *         Character ID
 *
 * Update Position STC ->>>> versions 104 and above
 * C1 07 DF 19 4B 30 84
 *           ^ ^  ^  ^
 *           ^ ^  ^  Y position
 *            ^   X position
 *        Character ID
 *
 */
class CUpdatePosSTCPacket
	: public CPacket
{
public:
	CUpdatePosSTCPacket(WORD wId, BYTE x, BYTE y);

	static CStringA DescribePacket(CPacket& pkt)
	{
		CUpdatePosSTCPacket pkt2(pkt);

		CStringA str;
		str.Format("       Id: 0x%02X\n       x: %d, y: %d", pkt2.GetId(), pkt2.GetX(), pkt2.GetY());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CUpdatePosSTCPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x07, 0x15)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x07, 0xD6)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x07, 0xDF)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x07, 0xD0)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x07, 0xDC)

			PACKET_PATT_V3(		   0, 0xC1, 0x07, 0x15)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Update position STC")
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[5] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[6] : 0; }
	WORD GetId() { BYTE* pPacket = AnyBuffer(); return pPacket ? (WORD)((pPacket[3] << 8) | pPacket[4]) : 0; }
};


/**
 * \brief Update position client-to-server packet
 *
 * Update Position CTS ->>>> versions 104 and above
 * C1 06 D3 30 84 00
 *           ^  ^
 *           ^  Y Position
 *           X position
 */
class CUpdatePosCTSPacket
	: public CPacket
{
public:
	CUpdatePosCTSPacket(BYTE x, BYTE y, BYTE dir = 0);

	static CStringA DescribePacket(CPacket& pkt)
	{
		CUpdatePosCTSPacket pkt2(pkt);

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       x: %d, y: %d\n       dir: %s", 
			pkt2.GetX(), pkt2.GetY(), dirs[pkt2.GetDir()]);

		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CUpdatePosCTSPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x06, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x06, 0x1D)			
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x06, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x06, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x06, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x06, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Update position CTS")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	BYTE GetDir(){ BYTE* pPacket = AnyBuffer(); return pPacket ? ((pPacket[5] >> 4) % 8) : 0; }
};



/**
 * \brief 
 */
class CLevelUpPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CLevelUpPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF3, 0x05)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Character level-up")
	END_COMMON_PACKET_DECL()

	BYTE GetLevel(){ BYTE* pPacket = AnyBuffer(); return (WORD)(BYTE)pPacket[4] | ((WORD)pPacket[5] << 8); }
};



/**
 * \brief 
 *
 * C3 08 0E 19 00 11 02 8A <-  season 4
 *                ^   ^ ^
 *              Skill  |
 *                   Target
 * 
 * C3 09 0E 19 11 02 8A 00 00 <- season 3
 *              ^  ^ ^
 *            skill |
 *                target
 *
 * C3 09 0E 19 00 11 00 02 8A <-  season 6.2
 *                 ^     ^ ^
 *               Skill    |
 *                      Target
 *
 */
class CSingleSkillPacket
	: public CPacket
{
public:
	CSingleSkillPacket(WORD wSkill, WORD wTarget, int fInjected = 1);

	BEGIN_COMMON_PACKET_DECL(CSingleSkillPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V4(PTYPE_ENG, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_JPN, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_KOR, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_CHS, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_VTM, 0xC3, 0x00, 0x00, 0x19)

			PACKET_PATT_V4(		   0, 0xC3, 0x00, 0x00, 0x19)
		END_PACKET_PATT()
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Single target skill")
	END_COMMON_PACKET_DECL()

public:
	WORD GetSkill();
	static DWORD GetVersion();

	static void InitStatic(CPacket& pkt);

	static void Load();
	static void Save();

private:
	static bool m_fInitStatic;
	static BYTE m_sAttackPkt[16];
};



/**
 * \brief 
 *
 * C3 0B 12 19 00 3D 2F 11 AF 14 00
 *                 ^  ^-^   ^_^
 *               Skill |   Victim 
 *                   Attacker
 */
class CSkillUsedPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CSkillUsedPacket pkt2(pkt);

		CStringA str;
		str.Format("       Attacker: 0x%04X\n       Target: 0x%04X", pkt2.GetAttacker(), pkt2.GetTarget());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CSkillUsedPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V4(PTYPE_ENG, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_JPN, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_KOR, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_CHS, 0xC3, 0x00, 0x00, 0x19)
			PACKET_PATT_V4(PTYPE_VTM, 0xC3, 0x00, 0x00, 0x19)

			PACKET_PATT_V4(		   0, 0xC3, 0x00, 0x00, 0x19)
		END_PACKET_PATT()
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Skill attacked")
	END_COMMON_PACKET_DECL()

	WORD GetTarget();
	WORD GetAttacker();
};


/**
 * \brief 
 *
 * Season 3
 * C3 0E 07 1E 3E D1 5F 2C 98 89 FF FF 00 05 
 *              ^  ^ ^  ^         ^ ^        
 * 			    |  X Y  |       Target
 * 			    |      rot?
 * 			  skill
 *
 * Season 4
 * C3 0E B6 1E 00 D6 6A D7 DD 00 00 07 7A 00
 *              ^-^   ^ ^  ^         ^ ^
 * 				 |    X Y  |        target
 * 				 |        rot?
 *             skill
 *
 * Season 6.2
 * C3 0E 70 1E 32 34 00 26 DB 00 00 00 FF FF
 *              ^ ^   ^-^  ^            ^ ^
 *              X Y  skill |          target
 *                  to be investigated
 */
class CMassiveSkillPacket
	: public CPacket
{
public:
	CMassiveSkillPacket(WORD wSkill, BYTE x, BYTE y, BYTE rot = 0, WORD wTarget = 0, int fInjected = 1);

	BEGIN_COMMON_PACKET_DECL(CMassiveSkillPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x1E)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Massive skill")
	END_COMMON_PACKET_DECL()

	WORD GetTarget();
	WORD GetSkill();
};



/**
 * \brief 
 *
 * C3 0B 12 19 00 3D 2F 11 AF 14 00
 *                 ^  ^-^   ^ ^   ^
 *               Skill |    X Y  Rot
 *                   Attacker
 */
class CMassiveSkillUsedPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CMassiveSkillUsedPacket pkt2(pkt);

		CStringA str;
		str.Format("       Attacker: 0x%04X\n       Target: %d, %d", pkt2.GetAttacker(), pkt2.GetTargetX(), pkt2.GetTargetY());

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CMassiveSkillUsedPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V4(PTYPE_ENG, 0xC3, 0x00, 0x00, 0x1E)
			PACKET_PATT_V4(PTYPE_JPN, 0xC3, 0x00, 0x00, 0x1E)
			PACKET_PATT_V4(PTYPE_KOR, 0xC3, 0x00, 0x00, 0x1E)
			PACKET_PATT_V4(PTYPE_CHS, 0xC3, 0x00, 0x00, 0x1E)
			PACKET_PATT_V4(PTYPE_VTM, 0xC3, 0x00, 0x00, 0x1E)

			PACKET_PATT_V4(		   0, 0xC3, 0x00, 0x00, 0x1E)
		END_PACKET_PATT()
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Massive skill used")
	END_COMMON_PACKET_DECL()

	BYTE GetTargetX();
	BYTE GetTargetY();
	WORD GetAttacker();
};



/**
 * \brief 
 *
 * C1 10 BF 0A 00 D7 | 1E 2F | 03 00 | 20 2F | 20 2F | EF 07
 *                ^     ^_^     ^_^     ^_^     ^_^     ^_^  
 *                |   Attacker   |     Target1 Target2 Target2 
 *              Skill  (LSBF)  Targets Count (LSBF)
 *
 *  Note: Attacker ID, Targets Count and Target IDs are received with least significant byte first
 */
class CMassiveSkillEffectPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CMassiveSkillEffectPacket)
		PACKET_PATT3(0xC1, 0x00, 0xBF)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Massive skill effect")
	END_COMMON_PACKET_DECL()

//	WORD GetAttacker(int idx);
//	WORD GetTargetsCount(int idx);
//	WORD GetTarget(int idx);
};



/**
 * \brief 
 *
 * Direction: SEND
 *
 * C1 07 D7 78 07 06 A1
 *           ^  ^  ^------ Target ID
 *           | Rotation
 *        Animation
 *
 * C1 08 DC 04 ED 78 05 59
 *           ^ ^   ^  ^
 *          Target | Rotation
 *               Attack animation
 *
 */
class CNormalAttackPacket
	: public CPacket
{
public:
	CNormalAttackPacket(WORD wId, BYTE rot = 0x03);

	static CStringA DescribePacket(CPacket& pkt)
	{
		CNormalAttackPacket pkt2(pkt);

		CStringA str;
		str.Format("       Target: 0x%04X", pkt2.GetId());

		return str;
	}

	static void InitStatic(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CNormalAttackPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x00, 0x11)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x00, 0xDC)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x00, 0xD7)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x00, 0xD9)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x00, 0x15)

			PACKET_PATT_V3(		   0, 0xC1, 0x00, 0x11)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Normal attack")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	WORD GetId(){ BYTE* buf = AnyBuffer(); return (((WORD)(buf[3] & 0x7F) << 8) | buf[4]); }

	static void Load();
	static void Save();

private:
	static void FindOffsets();


private:
	static bool m_fInitStatic;
	static BYTE m_sAttackPkt[16];
	static int m_sOffsetAnim;
	static int m_sOffsetID;
};



/**
 * \brief Character move client-to-server packet
 *
 * Char Move CTS
 * C1 07 D4 30 84 00 00
 *           ^  ^  ^  ^
 *           ^  ^  ^  Y Direction offs
 *           ^  ^  X Direction offs
 *           ^  Y Position
 *           X position
 */
class CCharMoveCTSPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharMoveCTSPacket pkt2(pkt);

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       x: %d, y: %d\n       dx: %d\n       dir: %s", 
				pkt2.GetX(), pkt2.GetY(), pkt2.GetDX(), dirs[pkt2.GetDir()]);
		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CCharMoveCTSPacket)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x07, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x07, 0x1D)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x07, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x07, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x07, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x07, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char Move CTS")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	BYTE GetDir(){ BYTE* pPacket = AnyBuffer(); return pPacket ? ((pPacket[5] >> 4) % 8) : 0; }

	BYTE GetDX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? (pPacket[5] & 0xF) : 0; }
};


/**
 * \brief Character rotate client-to-server packet
 *
 * Char Rotate CTS
 * C1 05 18 01 7A
 *           ^  
 *           Direction
 */
class CCharRotateCTSPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharRotateCTSPacket)
		PACKET_PATT3(0xC1, 0x05, 0x18)
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char rotate CTS")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()
};



/**
 * \brief Character move x2 client-to-server packet
 *
 * Char Move CTS
 * C1 08 1D C5 1E BF 31 52
 *           ^  ^  
 *           ^  ^  
 *           ^  ^  
 *           ^  Y Position
 *           X position
 */
class CCharMoveCTSx2Packet
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharMoveCTSPacket pkt2(pkt);

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       x: %d, y: %d, dx:  %d, dir: %s", 
				pkt2.GetX(), pkt2.GetY(), pkt2.GetDX(), dirs[pkt2.GetDir()]);
		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CCharMoveCTSx2Packet)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x08, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x08, 0x1D)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x08, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x08, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x08, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x08, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char Move CTS x2")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	BYTE GetDX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? (pPacket[5] & 0xF) : 0; }
	BYTE GetDir(){ BYTE* pPacket = AnyBuffer(); return pPacket ? ((pPacket[5] >> 4) % 8) : 0; }
};



/**
 * \brief Character move x3 client-to-server packet
 *
 * Char Move CTS
 * C1 09 1D C4 0A AE 20 42 70 
 *           ^  ^  
 *           ^  ^  
 *           ^  ^  
 *           ^  Y Position
 *           X position
 *
 */
class CCharMoveCTSx3Packet
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharMoveCTSPacket pkt2(pkt);

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       x: %d, y: %d, dir: %s", 
			pkt2.GetX(), pkt2.GetY(), dirs[pkt2.GetDir()]);
		return str;
	}



	BEGIN_COMMON_PACKET_DECL(CCharMoveCTSx3Packet)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x09, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x09, 0x1D)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x09, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x09, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x09, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x09, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char Move CTS x3")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	BYTE GetDX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? (pPacket[5] & 0xF) : 0; }
	BYTE GetDir(){ BYTE* pPacket = AnyBuffer(); return pPacket ? ((pPacket[5] >> 4) % 8) : 0; }
};




/**
 * \brief Character move x4 client-to-server packet
 *
 * Char Move CTS
 * C1 0A 1D FF 3C E9 01 05 51 99 
 *           ^  ^  
 *           ^  ^  
 *           ^  ^  
 *           ^  Y Position
 *           X position
 */
class CCharMoveCTSx4Packet
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCharMoveCTSPacket pkt2(pkt);

		const char* dirs[] = { "W", "SW", "S", "SE", "E", "NE", "N", "NW" };

		CStringA str;
		str.Format("       x: %d, y: %d, dir: %s", 
			pkt2.GetX(), pkt2.GetY(), dirs[pkt2.GetDir()]);
		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CCharMoveCTSx4Packet)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x0A, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x0A, 0x1D)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x0A, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x0A, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x0A, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x0A, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char Move CTS x4")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetX(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	BYTE GetY(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	BYTE GetDir(){ BYTE* pPacket = AnyBuffer(); return pPacket ? ((pPacket[5] >> 4) % 8) : 0; }
};



/**
 * \brief Character move x5 client-to-server packet
 *
 * Char Move CTS
 * C1 0B 1D FF 3C E9 01 05 51 99 00
 *           ^  ^  
 *           ^  ^  
 *           ^  ^  
 *           ^  Y Position
 *           X position
 */
class CCharMoveCTSx5Packet
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CCharMoveCTSx5Packet)
		BEGIN_PACKET_PATT()
			PACKET_PATT_V3(PTYPE_ENG, 0xC1, 0x0B, 0xD4)
			PACKET_PATT_V3(PTYPE_JPN, 0xC1, 0x0B, 0x1D)
			PACKET_PATT_V3(PTYPE_KOR, 0xC1, 0x0B, 0xD3)
			PACKET_PATT_V3(PTYPE_CHS, 0xC1, 0x0B, 0xD7)
			PACKET_PATT_V3(PTYPE_VTM, 0xC1, 0x0B, 0xD9)

			PACKET_PATT_V3(		   0, 0xC1, 0x0B, 0xD4)
		END_PACKET_PATT()
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Char Move CTS x5")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()
};



/**
 * \brief 
 *
 * C2 00 4D 12 
 *             02 - number of players 
 * 1F C7 - player ID 
 * C9 47 00 02 FF 33 33 3F 0D B6 C3 00 00 00 A0 F0 00 00 00 00 41 72 65 73 00 00 00 00 00 00 C9 47 73 00 
 *                                                              ^------ name -------------^   ^ ^  ^
 * 20 34                                                                                      X Y Dir
 * D4 47 20 03 0E 86 25 19 01 82 C0 00 2F 00 20 00 01 00 00 00 50 72 6F 42 4B 00 00 00 00 00 D4 47 23 00 
 *
 *
 * Version 0.99
 * C2 00 55 12 
 *             02 - number of players
 * 19 B7 - player ID
 * D5 2C 80 4A A5 AA AA AF 01 86 00 FD AC 00 01 00 00 00 00 00 00 00 4D 6F 6F 6E 57 61 6C 6B 65 72 D5 2C 23 00 00 00 
 * 1B 19 
 * D6 30 80 49 C0 FF FF FD 00 00 00 F8 05 00 00 00 00 00 00 00 00 00 6C 65 6C 71 74 69 42 6F 67 69 D6 30 31 00 00 00
 */
class CMeetPlayerPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CMeetPlayerPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x12)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Meet player")
	END_COMMON_PACKET_DECL()

	int GetPlayerCount(){ BYTE* pPacket = GetDecryptedPacket(); return pPacket ? pPacket[4] : 0; }
	CStringA GetPlayerName(int idx);
	WORD GetPlayerId(int idx);
	bool GetPos(int idx, BYTE& x, BYTE& y);

	int GetPlayerOffset(BYTE* pPacket, int idx, int exbPos, int iSize);
};



/**  
 * \brief 
 *
 * C2 00 2D 45  01  2F 9B D7 2F 02 24 50 75 72 65 41 45 00 00 00 00 D7 2F 33 50 13 07 DD DD D4 1F FF C0 01 01 00 80 80 00 00 60 00 02 28 57
 *               ^   ^-^   ^  ^  ^     ^--------------------------^
 *               |    |    X  Y Dir         Player Name
 *               | Player ID
 *             Count
 */ 
class CMeetDisguisedPlayerPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt);

	BEGIN_COMMON_PACKET_DECL(CMeetDisguisedPlayerPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x45)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Meet disguised player")
	END_COMMON_PACKET_DECL()

	int GetPlayerCount(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	
	CStringA GetPlayerName(int idx);
	WORD GetPlayerId(int idx);
	bool GetPos(int idx, BYTE& x, BYTE& y);
};




/**
 * \brief 
 */
class CForgetObjectPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CForgetObjectPacket pkt2(pkt);
	
		int iCount = pkt2.GetObjectCount();

		CStringA str;
		str.Format("       Count of Objects: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			CStringA str2;
			str2.Format("\n       [%d] 0x%04X", i, pkt2.GetObjectId(i));
			str += str2;
		}

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CForgetObjectPacket)
		PACKET_PATT3(0xC1, 0x00, 0x14)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Forget object")
	END_COMMON_PACKET_DECL()

	int GetObjectCount(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[3] : 0; }
	WORD GetObjectId(int idx);
};


/**
 * \brief 
 */
class CForgetItemPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CForgetItemPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x21)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Forget item(s)")
	END_COMMON_PACKET_DECL()

	int GetItemCount(){ BYTE* pPacket = AnyBuffer(); return pPacket ? pPacket[4] : 0; }
	WORD GetItemId(int idx);
};


/**
 * \brief 
 *
 * Direction: RECV
 *
 * C1 09 17 04 EA 00 00 21 27 --> Season 4
 *           ^ ^         ^ ^
 *          Victim      Killer
 *
 * C1 08 17 04 EA 00 21 27 --> Season 3
 *           ^ ^      ^ ^
 *          Victim   Killer
 */
class CObjectDeathPacket
	: public CPacket
{
public:
	CObjectDeathPacket(WORD wId);

	static CStringA DescribePacket(CPacket& pkt)
	{
		CObjectDeathPacket pkt2(pkt);

		CStringA str;
		str.Format("       Victim: 0x%04X\n       Killer: 0x%04X", pkt2.GetObjectId(), pkt2.GetKillerId());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CObjectDeathPacket)
		PACKET_PATT3(0xC1, 0x00, 0x17)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Object death")
	END_COMMON_PACKET_DECL()

	WORD GetObjectId(){ BYTE* pPacket = AnyBuffer(); return pPacket ? (((WORD)(pPacket[3] & 0x7F) << 8) | (WORD)pPacket[4]) : 0; }
	WORD GetKillerId()
	{ 
		BYTE* pPacket = AnyBuffer(); 
		int offs = pPacket[1] - 8;

		return pPacket ? (((WORD)(pPacket[6+offs] & 0x7F) << 8) | (WORD)pPacket[7+offs]) : 0; 
	}
};


/**
 * \brief 
 */
class CWingMix3lvlPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CWingMix3lvlPacket)
		PACKET_PATT4(0xC1, 0x04, 0x86, 0x28)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Mix 3lvl wings")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CFeatherMixPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CFeatherMixPacket)
		PACKET_PATT4(0xC1, 0x04, 0x86, 0x29)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Mix feather of condor")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CChaosMachineClosePacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CChaosMachineClosePacket)
		PACKET_PATT3(0xC1, 0x03, 0x87)
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Close chaos machine")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 */
class CChaosMixSuccessPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CChaosMixSuccessPacket)
		PACKET_PATT4(0xC1, 0x00, 0x86, 0x01)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Chaos machine mix succeeded")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief 
 *
 * Season 3 ->
 * C4 00 8B 01 F3 10 | 10 | 00 0D B8 30 00 00 00 00 01 0D B8 31 00 00 00 00 02 09 38 3C 00 00 70 00 03 09 38 3C 00 00 80 00 04 09 38 3C 00 00 90 00 05 09 38 3C 00 00 A0 00 06 09 38 3C 00 00 B0 00 0C 06 00 47 00 00 E0 00 0D 0E 00 01 00 00 E0 00 0E 0F 00 00 00 00 C0 00 0F 0D 00 01 00 00 E0 00 10 01 00 FF 00 00 D0 00 11 0D 00 01 00 00 E0 00 15 0F 00 00 00 00 C0 00 16 0F 00 00 00 00 C0 00 59 06 20 32 00 00 E0 00 00 00 3C 00
 *                      ^    ^  ^----------------^                    ^  ^----------------^
 *                      |    |     Item Code                          |      Item Code
 *                      |  Inventory position                    Inventory Position
 *                    Item count
 *
 * Season 4+ ->
 * C4 01 84 01 F3 10 | 1D | 00 12 4C FF 00 00 50 00 FF FF FF FF FF | 01 15 98 3F 00 00 50 00 FF FF FF FF FF  .....
 *                      ^    ^  ^----------------^                    ^  ^----------------^
 *                      |    |     Item Code                          |      Item Code
 *                      |  Inventory position                    Inventory Position
 *                    Item count
 */
class CInitInventoryPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CInitInventoryPacket& pkt2 = (CInitInventoryPacket&)pkt;

		CStringA str;
		int iCount = pkt2.GetCount();

		str.Format("       Count of items: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			CStringA str2;
			WORD wType = pkt2.GetItemType(i);
			str2.Format("\r\n         [%d] -> Position: 0x%02X, Type: %d %d %d (0x%04X), Code: %s", i, pkt2.GetPosition(i), 
				ITEM_GROUP(wType), ITEM_SUBGR(wType), ITEM_LEVEL(wType), wType, CBufferUtil::BufferToHex(pkt2.GetItemData(i), 7));

			str += str2;
		}

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CInitInventoryPacket)
		PACKET_PATT6(0xC4, 0x00, 0x00, 0x00, 0xF3, 0x10)
		PACKET_MASK6(0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Initialize inventory")
	END_COMMON_PACKET_DECL()

	int GetCount(){ BYTE* pPacket = AnyBuffer(); return !pPacket ? 0 : (int)pPacket[6]; }
	BYTE GetPosition(int idx);
	BYTE* GetItemData(int idx);

	WORD GetItemType(int idx)
	{
		BYTE* pPacket = GetItemData(idx);
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 0);
	}

	int GetDataSize()
	{
		int iSize = GetDecryptedLen() - 7;
		int iCount = GetCount();

		if (iCount <= 0 || iSize <= 0)
			return 0;

		return iSize/iCount;
	}
};


/**
 * \brief 
 * 
 * 0xC1, 0x05, 0x23, 0x01, 0x0C
 *                     ^     ^--..---- item positions
 *                  Number of items
 */
class CRemoveFromInventoryPacket
	: public CPacket
{
public:
	CRemoveFromInventoryPacket(BYTE pos)
	{
		BYTE buff[] = {0xC1, 0x05, 0x23, 0x01, pos};
		SetDecryptedPacket(buff, sizeof(buff));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL(CRemoveFromInventoryPacket)
		PACKET_PATT3(0xC1, 0x00, 0x23)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Remove from inventory")
	END_COMMON_PACKET_DECL()

	int GetCount(){ BYTE* pPacket = AnyBuffer(); return !pPacket ? 0 : (int)pPacket[3]; }
	int GetPosition(int idx);
};




/**  
 * \brief 
 *
 * C2 | 01 B3 | 31 | 00 21 | 00 00 00 C8 00 00 C0 00 | FF FF FF FF FF | 03 | 01 00 C8 00 00 C0 00 | FF FF FF FF FF || ...
 *                    ^_^     ^  ^----------------^      Some Junk       ^    ^----------------^      Some Junk
 *                 Item Count |      Item Code                           |        Item Code           
 *                        Item Position                              Item Position
 *
 * Season 3 ->
 * C2 | 01 CE | 31 | 00 39 | 00 06 8C 45 00 00 40 00 | 02 04 CC 39 00 00 40 00 | 04 02 00 FF 00 00 D0 00 | 05 0F 00 00 00 00 C0 00 
 *                    ^_^     ^  ^----------------^     ^  ^----------------^    
 *                 Item Count |      Item Code          |      Item Code           
 *                        Item Position            Item Position
 *
 */
class CVaultContentsPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CVaultContentsPacket& pkt2 = (CVaultContentsPacket&)pkt;

		CStringA str;
		int iCount = pkt2.GetCount();

		str.Format("       Count of items: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			CStringA str2;
			WORD wType = pkt2.GetItemType(i);
			str2.Format("\r\n         [%d] -> Position: 0x%02X, Type: %d %d %d (0x%04X), Code: %s", i, pkt2.GetPosition(i), 
				ITEM_GROUP(wType), ITEM_SUBGR(wType), ITEM_LEVEL(wType), wType, CBufferUtil::BufferToHex(pkt2.GetItemData(i), 7));

			str += str2;
		}

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CVaultContentsPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x31)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Vault contents")
	END_COMMON_PACKET_DECL()


	int GetCount(){ BYTE* pPacket = AnyBuffer(); return !pPacket ? 0 : (int)(((WORD)pPacket[4] << 8) | pPacket[5]); }
	BYTE GetPosition(int idx);
	BYTE* GetItemData(int idx);

	WORD GetItemType(int idx)
	{
		BYTE* pPacket = GetItemData(idx);
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 0);
	}

	int GetDataSize()
	{
		int iSize = GetDecryptedLen() - DATA_OFFS;
		int iCount = GetCount();

		if (iCount <= 0 || iSize <= 0)
			return 0;

		return iSize/iCount;
	}

	static const int DATA_OFFS = 6;
};



/**  
 * \brief 
 *
 * 0xC3, 0x07, 0x1E, 0x26, 0x0C, 0x00, 0x00
 *                          ^       ^-- junk 
 *                        Inventory Position
 */
class CUseItemPacket
	: public CPacket
{
public:
	CUseItemPacket(BYTE bPos)
	{
		BYTE buf[] = {0xC3, 0x07, 0x00, 0x26, bPos, 0x00, 0x00};

		SetDecryptedPacket(buf, sizeof(buf));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL(CUseItemPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x26)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Use item")
		PACKET_XORP2(4, 3)
	END_COMMON_PACKET_DECL()

	BYTE GetInvPos()
	{
		BYTE* pPacket = AnyBuffer();
		return (pPacket == 0) ? 0 : pPacket[4];
	}
};



/**
 * \brief Connect server hello packet
 *
 */
class CConnectServHelloPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CConnectServHelloPacket)
		PACKET_PATT4(0xC1, 0x04, 0x00, 0x01)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Connect server hello")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief Server list request
 *
 */
class CServerListReqPacket
	: public CPacket
{
public:
	CServerListReqPacket()
	{
		BYTE buf[] = {0xC1, 0x04, 0xF4, 0x06};

		SetDecryptedPacket(buf, sizeof(buf));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL_NOCONSTR(CServerListReqPacket)
		PACKET_PATT4(0xC1, 0x04, 0xF4, 0x06)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Server list request")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief Server list reply
 *
 * C2 00 1F F4 06
 *
 */
class CServerListReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CServerListReplyPacket)
		PACKET_PATT5(0xC2, 0x00, 0x00, 0xF4, 0x06)
		PACKET_MASK5(0xFF, 0x00, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Server list reply")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief Server info request
 *
 */
class CServerInfoReqPacket
	: public CPacket
{
public:
	CServerInfoReqPacket()
	{
		BYTE buf[] = {0xC1, 0x06, 0xF4, 0x03, 0x01, 0x00 };

		SetDecryptedPacket(buf, sizeof(buf));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL_NOCONSTR(CServerInfoReqPacket)
		PACKET_PATT4(0xC1, 0x06, 0xF4, 0x03)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Server info request")
	END_COMMON_PACKET_DECL()
};


/**
 * \brief Server info reply
 *
 */
class CServerInfoReplyPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CServerInfoReplyPacket)
		PACKET_PATT4(0xC1, 0x00, 0xF4, 0x03)
		PACKET_MASK4(0xFF, 0x00, 0xFF, 0xFF)
		PACKET_DESCR("Server info reply")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief Unknown send C3 packet
 *
 */
class CUnkSendC3Packet
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CUnkSendC3Packet)
		PACKET_PATT2(0xC3, 0x00)
		PACKET_MASK2(0xFF, 0x00)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Unknown packet")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief Unknown send C1 packet
 *
 */
class CUnkSendC1Packet
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CUnkSendC1Packet)
		PACKET_PATT2(0xC1, 0x00)
		PACKET_MASK2(0xFF, 0x00)
		PACKET_XORP2(3, 3)
		PACKET_DESCR("Unknown packet")
	END_COMMON_PACKET_DECL()
};



/**
 * \brief Item used
 *
 * C1 06 2A 0C 5E 01
 *          ^  ^   ^---- used amount
 *          |  Remaining Quantity
 *         Inventory Position
 */
class CItemUsedPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CItemUsedPacket pkt2(pkt);

		CStringA str;
		str.Format("       Inv. position: 0x%02X\n       Amount: %d\n       Left items: %d", pkt2.GetInvPos(), pkt2.GetUsedAmount(), pkt2.GetRemaining());
		
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CItemUsedPacket)
		PACKET_PATT3(0xC1, 0x06, 0x2A)
		PACKET_MASK3(0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Item used")
	END_COMMON_PACKET_DECL()

	BYTE GetInvPos() { BYTE* buf = AnyBuffer(); return buf ? buf[3] : 0; } 
	BYTE GetUsedAmount() { BYTE* buf = AnyBuffer(); return buf ? buf[5] : 0; } 
	BYTE GetRemaining() { BYTE* buf = AnyBuffer(); return buf ? buf[4] : 0; } 
};



/**  
 * \brief 
 *
 *   C1 05 28 0D 01
 *             ^  ^
 *             |  Storage Type: 1 - inventory
 *           Item Position
 *                
 */
class CItemLostPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CItemLostPacket)
		PACKET_PATT3(0xC1, 0x00, 0x28)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Item lost")
	END_COMMON_PACKET_DECL()

	BYTE GetPos(){ BYTE* buf = AnyBuffer(); return buf ? buf[3] : 0; }
	BYTE GetStgType(){ BYTE* buf = AnyBuffer(); return buf ? buf[4] : 0; }
};


/**
 * \brief Set max mana and A/G points
 *
 * C1 08 27 FE 01 8A 01 55
 *              ^ ^   ^ ^
 *               |     |
 *               |  A/G Points
 *          Mana Points
 */
class CSetMaxManaAGPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CSetMaxManaAGPacket pkt2(pkt);

		CStringA str;
		str.Format("       Life: %d\n       Mana: %d", pkt2.GetMana(), pkt2.GetAG());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CSetMaxManaAGPacket)
		PACKET_PATT4(0xC1, 0x08, 0x27, 0xFE)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Set max Mana & A/G points")
	END_COMMON_PACKET_DECL()

	WORD GetMana() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; } 
	WORD GetAG() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[6] << 8) | buf[7]) : 0; } 
};



/**
 * \brief Update mana and A/G points
 *
 * C1 08 27 FF 01 8A 01 B6
 *              ^ ^   ^ ^
 *               |     |
 *               |  AG Points
 *          Mana Points
 */
class CUpdateManaAGPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CUpdateManaAGPacket pkt2(pkt);

		CStringA str;
		str.Format("       Mana: %d\n       A/G: %d", pkt2.GetMana(), pkt2.GetAG());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CUpdateManaAGPacket)
		PACKET_PATT4(0xC1, 0x08, 0x27, 0xFF)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Update Mana & A/G")
	END_COMMON_PACKET_DECL()

	WORD GetMana() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; } 
	WORD GetAG() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[6] << 8) | buf[7]) : 0; } 

	void SetAG(WORD wAg){ BYTE* buf = GetDecryptedPacket(); if (buf) { buf[6] = HIBYTE(wAg); buf[7] = LOBYTE(wAg); } }
};




/**
 * \brief Update life and SD points
 *
 * C1 09 26 FE 02 77 00 0D 39
 *              ^ ^      ^ ^
 *               |        |
 *               |      SD Points
 *          Life Points
 */
class CUpdateLifeSDPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CUpdateLifeSDPacket pkt2(pkt);

		CStringA str;
		str.Format("       Life: %d\n       SD: %d", pkt2.GetLife(), pkt2.GetSD());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CUpdateLifeSDPacket)
		PACKET_PATT4(0xC1, 0x09, 0x26, 0xFF)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Update Life & SD")
	END_COMMON_PACKET_DECL()

	WORD GetLife() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; } 
	WORD GetSD() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[7] << 8) | buf[8]) : 0; } 
};



/**
 * \brief Set max life and SD points
 *
 * C1 09 26 FE 02 77 00 0D 39
 *              ^ ^      ^ ^
 *               |        |
 *               |      SD Points
 *          Life Points
 */
class CSetMaxLifeSDPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CSetMaxLifeSDPacket& pkt2 = (CSetMaxLifeSDPacket&)pkt;

		CStringA str;
		str.Format("       Life: %d\n       SD: %d", pkt2.GetLife(), pkt2.GetSD());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CSetMaxLifeSDPacket)
		PACKET_PATT4(0xC1, 0x09, 0x26, 0xFE)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Set max Life & SD points")
	END_COMMON_PACKET_DECL()

	WORD GetLife() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; } 
	WORD GetSD() { BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[7] << 8) | buf[8]) : 0; } 
};



/**
 * \brief Repair inventory packet
 *
 * C3 06 00 34 06 01
 *              ^
 *              inventory position
 */
class CRepairInvPacket
	: public CPacket
{
public:
	CRepairInvPacket(BYTE pos)
	{
		BYTE buf[] = {0xC3, 0x06, 0x00, 0x34, pos, 0x01};

		SetDecryptedPacket(buf, sizeof(buf));
		SetType(Type());
		SetInjected();
	}

	BEGIN_COMMON_PACKET_DECL(CRepairInvPacket)
		PACKET_PATT4(0xC3, 0x06, 0x00, 0x34)
		PACKET_MASK4(0xFF, 0xFF, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Repair inventory")
	END_COMMON_PACKET_DECL()
};


/**  
 * \brief Trade reply packet
 *
 * C1 14 37 01 50 72 61 73 34 6F 00 00 00 00 40 01 00 00 00 00
 *              ^_________________________^   ^_^
 *                       Name                Level
 */
class CTradeReplyPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CTradeReplyPacket& pkt2 = (CTradeReplyPacket&)pkt;

		char szName[16] = {0};
		pkt2.GetCharName(szName);

		CStringA str;
		str.Format("       Character: %s\n       Level: %d", szName, pkt2.GetLevel());
		return str;
	}


	BEGIN_COMMON_PACKET_DECL(CTradeReplyPacket)
		PACKET_PATT3(0xC1, 0x00, 0x37)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Trade reply")
	END_COMMON_PACKET_DECL()

	bool GetCharName(char* pszChar){ BYTE* buf = AnyBuffer(); if (buf) { memcpy(pszChar, buf+4, 10); pszChar[10] = 0; } return buf != 0; }
	WORD GetLevel(){ BYTE* buf = AnyBuffer(); return buf ? *((WORD*)(buf+14)) : 0; }
};


/**  
 * \brief 
 *
 * C3 06 A0 36 22 95
 *              ^_^
 *            Player ID
 */
class CTradeRequestPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		CTradeRequestPacket& pkt2 = (CTradeRequestPacket&)pkt;

		CStringA str;
		str.Format("       Player Id: 0x%04X", pkt2.GetPlayerId());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CTradeRequestPacket)
		PACKET_PATT4(0xC3, 0x06, 0x00, 0x36)
		PACKET_MASK4(0xFF, 0xFF, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Trade request")
	END_COMMON_PACKET_DECL()

	WORD GetPlayerId(){ BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; }
};



/**  
 * \brief 
 *
 * C1 04 3D 03
 *
 */
class CTradeCanceledPacket
	: public CPacket
{
public:
	BEGIN_COMMON_PACKET_DECL(CTradeCanceledPacket)
		PACKET_PATT4(0xC1, 0x04, 0x3D, 0x03)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Trade canceled")
	END_COMMON_PACKET_DECL()
};



/**  
 * \brief Open Vault request
 *
 * C3 06 0B 30 00 3D
 *              ^_^
 *            NPC Id
 *
 */
class COpenVaultRequestPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		COpenVaultRequestPacket& pkt2 = (COpenVaultRequestPacket&)pkt;

		CStringA str;
		str.Format("       NPC Id: 0x%04X", pkt2.GetNpcId());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(COpenVaultRequestPacket)
		PACKET_PATT4(0xC3, 0x06, 0x00, 0x30)
		PACKET_MASK4(0xFF, 0xFF, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Open vault request")
	END_COMMON_PACKET_DECL()

	WORD GetNpcId(){ BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; }
};




/**  
 * \brief Open Vault request
 *
 * C3 06 0B 31 00 3D
 *              ^_^
 *            NPC Id
 *
 */
class COpenStoreRequestPacket
	: public CPacket
{
public:
	static CStringA DescribePacket(CPacket& pkt)
	{
		COpenStoreRequestPacket& pkt2 = (COpenStoreRequestPacket&)pkt;

		CStringA str;
		str.Format("       NPC Id: 0x%04X", pkt2.GetNpcId());
		return str;
	}

	BEGIN_COMMON_PACKET_DECL(COpenStoreRequestPacket)
		PACKET_PATT4(0xC3, 0x06, 0x00, 0x31)
		PACKET_MASK4(0xFF, 0xFF, 0x00, 0xFF)
		PACKET_XORP2(4, 3)
		PACKET_DESCR("Open store request")
	END_COMMON_PACKET_DECL()

	WORD GetNpcId(){ BYTE* buf = AnyBuffer(); return buf ? (((WORD)buf[4] << 8) | buf[5]) : 0; }
};




							 
/**  
 * \brief Update skill packet
 *
 * 
 * C1 09 F3 11 FE 00 09 3D 00
 *                    ^  ^
 *                    | Skill
 *                    Slot
 * 		 3D - FireBurst 
 *		 3C - Force  
 *		 29 - TwistingSlash
 *		 2B - DeathStab
 *		 13 - FallingSlash
 *		 16 - Cyclone
 *		 14 - Lunge
 *		 11 - EnergyBall
 *		 01 - Poison
 *		 02 - Meteorite
 *		 03 - Lightning
 *		 04 - Fireball
 *		 0B - PowerWave
 *		 07 - Ice
 *		 33 - IceArrow
 *		 D6 - ChainLight
 *		 DB - Sleep
 *		 D6 - DrainLife
 *		 09 - EvilSpirit
 *		 3E - DarkHorse
 * 
 */
class CUpdateSkillPacket
	: public CPacket
{
	BEGIN_COMMON_PACKET_DECL(CUpdateSkillPacket)
		PACKET_PATT5(0xC1, 0x09, 0xF3, 0x11, 0xFE)
		PACKET_MASK5(0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Update skill")
	END_COMMON_PACKET_DECL()
};




/**  
 * \brief Initialize skills
 *
 * C1 24 F3 11 0A 00  00 2C 04  01 43 03  02 44 04  03 45 05  04 46 06  05 47 07  06 29 01  07 30 00  08 16 3E  09 16 3E
 * C1 24 F3 11 0A 00  00 3C 04  01 4A 02  02 43 03  03 44 04  04 45 05  05 46 06  06 47 07  07 48 00  08 3D 05  09 16 3E
 * C1 21 F3 11 09 00  00 3C 04  01 4A 02  02 43 03  03 44 04  04 45 05  05 46 06  06 47 07  07 48 00  08 16 3E
 *              ^_^
 *             Count
 */
class CInitSkillsPacket
	: public CPacket
{
	BEGIN_COMMON_PACKET_DECL(CInitSkillsPacket)
		PACKET_PATT5(0xC1, 0x00, 0xF3, 0x11, 0x00)
		PACKET_MASK5(0xFF, 0x00, 0xFF, 0xFF, 0x80)
		PACKET_DESCR("Init skills")
	END_COMMON_PACKET_DECL()
};



/**  
 * \brief 
 *
 * C1 0B 32 12 0B 10 03 00 00 E0 00
 *           ^  ^----------------^
 *           |       Item Code
 *        Position
 */
class CCreateInvItemPacket
	: public CPacket
{
	static CStringA DescribePacket(CPacket& pkt)
	{
		CCreateInvItemPacket& pkt2 = (CCreateInvItemPacket&)pkt;

		CStringA str;
		WORD wType = pkt2.GetItemType();
		str.Format("         Position: 0x%02X, Type: %d %d %d (0x%04X), Code: %s", pkt2.GetInvPos(), 
			ITEM_GROUP(wType), ITEM_SUBGR(wType), ITEM_LEVEL(wType), wType, CBufferUtil::BufferToHex(pkt2.GetItemData(), 7));

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CCreateInvItemPacket)
		PACKET_PATT3(0xC1, 0x00, 0x32)
		PACKET_MASK3(0xFF, 0x00, 0xFF)
		PACKET_DESCR("Set inventory item")
	END_COMMON_PACKET_DECL()

	BYTE  GetInvPos(){ BYTE* buf = AnyBuffer(); return buf ? buf[3] : 0; }
	BYTE* GetItemData(){ BYTE* buf = AnyBuffer(); return (buf + 4); }
	WORD GetItemType()
	{
		BYTE* pPacket = GetItemData();
		return (pPacket == 0) ? 0 : ITEM_CODE(pPacket, 0);
	}
};


/**  
 * \brief 
 *
 * C3 05 4D 32 36
 *              ^
 *           Position in store
 *
 */
class CBuyStoreItemPacket
	: public CPacket
{
	BEGIN_COMMON_PACKET_DECL(CBuyStoreItemPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x32)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Buy item from store")
		PACKET_XORP2(4, 3)
	END_COMMON_PACKET_DECL()
};



/**  
 * \brief 
 *
 * C3 05 BA 33 1C
 *              ^
 *           Position in inventory
 *
 */
class CSellItemPacket
	: public CPacket
{
public:
	CSellItemPacket(BYTE pos)
	{
		SetType(Type());
		SetInjected();

		BYTE buf[] = {0xC3, 0x06, 0x00, 0x33, pos};
		SetDecryptedPacket(buf, buf[1]);
	}

	BEGIN_COMMON_PACKET_DECL(CSellItemPacket)
		PACKET_PATT4(0xC3, 0x00, 0x00, 0x33)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Sell item to store")
		PACKET_XORP2(4, 3)
	END_COMMON_PACKET_DECL()
};



/**  
 * \brief 
 *
 * C1 05 F3 06 03
 *              ^
 *           Stat to update: 0 - strength, 1 - agility, 2 - vitality, 3 - energy
 *
 */
class CAddStatReqPacket
	: public CPacket
{
public:
	CAddStatReqPacket(BYTE pos)
	{
		SetType(Type());
		SetInjected();

		BYTE buf[] = {0xC1, 0x05, 0xF3, 0x06, pos};
		SetDecryptedPacket(buf, buf[1]);
	}

	BEGIN_COMMON_PACKET_DECL(CAddStatReqPacket)
		PACKET_PATT4(0xC1, 0x05, 0xF3, 0x06)
		PACKET_MASK4(0xFF, 0xFF, 0xFF, 0xFF)
		PACKET_DESCR("Add stat point request")
		PACKET_XORP2(3, 3)
	END_COMMON_PACKET_DECL()
};


/**
 * \brief
 *
 * Describe guild
 * C1 08 66 00 7C 00 00 00
 *              ^- guild ID
 * Guild Info
 * C1 3C 66 00 7C 00 00 00 00 44 65 73 74 69 6E 79 00 44 65 73 74 69 6E 79 00 00 00 00 00 04 44 00 00 04 00 40 00 04 00 40 00 04 00 40 00 04 00 40 00 04 44 00 00 00 00 00 00 00 9C 0E
 *              ^- guild id
 *
 * Assign player to guild
 * C2 00 11 65 01 7C 00 00 00 00 00 00 1B B4 00 00 00 
 *                 ^- guild id          ^- player id
 *
 * C2 00 19 67 01 1B B4 00 00 7C 00 00 00 00 44 65 73 74 69 6E 79 00 00 00 00 
 *              ^  ^-^                        ^-------- Guild Name -----^
 *              | Player ID
 *            Count
 */
class CAssignPlayerToGuildPacket
	: public CPacket
{
	static CStringA DescribePacket(CPacket& pkt)
	{
		CAssignPlayerToGuildPacket& pkt2 = (CAssignPlayerToGuildPacket&)pkt;

		int iCount = pkt2.GetCount();

		CStringA str;
		str.Format("       Count of players: %d", iCount);

		for (int i=0; i < iCount; i++)
		{
			CStringA str2;
			str2.Format("\n       [%d] Id: 0x%04X, Guid: %s", i, pkt2.GetPlayerId(i), pkt2.GetGuildName(i));
			str += str2;
		}

		return str;
	}

	BEGIN_COMMON_PACKET_DECL(CAssignPlayerToGuildPacket)
		PACKET_PATT4(0xC2, 0x00, 0x00, 0x67)
		PACKET_MASK4(0xFF, 0x00, 0x00, 0xFF)
		PACKET_DESCR("Assign player to guild")
	END_COMMON_PACKET_DECL()

	int GetCount(){ BYTE* buf = GetDecryptedPacket(); return buf ? ((int)buf[4]) : 0; }

	WORD GetPlayerId(int idx)
	{
		BYTE* pPacket = GetDecryptedPacket();
		int iCount = GetCount();

		if (idx < 0 || idx >= iCount)
			return 0;

		int iSize = (GetDecryptedLen() - 5)/iCount;
		int offs = 5 + idx*iSize;
		return ((WORD)pPacket[offs] << 8) | pPacket[offs+1];
	}

	CStringA GetGuildName(int idx)
	{
		BYTE* pPacket = GetDecryptedPacket();
		int iCount = GetCount();

		if (idx < 0 || idx >= iCount)
			return 0;

		int iSize = (GetDecryptedLen() - 5)/iCount;
		int offs = 5 + idx*iSize;

		char szName[12] = {0};
		memcpy(szName, pPacket + offs + 9, 10);

		return CStringA(szName);
	}
};


/**  
 * \brief Party request received packet
 *
 * C1 05 40 23 7D
 *           ^ ^
 *         Sender ID
 */

/**  
 * \brief Send party request
 *
 * C3 06 63 40 23 7D
 *              ^ ^
 *           Target ID
 */


/**  
 * \brief Party created
 *
 * C1 35 42 01 02 4A 65 72 69 63 6F 4F 6E 65 00 00 03 B3 65 00 00 22 03 00 00 22 03 00 00 50 72 61 73 34 6F 00 00 00 00 01 03 B3 64 00 00 DC 03 00 00 DC 03 00 00
 *              ^  ^-------------------------^   ^  ^  ^  ^        ^-^         ^-^         ^-------------------------^
 *              |       Player Name              |  |  X  Y       Life       Max Life            Player Name
 *          Member Count                         | Map Id
 *                                           Order In party
 */


/**  
 * \brief Exit party
 *
 * C1 04 43 01
 */


/**  
 * \brief Party reply
 *
 * C3 07 9A 41 00 23 7D
 * C3 07 88 41 01 23 7D
 *             ^   ^-^
 *             |  Requested Player ID
 *          0 - deny, 1 - accept
 */



#endif //__CommonPackets_H
