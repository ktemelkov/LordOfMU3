#pragma once

#include "winternl.h"
#include "tlhelp32.h"
#include <ws2spi.h>



int WINAPI MyWSPStartup(WORD wVer, LPWSPDATA pData, LPWSAPROTOCOL_INFOW pProtocol, WSPUPCALLTABLE upcalltable, LPWSPPROC_TABLE pProcTable);

HWND WINAPI MyGetForegroundWindow();

BOOL WINAPI MywglSwapBuffers(HDC hdc);

HHOOK WINAPI MySetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
HHOOK WINAPI MySetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);




HWND WINAPI MyFindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);


BOOL WINAPI MyProcess32FirstW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
BOOL WINAPI MyProcess32NextW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);

BOOL WINAPI MyModule32FirstW(HANDLE hSnapshot, LPMODULEENTRY32W lpme);
BOOL WINAPI MyModule32NextW(HANDLE hSnapshot, LPMODULEENTRY32W lpme);


BOOL WINAPI MyEnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
BOOL WINAPI MyEnumProcesses(DWORD *pProcessIds, DWORD cb, DWORD *pBytesReturned);

BOOL WINAPI MyEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam);
BOOL WINAPI MyEnumChildWindows(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);
BOOL WINAPI MyEnumThreadWindows(DWORD dwThreadId, WNDENUMPROC lpfn, LPARAM lParam);

HWND WINAPI MyFindWindowExA(HWND hwndParent, HWND hwndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
HWND WINAPI MyFindWindowExW(HWND hwndParent, HWND hwndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow);

HWND WINAPI MyGetWindow(HWND hWnd, UINT uCmd);

BOOL WINAPI MyRegisterShellHookWindow(HWND hWnd);
HWND WINAPI MyWindowFromPoint(POINT Point);
HWND WINAPI MyChildWindowFromPoint(HWND hWndParent, POINT Point);
HWND WINAPI MyGetNextWindow(HWND hWnd, UINT wCmd);
HWND WINAPI MyGetTopWindow(HWND hWnd);
