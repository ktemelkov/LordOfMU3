#ifndef __AsyncKeyQueue_H
#define __AsyncKeyQueue_H

#pragma once

#include <map>

class CAsyncKeyQueue
{
public:
	CAsyncKeyQueue()
	{
		InitializeCriticalSection(&m_cs);
		m_hEmptyEvent = CreateEvent(0, TRUE, 1, 0);
		m_iNonpermSize = 0;
	}

	virtual ~CAsyncKeyQueue()
	{
		DeleteCriticalSection(&m_cs);
		CloseHandle(m_hEmptyEvent);
	}

public:
	static CAsyncKeyQueue* GetInstance()
	{
		CAsyncKeyQueue*& pInstance = GetInstancePtr();

		if (!pInstance)
		{
			Init();
		}

		return pInstance;
	};

	static void Init()
	{ 
		GetInstancePtr() = new CAsyncKeyQueue(); 
	}

	static void Term()
	{ 
		CAsyncKeyQueue*& pInstance = GetInstancePtr();
		
		if (pInstance)
			delete pInstance;

		pInstance = NULL;
	}

public:
	BOOL IsEmpty()
	{ 
		EnterCriticalSection(&m_cs);	

		BOOL bRes = (m_iNonpermSize == 0); 
		
		LeaveCriticalSection(&m_cs);
		return bRes;
	}

	BOOL WaitEmpty(DWORD dwMilliseconds)
	{
		return WaitForSingleObject(m_hEmptyEvent, dwMilliseconds) != WAIT_TIMEOUT;
	}

	BOOL GetKey(int vk)
	{
		BOOL res = FALSE;
		EnterCriticalSection(&m_cs);
		
		if (m_vQueue.size() != 0)
		{
			std::map<int,int>::iterator it = m_vQueue.find(vk);
			if (it != m_vQueue.end())
			{
				if (it->second == 0)
				{
					m_vQueue.erase(vk);
					m_iNonpermSize--;
				}

				res = TRUE;
			}

			if (m_iNonpermSize <= 0)
			{
				m_iNonpermSize = 0;
				SetEvent(m_hEmptyEvent);
			}
		}

		LeaveCriticalSection(&m_cs);
		return res;
	}

	void SendKey(int vk)
	{
		EnterCriticalSection(&m_cs);
		
		m_vQueue.insert(std::pair<int,int>(vk, 0));
		m_iNonpermSize++;

		if (m_iNonpermSize >= 1)
			ResetEvent(m_hEmptyEvent);

		LeaveCriticalSection(&m_cs);
	}

	void PressKey(int vk)
	{
		EnterCriticalSection(&m_cs);
		m_vQueue.insert(std::pair<int,int>(vk, 1));
		LeaveCriticalSection(&m_cs);
	}

	void ReleaseKey(int vk)
	{
		EnterCriticalSection(&m_cs);
		m_vQueue.erase(vk);

		if (m_iNonpermSize <= 0)
		{
			m_iNonpermSize = 0;
			SetEvent(m_hEmptyEvent);
		}

		LeaveCriticalSection(&m_cs);
	}

private:
	static CAsyncKeyQueue*& GetInstancePtr()
	{
		static CAsyncKeyQueue* pInstance = 0;
		return pInstance;
	}

private:
	CRITICAL_SECTION m_cs;
	HANDLE m_hEmptyEvent;
	std::map<int,int> m_vQueue;
	int m_iNonpermSize;
};

#endif //__AsyncKeyQueue_H