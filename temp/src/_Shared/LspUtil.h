#pragma once

//#include "WinsockUtil.h"
#include "OleUtil.h"

#define _CRT_SECURE_NO_WARNING

#include <intrin.h>
#pragma intrinsic(memcpy,memset)


//#define WINSOCK_UTIL_CLASS CWinsockUtil

#include "winsock2.h"
#include "ws2spi.h"
#include "sporder.h"
#define WINSOCK_UTIL_CLASS 

#include "base64.h"
#define MACLOCATOR_DLL_NAME base64_decode("TUFDTG9jYXRvci5kbGw=").c_str()

#define OLE_UTIL_CLASS 

//CStringTable::GetString(_STRING_LordOfMU_LspDllName);
// COleUtil

/**  
 * \brief 
 */
class CLspConfig
{
public:
	/**  
	 * \brief 
	 */
	struct ProviderGUID
	{
	public: 
		operator GUID(){ return *GetGuid(); }
		operator GUID*(){ return GetGuid(); }
		operator const wchar_t*(){ OLE_UTIL_CLASS::StringFromGUID2(*GetGuid(), m_strGuid, 255); return m_strGuid; }

	private:
		GUID* GetGuid()
		{
			// {E2AAF440-64A3-49b7-AEBF-192DB463893E}
			static GUID providerGuid = { 0xe2aaf440, 0x64a3, 0x49b7, { 0xae, 0xbf, 0x19, 0x2d, 0xb4, 0x63, 0x89, 0x3e } };
			return &providerGuid;
		}

		void operator=(const GUID*){}
		void operator=(const GUID&){}

	private:
		wchar_t m_strGuid[256];
	};



	/**  
	 * \brief 
	 */
	struct ProviderPath
	{
	public:
		ProviderPath(){ memset(m_strPath, 0, sizeof(wchar_t)*(_MAX_PATH+1)); }

		operator LPWSTR()
		{
			CStdString str;
			str = MACLOCATOR_DLL_NAME;

			MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, m_strPath, _MAX_PATH);
			return m_strPath;
		}

	private:
		void operator=(const wchar_t*){}

	private:
		wchar_t m_strPath[_MAX_PATH+1];
	};

};




/**  
 * \brief 
 */
class CLspUtil
{
public:
	/**  
	 * \brief 
	 */
	static bool EnumerateProviders(WSAPROTOCOL_INFOW** ppProtocols, int* piCount)
	{
		DWORD dwProtocolInfoSize = 0;
		int iErrorCode = NO_ERROR;

		// Find the size of the buffer
		if (SOCKET_ERROR != WINSOCK_UTIL_CLASS::WSCEnumProtocols(0, 0, &dwProtocolInfoSize, &iErrorCode)
				|| WSAENOBUFS != iErrorCode)
		{
			return false;
		}

		// Allocate the buffer
		LPWSAPROTOCOL_INFOW pProtocolInfo = (LPWSAPROTOCOL_INFOW)(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwProtocolInfoSize+1));

		if (!pProtocolInfo)
			return false;

		int iCount = WINSOCK_UTIL_CLASS::WSCEnumProtocols(0, pProtocolInfo, &dwProtocolInfoSize, &iErrorCode);

		if (iCount >= 0)
		{
			*ppProtocols = pProtocolInfo;
			*piCount = iCount;
		}
		else
		{
			HeapFree(GetProcessHeap(), 0, pProtocolInfo);
		}

		return iCount >= 0;
	}


	/**  
	 * \brief 
	 */
	static void FreeProviders(WSAPROTOCOL_INFOW* pProtocols)
	{
		if (pProtocols)
			HeapFree(GetProcessHeap(), 0, pProtocols);
	}


	/**  
	 * \brief 
	 */
	static bool IsLspInstalled()
	{
		WSAPROTOCOL_INFOW provider;
		return CLspUtil::FindLspProvider(provider);
	}


	/**  
	 * \brief 
	 */
	static bool FindLspProvider(WSAPROTOCOL_INFOW& outProvider)
	{
		WSAPROTOCOL_INFOW* vProtocols;
		int iSize = 0;

		if (!CLspUtil::EnumerateProviders(&vProtocols, &iSize))
			return false;

		for (int i = (int)iSize - 1; i >= 0; i--)
		{
			if (InlineIsEqualGUID(vProtocols[i].ProviderId, CLspConfig::ProviderGUID()))
			{
				memcpy(&outProvider, &vProtocols[i], sizeof(WSAPROTOCOL_INFOW));
				CLspUtil::FreeProviders(vProtocols);
				return true;
			}
		}

		CLspUtil::FreeProviders(vProtocols);
		return false;
	}


	/**  
	 * \brief 
	 */
	static bool InstallLsp()
	{
		WSAPROTOCOL_INFOW baseProvider;
		WSAPROTOCOL_INFOW lspProvider;

		return CLspUtil::FindBaseProvider(baseProvider)
			&& CLspUtil::AddProvider(baseProvider)
			&& CLspUtil::FindLspProvider(lspProvider)
			&& CLspUtil::AddChain(baseProvider, lspProvider)
			;
	}

	
	/**  
	 * \brief 
	 */
	static bool FindBaseProvider(WSAPROTOCOL_INFOW& outProvider)
	{
		WSAPROTOCOL_INFOW* vProtocols;
		int iSize = 0;

		if (!CLspUtil::EnumerateProviders(&vProtocols, &iSize))
			return false;

		int iCount = (int)iSize;
		int idx = -1;

		for (int i = 0; i < iCount; i++)
		{
			if (vProtocols[i].iAddressFamily == AF_INET
				&& vProtocols[i].iProtocol == IPPROTO_TCP
				&& vProtocols[i].ProtocolChain.ChainLen == 1)
			{
				// found base TCP provider
				idx = i;
				break;
			}
		}

		if (idx < 0)
		{
			CLspUtil::FreeProviders(vProtocols);
			return false;
		}

		memcpy(&outProvider, &vProtocols[idx], sizeof(WSAPROTOCOL_INFOW));
		CLspUtil::FreeProviders(vProtocols);
		return true;
	}


	/**  
	 * \brief 
	 */
	static bool AddProvider(WSAPROTOCOL_INFOW& baseProvider)
	{
		WSAPROTOCOL_INFOW addProtocol;
		memcpy(&addProtocol, &baseProvider, sizeof(WSAPROTOCOL_INFOW));

		addProtocol.dwServiceFlags1 &= (~XP1_IFS_HANDLES);
		addProtocol.iSocketType = 0;
		addProtocol.iProtocol   = 0;
		addProtocol.dwProviderFlags |= PFL_HIDDEN;
		addProtocol.dwProviderFlags &= (~PFL_MATCHES_PROTOCOL_ZERO);
		addProtocol.ProtocolChain.ChainLen = LAYERED_PROTOCOL;

		lstrcpynW(addProtocol.szProtocol, CLspConfig::ProviderGUID(), WSAPROTOCOL_LEN);

		INT iErr = 0;
		return 0 == WINSOCK_UTIL_CLASS::WSCInstallProvider(CLspConfig::ProviderGUID(), CLspConfig::ProviderPath(), &addProtocol, 1, &iErr);
	}


	/**  
	 * \brief 
	 */
	static bool AddChain(WSAPROTOCOL_INFOW& baseProvider, WSAPROTOCOL_INFOW& lspProvider)
	{
		WSAPROTOCOL_INFOW chainProvider;
		memcpy(&chainProvider, &baseProvider, sizeof(WSAPROTOCOL_INFOW));

		lstrcatW(chainProvider.szProtocol, L"*");

		chainProvider.ProtocolChain.ChainEntries[0] = lspProvider.dwCatalogEntryId;
		chainProvider.ProtocolChain.ChainEntries[1] = baseProvider.dwCatalogEntryId;
		chainProvider.ProtocolChain.ChainLen = 2;

		chainProvider.dwServiceFlags1 &= (~XP1_IFS_HANDLES);

		if (S_OK != CoCreateGuid(&chainProvider.ProviderId))
			return false;
		
		INT iErr = 0;
		return 0 == WINSOCK_UTIL_CLASS::WSCInstallProvider(&chainProvider.ProviderId, CLspConfig::ProviderPath(), &chainProvider, 1, &iErr);
	}


	/**  
	 * \brief 
	 */
	static bool ReorderCatalog()
	{
		WSAPROTOCOL_INFOW* vProtocols;
		int iCount = 0;

		if (!CLspUtil::EnumerateProviders(&vProtocols, &iCount))
			return false;


		int iLspIdx = -1;

		for (int i = iCount - 1; i >= 0; i--)
		{
			if (InlineIsEqualGUID(vProtocols[i].ProviderId, CLspConfig::ProviderGUID()))
			{
				iLspIdx = i;
				break;
			}
		}

		if (iLspIdx < 0)
		{
			CLspUtil::FreeProviders(vProtocols);
			return false;
		}


		DWORD dwLspCatId = vProtocols[iLspIdx].dwCatalogEntryId;
		DWORD* dwOrderedIds = (DWORD*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iCount*sizeof(DWORD));
		int idx = 0;


		for (int i = 0; i < iCount; i++)
		{
			if (CLspUtil::IsCatalogIdInChain(dwLspCatId, vProtocols[i].ProtocolChain))
				dwOrderedIds[idx++] = vProtocols[i].dwCatalogEntryId;
		}

		for (int i = 0; i < iCount; i++)
		{
			if (!CLspUtil::IsCatalogIdInChain(dwLspCatId, vProtocols[i].ProtocolChain))
				dwOrderedIds[idx++] = vProtocols[i].dwCatalogEntryId;
		}


		bool fRet = (0 == WINSOCK_UTIL_CLASS::WSCWriteProviderOrder(dwOrderedIds, (DWORD)iCount));

		HeapFree(GetProcessHeap(), 0, dwOrderedIds);
		CLspUtil::FreeProviders(vProtocols);
		return fRet;
	}


	/**  
	 * \brief 
	 */
	static bool IsCatalogIdInChain(DWORD dwCatID, WSAPROTOCOLCHAIN& chain)
	{
		for (int i = 0; i < chain.ChainLen; i++)
		{
			if (chain.ChainEntries[i] == dwCatID)
				return true;
		}

		return false;
	}


	/**  
	 * \brief 
	 */
	static bool RemoveLsp()
	{
		WSAPROTOCOL_INFOW* vProtocols;
		int iCount = 0;


		if (!CLspUtil::EnumerateProviders(&vProtocols, &iCount))
			return false;

		int iLspIdx = -1;

		for (int i = iCount - 1; i >= 0; i--)
		{
			if (InlineIsEqualGUID(vProtocols[i].ProviderId, CLspConfig::ProviderGUID()))
			{
				iLspIdx = i;
				break;
			}
		}

		if (iLspIdx < 0)
		{
			CLspUtil::FreeProviders(vProtocols);
			return false;
		}


		DWORD dwLspCatId = vProtocols[iLspIdx].dwCatalogEntryId;

		int* vRemCatIdx = (int*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(int)*(iCount+1));
		int idxPos = 0;

		CLspUtil::BuildRemoveList(dwLspCatId, vProtocols, iCount, vRemCatIdx, idxPos);

		for (int i = (int)idxPos-1; i >= 0; i--)
		{
			INT iErrNo = 0;
			if (0 != WINSOCK_UTIL_CLASS::WSCDeinstallProvider(&vProtocols[vRemCatIdx[i]].ProviderId, &iErrNo))
			{
				CLspUtil::FreeProviders(vProtocols);
				HeapFree(GetProcessHeap(), 0, vRemCatIdx);
				return false;
			}
		}

		INT iErr = 0;
		bool fRes = (0 == WINSOCK_UTIL_CLASS::WSCDeinstallProvider(&vProtocols[iLspIdx].ProviderId, &iErr));
		
		CLspUtil::FreeProviders(vProtocols);
		HeapFree(GetProcessHeap(), 0, vRemCatIdx);
		return fRes;
	}


	/**  
	 * \brief 
	 */
	static bool BuildRemoveList(DWORD dwRemCatId, WSAPROTOCOL_INFOW* vProtocols, int iCount, int* indexes, int& idxPos)
	{
		for (int i = 0; i < iCount; i++)
		{
			if (CLspUtil::IsCatalogIdInChain(dwRemCatId, vProtocols[i].ProtocolChain))
			{
				int j = idxPos-1;

				for (j; j >= 0; --j)
					if (indexes[j] == i)
						break;

				if (j >= 0)
					indexes[idxPos++] = i;
				
				if (!CLspUtil::BuildRemoveList(vProtocols[i].dwCatalogEntryId, vProtocols, iCount, indexes, idxPos))
					return false;
			}
		}

		return true;
	}
};

#ifdef _UNDEF_CRT_SECURE_NO_WARNINGS
	#undef _CRT_SECURE_NO_WARNINGS
#endif
