#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string>
#include "MemoryViewer.h"

int UpdateProcessList() {
	DWORD processes[1024];
	DWORD bytesReturned;

	// Получаем идентификаторы запущенных процессов
	if (!EnumProcesses(processes, sizeof(processes), &bytesReturned)) {
		std::cout << "Error with call EnumProcesses" << std::endl;
		return 1;
	}

	// Вычисление количества процессов
	int numProcesses = bytesReturned / sizeof(DWORD);

	// Выводим идентификаторы процессор
	int row = 0;
	int count = ListView_GetItemCount(hList);
	for (int i = 0; i < numProcesses; i++) {
		// Открываем процесс для получения дополнительной ифнормации
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

		if (hProcess != NULL) {
			HMODULE hModule;
			DWORD cbNeeded;
			PROCESS_MEMORY_COUNTERS pmc;
			pmc.cb = sizeof(pmc);
			char path[MAX_PATH];
			DWORD size = MAX_PATH;

			bool condition = EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded) &&
				(GetModuleBaseNameA(hProcess, hModule, path, sizeof(path)) != 0) &&
				GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));

			//// Получаем базовый модуль процесса
			if (condition) {
				

				if (count <= row)
				{
					LVITEM lvItem;
					lvItem.mask = LVIF_TEXT;
					lvItem.iItem = row;
					lvItem.iSubItem = 0;
					lvItem.pszText = new wchar_t[0];
					ListView_InsertItem(hList, &lvItem);
				}


				wchar_t buffer[256];
				swprintf_s(buffer, L"%d", processes[i]);
				ListView_SetItemText(hList, row, 0, buffer);


				int length = strlen(path) + 1;
				int size = MultiByteToWideChar(CP_UTF8, 0, path, length, NULL, 0);
				MultiByteToWideChar(CP_UTF8, 0, path, length, buffer, size);
				ListView_SetItemText(hList, row, 1, buffer);

				swprintf_s(buffer, L"% .3lf", (double)pmc.WorkingSetSize / (1024 * 1024));
				ListView_SetItemText(hList, row, 2, buffer);

				row++;
				std::cout << "Process Identificator: " << processes[i] << " Process Name: " << path << " Memory usage: " << std::fixed << std::setprecision(2) << (double)pmc.WorkingSetSize / (1024 * 1024) << " Mb" << std::endl;
			}
			// Закрываем дескриптор процесса
			CloseHandle(hProcess);
		}
	}
	for (int i = row; i < count; i++) {
		ListView_DeleteItem(hList, i);
	}

	return 0;
}


VOID CALLBACK TimerCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	hInstance = hInst;

	WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"MyWindowClass";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Register window class error!", L"Error!", MB_ICONERROR | MB_OK);
		return 1;
	}

	CreateMainWindow();
	if (hwnd == NULL) {
		MessageBox(NULL, L"Creating main window error!", L"Error!", MB_ICONERROR | MB_OK);
		return 1;
	}

	UpdateProcessList();
	UINT_PTR timerId = SetTimer(NULL, 0, interval, TimerCallback);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(NULL, timerId);

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_SIZE:
	{
		break;
	}
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

VOID CALLBACK TimerCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	UpdateProcessList();
}

void CreateMainWindow() {
	hwnd = CreateWindow(_T("MyWindowClass"), L"MemoryViewer",WS_OVERLAPPEDWINDOW, 100, 100, 600, 700, NULL, NULL, hInstance, NULL);
	hList = CreateWindow(WC_LISTVIEW, _T(""), WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 0, 585, 700, hwnd, NULL, NULL, NULL);

	ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvColumn;
	lvColumn.mask = LVCF_TEXT | LVCF_WIDTH;
	lvColumn.cx = 100;
	lvColumn.pszText = const_cast<wchar_t*>(L"Process Id");
	ListView_InsertColumn(hList, 0, &lvColumn);

	LVCOLUMN lvColumn2;
	lvColumn2.mask = LVCF_TEXT | LVCF_WIDTH;
	lvColumn2.cx = 250;
	lvColumn2.pszText = const_cast<wchar_t*>(L"Process Name");
	ListView_InsertColumn(hList, 1, &lvColumn2);

	LVCOLUMN lvColumn3;
	lvColumn3.mask = LVCF_TEXT | LVCF_WIDTH;
	lvColumn3.cx = 200;
	lvColumn3.pszText = const_cast<wchar_t*>(L"Memory Usage (Mb)");
	ListView_InsertColumn(hList, 2, &lvColumn3);
}