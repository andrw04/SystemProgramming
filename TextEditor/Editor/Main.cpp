#include <windows.h>
#include <tchar.h>
#include <fstream>
#include <iostream>
#include <string>
#include <codecvt>
#include <Shlwapi.h>
#include "Main.h"

// nCode - указывает что нужно сделать, wParam - инфа о клавише, lParam - доп данные
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbData = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN && GetAsyncKeyState(VK_SHIFT) & 0x8000 && kbData->vkCode == VK_ADD) {
            WindowData* data = (WindowData*)GetWindowLongPtr(GetForegroundWindow(), GWLP_USERDATA);
            HWND edit = GetDlgItem(GetForegroundWindow(), IDC_EDIT_ID1);
            // структура для определения параметров шрифта
            LOGFONT lf;
            GetObject(data->hFont, sizeof(LOGFONT), &lf);
            lf.lfHeight = abs(lf.lfHeight) < 72 ? -(abs(lf.lfHeight) + 2) : lf.lfHeight;
            data->hFont = CreateFontIndirect(&lf);
            SendMessage(edit, WM_SETFONT, (WPARAM)data->hFont, TRUE);
            return 1;
        }
        if (wParam == WM_KEYDOWN && GetAsyncKeyState(VK_SHIFT) & 0x8000 && kbData->vkCode == VK_SUBTRACT) {
            WindowData* data = (WindowData*)GetWindowLongPtr(GetForegroundWindow(), GWLP_USERDATA);
            HWND edit = GetDlgItem(GetForegroundWindow(), IDC_EDIT_ID1);
            LOGFONT lf;
            GetObject(data->hFont, sizeof(LOGFONT), &lf);
            lf.lfHeight = abs(lf.lfHeight) > 10 ? -(abs(lf.lfHeight) - 2) : lf.lfHeight;
            data->hFont = CreateFontIndirect(&lf);
            SendMessage(edit, WM_SETFONT, (WPARAM)data->hFont, TRUE);
            return 1;
        }
    }
    // передать управление следующему хуку в цепочке
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


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
        MessageBox(NULL, L"Ошибка регистрации окна!", L"Ошибка!", MB_ICONERROR | MB_OK);
        return 1;
    }
    HWND hwndMain = CreateNewWindow(hInstance);
    if (!IsValidWindow(hwndMain))
        return 1;
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, hInstance, 0);
    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
    UnregisterClass(wc.lpszClassName, hInstance);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case OnExitClicked:
            PostQuitMessage(0);
            break;
        case OnOpenClicked:
            LoadData(hwnd);
            break;
        case OnSaveClicked:
            SaveData(hwnd);
            break;
        case OnCreateClicked:
            ResetWindowData(hwnd);
            break;
        case OnTextColorClicked:
            SetTextColor(hwnd);
            break;
        case OnBGColorClicked:
            SetBGColor(hwnd);
            break;
        case OnFontClicked:
            SetFont(hwnd);
            break;
        case OnNewWindowClicked:
            HWND hwndNew = CreateNewWindow(hInstance);
            windowCount++;
            ShowWindow(hwndNew, SW_SHOWNORMAL);
            break;
        }

        break;
    }
    case WM_SIZE:
    {
        int newWidth = LOWORD(lParam) - 5;
        int newHeight = HIWORD(lParam) - 5;
        HWND hEdit = GetDlgItem(hwnd, IDC_EDIT_ID1);
        MoveWindow(hEdit, 0, 0, newWidth, newHeight, TRUE);

        return 0;
    }
    case WM_CLOSE:
    {
        if (hwnd != GetConsoleWindow()) {
            WindowData* windowData = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            if (windowData) {
                delete windowData;
            }
            DestroyWindow(hwnd);
            windowCount--;
        }
    }
    case WM_DESTROY:
    {
        if (windowCount == 0) {
            PostQuitMessage(0);
        }
        break;
    }
    case WM_CTLCOLOREDIT:
    {
        WindowData* data = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, data->textColor);
        SetBkColor(hdcEdit, data->bgColor);
        return (LRESULT)data->hEditTextBrush;
    }
    default:
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }
}

HWND CreateNewWindow(HINSTANCE hInst) {

    WindowData* windowData = new WindowData;
    windowData->windowText = L"unnamed.txt";
    windowData->hEditBgBrush = CreateSolidBrush(windowData->bgColor);
    windowData->hEditTextBrush = CreateSolidBrush(windowData->bgColor);
    std::wstring title = windowData->windowText + L" - TextEditor";
    HWND hwnd = CreateWindow(_T("MyWindowClass"), title.c_str(), WS_OVERLAPPEDWINDOW, 100, 100, 600, 700, NULL, NULL, hInstance, NULL);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)windowData);
    HMENU RootMenu = CreateMenu();
    HMENU SubMenu = CreateMenu();
    HMENU SubMenu1 = CreateMenu();
    AppendMenu(SubMenu, MF_STRING, OnCreateClicked, L"Создать");
    AppendMenu(SubMenu, MF_STRING, OnNewWindowClicked, L"Новое окно");
    AppendMenu(SubMenu, MF_STRING, OnOpenClicked, L"Открыть");
    AppendMenu(SubMenu, MF_STRING, OnSaveClicked, L"Сохранить");
    AppendMenu(SubMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(SubMenu, MF_STRING, OnExitClicked, L"Выход");
    AppendMenu(SubMenu1, MF_STRING, OnTextColorClicked, L"Цвет текста");
    AppendMenu(SubMenu1, MF_STRING, OnBGColorClicked, L"Цвет фона");
    AppendMenu(SubMenu1, MF_STRING, OnFontClicked, L"Стиль шрифта");
    AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu, L"Файл");
    AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenu1, L"Формат");
    SetMenu(hwnd, RootMenu);
    HWND edit = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL, 0, 0, 585, 700, hwnd, (HMENU)IDC_EDIT_ID1, NULL, NULL);
    SendMessage(edit, WM_SETFONT, (WPARAM)windowData->hFont, TRUE);
    return hwnd;
}

bool IsValidWindow(HWND hwnd)
{
    if (hwnd == NULL) {
        MessageBox(NULL, _T("Ошибка при создании окна."), _T("Ошибка"), MB_ICONERROR);
        return false;
    }
    return true;
}

void SaveData(HWND hwnd) {
    HWND edit = GetDlgItem(hwnd, IDC_EDIT_ID1);
    WindowData* data = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    int textLength = GetWindowTextLength(edit);
    wchar_t* text = new wchar_t[textLength + 1];
    GetWindowText(edit, text, textLength + 1);
    OPENFILENAME ofn = { 0 };
    wchar_t filePath[MAX_PATH];
    wcscpy_s(filePath, data->windowText.c_str());
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files\0*.txt\0All Files\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_CREATEPROMPT;
    if (GetSaveFileName(&ofn)) {
        std::wofstream file(ofn.lpstrFile, std::ios::out | std::ios::binary);
        file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));
        if (file.is_open()) {
            file.write(text, wcslen(text));
            file.close();
            MessageBox(NULL, L"Файл был успешно сохранен!", L"Внимание!", MB_OK);
        }
    }
    delete[] text;
}

void LoadData(HWND hwnd)
{
    HWND edit = GetDlgItem(hwnd, IDC_EDIT_ID1);
    WindowData* data = (WindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    OPENFILENAME ofn = { 0 };
    wchar_t filePath[MAX_PATH] = L"";
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Текстовые файлы\0*.txt\0Все файлы\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn)) {
        std::wifstream file(ofn.lpstrFile, std::ios::in | std::ios::binary);
        if (file.is_open()) {
            // упаковка новой локали
            std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
            file.imbue(utf8_locale);
            std::wstring fileContent((std::istreambuf_iterator<wchar_t>(file)), std::istreambuf_iterator<wchar_t>());
            file.close();
            if (!fileContent.empty())
            {
                SetWindowTextW(edit, fileContent.c_str());
                // PathFindFileName извлекает имя файла из полного имени файла
                data->windowText = PathFindFileName(ofn.lpstrFile);
                std::wstring title = data->windowText + L" - TextEditor";
                SetWindowText(hwnd, title.c_str());
                MessageBox(NULL, L"Файл был успешно загружен!", L"Внимание!", MB_OK);
            }
            else
            {
                MessageBox(NULL, L"Файл пустой.", L"Предупреждение", MB_ICONWARNING);
            }
        }
        else
        {
            MessageBox(NULL, L"Не удалось открыть файл.", L"Ошибка", MB_ICONERROR);
        }
    }
}

void ResetWindowData(HWND hWnd) 
{
    HWND edit = GetDlgItem(hWnd, IDC_EDIT_ID1);
    WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    SetWindowTextW(edit, L"");
    data->windowText = L"unnamed.txt";
    std::wstring title = data->windowText + L" - TextEditor";
    SetWindowText(hWnd, title.c_str());
}


void SetTextColor(HWND hWnd)
{
    WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HWND edit = GetDlgItem(hWnd, IDC_EDIT_ID1);
    CHOOSECOLOR cc;
    static COLORREF customColors[16] = { 0 };
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = (LPDWORD)customColors;
    cc.rgbResult = data->textColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColor(&cc)) {
        data->textColor = cc.rgbResult;
        InvalidateRect(edit, NULL, TRUE);
    }
}

void SetBGColor(HWND hWnd)
{
    WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HWND edit = GetDlgItem(hWnd, IDC_EDIT_ID1);
    CHOOSECOLOR cc;
    static COLORREF customColors[16] = { 0 };
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hWnd;
    cc.lpCustColors = (LPDWORD)customColors;
    cc.rgbResult = data->bgColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColor(&cc)) {
        data->bgColor = cc.rgbResult;
        data->hEditBgBrush = CreateSolidBrush(cc.rgbResult);
        data->hEditTextBrush = CreateSolidBrush(cc.rgbResult);
        InvalidateRect(edit, NULL, TRUE);
    }
}

void SetFont(HWND hWnd)
{
    WindowData* data = (WindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HWND edit = GetDlgItem(hWnd, IDC_EDIT_ID1);
    CHOOSEFONT cf;
    LOGFONT lf;
    ZeroMemory(&cf, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hWnd;
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
    if (ChooseFont(&cf)) {
        data->hFont = CreateFontIndirect(&lf);
        SendMessage(edit, WM_SETFONT, (WPARAM)data->hFont, TRUE);
    }
}