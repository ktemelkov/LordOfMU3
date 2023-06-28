#pragma once


/**  
 * \brief 
 */
template<int iMxStringId>
class CInstanceLock
{
public:
	/**  
	 * \brief 
	 */
	CInstanceLock()
	{
		m_hInstMutex = CKernelUtil::CreateMutexA(0, 0, CStringTable::GetString(iMxStringId).c_str());
		m_fFirst = CKernelUtil::GetLastError() != ERROR_ALREADY_EXISTS;
	}


	/**  
	 * \brief 
	 */
	virtual ~CInstanceLock()
	{
		if (m_hInstMutex != 0 && m_hInstMutex != INVALID_HANDLE_VALUE)
			CloseHandle(m_hInstMutex);
		
		m_hInstMutex = 0;
	}

	/**  
	 * \brief 
	 */
	bool IsFirst()
	{ 
		return m_fFirst; 
	}

private:
	HANDLE	m_hInstMutex;
	bool	m_fFirst;
};