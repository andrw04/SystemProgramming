#pragma once
#include <windows.h>
#include <tchar.h>
#include "commctrl.h"

#define OnCreateClicked 2
#define OnSuspendClicked 3
#define OnResumeClicked 4
#define OnTerminateClicked 5

HINSTANCE hInstance;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMainWindow();
HWND hwnd;
HWND hList;

HWND createButton;
HWND suspendButton;
HWND resumeButton;
HWND terminateButton;

DWORD interval = 1000;
