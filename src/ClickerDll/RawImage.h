// RawImage.h : Declaration of the CRawImage
#ifndef __RawImage_H
#define __RawImage_H

#pragma once



/**  
 * \brief 
 */
class CRawImage
{
public:
	CRawImage();
	virtual ~CRawImage();

public:
	int width();
	int height();
	BYTE* PixelBuffer();

	bool LoadBitmapFile(LPWSTR pszFile);

protected:
	bool LoadDIBFromFile(FILE* f, HBITMAP& hBitmap);
	bool AllocBuffer(int cx, int cy, BYTE* pInitData);

protected:
	int  m_cx;
	int  m_cy;
	LPBYTE m_pBuffer;
	ULONG  m_uBuffSize;	
};



/**  
 * \brief 
 */
class _Convert_DstBuff
{
public:
	/**  
	 * \brief 
	 */
	inline _Convert_DstBuff(LPBYTE* pBuffer, int cx, int cy) 
	{
		m_pBuffer = 0;

		if (pBuffer)
		{
			*pBuffer = 0;

			if (cx > 0 && cy > 0)
			{
				m_pBuffer = (LPBYTE)new BYTE[cx*cy*4];
				*pBuffer = m_pBuffer;
			}
		}
	};


	/**  
	 * \brief 
	 */
	inline void SetPixel(int idx, DWORD rgb)
	{
		*(DWORD*)(m_pBuffer + (idx<<2)) = rgb;//((rgb >> 16) & 0xFF) | ((rgb << 16) & 0xFF0000) | (rgb & 0x00FF00);

		if (rgb != 0)
			*(DWORD*)(m_pBuffer + (idx<<2)) = 0xFF00FF00;
		else
			*(DWORD*)(m_pBuffer + (idx<<2)) = 0x00000000;
	};

protected:
	LPBYTE m_pBuffer;
};



#endif //__RawImage_H