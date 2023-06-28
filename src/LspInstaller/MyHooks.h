#pragma once

#include "winternl.h"
#include "tlhelp32.h"
#include <ws2spi.h>



int WINAPI MyWSPStartup(WORD wVer, LPWSPDATA pData, LPWSAPROTOCOL_INFOW pProtocol, WSPUPCALLTABLE upcalltable, LPWSPPROC_TABLE pProcTable);


BOOL WINAPI MySwapBuffers(HDC hdc);
BOOL WINAPI MySetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd);

HHOOK WINAPI MySetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);
HHOOK WINAPI MySetWindowsHookExW(int idHook, HOOKPROC lpfn, HINSTANCE hMod, DWORD dwThreadId);

HWND WINAPI MyGetForegroundWindow();
HWND WINAPI MyFindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND WINAPI MyFindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);
BOOL WINAPI MyEnumWindows(WNDENUMPROC lpEnumFunc, LPARAM lParam);
BOOL WINAPI MyEnumChildWindows(HWND hWndParent, WNDENUMPROC lpEnumFunc, LPARAM lParam);
BOOL WINAPI MyEnumThreadWindows(DWORD dwThreadId, WNDENUMPROC lpfn, LPARAM lParam);

HWND WINAPI MyFindWindowExA(HWND hwndParent, HWND hwndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
HWND WINAPI MyFindWindowExW(HWND hwndParent, HWND hwndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow);

HWND WINAPI MyGetWindow(HWND hWnd, UINT uCmd);

BOOL WINAPI MyRegisterShellHookWindow(HWND hWnd);
HWND WINAPI MyWindowFromPoint(POINT Point);
HWND WINAPI MyChildWindowFromPoint(HWND hWndParent, POINT Point);
HWND WINAPI MyGetTopWindow(HWND hWnd);

BOOL __stdcall MyVirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);	
