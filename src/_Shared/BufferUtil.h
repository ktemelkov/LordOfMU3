#ifndef __BufferUtil_H
#define __BufferUtil_H

#pragma once

#include "atlstr.h"

/**
 *
 */
class CBufferUtil
{
public:
	/**
	 * \brief 
	 */
	static CStringA BufferToHex(BYTE* buf, int len)
	{
		return BufferToHex((char*)buf, len);
	}


	/**
	 * \brief
	 */
	static CStringA BufferToHex(char* buf, int len)
	{
		CStringA res("");

		if (buf && len != 0)
		{
			for (int i=0; i < len; ++i)
			{
				char hex_buff[4] = {0};

				hex_buff[0] = (buf[i] >> 4) & 0xF;
				hex_buff[1] = buf[i] & 0xF;

				hex_buff[0] += (hex_buff[0] < 10) ? 0x30 : 0x37;
				hex_buff[1] += (hex_buff[1] < 10) ? 0x30 : 0x37;

				hex_buff[2] = ' ';

				res += hex_buff;
			}
		}

		return res;
	}


	/**  
	 * \brief 
	 */
	static int HexToBufferLen(const char* hex)
	{
		if (!hex)
			return false;

		int len1 = strlen(hex);

		while (len1 > 0 && hex[len1-1] == ' ') len1--;

		if ((++len1 % 3) != 0)
			return 0;

		return len1/3;
	}


	/**  
	 * \brief 
	 *
	 * \param hex - space delimited hexadecimal bytes
	 *		example: hex = "0A AA BB CC"
	 */
	static bool HexToBuffer(const char* hex, BYTE* buf, int len)
	{
		if (!hex || !buf || len <= 0)
			return false;

		int len1 = strlen(hex);

		while (len1 > 0 && hex[len1-1] == ' ') len1--;

		if ((++len1 % 3) != 0 || len < len1/3)
			return false;

		int j=0;

		for (int i=0; i < len1; i += 3)
		{
			if (hex[i+2] != 0 && hex[i+2] != 0x20)
				return false;
			
			BYTE b1 = 0;
			BYTE b2 = 0;

			if (hex[i] >= 0x30 && hex[i] <= 0x39)
				b1 = (BYTE)hex[i] - 0x30;
			else if (hex[i] >= 0x41 && hex[i] <= 0x46)
				b1 = (BYTE)hex[i] - 0x37;
			else if (hex[i] >= 0x61 && hex[i] <= 0x66)
				b1 = (BYTE)hex[i] - 0x57;
			else
				return false;

			if (hex[i+1] >= 0x30 && hex[i+1] <= 0x39)
				b2 = (BYTE)hex[i+1] - 0x30;
			else if (hex[i+1] >= 0x41 && hex[i+1] <= 0x46)
				b2 = (BYTE)hex[i+1] - 0x37;
			else if (hex[i+1] >= 0x61 && hex[i+1] <= 0x66)
				b2 = (BYTE)hex[i+1] - 0x57;
			else
				return false;
			
			buf[j++] = b1 << 4 | b2;
		}

		return true;
	}
};

#endif //__BufferUtil_H