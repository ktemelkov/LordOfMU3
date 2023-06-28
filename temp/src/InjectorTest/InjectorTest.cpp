// InjectorTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\_Shared\MemoryModule.h"


/**  
 * \brief 
 */
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc > 1)
		LoadLibraryA("LspInstaller.dll");


	char szFile[MAX_PATH+1] = {0};
	GetModuleFileNameA(0, szFile, MAX_PATH);

	for (int i=lstrlenA(szFile)-1; i >= 0 && szFile[i] != '\\'; szFile[i--] = 0);

	char szLspInstaller[MAX_PATH+1] = {0};
	lstrcpyA(szLspInstaller, szFile);
	lstrcatA(szLspInstaller, "LspInstaller.dll");

	FILE* f = fopen(szLspInstaller, "rb");

	if (!f)
	{
		printf("ERROR: Cannot open DLL\r\n");
		return -1;
	}

	fseek(f, 0, SEEK_END);
	int len = ftell(f);

	if (len <= 0)
	{
		printf("ERROR: Bad DLL file size.\r\n");
		fclose(f);
		return -1;
	}
	
	fseek(f, 0, SEEK_SET);

	BYTE* buff = new BYTE[len];
	fread(buff, 1, len, f);

	MemoryLoadLibrary(buff, (LPVOID)szFile);

	delete[] buff;
//
//	const char* pszApp = "C:\\Games\\TelNET MuOnline\\main.exe";
	const char* pszApp = "C:\\Program Files\\FileZilla FTP Client\\filezilla.exe";

	ShellExecuteA(0, "open", pszApp, 0, 0, SW_SHOW);
	return 0;
}

