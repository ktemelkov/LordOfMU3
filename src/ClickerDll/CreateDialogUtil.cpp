//ResRead.Cpp
#include "stdafx.h"
#include <stddef.h>
#include "CreateDialogUtil.h"



/**  
 * \brief 
 */
int WINAPI CCreateDialogUtil::MyResReadCallback(const RESOURCEHEADER* pResHdr, LPVOID lpData, LPARAM lParam)
{
	RESCALLBACKPARAM* pParam = (RESCALLBACKPARAM*)lParam; 

	if (pResHdr->pszType == MAKEINTRESOURCEA(5) && (WORD)pResHdr->pszName == (WORD)pParam->uiIdd)
	{
		DLGTEMPLATE* pTemplate = (DLGTEMPLATE*)new BYTE[pResHdr->dwDataSize];
		memcpy(pTemplate, lpData, pResHdr->dwDataSize);
		
		*pParam->ppDlgTemplate = pTemplate;
	}
	else if (pResHdr->pszType == MAKEINTRESOURCEA(240) && (WORD)pResHdr->pszName == (WORD)pParam->uiIdd)
	{
		BYTE* pInitData = new BYTE[pResHdr->dwDataSize];
		memcpy(pInitData, lpData, pResHdr->dwDataSize);

		*pParam->ppInitData = pInitData;
	}

	return 1;
}




/**  
 * \brief 
 */
LPCSTR CCreateDialogUtil::GetIdOrString(LPWORD* ppWord, LPSTR buffer, int bufsize)
{
	LPCSTR pIdStr;
	LPWORD pWord=*ppWord;

	if (pWord[0]==0xFFFF)
	{
		pIdStr=(LPCSTR)(pWord[1]);
		pWord+=2;
	}
	else
	{
		if (IsBadStringPtrW((LPCWSTR)pWord,bufsize)) return NULL;

		pWord += WideCharToMultiByte(0,0,(LPCWSTR)pWord,-1,buffer,bufsize,NULL,NULL);
		pIdStr = buffer;        
	}

	*ppWord=pWord;
	return pIdStr;
}



/**  
 * \brief 
 */
int WINAPI CCreateDialogUtil::ReadResFile(LPCTSTR pszFilename, READRESFILECALLBACK ReadResFileCallback, LPARAM userParam)
{
	RESOURCEHEADER rh;
	HANDLE h;
	LPBYTE pData;
	LPWORD pWord;
	CHAR szType[256];
	CHAR szName[256];
	DWORD dw,dwGot;
	int error;

	//Validate parameters
	if (IsBadCodePtr((FARPROC)ReadResFileCallback) || IsBadStringPtr(pszFilename,MAX_PATH))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return ERROR_INVALID_PARAMETER;
	}

	dw=GetFileAttributes(pszFilename);
	if (dw&FILE_ATTRIBUTE_DIRECTORY)//file not present or is a directory
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return ERROR_FILE_NOT_FOUND;
	}            

	//Open file
	h=CreateFile(pszFilename,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	if (h==INVALID_HANDLE_VALUE) return GetLastError();      

	//Check RES file starts with blank record
	ReadFile(h,&rh,sizeof(rh),&dwGot,NULL);      
	if (dwGot!=sizeof(rh) ||            
		rh.dwHeaderSize!=sizeof(rh) ||
		rh.dwDataSize!=0 ||
		rh.pszType!=(LPCSTR)0xFFFF ||
		rh.pszName!=(LPCSTR)0xFFFF ||
		rh.dwDataVersion!=0 || 
		rh.wMemoryFlags!=0 ||
		rh.wLanguageId!=0 ||
		rh.dwVersion!=0 ||
		rh.dwCharacteristics!=0)
	{
		CloseHandle(h);
		SetLastError(ERROR_BAD_FORMAT);
		return ERROR_BAD_FORMAT;
	}      

	//Enter loop reading resources
	pData=NULL;
	error=ERROR_BAD_FORMAT;
	for(;;)
	{
		ZeroMemory(&rh,sizeof(rh));

		dw=sizeof(DWORD)*2;
		ReadFile(h,&rh,dw,&dwGot,NULL);
		if (dwGot==0)
		{
			//End of file, no more RESOURCEHEADER entries
			error=ERROR_SUCCESS;
			break; 
		}            

		if (dw!=dwGot) break;            
		if (rh.dwHeaderSize<sizeof(rh)) break;
		if (rh.dwDataSize==0) break;
		pData=(LPBYTE)GlobalAlloc(GPTR,rh.dwHeaderSize);
		if (!pData) break;


		dw=rh.dwHeaderSize-sizeof(DWORD)*2;
		ReadFile(h,pData,dw,&dwGot,NULL);
		if (dw!=dwGot) break;

		//Read the type & id be they numeric or string identifiers
		pWord=(LPWORD)pData;
		rh.pszType=GetIdOrString(&pWord,szType,sizeof(szType));
		if (rh.pszType==NULL) break;
		rh.pszName=GetIdOrString(&pWord,szName,sizeof(szName));
		if (rh.pszName==NULL) break;

		//Advance to next dword
		pWord=(LPWORD)(((int)pWord+3)&~3);

		//Read rest of the header
		dw=sizeof(RESOURCEHEADER)-offsetof(RESOURCEHEADER,dwDataVersion);
		if (IsBadReadPtr(pWord,dw)) break;

		memcpy(&rh.dwDataVersion,pWord,dw);
		GlobalFree(pData);
		pData=NULL;

		//Read the actual resource data (rounded up to dword)
		dw=(rh.dwDataSize+3)&~3;
		pData=(LPBYTE)GlobalAlloc(GPTR,dw);
		if (!pData) break;
		ReadFile(h,pData,dw,&dwGot,NULL);
		if (dw!=dwGot) break;                    

		if (ReadResFileCallback(&rh,pData,userParam)==0)
		{
			error=ERROR_SUCCESS;
			break;
		}

		GlobalFree(pData);
		pData=NULL;
	}

	if (pData)
	{
		GlobalFree(pData);
		pData=NULL;
	}      
	CloseHandle(h);

	SetLastError(error);
	return error;
}
