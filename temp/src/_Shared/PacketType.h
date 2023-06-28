#ifndef __PacketType_H
#define __PacketType_H

#pragma once

//old
//Phi   0xDD  0xD6    0xDF              0x10 ? 0x11 <-->0x1D


//         walk  attack  movefix obj_moved magicskill
//Phi/Eng  0xD4  0x11    0x15              0x1D
//jpn      0x1D  0xDC    0xD6    0x1D      0xD7
//KOR      0xD3  0xD7    0xDF    0xD3      0x1D ? 0x10
//chs	   0xD7  0xD9	 0xD0	           0x1D
//vtm	   0xD9  0x15    0xDC              0x1D


// Name	Walk  Damage  Teleport	Skill
// ENG	D4	  11	  15	    DB		//DONT CHANGE ENG PROTOCOL
// JPN	1D	  D6	  DC	    D7

// KOR	D3	  DF	  D7	    10

// VTM	D9	  DC	  15	    1D
// PHI	DD	  D6	  DF	    11
// CHS	D9	  D0	  D7	    1D
// TAI	D7	  D2	  D9	    1D



// -------------
// Name	Walk			attack			Teleport			Damage			Skill

// ENG	C1 07 D4		C1 00 11		15					11				DB		//DONT CHANGE ENG PROTOCOL
//		C1 07 D4

// JPN	C1 07 1D (cts)	C1 08 DC (cts)	C1 07 D6 (stc)		DC				D7
//		C1 08 1D (cts)
//		C1 09 1D (cts)
//		C1 0A 1D (cts)

//		C1 06 1D (cts)

//		C1 08 1D (stc)


// VTM	D9								DC					15				1D
// CHS	D9								D0					D7				1D

// PHI	DD								D6					DF				11
// KOR	D3				C1 00 D7		DF					D7				10
// TAI	D7								D2					D9				1D






// V1 = 104
// V2 = 105
#define PTYPE_KOR 0x00000001
#define PTYPE_CHS 0x00000002
#define PTYPE_VTM 0x00000003
#define PTYPE_JPN 0x00000004
#define PTYPE_PHI 0x00000005
#define PTYPE_ENG 0x00000006
#define PTYPE_TAI 0x00000007

#define GTYPE_S2  0x00000000
#define GTYPE_S3  0x01000000
#define GTYPE_S4  0x02000000



#define FEATURE_NO_TIMELIMIT	0x80000000
#define FEATURE_MOVE_TO_PICK	0x00000001
#define FEATURE_AUTOKILL		0x00000002
#define FEATURE_AUTOSPEAK		0x00000004
#define FEATURE_MOVETO			0x00000008
#define FEATURE_MULTIHIT		0x00000010
#define FEATURE_FASTMOVE		0x00000020
#define FEATURE_SCRIPT			0x00000040
#define FEATURE_LAHAP			0x00000080
#define FEATURE_ITEMCODE		0x00000100
#define FEATURE_REFLECT			0x00000200
#define FEATURE_INVENTORY		0x00000400
#define FEATURE_LOGLEVEL		0x00000800
#define FEATURE_PACKETS			0x00001000
#define FEATURE_AFK_PROTECT		0x00002000
#define FEATURE_AGILITY_HACK	0x00004000
#define FEATURE_KILL_PLAYERS	0x00008000


class CPacket;


/**
 * \brief 
 */
class CPacketType
{
public:
	typedef CStringA (*DescribePacketPtr)(CPacket&);

public:
	CPacketType();
	CPacketType(const CPacketType& in);
	virtual ~CPacketType();

	CPacketType& operator=(const CPacketType& in);
	CStringA DescribePacket(CPacket& pkt);

public:
	// Packet Type Interface
	// Note: The first elements mask and pattern arrays contain the number of following elements
	virtual const BYTE* GetPattern() const { return m_pPat; }
	virtual const BYTE* GetMask() const { return m_pMask; }
	virtual const BYTE* GetXorParams() const { return m_abXorP; }
	virtual const char* GetDescription() const { return m_szDescr; }	
	virtual DescribePacketPtr GetDescribeProc() const { return m_pfnDescribe; }
	virtual bool NeedPostDetect() const { return m_fPostDetect; }

	static ULONG GetVersion(){ return m_ulVersion; }
	static void SetVersion(ULONG ulVersion){ m_ulVersion = (m_ulVersion & 0x00FFFFFF) | (ulVersion & 0xFF000000); }
	static void SetProtocol(ULONG ulProto){ m_ulVersion = (m_ulVersion & 0xFFFFFF00) | (ulProto & 0x000000FF); }

	static void SetFeatures(DWORD dwFt){ m_dwFeatures = dwFt; }
	static DWORD GetFeatures(){ return m_dwFeatures; }

protected:
	BYTE* m_pPat;
	BYTE* m_pMask;
	BYTE  m_abXorP[2]; // 0 - offset for packet start, 1 - size of header
	char* m_szDescr;
	bool m_fPostDetect;
	DescribePacketPtr m_pfnDescribe;
	static ULONG m_ulVersion;
	static DWORD  m_dwFeatures;
};

#endif //__PacketType_H