#ifndef __CommonPacketsMacros_H
#define __CommonPacketsMacros_H

#pragma once


/**  
 * \brief 
 */
#define BEGIN_COMMON_PACKET_DECL_TYPE(ClassName) \
	public:\
		virtual CPacketType GetType() const { return Type(); }\
		virtual void SetType(const CPacketType&) { } \
	\
	public: \
		struct Type : public CPacketType \
		{ \
			Type() : CPacketType(){} \
			virtual DescribePacketPtr GetDescribeProc()	const { return ClassName::DescribePacket; } \


/**  
 * \brief 
 */
#define NEED_POST_DETECT() \
			virtual bool NeedPostDetect() const { return true; }



/**
 * \brief 
 */
#define BEGIN_COMMON_PACKET_DECL(ClassName) \
public: \
	ClassName(){} \
	virtual ~ClassName(){} \
	\
	ClassName(CPacket& in) : CPacket(in) { SetType(Type()); } \
	ClassName& operator=(const CPacket& in){ CPacket::operator=(in); return *this; }\
\
	BEGIN_COMMON_PACKET_DECL_TYPE(ClassName)


/**
 * \brief 
 */
#define BEGIN_COMMON_PACKET_DECL_NOCCONSTR(ClassName) \
public: \
	ClassName(){} \
	virtual ~ClassName(){} \
\
	ClassName& operator=(const CPacket& in){ CPacket::operator=(in); return *this; }\
\
	BEGIN_COMMON_PACKET_DECL_TYPE(ClassName)


/**
 * \brief 
 */
#define BEGIN_COMMON_PACKET_DECL_NOCONSTR(ClassName) \
public: \
	virtual ~ClassName(){} \
	\
	ClassName(CPacket& in) : CPacket(in) { SetType(Type()); } \
	ClassName& operator=(const CPacket& in){ CPacket::operator=(in); return *this; }\
\
	BEGIN_COMMON_PACKET_DECL_TYPE(ClassName)


/**
 * \brief 
 */
#define PACKET_BUFF_RET_STR(FuncName, str) \
		virtual const BYTE* FuncName()	const { \
		{ \
			static BYTE buff[sizeof(str)/3 + 1] = {0}; \
			if (buff[0] == 0) \
			{ \
				buff[0] = sizeof(buff) - 1; \
				CBufferUtil::HexToBuffer(str, buff+1, buff[0]); \
			} \
			return buff; \
		}; \

#define PACKET_BUFF_RET2(FuncName, p1, p2) \
			virtual const BYTE* FuncName()	const { static const BYTE buff[] = {2, p1, p2}; return buff; };

#define PACKET_BUFF_RET3(FuncName, p1, p2, p3) \
			virtual const BYTE* FuncName()	const { static const BYTE buff[] = {3, p1, p2, p3}; return buff; };

#define PACKET_BUFF_RET4(FuncName, p1, p2, p3, p4) \
			virtual const BYTE* FuncName()	const { static const BYTE buff[] = {4, p1, p2, p3, p4}; return buff; };

#define PACKET_BUFF_RET5(FuncName, p1, p2, p3, p4, p5) \
			virtual const BYTE* FuncName()	const { static const BYTE buff[] = {5, p1, p2, p3, p4, p5}; return buff; };

#define PACKET_BUFF_RET6(FuncName, p1, p2, p3, p4, p5, p6) \
	virtual const BYTE* FuncName()	const { static const BYTE buff[] = {6, p1, p2, p3, p4, p5, p6}; return buff; };


/**
 * \brief 
 */
#define PACKET_BUFF_RET_VSTR(versionNo, str) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static BYTE buff[sizeof(str)/3 + 1] = {0}; \
			if (buff[0] == 0) \
			{ \
				buff[0] = sizeof(buff) - 1; \
				CBufferUtil::HexToBuffer(str, buff+1, buff[0]); \
			} \
			return buff; \
		}

#define PACKET_BUFF_RET_V2(versionNo, p1, p2) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static const BYTE buff[] = {2, p1, p2}; \
			return buff; \
		}

#define PACKET_BUFF_RET_V3(versionNo, p1, p2, p3) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static const BYTE buff[] = {3, p1, p2, p3}; \
			return buff; \
		}

#define PACKET_BUFF_RET_V4(versionNo, p1, p2, p3, p4) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static const BYTE buff[] = {4, p1, p2, p3, p4}; \
			return buff; \
		}

#define PACKET_BUFF_RET_V5(versionNo, p1, p2, p3, p4, p5) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static const BYTE buff[] = {5, p1, p2, p3, p4, p5}; \
			return buff; \
		}


/**
 * \brief 
 */
#define PACKET_PATT_STR(str) PACKET_BUFF_RET2(GetPattern, str)
#define PACKET_PATT2(p1, p2) PACKET_BUFF_RET2(GetPattern, p1, p2)
#define PACKET_PATT3(p1, p2, p3) PACKET_BUFF_RET3(GetPattern, p1, p2, p3)
#define PACKET_PATT4(p1, p2, p3, p4) PACKET_BUFF_RET4(GetPattern, p1, p2, p3, p4)
#define PACKET_PATT5(p1, p2, p3, p4, p5) PACKET_BUFF_RET5(GetPattern, p1, p2, p3, p4, p5)
#define PACKET_PATT6(p1, p2, p3, p4, p5, p6) PACKET_BUFF_RET6(GetPattern, p1, p2, p3, p4, p5, p6)

#define BEGIN_PACKET_PATT() \
			virtual const BYTE* GetPattern() const \
			{ 

#define END_PACKET_PATT() \
				static const BYTE defBuff[] = {0}; return defBuff; \
			}

#define PACKET_PATT_VSTR(version, str) PACKET_BUFF_RET_VSTR(version, str)

#define PACKET_PATT_V2(version, p1, p2) PACKET_BUFF_RET_V2(version, p1, p2)
#define PACKET_PATT_V3(version, p1, p2, p3) PACKET_BUFF_RET_V3(version, p1, p2, p3)
#define PACKET_PATT_V4(version, p1, p2, p3, p4) PACKET_BUFF_RET_V4(version, p1, p2, p3, p4)
#define PACKET_PATT_V5(version, p1, p2, p3, p4, p5) PACKET_BUFF_RET_V5(version, p1, p2, p3, p4, p5)
#define PACKET_PATT_V6(version, p1, p2, p3, p4, p5, p6) PACKET_BUFF_RET_V6(version, p1, p2, p3, p4, p5, p6)

/**
 * \brief 
 */
#define PACKET_MASK_STR(str) PACKET_BUFF_RET_STR(GetMask, str)
#define PACKET_MASK2(p1, p2) PACKET_BUFF_RET2(GetMask, p1, p2)
#define PACKET_MASK3(p1, p2, p3) PACKET_BUFF_RET3(GetMask, p1, p2, p3)
#define PACKET_MASK4(p1, p2, p3, p4) PACKET_BUFF_RET4(GetMask, p1, p2, p3, p4)
#define PACKET_MASK5(p1, p2, p3, p4, p5) PACKET_BUFF_RET5(GetMask, p1, p2, p3, p4, p5)
#define PACKET_MASK6(p1, p2, p3, p4, p5, p6) PACKET_BUFF_RET6(GetMask, p1, p2, p3, p4, p5, p6)

#define BEGIN_PACKET_MASK() \
			virtual const BYTE* GetMask() const \
			{ 

#define END_PACKET_MASK() \
				static const BYTE defBuff[] = {0}; return defBuff; \
			}

#define PACKET_MASK_VSTR(version, str) PACKET_BUFF_RET_VSTR(version, str)
#define PACKET_MASK_V2(version, p1, p2) PACKET_BUFF_RET_V2(version, p1, p2)
#define PACKET_MASK_V3(version, p1, p2, p3) PACKET_BUFF_RET_V3(version, p1, p2, p3)
#define PACKET_MASK_V4(version, p1, p2, p3, p4) PACKET_BUFF_RET_V4(version, p1, p2, p3, p4)
#define PACKET_MASK_V5(version, p1, p2, p3, p4, p5) PACKET_BUFF_RET_V5(version, p1, p2, p3, p4, p5)
#define PACKET_MASK_V6(version, p1, p2, p3, p4, p5, p6) PACKET_BUFF_RET_V6(version, p1, p2, p3, p4, p5, p6)

/**
 * \brief 
 */
#define PACKET_XORP2(p1, p2) \
			virtual const BYTE* GetXorParams()	const { static const BYTE buff[] = {p1, p2}; return buff; };

#define BEGIN_PACKET_XORP() \
			virtual const BYTE* GetXorParams() const \
			{ 

#define END_PACKET_XORP() \
				static const BYTE defBuff[] = {0, 0}; return defBuff; \
			}

#define PACKET_XORP_V2(versionNo, p1, p2) \
		if (versionNo == 0 || (m_ulVersion & 0xFF) == versionNo) \
		{ \
			static const BYTE buff[] = {p1, p2}; \
			return buff; \
		}

/**
 * \brief 
 */
#define PACKET_DESCR(descr) \
			virtual const char* GetDescription() const { return descr; };


/**
 * \brief 
 */
#define END_COMMON_PACKET_DECL() \
		};


#endif // __CommonPacketsMacros_H
