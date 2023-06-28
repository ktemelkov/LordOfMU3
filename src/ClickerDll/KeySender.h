#ifndef __KeySender_H
#define __KeySender_H

#pragma once

#include "AsyncKeyQueue.h"

class CKeySender
{
public:
	/**
	 * \brief 
	 */
	static BOOL SendAsync(int vk, BOOL fWait = FALSE, DWORD dwWaitTimeout = 5000)
	{ 
		CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();
		pQueue->SendKey(vk);

		if (fWait)
			return pQueue->WaitEmpty(dwWaitTimeout);

		return TRUE; 
	}


	/**
	 * \brief 
	 */
	static BOOL SendAsync(char key, BOOL fWait = FALSE, DWORD dwWaitTimeout = 5000)
	{
		return SendAsync((int)(VkKeyScan(key) & 0xFF), fWait, dwWaitTimeout);
	}

	/**
	 * \brief 
	 */
	static void PressKey(int vk)
	{ 
		CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();
		pQueue->PressKey(vk);
	}

	/**
	 * \brief 
	 */
	static void ReleaseKey(int vk)
	{ 
		CAsyncKeyQueue* pQueue = CAsyncKeyQueue::GetInstance();
		pQueue->ReleaseKey(vk);
	}
};

#endif //__KeySender_H