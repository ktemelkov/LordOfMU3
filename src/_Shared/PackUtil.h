#pragma once


#define _MAX_PACKED_FILES 64

#include <windows.h>

#include "PathUtil.h"
#include "../_Shared/version.h"


#define _PACKUTIL_GET_REAL_FILE_SIZE(fileSizeOrId) (fileSizeOrId & 0x00FFFFFF)
#define _PACKUTIL_GET_DATA_ID(fileSizeOrId) ((fileSizeOrId & 0xFF000000) >> 24)
#define _PACKUTIL_MAKE_FILE_SIZE_OR_ID(fileSize, Id) ((Id << 24) | (fileSize & 0x00FFFFFF));




#define  _PACKUTIL_LSP_FILE_ID		1
#define  _PACKUTIL_PROXY_FILE_ID	2
#define  _PACKUTIL_GUI_FILE_ID		3

#define  _PACKUTIL_ROOT_DIR_ID		4

#define  _PACKUTIL_INSTALLER_FILE_ID 5



#include "shlwapi.h"
#define PATH_UTIL_CLASS
#define TEMP_DIRECTORY_ENV "%TEMP%"



/**  
 * \brief 
 */
class CPackUtil
{
public:
	typedef void(*ExtraCryptFn)(BYTE*,int);


	static void VersionCrypt(BYTE* buff, int size)
	{
		unsigned long ulVer[] = {__SOFTWARE_VERSION};
		BYTE crypt[] = {(BYTE)ulVer[0], (BYTE)(ulVer[1] / 100), (BYTE)(ulVer[1] % 100)};

		for (int i=0; i < size; i++)
		{
			buff[i] = buff[i] ^ crypt[i % sizeof(crypt)];
		}
	}


#pragma pack(push)
#pragma pack(1)
	/**  
	 * \brief 
	 */
	struct FileInfo
	{
		char szFileName[512];
		int  iFileSizeOrId;
		unsigned char crypt[4];
	};

	/**  
	 * \brief 
	 */
	struct PackInfo
	{
		FileInfo	fileInfo;
		int			fileOffset;
		int			dataId;
		ExtraCryptFn crypt;
	};
#pragma pack(pop)


public:
	/**  
	 * \brief 
	 */
	CPackUtil()
	{
		memset(m_szSrcFile,0, _MAX_PATH+1);
		m_lSrcRealSize = 0;
		m_lPackedFiles = 0;
		memset(&m_vPackFiles, 0, sizeof(m_vPackFiles));
	}


	/**  
	 * \brief 
	 */
	bool Init(const char* pszFileContainer)
	{
		HANDLE hFile = CreateFileA(pszFileContainer, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
		
		if (INVALID_HANDLE_VALUE == hFile)
			return false;

		lstrcpyA(m_szSrcFile, pszFileContainer);

		long offs = 0;

		while (1)
		{
			if (!CheckMagic(offs, hFile))
				break;


			PackInfo info;
			memset(&info, 0, sizeof(PackInfo));

			if (!ReadPackInfo(offs, hFile, info))
				break;

			m_vPackFiles[m_lPackedFiles++] = info;
		}

		m_lSrcRealSize = (int)GetFileSize(hFile, 0) - offs;
		CloseHandle(hFile);
		return true;
	}


	/**  
	 * \brief 
	 */
	bool AddFile(const char* pszPath, int fileId, ExtraCryptFn cr2 = 0)
	{
		int idx = 0;

		if (!IdExists(fileId, &idx))
		{
			PackInfo info = {0};
			memcpy(&m_vPackFiles[m_lPackedFiles++], &info, sizeof(PackInfo));

			idx = (int)m_lPackedFiles-1;
		}

		m_vPackFiles[idx].fileOffset = 0;
		m_vPackFiles[idx].dataId = fileId;

		memset(m_vPackFiles[idx].fileInfo.crypt, 0, sizeof(m_vPackFiles[idx].fileInfo.crypt));
		m_vPackFiles[idx].fileInfo.szFileName[511] = 0;
		lstrcpyA(m_vPackFiles[idx].fileInfo.szFileName, pszPath);

		HANDLE hFile = CreateFileA(pszPath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);

		if (INVALID_HANDLE_VALUE == hFile)
			return false;

		long lSize = (long)GetFileSize(hFile, 0);

		CloseHandle(hFile);

		m_vPackFiles[idx].fileInfo.iFileSizeOrId = _PACKUTIL_MAKE_FILE_SIZE_OR_ID(lSize, fileId);
		m_vPackFiles[idx].crypt = cr2;

		return true;
	}


	/**  
	 * \brief 
	 */
	bool AddData(const char pszData[512], int dataId)
	{
		int idx = 0;

		if (!IdExists(dataId, &idx))
		{
			memset(&m_vPackFiles[m_lPackedFiles++], 0, sizeof(PackInfo));

			idx = (int)m_lPackedFiles-1;
		}

		m_vPackFiles[idx].fileOffset = 0;
		m_vPackFiles[idx].dataId = dataId;

		memset(m_vPackFiles[idx].fileInfo.crypt, 0, sizeof(m_vPackFiles[idx].fileInfo.crypt));
		memcpy(m_vPackFiles[idx].fileInfo.szFileName, pszData, 512);
		
		m_vPackFiles[idx].fileInfo.iFileSizeOrId = _PACKUTIL_MAKE_FILE_SIZE_OR_ID(0, dataId);

		return true;
	}


	/**  
	 * \brief 
	 */
	bool SaveFile(const char *pszOutFolder, int fileId)
	{
		int idx = 0;

		char szOutFolder[_MAX_PATH+1];
		szOutFolder[_MAX_PATH] = 0;

		lstrcpyA(szOutFolder, pszOutFolder);

		for (int i=(int)lstrlenA(szOutFolder)-1; i >= 0 && szOutFolder[i] != '\\'; szOutFolder[i--] = 0);

		if (!IdExists(fileId, &idx))
			return false;
		
		char szFileName[512];
		memcpy(szFileName, m_vPackFiles[idx].fileInfo.szFileName, 512);

		DecodeData(szFileName, 512, m_vPackFiles[idx].fileInfo);

		char szFilePath[512];
		szFilePath[511] = 0;
		lstrcpyA(szFilePath, szOutFolder);

		PATH_UTIL_CLASS::PathAddBackslashA(szFilePath);
		lstrcatA(szFilePath, PATH_UTIL_CLASS::PathFindFileNameA(szFileName));

		bool ret = true;

		if (m_vPackFiles[idx].fileOffset == 0)
		{
			PATH_UTIL_CLASS::CopyFileA(szFileName, szFilePath, FALSE);
		}
		else
		{
			HANDLE hFile = CreateFileA(m_szSrcFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);

			if (INVALID_HANDLE_VALUE == hFile)
				return false;
	
			int iSize = _PACKUTIL_GET_REAL_FILE_SIZE(m_vPackFiles[idx].fileInfo.iFileSizeOrId);
			unsigned char* buff = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iSize);

			SetFilePointer(hFile, -m_vPackFiles[idx].fileOffset, 0, FILE_END);

			DWORD dwRead = 0;
			ReadFile(hFile, buff, (DWORD)iSize, &dwRead, 0);

			CloseHandle(hFile);

			if (dwRead != (DWORD)iSize)
			{
				HeapFree(GetProcessHeap(), 0, buff);
				return false;
			}

			DecodeData((char*)buff, iSize, m_vPackFiles[idx].fileInfo);

			hFile = CreateFileA(szFilePath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);

			if (INVALID_HANDLE_VALUE == hFile)
			{
				HeapFree(GetProcessHeap(), 0, buff);
				return false;
			}

			DWORD dwWritten = 0;
			WriteFile(hFile, buff, (DWORD)iSize, &dwWritten, 0);

			if ((DWORD)iSize != dwWritten)
			{
				ret = false;
			}

			CloseHandle(hFile);
			HeapFree(GetProcessHeap(), 0, buff);
		}

		return ret;
	}


	/**  
	 * \brief 
	 */
	bool ExtractFileToMemory(unsigned char* pszOutBuffer, int iBuffSize, int fileId)
	{
		int idx = 0;

		if (!IdExists(fileId, &idx))
			return false;
		
		if (m_vPackFiles[idx].fileOffset == 0)
			return false; // not packed

		int iSize = _PACKUTIL_GET_REAL_FILE_SIZE(m_vPackFiles[idx].fileInfo.iFileSizeOrId);

		if (iSize > iBuffSize)
			return false;  // buffer too small

		HANDLE hFile = CreateFileA(m_szSrcFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);

		if (INVALID_HANDLE_VALUE == hFile)
			return false;

		memset(pszOutBuffer, 0, iSize);

		SetFilePointer(hFile, -m_vPackFiles[idx].fileOffset, 0, FILE_END);

		DWORD dwRead = 0;
		ReadFile(hFile, pszOutBuffer, iSize, &dwRead, 0);
		CloseHandle(hFile);

		return DecodeData((char*)pszOutBuffer, iSize, m_vPackFiles[idx].fileInfo);
	}



	/**  
	 * \brief 
	 */
	bool GetData(char pszData[512], int dataId)
	{
		int idx = 0;

		if (!IdExists(dataId, &idx))
			return false;

		memcpy(pszData, m_vPackFiles[idx].fileInfo.szFileName, 512);
		return DecodeData(pszData, 512, m_vPackFiles[idx].fileInfo);
	}


	/**  
	 * \brief 
	 */
	bool IsFile(int id)
	{
		int idx = 0;

		if (!IdExists(id, &idx))
			return false;

		return _PACKUTIL_GET_REAL_FILE_SIZE(m_vPackFiles[idx].fileInfo.iFileSizeOrId) != 0;
	}

	/**  
	 * \brief 
	 */
	int GetPackedFileSize(int fileId)
	{
		int idx = 0;

		if (!IdExists(fileId, &idx))
			return 0;

		return _PACKUTIL_GET_REAL_FILE_SIZE(m_vPackFiles[idx].fileInfo.iFileSizeOrId);
	}


	/**  
	 * \brief 
	 */
	bool PackFile(const char* pszDstFilename, int iSrcSize = -1)
	{
		if (m_szSrcFile[0] == 0)
			return false;

		CStdString::ToLower(m_szSrcFile);
		CStdString::ToLower((char*)pszDstFilename);

		bool fTempFile = (lstrcmpA(m_szSrcFile, pszDstFilename) == 0);
		char szDestFile[_MAX_PATH+1];
		szDestFile[_MAX_PATH] = 0;

		if (fTempFile)
		{
			char szTempDir[_MAX_PATH+1];
			szTempDir[_MAX_PATH] = 0;

			PATH_UTIL_CLASS::ExpandEnvironmentStringsA(TEMP_DIRECTORY_ENV, szTempDir, _MAX_PATH);
			PATH_UTIL_CLASS::PathAddBackslashA(szTempDir);

			GetTempFileNameA(szTempDir, "pck", 0, szDestFile);
		}
		else
		{
			lstrcpyA(szDestFile, pszDstFilename);
		}


		HANDLE hSrc = INVALID_HANDLE_VALUE;
		HANDLE hDst = INVALID_HANDLE_VALUE;
		bool fRet = false;

		while(1)
		{
			hSrc = CreateFileA(m_szSrcFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);

			if (INVALID_HANDLE_VALUE == hSrc)
				break;

			hDst = CreateFileA(szDestFile, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_FLAG_RANDOM_ACCESS, 0);
			
			if (INVALID_HANDLE_VALUE == hDst)
				break;

			long lSrcFileSize = (iSrcSize > 0) ? iSrcSize : m_lSrcRealSize;
			unsigned char* buff = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lSrcFileSize);

			DWORD dwIOBytes = 0;
			ReadFile(hSrc, buff, lSrcFileSize, &dwIOBytes, 0);
			WriteFile(hDst, buff, lSrcFileSize, &dwIOBytes, 0);

			HeapFree(GetProcessHeap(), 0, buff);

			for (int i = 0; i < (int)m_lPackedFiles; i++)
				PackDataOrFile(hSrc, hDst, m_vPackFiles[i]);

			fRet = true;
			break;
		}


		if (INVALID_HANDLE_VALUE != hSrc)
			CloseHandle(hSrc);

		if (INVALID_HANDLE_VALUE != hDst)
			CloseHandle(hDst);

		if (fTempFile)
		{
			if (fRet)
			{
				DeleteFileA(pszDstFilename);
				MoveFileA(szDestFile, pszDstFilename);
			}
			else
				DeleteFileA(szDestFile);
		}

		return fRet;
	}


	/**  
	 * \brief 
	 */
	bool IdExists(int id, int* idx = 0)
	{
		for (int i = (int)m_lPackedFiles-1; i >= 0; i--)
		{
			if (m_vPackFiles[i].dataId == id)
			{
				if (idx)
					*idx = i;
				return true;
			}
		}

		return false;
	}


	/**  
	 * \brief 
	 */
	bool DecodeData(char* pszData, int iSize, const FileInfo& fileInfo)
	{
		int iCtyptLen = sizeof(fileInfo.crypt);

		for (int i=0; i < iSize; i++)
		{
			unsigned char a = fileInfo.crypt[i % iCtyptLen];
			unsigned char b = (unsigned char)pszData[i];

			pszData[i] = (char)(a ^ b);
		}

		return true;
	}


	/**  
	 * \brief 
	 */
	bool EncodeData(char* pszData, int iSize, const FileInfo& fileInfo)
	{
		int iCtyptLen = sizeof(fileInfo.crypt);
		
		for (int i=0; i < iSize; i++)
		{
			unsigned char a = fileInfo.crypt[i % iCtyptLen];
			unsigned char b = (unsigned char)pszData[i];

			pszData[i] = (char)(a ^ b);
		}

		return true;
	}


protected:
	/**  
	 * \brief 
	 */
	bool CheckMagic(long& offs, HANDLE hFile)
	{
		DWORD dwMagic = 0;

		SetFilePointer(hFile, -offs - sizeof(dwMagic), 0, FILE_END);

		DWORD dwRead = 0;
		ReadFile(hFile, &dwMagic, sizeof(dwMagic), &dwRead, 0);

		if (sizeof(dwMagic) != dwRead || dwMagic != CPackUtil::s_packMagic)
		{
			return false;
		}

		offs += sizeof(dwMagic);
		return true;
	}


	/**  
	 * \brief 
	 */
	bool ReadPackInfo(long& offs, HANDLE hFile, PackInfo& info)
	{
		offs += sizeof(info.fileInfo);

		SetFilePointer(hFile, -offs, 0, FILE_END);

		DWORD dwRead = 0;
		ReadFile(hFile, &info.fileInfo, sizeof(info.fileInfo), &dwRead, 0);

		if (sizeof(info.fileInfo) != dwRead)
			return false;

		offs += _PACKUTIL_GET_REAL_FILE_SIZE(info.fileInfo.iFileSizeOrId);
		info.fileOffset = offs;
		info.dataId = _PACKUTIL_GET_DATA_ID(info.fileInfo.iFileSizeOrId);

		return true;
	}


	/**  
	 * \brief 
	 */
	bool PackDataOrFile(HANDLE hSrc, HANDLE hDst, PackInfo& packInfo)
	{
		int iFileSize = _PACKUTIL_GET_REAL_FILE_SIZE(packInfo.fileInfo.iFileSizeOrId);
		
		FileInfo fileInfo;
		memset(&fileInfo, 0, sizeof(FileInfo));

		if (iFileSize == 0)
		{
			PrepareOutputDataFileInfo(packInfo, fileInfo);			
		}
		else
		{
			unsigned char* buff = (unsigned char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iFileSize);

			if (packInfo.fileOffset != 0)
			{
				SetFilePointer(hSrc, -packInfo.fileOffset, 0, FILE_END);

				DWORD dwRead = 0;
				ReadFile(hSrc, buff, (DWORD)iFileSize, &dwRead, 0);
			}
			else
			{
				HANDLE hSrcExt = CreateFileA(packInfo.fileInfo.szFileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);

				if (INVALID_HANDLE_VALUE != hSrcExt)
				{
					DWORD dwRead = 0;
					ReadFile(hSrcExt, buff, (DWORD)iFileSize, &dwRead, 0);
					CloseHandle(hSrcExt);
				}

				if (packInfo.crypt)
					packInfo.crypt(buff, iFileSize);
			}

			PrepareOutputFileFileInfo(packInfo, fileInfo, buff);

			DWORD dwWritten = 0;
			WriteFile(hDst, buff, (DWORD)iFileSize, &dwWritten, 0);
			HeapFree(GetProcessHeap(), 0, buff);
		}

		DWORD dwWritten = 0;
		WriteFile(hDst, &fileInfo, sizeof(fileInfo), &dwWritten, 0);
		WriteFile(hDst, &s_packMagic, sizeof(s_packMagic), &dwWritten, 0);

		return true;
	}


	/**  
	 * \brief 
	 */
	bool PrepareOutputDataFileInfo(const PackInfo& packInfo, FileInfo& fileInfo)
	{
		memcpy(fileInfo.szFileName, packInfo.fileInfo.szFileName, 512);
		fileInfo.iFileSizeOrId = _PACKUTIL_MAKE_FILE_SIZE_OR_ID(0, packInfo.dataId);

		CryptInit(fileInfo);

		DecodeData(fileInfo.szFileName, 512, packInfo.fileInfo);
		EncodeData(fileInfo.szFileName, 512, fileInfo);

		return true;
	}

	/**  
	 * \brief 
	 */
	bool PrepareOutputFileFileInfo(const PackInfo& packInfo, FileInfo& fileInfo, unsigned char* buff)
	{
		int iFileSize = _PACKUTIL_GET_REAL_FILE_SIZE(packInfo.fileInfo.iFileSizeOrId);
		fileInfo.iFileSizeOrId = packInfo.fileInfo.iFileSizeOrId;

		CryptInit(fileInfo);
		DecodeData((char*)buff, iFileSize, packInfo.fileInfo);
		EncodeData((char*)buff, iFileSize, fileInfo);

		char szTemp[512];
		memcpy(szTemp, packInfo.fileInfo.szFileName, 512);
		DecodeData(szTemp, 512, packInfo.fileInfo);

		lstrcpyA(fileInfo.szFileName, PATH_UTIL_CLASS::PathFindFileNameA(szTemp));
		EncodeData(fileInfo.szFileName, 512, fileInfo);

		return true;
	}


	/**  
	 * \brief 
	 */
	bool CryptInit(FileInfo& fileInfo)
	{
		BYTE randBuf[4];
		*((DWORD*)randBuf) = GetTickCount();

		randBuf[1] ^= randBuf[0];
		randBuf[2] ^= (BYTE)(randBuf[1] & 0x7F);
		randBuf[3] ^= (BYTE)(randBuf[2] & 0x7F);

		fileInfo.crypt[0] = (unsigned char)((randBuf[0] % 254) + 1);
		fileInfo.crypt[1] = (unsigned char)((randBuf[1] % 254) + 1);
		fileInfo.crypt[2] = (unsigned char)((randBuf[2] % 254) + 1);
		fileInfo.crypt[3] = (unsigned char)((randBuf[3] % 254) + 1);

		return true;
	}

private:
	PackInfo m_vPackFiles[_MAX_PACKED_FILES];
	long m_lPackedFiles;
	char m_szSrcFile[_MAX_PATH+1];
	long m_lSrcRealSize;

private:
	static const unsigned long s_packMagic = 0x50BADA55;
};

