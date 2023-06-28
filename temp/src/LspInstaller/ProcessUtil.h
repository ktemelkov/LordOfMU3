#pragma once

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "Winternl.h"
#include <ws2spi.h>
#include <tlhelp32.h>



/**  
 * \brief 
 */
namespace ProcessUtil
{
	typedef struct _CLIENT_ID { HANDLE UniqueProcess; HANDLE UniqueThread;} CLIENT_ID, *PCLIENT_ID;
	typedef enum _MEMORY_INFORMATION_CLASS {MemoryBasicInformation} MEMORY_INFORMATION_CLASS, *PMEMORY_INFORMATION_CLASS;

	typedef struct _PEB_LDR_DATA {
		ULONG                   Length;
		BOOLEAN                 Initialized;
		PVOID                   SsHandle;
		LIST_ENTRY              InLoadOrderModuleList;
		LIST_ENTRY              InMemoryOrderModuleList;
		LIST_ENTRY              InInitializationOrderModuleList;
	} PEB_LDR_DATA, *PPEB_LDR_DATA;

	typedef struct _LDR_MODULE {
		LIST_ENTRY              InLoadOrderModuleList;
		LIST_ENTRY              InMemoryOrderModuleList;
		LIST_ENTRY              InInitializationOrderModuleList;
		PVOID                   BaseAddress;
		PVOID                   EntryPoint;
		ULONG                   SizeOfImage;
		UNICODE_STRING          FullDllName;
		UNICODE_STRING          BaseDllName;
		ULONG                   Flags;
		SHORT                   LoadCount;
		SHORT                   TlsIndex;
		LIST_ENTRY              HashTableEntry;
		ULONG                   TimeDateStamp;
	} LDR_MODULE, *PLDR_MODULE;


	typedef LONG (__stdcall* PRTL_HEAP_COMMIT_ROUTINE)(IN PVOID Base, IN OUT PVOID *CommitAddress, IN OUT PSIZE_T CommitSize);

	typedef struct _RTL_HEAP_PARAMETERS {
		ULONG Length;
		SIZE_T SegmentReserve;
		SIZE_T SegmentCommit;
		SIZE_T DeCommitFreeBlockThreshold;
		SIZE_T DeCommitTotalFreeThreshold;
		SIZE_T MaximumAllocationSize;
		SIZE_T VirtualMemoryThreshold;
		SIZE_T InitialCommit;
		SIZE_T InitialReserve;
		PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
		SIZE_T Reserved[ 2 ];
	} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;


	// ntdll.dll

	typedef NTSTATUS(__stdcall* NtQueryInformationProcessPtr)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
	typedef NTSTATUS(__stdcall* NtReadVirtualMemoryPtr)(HANDLE, PVOID, PVOID, ULONG, PULONG);
	typedef NTSTATUS(__stdcall* NtWriteVirtualMemoryPtr)(HANDLE, PVOID, PVOID, ULONG, PULONG);
	typedef NTSTATUS(__stdcall* NtAllocateVirtualMemoryPtr)(HANDLE, PVOID*, ULONG, PULONG, ULONG, ULONG);
	typedef NTSTATUS(__stdcall* NtOpenProcessPtr)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PCLIENT_ID);
	typedef NTSTATUS(__stdcall* NtProtectVirtualMemoryPtr)(HANDLE,PVOID*, PULONG,ULONG,PULONG);
	typedef NTSTATUS(__stdcall* NtClosePtr)(HANDLE);
	typedef NTSTATUS(__stdcall* NtQueryVirtualMemoryPtr)(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, ULONG, PULONG);
	typedef NTSTATUS(__stdcall* NtFreeVirtualMemoryPtr)(HANDLE, PVOID*, PULONG, ULONG);
	typedef NTSTATUS(__stdcall* LdrGetDllHandlePtr)(PWORD, PVOID, PUNICODE_STRING, PHANDLE);
	typedef NTSTATUS(__stdcall* LdrGetProcedureAddressPtr)(HMODULE, PANSI_STRING, WORD, PVOID*);
	typedef NTSTATUS(__stdcall* NtCreateFilePtr)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
	typedef NTSTATUS(__stdcall* NtWriteFilePtr)(HANDLE, HANDLE, PIO_APC_ROUTINE, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, PLARGE_INTEGER, PULONG);
	typedef NTSTATUS(__stdcall* NtReadFilePtr)(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext, PIO_STATUS_BLOCK IoStatusBlock, PVOID Buffer, ULONG Length, PLARGE_INTEGER ByteOffset, PULONG Key);
	typedef NTSTATUS(__stdcall* NtSetInformationFilePtr)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID FileInformation, ULONG Length, FILE_INFORMATION_CLASS FileInformationClass);

	typedef NTSTATUS(__stdcall* NtQuerySystemInformationPtr)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

	typedef ULONG (__stdcall* LdrLoadDllPtr)(PWCHAR,ULONG,PUNICODE_STRING,PHANDLE);
	typedef ULONG (__stdcall* LdrInitializeThunkPtr)(PVOID,PVOID,PVOID);

	typedef PVOID (__stdcall* RtlCreateHeapPtr)(ULONG Flags, PVOID HeapBase, SIZE_T ReserveSize, SIZE_T CommitSize, PVOID Lock, PRTL_HEAP_PARAMETERS Parameters);

	typedef PVOID (__stdcall* RtlAllocateHeapPtr)(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
	typedef PVOID (__stdcall* RtlReAllocateHeapPtr)(HANDLE heap, ULONG  flags, PVOID  ptr, SIZE_T size);
	typedef BOOLEAN (__stdcall* RtlFreeHeapPtr)(PVOID HeapHandle, ULONG Flags, PVOID HeapBase);
	typedef PVOID (__stdcall* RtlDestroyHeapPtr)(PVOID HeapHandle);


	// kernel32.dll

	typedef HMODULE(__stdcall* LoadLibraryAPtr)(LPCSTR);
	typedef HMODULE(__stdcall* GetModuleHandleAPtr)(LPCSTR);
	typedef FARPROC(__stdcall* GetProcAddressPtr)(HMODULE,LPCSTR);
	typedef BOOL(__stdcall* IsBadReadPtrPtr)(const VOID*, UINT_PTR);
	typedef BOOL(__stdcall* VirtualProtectPtr)(LPVOID, SIZE_T, DWORD, PDWORD);
	typedef BOOL(__stdcall* BeepPtr)(DWORD,DWORD);
	typedef void(__stdcall* SleepPtr)(DWORD);
	typedef DWORD(__stdcall* ResumeThreadPtr)(HANDLE);

	typedef HANDLE (WINAPI* CreateThreadPtr)(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

	typedef BOOL (__stdcall* CreateProcessWPtr)(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION);
	typedef BOOL (__stdcall* CreateProcessAPtr)(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION);
	typedef void (__stdcall* GetStartupInfoWPtr)(LPSTARTUPINFOW);
	typedef void (__stdcall* GetStartupInfoAPtr)(LPSTARTUPINFOA);

	typedef DWORD (__stdcall* CreateProcessInternalWPtr)(DWORD unknown1, LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation, DWORD unknown2);

	typedef BOOL (WINAPI* EnumProcessesPtr)(DWORD *pProcessIds, DWORD cb, DWORD *pBytesReturned);
	typedef BOOL (WINAPI* EnumProcessModulesPtr)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);

	typedef BOOL (WINAPI* Process32FirstWPtr)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
	typedef BOOL (WINAPI* Process32NextWPtr)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);

	typedef BOOL (WINAPI* Module32FirstWPtr)(HANDLE hSnapshot, LPMODULEENTRY32W lpme);
	typedef BOOL (WINAPI* Module32NextWPtr)(HANDLE hSnapshot, LPMODULEENTRY32W lpme);

	typedef void (WINAPI *ExitProcessPtr)(UINT);
	typedef BOOL (WINAPI *TerminateProcessPtr)(HANDLE, UINT);

	typedef HANDLE (WINAPI* CreateFileWPtr)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);

	typedef VOID (WINAPI* GetStartupInfoAPtr)(LPSTARTUPINFOA lpStartupInfo);
	typedef VOID (WINAPI* GetStartupInfoWPtr)(LPSTARTUPINFOW lpStartupInfo);


	// mswsock.dll

	typedef int (__stdcall* WSPStartupPtr)(WORD, LPWSPDATA, LPWSAPROTOCOL_INFOW, WSPUPCALLTABLE, LPWSPPROC_TABLE);


	// ws2_32.dll

	typedef int (__stdcall* sendPtr)(SOCKET s, const char FAR* buf, int len, int flags);
	typedef int (__stdcall* WSASendPtr)(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

	//shell32.dll

	typedef HINSTANCE (WINAPI *ShellExecuteAPtr)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
	typedef HINSTANCE (WINAPI *ShellExecuteWPtr)(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);


	// user32.dll

	typedef HHOOK (WINAPI* SetWindowsHookExAPtr)(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
	typedef HHOOK (WINAPI* SetWindowsHookExWPtr)(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

	typedef DWORD (WINAPI* SetClassLongWPtr)(HWND hWnd, int nIndex, LONG dwNewLong);
	typedef DWORD (WINAPI* SetClassLongAPtr)(HWND hWnd, int nIndex, LONG dwNewLong);

	typedef DWORD (WINAPI* SetWindowLongWPtr)(HWND hWnd, int nIndex, LONG dwNewLong);
	typedef DWORD (WINAPI* SetWindowLongAPtr)(HWND hWnd, int nIndex, LONG dwNewLong);

	typedef SHORT (WINAPI *GetAsyncKeyStatePtr)(int vKey);
	typedef LONG (WINAPI* ChangeDisplaySettingsAPtr)(LPDEVMODEA lpDevMode, DWORD dwflags);
	typedef HWND (WINAPI *GetForegroundWindowPtr)();

	typedef HWND (WINAPI* FindWindowAPtr)(LPCSTR lpClassName, LPCSTR lpWindowName);
	typedef HWND (WINAPI* FindWindowWPtr)(LPCWSTR lpClassName, LPCWSTR lpWindowName);

	typedef HWND (WINAPI* FindWindowExAPtr)(HWND hwndParent, HWND hwndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
	typedef HWND (WINAPI* FindWindowExWPtr)(HWND hwndParent, HWND hwndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow);

	typedef HWND (WINAPI* GetWindowPtr)(HWND hWnd, UINT uCmd);

	typedef ATOM (WINAPI* RegisterClassAPtr)(const WNDCLASSA *lpWndClass);
	typedef ATOM (WINAPI* RegisterClassWPtr)(const WNDCLASSW *lpWndClass);
	typedef ATOM (WINAPI* RegisterClassExAPtr)(const WNDCLASSEXA *lpWndClass);
	typedef ATOM (WINAPI* RegisterClassExWPtr)(const WNDCLASSEXW *lpWndClass);

	typedef BOOL (WINAPI* EnumWindowsPtr)(WNDENUMPROC lpEnumFunc, LPARAM lParam);
	typedef BOOL (WINAPI* EnumChildWindowsPtr)(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);
	typedef BOOL (WINAPI* EnumThreadWindowsPtr)(DWORD dwThreadId, WNDENUMPROC lpfn, LPARAM lParam);

	typedef BOOL (WINAPI* RegisterShellHookWindowPtr)(HWND hWnd);

	typedef HWND (WINAPI* WindowFromPointPtr)(POINT Point);
	typedef HWND (WINAPI* ChildWindowFromPointPtr)(HWND hWndParent, POINT Point);

	typedef HWND (WINAPI* GetNextWindowPtr)(HWND hWnd, UINT wCmd);
	
	typedef HWND (WINAPI* GetTopWindowPtr)(HWND hWnd);


	// Opengl32.dll
	typedef BOOL (WINAPI* wglSwapBuffersPtr)(HDC hdc);


	/**  
	 * \brief 
	 */
	struct TNtApiTable
	{
		NtQuerySystemInformationPtr NtQuerySystemInformation;
		NtQueryInformationProcessPtr NtQueryInformationProcess;
		NtReadVirtualMemoryPtr NtReadVirtualMemory;
		NtWriteVirtualMemoryPtr NtWriteVirtualMemory;
		NtAllocateVirtualMemoryPtr NtAllocateVirtualMemory;
		NtOpenProcessPtr NtOpenProcess;
		NtProtectVirtualMemoryPtr NtProtectVirtualMemory;
		NtClosePtr NtClose;
		NtQueryVirtualMemoryPtr NtQueryVirtualMemory;
		NtFreeVirtualMemoryPtr NtFreeVirtualMemory;
		LdrGetDllHandlePtr LdrGetDllHandle;
		LdrLoadDllPtr LdrLoadDll;

		RtlCreateHeapPtr RtlCreateHeap;
		RtlAllocateHeapPtr RtlAllocateHeap;
		RtlReAllocateHeapPtr RtlReAllocateHeap;
		RtlFreeHeapPtr RtlFreeHeap;
		RtlDestroyHeapPtr RtlDestroyHeap;
	};



	/**  
	 * \brief 
	 */
	struct TWinApiTable
	{
		LoadLibraryAPtr LoadLibraryA;
		GetModuleHandleAPtr GetModuleHandleA;
		GetProcAddressPtr GetProcAddress;
		IsBadReadPtrPtr IsBadReadPtr;
		VirtualProtectPtr VirtualProtect;
		BeepPtr Beep;
		SleepPtr Sleep;
		ResumeThreadPtr ResumeThread;
	};


	/**  
	 * \brief 
	 */
	struct TModuleInfo
	{
		HMODULE hBase;
		PVOID	EntryPoint;
		ULONG   SizeOfImage;
		wchar_t szDllName[MAX_PATH+1];
		wchar_t szFullPath[MAX_PATH+1];
	};


	/**  
	 * \brief Headers handed back from PE parsing in GetPEImage()
	 */ 
	struct TPEImage
	{
		void*  img_map;
		size_t img_size;
		PIMAGE_DOS_HEADER h_dos;
		PIMAGE_NT_HEADERS h_nt;
		PIMAGE_SECTION_HEADER h_sec;
	};


	/**  
	 * \brief PE reloc block header
	 */
	struct TRelocBlock 
	{
		DWORD pg_rva;
		DWORD block_sz;
	};


	/**  
	 * \brief PE import table header
	 */
	struct TImportDataEntry 
	{
		DWORD ilt_rva;
		DWORD timestamp;
		DWORD forwarder_idx;
		DWORD name_rva;
		DWORD iat_rva;
	};


	void* GetPeb();
	void* GetLocalProcessBaseAddress();
	void* GetProcessPebAddress(HANDLE hProcess);
	void* GetProcessBaseAddress(HANDLE hProcess);
	void* GetLocalProcessHeap();
	HMODULE FindModuleBase(const char* pszModSearch);
	bool GetWinApi(TWinApiTable& winApiTable);
	bool GetNtApi(TNtApiTable& NtApiTable);
	bool CheckForIATHook(const char* szDll, const char* szFunc);
	bool CheckForHotPatch(const char* szDll, const char* szFunc);
	bool CheckForBreakPoint(const char* szDll, const char* szFunc);
	bool IsHooked(const char* szDll, const char* szFunc);
	bool IsBadReadMem(void *ptr);
	int IsBadWriteMem(void* ptr);

	BOOL GetFunctionPtrFromIAT(void* pDosHdr, LPCSTR pImportModuleName, LPCSTR pFunctionSymbol, PVOID* ppvFn);
	const char* GetFnNameByOrdinal(HMODULE hMod, DWORD dwOrd);
	FARPROC FindFnAddressByName(HMODULE hMod, const char* pszFn);
	FARPROC* FindFnExportEntry(HMODULE hMod, const char* pszFn, bool* pfIsForwarded);
	FARPROC* FindFnImportEntry(HMODULE hMod, const char* pImportModuleName, const char* pFunctionSymbol, WORD wOrd);
	DWORD* FindFnImportEntryByOrd(HMODULE hMod, const char* pImportModuleName, WORD ordSymbol);
	FARPROC FindFnAddressByOrd(HMODULE hMod, WORD wOrd);
	const char* FindFnNameByOrd(HMODULE hMod, WORD wOrd);
	DWORD FindModuleEntryProc(HMODULE hMod);
	DWORD GetModuleSize(HMODULE hMod);
	const wchar_t* FindModuleName(HMODULE hMod);
	const wchar_t* FindModuleFullPath(HMODULE hMod);

	bool ReplicateImageIntoRemoteProcess(HANDLE hProcess, void* base, void** ppvNewBase, ULONG* pulImgSize);
	DWORD __stdcall BindImageImports(HMODULE hImage);
	bool BindImportsForModule(HMODULE hBase, LPCSTR pImportModuleName, HMODULE hModBind);
	bool BindFnForModule(HMODULE hBase, LPCSTR pImportModuleName, LPCSTR pFnName, DWORD dwAddr);

	HMODULE GetDllHandleA(LPCSTR pszDllName);
	HMODULE GetDllHandleW(LPCWSTR pszDllName);

	int ListProcessModules(HANDLE hProcess, TModuleInfo* parrMods, ULONG ulCount);


	bool CopyImageCode(HANDLE hProcess);

	DWORD_PTR FindCodeBytes(HMODULE hMod, BYTE* code, int len, int offs = 0);


	typedef bool (*TraverseIATProc)(LPCSTR pImportModuleName, LPCSTR pFunctionSymbol, DWORD dwOrd, FARPROC* ppfn, LPVOID pvParam);
	BOOL TraverseIAT(void* pDosHdr, TraverseIATProc fnCallback, LPVOID pvParam);

	typedef bool (*TraverseRelocProc)(void* base, int ofs, DWORD* pdwPatchAddr, LPVOID pvParam);
	BOOL TraverseReloc(void* pDosHdr, TraverseRelocProc fnCallback, LPVOID pvParam);

	bool LdrGetModuleNameA(HMODULE hMod, char* pszName, int len);
	HMODULE LdrLoadDllA(const char* pszDllName);
}
