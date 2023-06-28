#include "stdafx.h"
#include "EncDec.h"
#include <malloc.h>
#include "AutoBuffer.h"
#include "DebugOut.h"


const unsigned char Enc1dat[] = { 0x12, 0x11, 0x36, 0x00, 0x00, 0x00, 
0xD4, 0x53, 0x09, 0x3F, 0x01, 0x41, 0x5E, 0xE2, 0xE2, 0x68, 0xD3, 0x93, 0x2D, 0x06, 0xDF, 0x20, 
0x5A, 0xFC, 0x08, 0x3F, 0x00, 0xEC, 0x5C, 0xE2, 0xD1, 0x37, 0xD2, 0x93, 0xF0, 0x92, 0xDE, 0x20,
0x86, 0x1A, 0x08, 0x3F, 0xD2, 0x76, 0x5C, 0xE2, 0xFA, 0x41, 0xD2, 0x93, 0x86, 0x35, 0xDE, 0x20 };

const unsigned char Enc2dat[] = { 0x12, 0x11, 0x36, 0x00, 0x00, 0x00, 
0xF5, 0xB9, 0x09, 0x3F, 0x22, 0x6F, 0x5D, 0xE2, 0xA2, 0xF8, 0xD3, 0x93, 0x8D, 0x3B, 0xDC, 0x20, 
0xEA, 0x94, 0x08, 0x3F, 0xDB, 0x88, 0x5C, 0xE2, 0x23, 0xF0, 0xD2, 0x93, 0x2C, 0xD4, 0xDE, 0x20, 
0xAF, 0x55, 0x08, 0x3F, 0x1E, 0x39, 0x5C, 0xE2, 0x97, 0xF0, 0xD2, 0x93, 0xE8, 0x5B, 0xDE, 0x20 };

const unsigned char Dec1dat[] = { 0x12, 0x11, 0x36, 0x00, 0x00, 0x00, 
0xD4, 0x53, 0x09, 0x3F, 0x01, 0x41, 0x5E, 0xE2, 0xE2, 0x68, 0xD3, 0x93, 0x2D, 0x06, 0xDF, 0x20, 
0xA3, 0xDC, 0x08, 0x3F, 0x78, 0xC5, 0x5C, 0xE2, 0x0A, 0xA4, 0xD2, 0x93, 0x78, 0x80, 0xDE, 0x20, 
0x86, 0x1A, 0x08, 0x3F, 0xD2, 0x76, 0x5C, 0xE2, 0xFA, 0x41, 0xD2, 0x93, 0x86, 0x35, 0xDE, 0x20 };

const unsigned char Dec2dat[] = { 0x12, 0x11, 0x36, 0x00, 0x00, 0x00, 
0xF5, 0xB9, 0x09, 0x3F, 0x22, 0x6F, 0x5D, 0xE2, 0xA2, 0xF8, 0xD3, 0x93, 0x8D, 0x3B, 0xDC, 0x20, 
0xE8, 0xE1, 0x08, 0x3F, 0x03, 0xB4, 0x5C, 0xE2, 0xC4, 0x1A, 0xD2, 0x93, 0x3A, 0x8C, 0xDE, 0x20, 
0xAF, 0x55, 0x08, 0x3F, 0x1E, 0x39, 0x5C, 0xE2, 0x97, 0xF0, 0xD2, 0x93, 0xE8, 0x5B, 0xDE, 0x20 };


CEncDec* CEncDec::m_spInstance = 0;

unsigned char const CEncDec::C3Keys[16] = {0x9B,0xA7,0x08,0x3F,0x87,0xC2,0x5C,0xE2,
											0xB9,0x7A,0xD2,0x93,0xBF,0xA7,0xDE,0x20};

unsigned char const CEncDec::C2Keys[32] = {0xE7,0x6D,0x3A,0x89,0xBC,0xB2,0x9F,0x73,
											0x23,0xA8,0xFE,0xB6,0x49,0x5D,0x39,0x5D,
											0x8A,0xCB,0x63,0x8D,0xEA,0x7D,0x2B,0x5F,
											0xC3,0xB1,0xE9,0x83,0x29,0x51,0xE8,0x56};

unsigned char const CEncDec::LoginKeys[3] = {0xFC, 0xCF, 0xAB};

unsigned char const CEncDec::ExtraXorKeys[32] = { 0x77, 0xA4, 0x91, 0x5A, 0xD0, 0xE9, 0x9B, 0x3F, 
													0x7A, 0x2F, 0x3A, 0x18, 0xAE, 0xB8, 0xD9, 0x76, 
													0xB8, 0xC3, 0x82, 0xD5, 0x42, 0x87, 0x5E, 0x24, 
													0xC2, 0x0F, 0x6D, 0xA6, 0x1B, 0x4C, 0x7C, 0xF7 };

bool CEncDec::m_fExtraCrypt = false;


//////////////////////////////////////////////////////////////////////////


/**
 * \brief 
 */
CEncDec::CEncDec()
{
	ClientDecryptKeysLoaded = (0 != LoadKeys(Dec2dat, ClientDecryptKeys));
	ClientEncryptKeysLoaded = (0 != LoadKeys(Enc1dat, ClientEncryptKeys));
	ServerDecryptKeysLoaded = (0 != LoadKeys(Dec1dat, ServerDecryptKeys));
	ServerEncryptKeysLoaded = (0 != LoadKeys(Enc2dat, ServerEncryptKeys));
}


/**
 * \brief 
 */
CEncDec* CEncDec::GetInstance()
{
	if (!m_spInstance)
		m_spInstance = new CEncDec();

	return m_spInstance;
}


/**
 * \brief 
 */
void CEncDec::CleanUp()
{
	if (m_spInstance)
		delete m_spInstance;

	m_spInstance = 0;
}


/**
 * \brief 
 */
int CEncDec::DecryptC3asClient(unsigned char* Dest, unsigned char* Src, int Len)
{
	return GetInstance()->IntDecryptC3asClient(Dest, Src, Len);
}


/**
 * \brief 
 */
int CEncDec::EncryptC3asClient(unsigned char* Dest, unsigned char* Src, int Len)
{
	return GetInstance()->IntEncryptC3asClient(Dest, Src, Len);
}


/**
 * \brief 
 */
int CEncDec::DecryptC3asServer(unsigned char* Dest, unsigned char* Src, int Len)
{
	return GetInstance()->IntDecryptC3asServer(Dest, Src, Len);
}


/**
 * \brief 
 */
int CEncDec::EncryptC3asServer(unsigned char* Dest, unsigned char* Src, int Len)
{
	return GetInstance()->IntEncryptC3asServer(Dest, Src, Len);
}


/**
 * \brief 
 */
void CEncDec::DecXor32(unsigned char* Buff, int SizeOfHeader, int Len, bool fSend)
{
	GetInstance()->IntDecXor32(Buff, SizeOfHeader, Len);

	if (fSend)
		CEncDec::ExtraEncDec(Buff, Len);
}


/**
 * \brief 
 */
void CEncDec::EncXor32(unsigned char* Buff, int SizeOfHeader, int Len, bool fSend)
{
	if (fSend)
		CEncDec::ExtraEncDec(Buff, Len);

	GetInstance()->IntEncXor32(Buff, SizeOfHeader, Len);
}


/**  
 * \brief 
 */
void CEncDec::ExtraEncDec(BYTE* buf, int nLen)
{
	if (!CEncDec::m_fExtraCrypt)
		return;

	for (int i=0; i < nLen; i++)
		buf[i] ^= CEncDec::ExtraXorKeys[i % sizeof(ExtraXorKeys)];
}


/**
 * \brief 
 */
void CEncDec::EncDecLogin(unsigned char* Buff, int Len)
{
	GetInstance()->IntEncDecLogin(Buff, Len);
}


/**
 * \brief 
 */
int CEncDec::IntDecryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len)
{
	if (!ClientDecryptKeysLoaded)
		return 0;

	return DecryptC3(Dest,Src,Len,ClientDecryptKeys);
}


/**
 * \brief 
 */
int CEncDec::IntEncryptC3asClient(unsigned char*Dest,unsigned char*Src,int Len)
{
	if (!ClientEncryptKeysLoaded)
		return 0;

	return EncryptC3(Dest,Src,Len,ClientEncryptKeys);
}


/**
 * \brief 
 */
int CEncDec::IntDecryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len)
{
	if (!ServerDecryptKeysLoaded)
		return 0;

	return DecryptC3(Dest,Src,Len,ServerDecryptKeys);
}


/**
 * \brief 
 */
int CEncDec::IntEncryptC3asServer(unsigned char*Dest,unsigned char*Src,int Len)
{
	if (!ServerEncryptKeysLoaded)
		return 0;

	return EncryptC3(Dest,Src,Len,ServerEncryptKeys);
}


/**
 * \brief 
 */
int CEncDec::LoadKeys(const unsigned char* data, unsigned long* Where)
{
	const unsigned char* Buff = data + 6;

	for(int i=0;i<4;i++)
		Where[i]=((unsigned long*)C3Keys)[i]^((unsigned long*)Buff)[i];

	Buff += 16;
	
	for (int i=0;i<4;i++)
		Where[i+4]=((unsigned long*)C3Keys)[i]^((unsigned long*)Buff)[i];
	
	Buff += 16;
	
	for (int i=0;i<4;i++)
		Where[i+8]=((unsigned long*)C3Keys)[i]^((unsigned long*)Buff)[i];
	
    return 1;
}


/**
 * \brief 
 */
int CEncDec::DecryptC3(unsigned char* Dest, unsigned char* Src, int Len, unsigned long* Keys)
{
	if (Dest == 0)
		return (Len/11)*8;

	unsigned short dec_size = 0;

	if (!MU_ForceDecodeC3C4(Dest, &dec_size, Src, Len, (unsigned int*)Keys))
		return 0; //decryption fails

	return dec_size;
}


/**
 * \brief 
 */
int CEncDec::EncryptC3(unsigned char* Dest, unsigned char* Src, int Len, unsigned long* Keys)
{
	if (Dest == 0)
		return ((Len+7)/8)*11;

	unsigned short enc_len = 0;

	if (!MU_ForceEncodeC3C4(Dest, &enc_len, Src, Len, (unsigned int*)Keys))
		return 0;

	return enc_len;
}


/**
 * \brief 
 */
void CEncDec::IntDecXor32(unsigned char*Buff,int SizeOfHeader,int Len)
{
	for(int i=Len-1;i>=0;i--)
		Buff[i]^=(C2Keys[(i+SizeOfHeader)&31]^Buff[i-1]);
}


/**
 * \brief 
 */
void CEncDec::IntEncXor32(unsigned char*Buff,int SizeOfHeader,int Len)
{
	for(int i=0;i<Len;i++)
		Buff[i]^=(C2Keys[(i+SizeOfHeader)&31]^Buff[i-1]);
}


/**
 * \brief 
 */
void CEncDec::IntEncDecLogin(unsigned char*Buff,int Len)
{
	for(int i=0;i<Len;i++)
		Buff[i]=Buff[i]^LoginKeys[i%3];
}


/**
 * \brief 
 */
bool CEncDec::DecryptC3asClient(CPacket& pkt)
{
	if (pkt.GetPktClass() != 0xC3 && pkt.GetPktClass() != 0xC4)
		return false;

	BYTE* buf = pkt.GetRawPacket();
	int len = pkt.GetPktLen();
	BYTE hrdLen = pkt.GetHdrLen();

	if (!buf || len < (int)hrdLen)
		return false;

	int newLen = DecryptC3asClient(0, 0, len-hrdLen) + hrdLen;

	if (newLen <= 0)
		return false;

	CAutoBuffer newBuf(newLen);
	memset(newBuf, 0, newLen);
	memcpy(newBuf, buf, hrdLen);

	int res = DecryptC3asClient(newBuf+hrdLen, buf+hrdLen, len-hrdLen);

	if (res == 0)
		return false;

	res += hrdLen;

	if (pkt.GetPktClass() == 0xC3)
	{
		newBuf[1] = (BYTE)res;
	}
	else // 0xC4
	{
		newBuf[1] = HIBYTE(res);
		newBuf[2] = LOBYTE(res);
	}
		
	pkt.SetDecryptedPacket(newBuf, res);
	return true;
}

/**
 * \brief 
 */
bool CEncDec::DecryptC3asServer(CPacket& pkt)
{
	if (pkt.GetPktClass() != 0xC3 && pkt.GetPktClass() != 0xC4)
	{
		CDebugOut::PrintError("CEncDec -> Invalid packet class!");
		return false;
	}

	BYTE* buf = pkt.GetRawPacket();
	int len = pkt.GetPktLen();
	BYTE hrdLen = pkt.GetHdrLen();

	if (!buf || len < (int)hrdLen)
	{
		CDebugOut::PrintError("CEncDec -> Invalid buffer length!");
		return false;
	}

	int newLen = DecryptC3asServer(0, 0, len-hrdLen) + hrdLen;

	if (newLen <= 0)
	{
		CDebugOut::PrintError("CEncDec -> Invalid decoded length!");
		return false;
	}


	CAutoBuffer newBuf(newLen);
	memset(newBuf, 0, newLen);
	memcpy(newBuf, buf, hrdLen);

	int res = DecryptC3asServer(newBuf+hrdLen, buf+hrdLen, len-hrdLen);

	if (res == 0)
	{
//		CDebugOut::PrintError("CEncDec -> DecryptC3asServer() failed!");
		return false;
	}


	res += hrdLen;

	if (pkt.GetPktClass() == 0xC3)
	{
		newBuf[1] = (BYTE)res;
	}
	else // 0xC4
	{
		newBuf[1] = HIBYTE(res);
		newBuf[2] = LOBYTE(res);
	}
		
	pkt.SetDecryptedPacket(newBuf, res);
	return true;
}

/**
 * \brief 
 */
bool CEncDec::EncryptC3asClient(CPacket& pkt)
{
	BYTE* buf = pkt.GetDecryptedPacket();
	int len = pkt.GetDecryptedLen();

	if (!buf || len <= 0 || (buf[0] != 0xC3 && buf[0] != 0xC4))
		return false;

	int hdrSize = buf[0] == 0xC3 ? 2 : 3;

	if (len - hdrSize < 0)
		return false;

	int newLen = EncryptC3asClient(0, 0, len - hdrSize) + hdrSize;

	if (newLen <= 0)
		return false;

	CAutoBuffer newBuf(newLen);
	memset(newBuf, 0, newLen);
	memcpy(newBuf, buf, hdrSize);

	int res = EncryptC3asClient(newBuf+hdrSize, buf+hdrSize, len-hdrSize);

	if (res == 0 || (res + hdrSize) != newLen)
		return false;

	if (buf[0] == 0xC3)
	{
		newBuf[1] = (BYTE)newLen;
	}
	else // 0xC4
	{
		newBuf[1] = HIBYTE(newLen);
		newBuf[2] = LOBYTE(newLen);
	}

	CPacket newPacket(newBuf, newLen);
	newPacket.SetDecryptedPacket(buf, len);
	
	pkt = newPacket;
	return true;
}

/**
 * \brief 
 */
bool CEncDec::EncryptC3asServer(CPacket& pkt)
{
	BYTE* buf = pkt.GetDecryptedPacket();
	int len = pkt.GetDecryptedLen();

	if (!buf || len <= 0 || (buf[0] != 0xC3 && buf[0] != 0xC4))
		return false;

	int hdrSize = buf[0] == 0xC3 ? 2 : 3;

	if (len - hdrSize < 0)
		return false;

	int newLen = EncryptC3asServer(0, 0, len-hdrSize) + hdrSize;

	if (newLen <= 0)
		return false;

	CAutoBuffer newBuf(newLen);
	memset(newBuf, 0, newLen);
	memcpy(newBuf, buf, hdrSize);

	int res = EncryptC3asServer(newBuf+hdrSize, buf+hdrSize, len-hdrSize);

	if (res == 0 || (res + hdrSize) != newLen)
		return false;

	if (buf[0] == 0xC3)
	{
		newBuf[1] = (BYTE)newLen;
	}
	else // 0xC4
	{
		newBuf[1] = HIBYTE(newLen);
		newBuf[2] = LOBYTE(newLen);
	}

	CPacket newPacket(newBuf, newLen);
	newPacket.SetDecryptedPacket(buf, len);
	
	pkt = newPacket;
	return true;
}




/**
 * \brief 
 */
void CEncDec::ShiftRight(unsigned char* ptr, unsigned int len, unsigned int shift) 
{
	if (shift == 0) 
		return;

	for (unsigned int i = 1; i < len; ++i) 
	{
		*ptr = (*ptr << shift) | (*(ptr+1) >> (8 - shift));
		++ptr;
	}

	*ptr <<= shift;
}


/**
 * \brief 
 */
void CEncDec::ShiftLeft(unsigned char* ptr, unsigned int len, unsigned int shift) 
{
	if (shift == 0) 
		return;

	ptr += len - 1;

	for (unsigned int i = 1; i < len; ++i) 
	{
		*ptr = (*ptr >> shift) | (*(ptr-1) << (8 - shift));
		--ptr;
	}

	*ptr >>= shift;
}



/**
 * \brief 
 */
unsigned int CEncDec::ShiftBytes(unsigned char* buf, unsigned int arg_4, unsigned char* pkt, unsigned int arg_C, unsigned int arg_10)
{
	unsigned int size_ = ((((arg_10 + arg_C) - 1) / 8) + (1 - (arg_C / 8)));
	unsigned char tmp1[20] = {0};

	memcpy(tmp1, &pkt[arg_C/8], size_);
	
	unsigned int var_4 = (arg_10 + arg_C) & 0x7;
	
	if (var_4) 
		tmp1[size_ - 1] &= 0xFF << (8 - var_4);

	arg_C &= 0x7;
	
	ShiftRight(tmp1, size_, arg_C);
	ShiftLeft(tmp1, size_ + 1, arg_4 & 0x7);
	
	if ((arg_4 & 0x7) > arg_C)
		++size_;
	
	if (size_)
	{
		for (unsigned int i = 0; i < size_; ++i)
			buf[i+(arg_4/8)] |= tmp1[i];
	}

	return arg_10 + arg_4;
}


/**  
 * \brief 
 */
void CEncDec::Encode8BytesTo11Bytes_old(unsigned char* outbuf, unsigned char* pktptr, unsigned int num_bytes, unsigned int* dec_dat) 
{
	unsigned char finale[2];
	finale[0] = (unsigned char)num_bytes;
	finale[0] ^= 0x3D;
	finale[1] = 0xF8;

	for (int k = 0; k < 8; ++k)
		finale[1] ^= pktptr[k];

	finale[0] ^= finale[1];

	ShiftBytes(outbuf, 0x48, finale, 0x00, 0x10);
	
	unsigned int ring[4] = { 0x000000000, 0x00000000, 0x00000000, 0x00000000 };
	unsigned short* cryptbuf = (unsigned short*)pktptr;
	
	ring[0] =((dec_dat[ 8] ^(cryptbuf[0])) *dec_dat[ 4]) %dec_dat[ 0];
	ring[1] =((dec_dat[ 9] ^(cryptbuf[1] ^(ring[0] &0xFFFF))) *dec_dat[ 5]) %dec_dat[ 1];
	ring[2] =((dec_dat[10] ^(cryptbuf[2] ^(ring[1] &0xFFFF))) *dec_dat[ 6]) %dec_dat[ 2];
	ring[3] =((dec_dat[11] ^(cryptbuf[3] ^(ring[2] &0xFFFF))) *dec_dat[ 7]) %dec_dat[ 3];
	

	unsigned int ring_backup[4] ={ ring[0], ring[1], ring[2], ring[3] };
	
	ring[2] =ring[2] ^dec_dat[10] ^(ring_backup[3] &0xFFFF);
	ring[1] =ring[1] ^dec_dat[ 9] ^(ring_backup[2] &0xFFFF);
	ring[0] =ring[0] ^dec_dat[ 8] ^(ring_backup[1] &0xFFFF);

	ShiftBytes( outbuf, 0x00, (unsigned char*)(&ring[0]), 0x00, 0x10);
	ShiftBytes( outbuf, 0x10, (unsigned char*)(&ring[0]), 0x16, 0x02);
	ShiftBytes( outbuf, 0x12, (unsigned char*)(&ring[1]), 0x00, 0x10);
	ShiftBytes( outbuf, 0x22, (unsigned char*)(&ring[1]), 0x16, 0x02);
	ShiftBytes( outbuf, 0x24, (unsigned char*)(&ring[2]), 0x00, 0x10);
	ShiftBytes( outbuf, 0x34, (unsigned char*)(&ring[2]), 0x16, 0x02);
	ShiftBytes( outbuf, 0x36, (unsigned char*)(&ring[3]), 0x00, 0x10);
	ShiftBytes( outbuf, 0x46, (unsigned char*)(&ring[3]), 0x16, 0x02);
}


/**
 * \brief 
 */
void CEncDec::Encode8BytesTo11Bytes(unsigned char* outbuf, unsigned char* pktptr1, unsigned int num_bytes, unsigned int* dec_dat, int add) 
{
	unsigned char pktptr[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	memcpy(pktptr, pktptr1, num_bytes);

	unsigned char finale[2] = {0};
	finale[0] = (unsigned char)num_bytes-add;
	finale[1] = 0xF8;
	finale[0] ^= 0x3D;

	for (int k = 0; k < (int)8; ++k)
		finale[1] ^= pktptr[k];

	if ((num_bytes % 8) != 0)
	{
//		finale[0] += add;
	}

	finale[0] ^= finale[1];

	ShiftBytes(outbuf, 0x48, finale, 0x00, 0x10);
	
	unsigned int ring[4] = {0x000000000, 0x00000000, 0x00000000, 0x00000000};	
	unsigned short* cryptbuf = (unsigned short*)pktptr;

	ring[0] = ((dec_dat[ 8] ^ (cryptbuf[0])) * dec_dat[4]) % dec_dat[0];
	ring[1] = ((dec_dat[ 9] ^ (cryptbuf[1] ^ (ring[0] &0xFFFF))) * dec_dat[5]) % dec_dat[1];
	ring[2] = ((dec_dat[10] ^ (cryptbuf[2] ^ (ring[1] &0xFFFF))) * dec_dat[6]) % dec_dat[2];
	ring[3] = ((dec_dat[11] ^ (cryptbuf[3] ^ (ring[2] &0xFFFF))) * dec_dat[7]) % dec_dat[3];
	
	unsigned int ring_backup[4] = {ring[0], ring[1], ring[2], ring[3]};
	ring[2] = ring[2] ^ dec_dat[10] ^ (ring_backup[3] & 0xFFFF);
	ring[1] = ring[1] ^ dec_dat[ 9] ^ (ring_backup[2] & 0xFFFF);
	ring[0] = ring[0] ^ dec_dat[ 8] ^ (ring_backup[1] & 0xFFFF);
	
	ShiftBytes(outbuf, 0x00, (unsigned char*)(&ring[0]), 0x00, 0x10);
	ShiftBytes(outbuf, 0x10, (unsigned char*)(&ring[0]), 0x16, 0x02);
	ShiftBytes(outbuf, 0x12, (unsigned char*)(&ring[1]), 0x00, 0x10);
	ShiftBytes(outbuf, 0x22, (unsigned char*)(&ring[1]), 0x16, 0x02);
	ShiftBytes(outbuf, 0x24, (unsigned char*)(&ring[2]), 0x00, 0x10);
	ShiftBytes(outbuf, 0x34, (unsigned char*)(&ring[2]), 0x16, 0x02);
	ShiftBytes(outbuf, 0x36, (unsigned char*)(&ring[3]), 0x00, 0x10);
	ShiftBytes(outbuf, 0x46, (unsigned char*)(&ring[3]), 0x16, 0x02);
}



/**
 * \brief 
 */
int CEncDec::Decode11BytesTo8Bytes(unsigned char* outbuf, unsigned char* pktptr, unsigned int* dec_dat)
{
	unsigned int ring[4] = {0x00000000, 0x00000000, 0x00000000, 0x00000000};

	ShiftBytes((unsigned char*)&ring[0], 0x00, pktptr, 0x00, 0x10);
	ShiftBytes((unsigned char*)&ring[0], 0x16, pktptr, 0x10, 0x02);
	ShiftBytes((unsigned char*)&ring[1], 0x00, pktptr, 0x12, 0x10);
	ShiftBytes((unsigned char*)&ring[1], 0x16, pktptr, 0x22, 0x02);
	ShiftBytes((unsigned char*)&ring[2], 0x00, pktptr, 0x24, 0x10);
	ShiftBytes((unsigned char*)&ring[2], 0x16, pktptr, 0x34, 0x02);
	ShiftBytes((unsigned char*)&ring[3], 0x00, pktptr, 0x36, 0x10);
	ShiftBytes((unsigned char*)&ring[3], 0x16, pktptr, 0x46, 0x02);
	
	ring[2] = ring[2] ^ dec_dat[10] ^ (ring[3] & 0xFFFF);
	ring[1] = ring[1] ^ dec_dat[ 9] ^ (ring[2] & 0xFFFF);
	ring[0] = ring[0] ^ dec_dat[ 8] ^ (ring[1] & 0xFFFF);
	
	unsigned short* cryptbuf = (unsigned short*)outbuf;
	cryptbuf[0] = dec_dat[ 8] ^ ((ring[0] * dec_dat[4]) % dec_dat[0]);
	cryptbuf[1] = dec_dat[ 9] ^ ((ring[1] * dec_dat[5]) % dec_dat[1]) ^ (ring[0] & 0xFFFF);
	cryptbuf[2] = dec_dat[10] ^ ((ring[2] * dec_dat[6]) % dec_dat[2]) ^ (ring[1] & 0xFFFF);
	cryptbuf[3] = dec_dat[11] ^ ((ring[3] * dec_dat[7]) % dec_dat[3]) ^ (ring[2] & 0xFFFF);

	unsigned char finale[2] = {0x00, 0x00};
	ShiftBytes(finale, 0, pktptr, 0x48, 0x10);

	finale[0] ^= finale[1];
	finale[0] ^= 0x3D;
	
	unsigned char m = 0xF8;

	for (int k = 0; k < (int)8; ++k)
		m ^= outbuf[k];

	if (m == finale[1])
		return finale[0];

	return -1;
}


/**
 * \brief encode c3/c4 packet
 */
bool CEncDec::MU_ForceEncodeC3C4(unsigned char* outbuf, unsigned short* outlen, unsigned char* inbuf, unsigned short len, unsigned int* dec_dat) 
{
	*outlen = 0;

	unsigned int offset = 0;
//	int cnt = 1;

	for (offset = 0; (offset+8) <= len; offset += 8) 
	{
		memset(outbuf, 0, 11);
		Encode8BytesTo11Bytes_old(outbuf, &inbuf[offset], 8, dec_dat);

		*outlen += 11;
		outbuf += 11;
	}

	if (offset < len)
	{
		memset(outbuf, 0, 11);
		Encode8BytesTo11Bytes_old(outbuf, &inbuf[offset], len - offset, dec_dat);

		*outlen += 11;
	}

	return true;
}


/*
// encode c3/c4 packet
bool MU_ForceEncodeC3C4( unsigned char* outbuf, unsigned short* outlen, unsigned char* inbuf, unsigned short len, unsigned int* dec_dat) 
{
	*outlen =0;

	unsigned int offset =0;

	for( offset =0; (offset+8) <= len; offset +=8) 
	{
		memset(outbuf, 0, 11);
		Encode8BytesTo11Bytes(outbuf, &inbuf[offset], 8, dec_dat);
		*outlen += 11;
		outbuf += 11;
	}

	if ( offset < len) {
		memset( outbuf, 0, 11);
		Encode8BytesTo11Bytes( outbuf, &inbuf[offset], len - offset, dec_dat);
		*outlen += 11;
	}
	return true;
}
*/

/**
 * \brief decode c3/c4 packet
 */ 
bool CEncDec::MU_ForceDecodeC3C4(unsigned char* outbuf, unsigned short* outlen, unsigned char* inbuf, unsigned short len, unsigned int* dec_dat) 
{
	if ((len % 11) != 0)
		return false; // invalid size specified

	*outlen = 0;
	int rez = 0;
//	int cnt = 0;

	for (unsigned int offset = 0; offset < len; offset += 11) 
	{
		rez = Decode11BytesTo8Bytes(outbuf, &inbuf[offset], dec_dat);
		
		if (rez <= 0)
			return false;// failed to decrypt

		*outlen += (unsigned int)rez;
		outbuf += 8;
//		cnt++;
	}

//	(*outlen) += ((cnt > 1) ? 4 : 1);
	return true;
}

