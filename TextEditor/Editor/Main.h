#pragma once
#define IDC_EDIT_ID1 1001
#define OnCreateClicked 1
#define OnNewWindowClicked 2
#define OnOpenClicked 3
#define OnSaveClicked 4
#define OnExitClicked 5
#define OnTextColorClicked 6
#define OnBGColorClicked 7
#define OnFontClicked 8
#define TextBufferSize 256
HINSTANCE hInstance;
char Buffer[TextBufferSize];
int windowCount = 1;

struct WindowData {
    std::wstring windowText;
    COLORREF textColor = RGB(0, 0, 0);
    COLORREF bgColor = RGB(255, 255, 255);
    HBRUSH hEditBgBrush = NULL;
    HBRUSH hEditTextBrush = NULL;
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    ~WindowData() {
        DeleteObject(hEditBgBrush);
        DeleteObject(hEditTextBrush);
        DeleteObject(hFont);
    }
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND CreateNewWindow(HINSTANCE hInst);
bool IsValidWindow(HWND hwnd);
void ResetWindowData(HWND hWnd);
void SetTextColor(HWND hWnd);
void SetBGColor(HWND hWnd);
void SetFont(HWND hWnd);
void SaveData(HWND hWnd);
void LoadData(HWND hWnd);