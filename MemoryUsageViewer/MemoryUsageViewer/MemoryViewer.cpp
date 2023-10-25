#include <iostream>
#include <iomanip>
#include <Windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string>
#include <TlHelp32.h>
#include "MemoryViewer.h"
#include <windows.h>


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

				DWORD exitCode;
				if (GetExitCodeProcess(hProcess, &exitCode))
				{
					if (exitCode == STILL_ACTIVE)
					{
						length = strlen(path) + 1;
						size = MultiByteToWideChar(CP_UTF8, 0, "Active", length, NULL, 0);
						MultiByteToWideChar(CP_UTF8, 0, "Active", length, buffer, size);
					}
					else {
						// Процесс приостановлен
						length = strlen(path) + 1;
						size = MultiByteToWideChar(CP_UTF8, 0, "Suspended", length, NULL, 0);
						MultiByteToWideChar(CP_UTF8, 0, "Suspended", length, buffer, size);
					}
				}
				else {
					length = strlen(path) + 1;
					size = MultiByteToWideChar(CP_UTF8, 0, "Undefined", length, NULL, 0);
					MultiByteToWideChar(CP_UTF8, 0, "Undefined", length, buffer, size);
				}

				ListView_SetItemText(hList, row, 3, buffer);

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

void CreateNewProcess() {
	OPENFILENAME ofn;
	wchar_t filePath[MAX_PATH] = L"";

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn)) {
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };

		if (CreateProcess(NULL, filePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else {
			MessageBox(NULL, L"Failed to start process!", L"Error", MB_OK | MB_ICONERROR);
		}
	}
}

void SuspendProcess() {
	// Получаем индекс выбранной строки
	int iSelectedCount = ListView_GetSelectedCount(hList);

	if (iSelectedCount > 0)
	{
		int selectedIndex = ListView_GetSelectionMark(hList);

		// Выделям память под структуру для хранения информации о ячейке
		LVITEM item;
		item.mask = LVIF_TEXT;
		item.iItem = selectedIndex;
		item.iSubItem = 0;
		item.pszText = new wchar_t[MAX_PATH];
		item.cchTextMax = MAX_PATH;

		// Получаем значение выбранной ячейки
		ListView_GetItemText(hList, selectedIndex, 0, item.pszText, MAX_PATH);

		DWORD processId = wcstoul(item.pszText, NULL, 10);

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		if (hProcess == NULL)
		{
			MessageBox(hwnd, L"Can't open process.", L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		// Приостановка процесса
		DWORD threadCount = 0;
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);

		if (Thread32First(hThreadSnapshot, &te32))
		{
			do
			{
				if (te32.th32OwnerProcessID == GetProcessId(hProcess))
				{
					HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
					if (hThread != NULL)
					{
						SuspendThread(hThread);
						CloseHandle(hThread);
						threadCount++;
					}
				}
			} while (Thread32Next(hThreadSnapshot, &te32));
		}
		MessageBox(hwnd, L"Process suspended!", L"Info", MB_OK | MB_ICONINFORMATION);

		CloseHandle(hThreadSnapshot);

		delete[] item.pszText;
	}
	else {
		MessageBox(hwnd, L"Element not selected!", L"Error", MB_OK | MB_ICONERROR);
	}
}



void ResumeProcess() {
	// Получаем индекс выбранной строки
	int iSelectedCount = ListView_GetSelectedCount(hList);

	if (iSelectedCount > 0)
	{
		int selectedIndex = ListView_GetSelectionMark(hList);

		// Выделям память под структуру для хранения информации о ячейке
		LVITEM item;
		item.mask = LVIF_TEXT;
		item.iItem = selectedIndex;
		item.iSubItem = 0;
		item.pszText = new wchar_t[MAX_PATH];
		item.cchTextMax = MAX_PATH;

		// Получаем значение выбранной ячейки
		ListView_GetItemText(hList, selectedIndex, 0, item.pszText, MAX_PATH);

		DWORD processId = wcstoul(item.pszText, NULL, 10);

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		if (hProcess == NULL)
		{
			MessageBox(hwnd, L"Can't open process.", L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		// Возобновление процесса
		DWORD threadCount = 0;
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);

		if (Thread32First(hThreadSnapshot, &te32)) {
			do {
				if (te32.th32OwnerProcessID == GetProcessId(hProcess)) {
					HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
					if (hThread != NULL) {
						ResumeThread(hThread);
						CloseHandle(hThread);
					}
				}
			} while (Thread32Next(hThreadSnapshot, &te32));
		}

		MessageBox(hwnd, L"Process resumed!", L"Info", MB_OK | MB_ICONINFORMATION);

		CloseHandle(hThreadSnapshot);
		CloseHandle(hProcess);

		delete[] item.pszText;
	}
	else {
		MessageBox(hwnd, L"Element not selected!", L"Error", MB_OK | MB_ICONERROR);
	}
}

void TerminateProcess() {
	int iSelectedCount = ListView_GetSelectedCount(hList);

	if (iSelectedCount > 0)
	{
		int selectedIndex = ListView_GetSelectionMark(hList);

		// Выделям память под структуру для хранения информации о ячейке
		LVITEM item;
		item.mask = LVIF_TEXT;
		item.iItem = selectedIndex;
		item.iSubItem = 0;
		item.pszText = new wchar_t[MAX_PATH];
		item.cchTextMax = MAX_PATH;

		// Получаем значение выбранной ячейки
		ListView_GetItemText(hList, selectedIndex, 0, item.pszText, MAX_PATH);

		DWORD processId = wcstoul(item.pszText, NULL, 10);

		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
		if (hProcess == NULL)
		{
			MessageBox(hwnd, L"Can't open process.", L"Error", MB_OK | MB_ICONERROR);
			return;
		}

		// Закрытие процесса
		TerminateProcess(hProcess, 0);

		CloseHandle(hProcess);
	}
	else {
		MessageBox(hwnd, L"Element not selected!", L"Error", MB_OK | MB_ICONERROR);
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		if (lParam == (LPARAM)createButton)
			CreateNewProcess();
		else if (lParam == (LPARAM)suspendButton)
			SuspendProcess();
		else if (lParam == (LPARAM)resumeButton)
			ResumeProcess();
		else if (lParam == (LPARAM)terminateButton)
			TerminateProcess();
		break;

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
	hList = CreateWindow(WC_LISTVIEW, _T(""), WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 40, 585, 620, hwnd, NULL, NULL, NULL);

	createButton = CreateWindowEx(0, L"BUTTON", L"Create", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 5, 5, 100, 30, hwnd, NULL, hInstance, NULL);
	suspendButton = CreateWindowEx(0, L"BUTTON", L"Suspend", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 110, 5, 100, 30, hwnd, NULL, hInstance, NULL);
	resumeButton = CreateWindowEx(0, L"BUTTON", L"Resume", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 215, 5, 100, 30, hwnd, NULL, hInstance, NULL);
	terminateButton = CreateWindowEx(0, L"BUTTON", L"Terminate", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 320, 5, 100, 30, hwnd, NULL, hInstance, NULL);

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
	lvColumn3.cx = 150;
	lvColumn3.pszText = const_cast<wchar_t*>(L"Memory Usage (Mb)");
	ListView_InsertColumn(hList, 2, &lvColumn3);

	LVCOLUMN lvColumn4;
	lvColumn4.mask = LVCF_TEXT | LVCF_WIDTH;
	lvColumn4.cx = 50;
	lvColumn4.pszText = const_cast<wchar_t*>(L"Status");
	ListView_InsertColumn(hList, 3, &lvColumn4);
}