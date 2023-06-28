#pragma once

#include <stdio.h>


/**  
 * \brief 
 */
class CProtocolSettings
{
public:
	struct SettingsData
	{
		DWORD dwClientType;
		DWORD dwProtocolType;
		DWORD dwFlags;
		char  szMuPath[_MAX_PATH+1];
	};

public:
	/**  
	 * \brief 
	 */
	CProtocolSettings(const char* pszRootPath = 0)
	{
		Default();

		if (pszRootPath)
			strcpy_s(m_szRootPath, _MAX_PATH, pszRootPath);
		else
			m_szRootPath[0] = 0;
	}


	/**  
	 * \brief 
	 */
	bool Load()
	{
		Default();

		char szFile[_MAX_PATH+1] = {0};
		GetSettingsFilename(szFile);

		FILE* f = 0;

		if (0 != fopen_s(&f, szFile, "rb"))
			return false;

		fread(&data, 1, sizeof(SettingsData), f);
		fclose(f);

		return true;
	}


	/**  
	 * \brief 
	 */
	bool Save()
	{
		char szFile[_MAX_PATH+1] = {0};
		GetSettingsFilename(szFile);

		FILE* f = 0;

		if (0 != fopen_s(&f, szFile, "wb"))
			return false;

		fwrite(&data, 1, sizeof(SettingsData), f);
		fflush(f);
		fclose(f);

		return true;
	}


	/**  
	 * \brief 
	 */
	void Default()
	{
		memset(&data, 0, sizeof(SettingsData));
		data.dwProtocolType = 1;
	}


	/**  
	 * \brief 
	 */
	void GetSettingsFilename(char szFile[_MAX_PATH+1])
	{
		if (m_szRootPath[0] == 0)
		{
			GetModuleFileNameA(0, szFile, _MAX_PATH);
			for (int i = strlen(szFile)-1; i >= 0 && szFile[i] != '\\'; szFile[i--] = 0);
		}
		else
		{
			strcpy_s(szFile, _MAX_PATH, m_szRootPath);
		}

		strcat_s(szFile, _MAX_PATH, "protocol.dat");
	}

public:
	SettingsData data;

private:
	char m_szRootPath[_MAX_PATH+1];
};
