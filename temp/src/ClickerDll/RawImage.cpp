// RawImage.cpp : Implementation of CRawImage
#include "stdafx.h"
#include "rawimage.h"


/**  
 * \brief 
 */
CRawImage::CRawImage()
{
	m_cx = 0;
	m_cy = 0;
	m_pBuffer = 0;
	m_uBuffSize = 0;
}


/**  
 * \brief 
 */
CRawImage::~CRawImage()
{
	if (m_pBuffer)
		delete[] m_pBuffer;
}


/**  
 * \brief 
 */
int CRawImage::width()
{
	return m_cx;
}


/**  
 * \brief 
 */
int CRawImage::height()
{
	return m_cy;
}


/**  
 * \brief 
 */
BYTE* CRawImage::PixelBuffer()
{
	return m_pBuffer;
}



/**  
 * \brief 
 */
bool CRawImage::AllocBuffer(int cx, int cy, BYTE* pInitData)
{
	if (cx <= 0 || cy <= 0)
		return false;

	m_uBuffSize = cx*cy*4;

	if (m_pBuffer)
		delete[] m_pBuffer;

	m_cx = 0;
	m_cy = 0;


	m_pBuffer = (LPBYTE)new BYTE[m_uBuffSize];

	if (!m_pBuffer)
		return false;

	m_cx = cx;
	m_cy = cy;
	
	if (pInitData)
		memcpy(m_pBuffer, pInitData, m_uBuffSize);

	return true;
}



/**  
 * \brief 
 */
bool CRawImage::LoadBitmapFile(LPWSTR pszFile)
{
	FILE *f = _wfopen(pszFile, L"rb");
	
	if (!f) 
		return false;


	HBITMAP hBitmap = 0;
	HRESULT hr = S_OK;

	double dpi = 1.0;

	if (!LoadDIBFromFile(f, hBitmap))
	{
		fclose(f);
		return false;
	}


	DIBSECTION dibs = {0};

	if (!GetObject(hBitmap, sizeof(DIBSECTION), &dibs))
	{
		DeleteObject(hBitmap);
		return false;
	}

	ULONG  cx  = abs(dibs.dsBm.bmWidth);
	ULONG  cy  = abs(dibs.dsBm.bmHeight);

	LPBYTE pBuffer = 0;
	_Convert_DstBuff dst(&pBuffer, cx, cy);

	if (!pBuffer)
	{
		DeleteObject(hBitmap);
		return false;
	}

	BYTE bmiBuff[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256];
	memset(bmiBuff, 0, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256);
	BITMAPINFO& bmi = *((BITMAPINFO*)bmiBuff);

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = -(LONG)cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;

	HDC hDC = CreateDC(_T("DISPLAY"),0,0,0);
	if (!hDC)
	{
		DeleteObject(hBitmap);
		delete[] pBuffer;
		return false;
	}

	if (GetDIBits(hDC, hBitmap, 0, cy, 0, &bmi, DIB_RGB_COLORS))
	{
		LPBYTE lpBits = new BYTE[bmi.bmiHeader.biSizeImage];

		if (!lpBits)
		{
			DeleteDC(hDC);
			DeleteObject(hBitmap);
			delete[] pBuffer;
			return false;
		}

		if (GetDIBits(hDC, hBitmap, 0, cy, lpBits, &bmi, DIB_RGB_COLORS))
			for (int i=(int)cx*cy-1; i >= 0; i--)
				dst.SetPixel(i, *(DWORD*)(lpBits + (i << 2)));

		delete[] lpBits;
	}

	DeleteDC(hDC);
	DeleteObject(hBitmap);


	if (!AllocBuffer(cx, cy, pBuffer))
	{
		delete[] pBuffer;
		return false;
	}

	delete[] pBuffer;
	return true;
}


/**  
 * \brief 
 */
bool CRawImage::LoadDIBFromFile(FILE* f, HBITMAP& hBitmap)
{
	if (!f)
		return false;

	hBitmap = 0;

	RGBQUAD ColorTable[256] = {0};

	for (int ii=0; ii < 256; ii++) 
		*((DWORD*)(ColorTable + ii)) = ii*0x00010101;

	BITMAPFILEHEADER bmfh = {0};
	BYTE bmihBuff[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
	memset(bmihBuff, 0, sizeof(BITMAPINFOHEADER));
	memcpy(bmihBuff + sizeof(BITMAPINFOHEADER), ColorTable , 256*sizeof(RGBQUAD));
	BITMAPINFOHEADER& bmih = *((BITMAPINFOHEADER*)bmihBuff);
	LPBYTE pBmBuff = 0;

	HDC hDC = CreateDC(_T("DISPLAY"),0,0,0);
	if (!hDC)
		return false;

	bool fRet = false;

	for (;;)
	{
		if (!fread(&bmfh, sizeof(BITMAPFILEHEADER), 1, f) ||
			bmfh.bfType != ((WORD) ('M' << 8) | 'B') ||
			!fread(&bmih, sizeof(BITMAPINFOHEADER), 1, f)) 
		{
			break;
		}

		bmih.biClrUsed = (bmfh.bfOffBits - sizeof(BITMAPFILEHEADER) - sizeof(BITMAPINFOHEADER))/sizeof(RGBQUAD);
		if (bmih.biClrUsed > 256) 
			bmih.biClrUsed = 256;

		if (bmih.biClrUsed && 
			!fread(bmihBuff+sizeof(BITMAPINFOHEADER), sizeof(RGBQUAD)*bmih.biClrUsed, 1, f))
		{
			break;
		}

		fseek(f, bmfh.bfOffBits, SEEK_SET);

		pBmBuff = new BYTE[bmfh.bfSize - bmfh.bfOffBits];
		if (!pBmBuff)
		{
			break;
		}

		if (!fread(pBmBuff, bmfh.bfSize - bmfh.bfOffBits, 1, f))
		{
			break;
		}

		if (0 == (hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)bmihBuff, DIB_RGB_COLORS, 0, 0, 0)))
		{
			break;
		}

		SetDIBits(hDC, hBitmap, 0, abs(bmih.biHeight), pBmBuff, (BITMAPINFO*)bmihBuff, DIB_RGB_COLORS);
		fRet = true;
		break;
	}

	if (pBmBuff)
		delete[] pBmBuff;

	DeleteDC(hDC);
	return fRet;
}
