#ifndef __version_H
#define __version_H

#pragma once


#define __SOFTWARE_VERSION 3,0501,1204,05000
#define __SOFTWARE_VERSION_STR "3.0501.1204.05000"

#define __SOFTWARE_VERSION_ABOUT _T("MUAutoClicker v3.0501")
#define __SOFTWARE_PRODUCT_NAME "MUAutoClicker"

#define __SOFTWARE_COPYRIGHT "Copyright 2012 (c) Kalin Temelkov"
#define __SOFTWARE_COMPANY_NAME "Kalin Temelkov"

#define __BOOTSTRP_DLL_NAME	"Bootstrapper.dll"
#define __CLICKER_DLL_NAME	"MUAutoClicker.dll"
#define __LOADER_DLL_NAME	"MUAutoLoader.dll"
#define __LORDOFMU_DLL_NAME "MUEliteClicker.dll"
#define __LAUNCHER_EXE_NAME "MUAutoClicker.exe"


namespace VersionUtil
{
	/**  
	 * \brief 
	 */
	inline bool IsFree()
	{
		DWORD dwVer[] = {__SOFTWARE_VERSION};
		return (dwVer[3] % 1000)/100 == 0;
	}
}

#endif // __version_H
