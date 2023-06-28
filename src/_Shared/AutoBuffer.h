#ifndef __AutoBuffer_H
#define __AutoBuffer_H

#pragma once


/**
 * \brief 
 */
class CAutoBuffer
{
public:
	CAutoBuffer(int len)
	{
		m_pBuffer = 0;

		if (len > 0)
		{
			m_pBuffer = new BYTE[len+8];
			memset(m_pBuffer, 0, len+8);
		}
	}

	virtual ~CAutoBuffer()
	{
		if (m_pBuffer)
			delete[] m_pBuffer;

		m_pBuffer = 0;
	}

	operator BYTE*(){ return m_pBuffer; }
	operator void*(){ return (void*)m_pBuffer; }
	
protected:
	BYTE* m_pBuffer;
};


/**
 * \brief 
 */
class CCharAutoBuffer
	: public CAutoBuffer
{
public:
	CCharAutoBuffer(int len) : CAutoBuffer(len){}

	operator char*(){ return (char*)m_pBuffer; }

private:
	operator BYTE*(){ return m_pBuffer; }
};

#endif //__AutoBuffer_H