#include <windows.h>
#include <string>
#include <sstream>

#define ID_BTN_GENERATE  1
#define ID_BTN_MORE      2
#define ID_EDIT_START    3
#define ID_EDIT_AMOUNT   4
#define ID_LISTBOX       5
#define ID_BTN_COPY      6
#define ID_BTN_PRINT     7
#define ID_EDIT_PREFIX   8

HWND hEditStart, hEditAmount, hEditPrefix, hListBox, hBtnMore, hBtnCopy, hBtnPrint;
int lastStart = 0, lastAmount = 0;

std::wstring GetPrefix() {
    wchar_t buf[32];
    GetWindowText(hEditPrefix, buf, 32);
    return std::wstring(buf);
}

void GenerateNumbers(int start, int amount) {
    std::wstring prefix = GetPrefix();
    for (int i = 0; i < amount; i++) {
        std::wstring item = prefix + std::to_wstring(start + i);
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    SendMessage(hListBox, LB_SETTOPINDEX, count - 1, 0);
}

void CopyToClipboard(HWND hwnd) {
    int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    if (count <= 0) return;

    std::wstring all;
    for (int i = 0; i < count; i++) {
        wchar_t buf[128];
        SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)buf);
        all += buf;
        all += L"\r\n";
    }

    if (!OpenClipboard(hwnd)) return;
    EmptyClipboard();

    size_t bytes = (all.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (hMem) {
        memcpy(GlobalLock(hMem), all.c_str(), bytes);
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    }
    CloseClipboard();

    MessageBox(hwnd, L"Numbers copied to clipboard!", L"Copied", MB_OK | MB_ICONINFORMATION);
}

void PrintNumbers(HWND hwnd) {
    int count = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    if (count <= 0) {
        MessageBox(hwnd, L"Nothing to print.", L"Print", MB_OK | MB_ICONWARNING);
        return;
    }

    PRINTDLG pd = {};
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner   = hwnd;
    pd.Flags       = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;

    if (!PrintDlg(&pd)) return; // user cancelled

    HDC hdc = pd.hDC;

    DOCINFO di = {};
    di.cbSize      = sizeof(di);
    di.lpszDocName = L"Number Generator";
    StartDoc(hdc, &di);
    StartPage(hdc);

    // Set up font
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Courier New");
    SelectObject(hdc, hFont);

    int pageHeight  = GetDeviceCaps(hdc, VERTRES);
    int lineHeight  = 32;
    int marginX     = 200;
    int marginY     = 200;
    int y           = marginY;

    // Print title
    HFONT hTitleFont = CreateFont(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Courier New");
    SelectObject(hdc, hTitleFont);
    TextOut(hdc, marginX, y, L"Generated Numbers", 17);
    y += lineHeight * 2;
    DeleteObject(hTitleFont);
    SelectObject(hdc, hFont);

    for (int i = 0; i < count; i++) {
        if (y + lineHeight > pageHeight - marginY) {
            EndPage(hdc);
            StartPage(hdc);
            y = marginY;
            SelectObject(hdc, hFont);
        }
        wchar_t buf[128];
        SendMessage(hListBox, LB_GETTEXT, i, (LPARAM)buf);
        TextOut(hdc, marginX, y, buf, (int)wcslen(buf));
        y += lineHeight;
    }

    EndPage(hdc);
    EndDoc(hdc);
    DeleteObject(hFont);
    DeleteDC(hdc);

    if (pd.hDevMode) GlobalFree(pd.hDevMode);
    if (pd.hDevNames) GlobalFree(pd.hDevNames);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

    case WM_CREATE: {
        // Row 1 — Prefix
        CreateWindow(L"STATIC", L"Prefix:",
            WS_VISIBLE | WS_CHILD, 20, 20, 80, 20, hwnd, NULL, NULL, NULL);
        hEditPrefix = CreateWindow(L"EDIT", L"C",
            WS_VISIBLE | WS_CHILD | WS_BORDER,
            110, 18, 60, 24, hwnd, (HMENU)ID_EDIT_PREFIX, NULL, NULL);

        // Row 2 — Start Number
        CreateWindow(L"STATIC", L"Start Number:",
            WS_VISIBLE | WS_CHILD, 20, 55, 110, 20, hwnd, NULL, NULL, NULL);
        hEditStart = CreateWindow(L"EDIT", L"1",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            150, 53, 80, 24, hwnd, (HMENU)ID_EDIT_START, NULL, NULL);

        // Row 3 — Amount
        CreateWindow(L"STATIC", L"Amount:",
            WS_VISIBLE | WS_CHILD, 20, 90, 110, 20, hwnd, NULL, NULL, NULL);
        hEditAmount = CreateWindow(L"EDIT", L"5",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            150, 88, 80, 24, hwnd, (HMENU)ID_EDIT_AMOUNT, NULL, NULL);

        // Row 4 — Generate buttons
        CreateWindow(L"BUTTON", L"Generate",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            20, 125, 100, 30, hwnd, (HMENU)ID_BTN_GENERATE, NULL, NULL);
        hBtnMore = CreateWindow(L"BUTTON", L"Generate More",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
            130, 125, 120, 30, hwnd, (HMENU)ID_BTN_MORE, NULL, NULL);

        // Listbox
        hListBox = CreateWindow(L"LISTBOX", NULL,
            WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
            20, 170, 340, 180, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);

        // Row — Copy / Print
        hBtnCopy = CreateWindow(L"BUTTON", L"Copy All",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
            20, 365, 100, 30, hwnd, (HMENU)ID_BTN_COPY, NULL, NULL);
        hBtnPrint = CreateWindow(L"BUTTON", L"Print...",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
            130, 365, 100, 30, hwnd, (HMENU)ID_BTN_PRINT, NULL, NULL);
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BTN_GENERATE) {
            wchar_t buf[64];
            GetWindowText(hEditStart,  buf, 64); int start  = _wtoi(buf);
            GetWindowText(hEditAmount, buf, 64); int amount = _wtoi(buf);

            if (amount <= 0) {
                MessageBox(hwnd, L"Amount must be greater than 0.", L"Error", MB_OK | MB_ICONWARNING);
                break;
            }
            SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
            lastStart = start; lastAmount = amount;
            GenerateNumbers(lastStart, lastAmount);
            EnableWindow(hBtnMore,  TRUE);
            EnableWindow(hBtnCopy,  TRUE);
            EnableWindow(hBtnPrint, TRUE);
        }
        if (LOWORD(wParam) == ID_BTN_MORE) {
            lastStart += lastAmount;
            GenerateNumbers(lastStart, lastAmount);
        }
        if (LOWORD(wParam) == ID_BTN_COPY)  CopyToClipboard(hwnd);
        if (LOWORD(wParam) == ID_BTN_PRINT) PrintNumbers(hwnd);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASS wc    = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.lpszClassName = L"NumberGenClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        L"NumberGenClass", L"Number Generator",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 440,
        NULL, NULL, hInst, NULL
    );
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}