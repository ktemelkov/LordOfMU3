#pragma once


/**  
 * \brief 
 */
class CStdString
{
public:
	CStdString() 
		: m_psz(0) {}


	/**  
	 * \brief 
	 */
	CStdString(const char* pszInit)
	{
		m_psz = 0;
		operator+=(pszInit);
	}

	/**  
	 * \brief 
	 */
	CStdString(const CStdString& in)
	{
		m_psz = 0;
		operator+=(in);
	}


	/**  
	 * \brief 
	 */
	virtual ~CStdString()
	{
		if (m_psz)
			HeapFree(GetProcessHeap(), 0, m_psz);
	}


	/**  
	 * \brief 
	 */
	const char* c_str()
	{
		return m_psz;
	}


	/**  
	 * \brief 
	 */
	CStdString& operator+=(const CStdString& in)
	{
		return operator+=(in.m_psz);
	}

	/**  
	 * \brief 
	 */
	CStdString& operator+=(char c)
	{
		char buff[2];
		buff[0] = c;
		buff[1] = 0;
		return operator+=(buff);
	}

	/**  
	 * \brief 
	 */
	CStdString& operator+=(const char* pszIn)
	{
		int len = pszIn ? lstrlenA(pszIn) : 0;
		int len1 = m_psz ? lstrlenA(m_psz) : 0;

		if (len + len1)
		{
			char* pszNew = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len+len1+1);

			if (len1)
			{
				lstrcpyA(pszNew, m_psz);
				HeapFree(GetProcessHeap(), 0, m_psz);
			}

			if (len)
				lstrcatA(pszNew, pszIn);

			m_psz = pszNew;
		}

		return *this;
	}


	/**  
	 * \brief 
	 */
	CStdString operator+(const char* pszIn)
	{
		int len = pszIn ? lstrlenA(pszIn) : 0;
		int len1 = m_psz ? lstrlenA(m_psz) : 0;

		char* pszNew = 0;

		if (len + len1)
		{
			pszNew = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len+len1+1);

			if (len1)
				lstrcpyA(pszNew, m_psz);

			if (len)
				lstrcatA(pszNew, pszIn);
		}

		CStdString ret(pszNew);

		if (pszNew)
			HeapFree(GetProcessHeap(), 0, pszNew);

		return ret;
	}

	/**  
	 * \brief 
	 */
	CStdString operator+(const CStdString& in)
	{
		return operator+(in.m_psz);
	}


	/**  
	 * \brief 
	 */
	CStdString& operator=(const char* pszIn)
	{
		if (m_psz)
			HeapFree(GetProcessHeap(), 0, m_psz);
		
		m_psz = 0;
		return operator+=(pszIn);
	}


	/**  
	 * \brief 
	 */
	CStdString& operator=(const CStdString& in)
	{
		return operator=(in.m_psz);
	}


	/**  
	 * \brief 
	 */
	char operator[](int idx)
	{
		return m_psz[idx];
	}


	/**  
	 * \brief 
	 */
	int find(char ch)
	{
		int len = m_psz ? lstrlenA(m_psz) : 0;

		for (int i=0; i < len; i++)
		{
			if (m_psz[i] == ch)
				return i;
		}

		return -1;
	}


	/**  
	 * \brief 
	 */
	CStdString& AppendHex(unsigned char* buf, int len)
	{
		if (buf && len != 0)
		{
			for (int i=0; i < len; ++i)
			{
				char hex_buff[4];
				hex_buff[2] = 0;
				hex_buff[3] = 0;

				hex_buff[0] = (buf[i] >> 4) & 0xF;
				hex_buff[1] = buf[i] & 0xF;

				hex_buff[0] += (hex_buff[0] < 10) ? 0x30 : 0x37;
				hex_buff[1] += (hex_buff[1] < 10) ? 0x30 : 0x37;

				operator+=(hex_buff);
			}
		}

		return *this;
	}


	/**  
	 * \brief 
	 */
	static void ToLower(char* str)
	{
		for (int i = (int)lstrlenA(str) - 1; i >= 0; --i)
		{
			if (str[i] >= 0x41 && str[i] <= 0x5A) 
				str[i] = str[i] + 0x20;
		}
	}


private:
	char* m_psz;
};

