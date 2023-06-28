// Packer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\_Shared\PackUtil.h"
#include "..\_Shared\InjectUtil.h"
#include <atlstr.h>


void PrintUsage();
int PackTarget(TCHAR* pszAction, TCHAR* pszTarget, TCHAR* pszInput, int iId);


/**  
 * \brief 
 */
void ExtraCrypt(BYTE* buff, int size)
{
	for (int i=0; i < size/2; i++)
	{
		BYTE tmp = buff[i];
		buff[i] = buff[size-i-1] ^ 0xAC;
		buff[size-i-1] = tmp ^ 0xDC;
	}
}


/**  
 * \brief 
 */
void __declspec(naked) myloader(BYTE* buff, int size) 
{ 
	__asm 
	{ 
		push        ebp					// Save ebp
		mov         ebp,esp				// Set stack frame pointer
		sub         esp,0xD8			// reserve space for local variables
		push        ebx  
		push        esi  
		push        edi  
		lea         edi,[ebp-0xD8] 
		mov         ecx,0x36 
		mov         eax,0xCCCCCCCC
		rep stos    dword ptr es:[edi]
	}

	for (int i=0; i < size/2; i++)
	{
		BYTE tmp = buff[i];
		buff[i] = buff[size-i-1] ^ 0xDC;
		buff[size-i-1] = tmp ^ 0xAC;
	}

	__asm
	{
		pop edi  
		pop esi  
		pop ebx
		add esp, 0xD8
		mov esp, ebp      // Restore stack pointer
		pop ebp           // Restore ebp

		ret 

		// end marker 
		DWORD_TYPE(LDR_CODE_END);
	} 
} 



/**  
 * \brief 
 */
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 2 && _strcmpi(CT2CA(argv[1]), "/packall") == 0)
	{
		CStdString strDst = CPathUtil::ConstructModulePath("Bootstrapper.dll");

		CPackUtil pack;
		pack.Init(strDst.c_str());

		int lSize = pack.GetPackedFileSize(_PACKUTIL_INSTALLER_FILE_ID);

		pack.AddFile(CPathUtil::ConstructModulePath("MUEliteClicker.dll").c_str(), _PACKUTIL_PROXY_FILE_ID);
		pack.AddFile(CPathUtil::ConstructModulePath("MUAutoClicker.dll").c_str(), _PACKUTIL_GUI_FILE_ID);
		pack.AddFile(CPathUtil::ConstructModulePath("MACLocator.dll").c_str(), _PACKUTIL_LSP_FILE_ID);
		pack.AddFile(CPathUtil::ConstructModulePath("LspInstaller.dll").c_str(), _PACKUTIL_INSTALLER_FILE_ID, ExtraCrypt);		
		pack.PackFile(strDst.c_str());


		BYTE* pLoader = (BYTE*)myloader;

		if (*pLoader == 0xE9)
		{
			DWORD dwOffs = *((DWORD*)(pLoader + 1));
			pLoader += dwOffs + 5;
		}

		DWORD dwDlSize = (DWORD)CInjectUtil::SearchDWORD(pLoader, LDR_CODE_END) - (DWORD)pLoader + 4; 

		// allocate local buffer for loader code 
		BYTE* pLoaderFile = new BYTE[dwDlSize];
		memset(pLoaderFile, 0, dwDlSize);

		memcpy(pLoaderFile, pLoader, dwDlSize); 

		FILE* f = 0;
		if (0 == fopen_s(&f, CPathUtil::ConstructModulePath("LspDecrypt.bin").c_str(), "wb"))
		{
			fwrite(pLoaderFile, 1, dwDlSize, f);
			fclose(f);
		}

		delete[] pLoaderFile;

		printf("Done.\r\n");
		return 0;
	}

	if (argc < 5)
	{
		PrintUsage();
		return 0;
	}

	return PackTarget(argv[1], argv[2], argv[3], _ttoi(argv[4]));
}


/**  
 * \brief 
 */
int PackTarget(TCHAR* pszAction, TCHAR* pszTarget, TCHAR* pszInput, int iId)
{
	if (!PathFileExists(pszTarget))
	{
		printf("Target file does not exist!\r\n");
		return 0;
	}

	CPackUtil packer;
	packer.Init(CT2CA(pszTarget));


	if (_strcmpi(CT2CA(pszAction), "/pack") == 0)
	{
		BOOL fFile = PathFileExists(pszInput) && !PathIsDirectory(pszInput);

		if (fFile)
			packer.AddFile(CT2CA(pszInput), iId);
		else
			packer.AddData(CT2CA(pszInput), iId);


		char szTempDir[_MAX_PATH+1] = {0};
		ExpandEnvironmentStringsA("%TEMP%", szTempDir, _MAX_PATH);
		PathAddBackslashA(szTempDir);

		char szTempFile[_MAX_PATH+1] = {0};
		GetTempFileNameA(szTempDir, "pkr", 0, szTempFile);

		if (!packer.PackFile(szTempFile))
		{
			printf("Error saving packed target!\r\n");
			return -1;
		}

		DeleteFile(pszTarget);
		MoveFile(CA2CT(szTempFile), pszTarget);
	}
	else
	{
		if (packer.IsFile(iId))
			packer.SaveFile(CT2CA(pszInput), iId);
		else
		{
			char szData[512] = {0};
			packer.GetData(szData, iId);

			szData[511] = 0;
			printf("Data: %s \r\b", szData);
		}
	}

	return 0;
}

/**  
 * \brief 
 */
void PrintUsage()
{
	printf("Usage: \r\n");
	printf("    packer.exe </pack|/unpack> <target_file> <input_file|output_dir|data_string> <input_file_id|data_string_id>\r\n");
	printf("	    target_file - the file which will be used as container\r\n");
	printf("	    input_file  - the file to be packed inside the container file\r\n");
	printf("	    data_string  - string data to be packed inside the container file\r\n");
	printf("	    input_file_id  - set an integer identifier for input file\r\n");
	printf("	    data_string_id  - set an integer identifier for string data\r\n");
}