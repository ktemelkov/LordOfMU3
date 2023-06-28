// stdafx.cpp : source file that includes just the standard includes
// LordOfMU.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include <psapi.h>


/**  
 * \brief 
 */
bool GetFileNameFromHandle(HANDLE hFile, char* pszOut, int iSize) 
{
	bool bSuccess = false;
	char pszFilename[MAX_PATH+1] = {0};

	// Get the file size.
	DWORD dwFileSizeHi = 0;
	DWORD dwFileSizeLo = GetFileSize(hFile, &dwFileSizeHi); 

	if(dwFileSizeLo == 0 && dwFileSizeHi == 0)
		return false;

	// Create a file mapping object.
	HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);

	if (hFileMap) 
	{
		// Create a file mapping to get the file name.
		void* pMem = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 1);

		if (pMem) 
		{
			if (GetMappedFileNameA(GetCurrentProcess(), pMem, pszFilename, _MAX_PATH)) 
			{
				// Translate path with device name to drive letters.
				char szTemp[_MAX_PATH] = {0};

				if (GetLogicalDriveStringsA(_MAX_PATH, szTemp)) 
				{
					char szName[_MAX_PATH] = {0};
					char szDrive[3] = " :";					
					bool bFound = false;
					char* p = szTemp;

					do 
					{
						// Copy the drive letter to the template string
						*szDrive = *p;

						// Look up each device name
						if (QueryDosDeviceA(szDrive, szName, _MAX_PATH))
						{
							size_t uNameLen = lstrlenA(szName);
							char szTemp2[_MAX_PATH+1] = {0};

							lstrcpynA(szTemp2, pszFilename, uNameLen+1);

							if (uNameLen < _MAX_PATH) 
							{
								bFound = lstrcmpiA(szTemp2, szName) == 0;

								if (bFound) 
								{
									if (iSize >= lstrlenA(pszFilename) - (int)uNameLen + 3)
									{
										bSuccess = true;

										lstrcpyA(pszOut, szDrive);
										lstrcatA(pszOut, pszFilename + uNameLen);
									}
								}
							}
						}

						// Go to the next NULL character.
						while (*p++);
					} while (!bFound && *p); // end of string
				}
			}

			UnmapViewOfFile(pMem);
		} 

		CloseHandle(hFileMap);
	}

	return bSuccess;
}
