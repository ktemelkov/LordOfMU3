#ifndef __EncDec_H
#define __EncDec_H

#pragma once

#include <windows.h>
#include "Packet.h"


/**
 * \brief 
 */
class CEncDec
{
public:
	CEncDec();

public:
	static int DecryptC3asClient(unsigned char* Dest, unsigned char* Src, int Len);
	static int EncryptC3asClient(unsigned char* Dest, unsigned char* Src, int Len);
	static int DecryptC3asServer(unsigned char* Dest, unsigned char* Src, int Len);
	static int EncryptC3asServer(unsigned char* Dest, unsigned char* Src, int Len);
	static void DecXor32(unsigned char* Buff, int SizeOfHeader, int Len, bool fSend = false);
	static void EncXor32(unsigned char* Buff, int SizeOfHeader, int Len, bool fSend = false);
	static void EncDecLogin(unsigned char* Buff, int Len);

	static bool DecryptC3asClient(CPacket& pkt);
	static bool DecryptC3asServer(CPacket& pkt);
	static bool EncryptC3asClient(CPacket& pkt);
	static bool EncryptC3asServer(CPacket& pkt);

	static void CleanUp();

	static void ExtraEncDec(BYTE* buf, int nLen);
	static void SetExtraCrypt(bool fCrypt = true){ CEncDec::m_fExtraCrypt = fCrypt; }

protected:
	static CEncDec* GetInstance();

	int IntDecryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len);
	int IntEncryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len);
	int IntDecryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len);
	int IntEncryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len);
	void IntDecXor32(unsigned char*Buff,int SizeOfHeader,int Len);
	void IntEncXor32(unsigned char*Buff,int SizeOfHeader,int Len);
	void IntEncDecLogin(unsigned char*Buff,int Len);

protected:
	int LoadKeys(const unsigned char* data, unsigned long* Where);
	int DecryptC3(unsigned char*Dest,unsigned char*Src,int Len,unsigned long*Keys);
	int EncryptC3(unsigned char*Dest,unsigned char*Src,int Len,unsigned long*Keys);

	void ShiftRight(unsigned char* ptr, unsigned int len, unsigned int shift);
	void ShiftLeft(unsigned char* ptr, unsigned int len, unsigned int shift);
	unsigned int ShiftBytes(unsigned char* buf, unsigned int arg_4, unsigned char* pkt, unsigned int arg_C, unsigned int arg_10);

	void Encode8BytesTo11Bytes(unsigned char* outbuf, unsigned char* pktptr, unsigned int num_bytes, unsigned int* dec_dat, int add = 0);
	int Decode11BytesTo8Bytes(unsigned char* outbuf, unsigned char* pktptr, unsigned int* dec_dat);

	void Encode8BytesTo11Bytes_old( unsigned char* outbuf, unsigned char* pktptr, unsigned int num_bytes, unsigned int* dec_dat);


	bool MU_ForceEncodeC3C4(unsigned char* outbuf, unsigned short* outlen, unsigned char* inbuf, unsigned short len, unsigned int* dec_dat);
	bool MU_ForceDecodeC3C4(unsigned char* outbuf, unsigned short* outlen, unsigned char* inbuf, unsigned short len, unsigned int* dec_dat);

private:
	unsigned long ClientDecryptKeys[12];
	unsigned long ClientEncryptKeys[12];
	unsigned long ServerDecryptKeys[12];
	unsigned long ServerEncryptKeys[12];

	static unsigned char const C3Keys[16];
	static unsigned char const C2Keys[32];
	static unsigned char const LoginKeys[3];
	static unsigned char const ExtraXorKeys[32];

	bool ClientDecryptKeysLoaded;
	bool ClientEncryptKeysLoaded;
	bool ServerDecryptKeysLoaded;
	bool ServerEncryptKeysLoaded;

	static CEncDec* m_spInstance;

	static bool m_fExtraCrypt;
};


#endif //__EncDec_H