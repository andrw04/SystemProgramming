#pragma once
#include <windows.h>
#include <tchar.h>
#include "commctrl.h"

HINSTANCE hInstance;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMainWindow();
HWND hwnd;
HWND hList;

DWORD interval = 1000;
