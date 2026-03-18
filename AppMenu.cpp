#include <windows.h>
#include <cwchar>
#include <algorithm>
#include "AppMenu.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"

extern MinesweeperLogic g_Logic;
extern void ChangeLevel(HWND hWnd, int w, int h, int m);


ScoreRecord g_HighScores[3] = {
    { 999, L"\x533f\x540d" }, 
    { 999, L"\x533f\x540d" },
    { 999, L"\x533f\x540d" }
};

bool g_bMarks = true;
bool g_bColor = true;
bool g_bSound = false;

#define DIALOG_ALIGN() while ((UINT_PTR)p % 4 != 0) { *(WORD*)p = 0; p += 2; }


void* CreateCustomDialogTemplate() {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = WS_EX_CONTEXTHELP;
    pDlg->cdit = 8;
    pDlg->x = 10; pDlg->y = 10; pDlg->cx = 120; pDlg->cy = 80;
    p += sizeof(DLGTEMPLATE);
    *(WORD*)p = 0; p += 2; *(WORD*)p = 0; p += 2;

    const wchar_t* title = L"\x81ea\x5b9a\x4e49\x96f7\x533a";
    wcscpy_s((wchar_t*)p, 64, title); p += (wcslen(title) + 1) * sizeof(wchar_t);

    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy; item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 64, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 64, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
        };

    AppendItem(IDC_STATIC, L"Static", L"\x9ad8\x5ea6(H):", 7, 7, 35, 9, SS_LEFT); 
    AppendItem(IDC_EDIT_HEIGHT, L"Edit", L"", 45, 5, 35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);
    AppendItem(IDC_STATIC, L"Static", L"\x5e3d\x5ea6(W):", 7, 22, 35, 9, SS_LEFT); 
    AppendItem(IDC_EDIT_WIDTH, L"Edit", L"", 45, 20, 35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);
    AppendItem(IDC_STATIC, L"Static", L"\x96f7\x6570(M):", 7, 37, 35, 9, SS_LEFT); 
    AppendItem(IDC_EDIT_MINES, L"Edit", L"", 45, 35, 35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);
    AppendItem(IDOK, L"Button", L"\x786e\x5b9a", 90, 5, 25, 14, BS_DEFPUSHBUTTON | WS_TABSTOP);
    AppendItem(IDCANCEL, L"Button", L"\x53d6\x6d88", 90, 22, 25, 14, BS_PUSHBUTTON | WS_TABSTOP); 


    return buffer;
}

void* CreateBestTimesTemplate() {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = WS_EX_CONTEXTHELP;
    pDlg->cdit = 11;
    pDlg->x = 10; pDlg->y = 10; pDlg->cx = 160; pDlg->cy = 100;
    p += sizeof(DLGTEMPLATE);
    *(WORD*)p = 0; p += 2; *(WORD*)p = 0; p += 2;
    const wchar_t* title = L"\x626b\x96f7\x82f1\x96c4\x699c";
    wcscpy_s((wchar_t*)p, 64, title); p += (wcslen(title) + 1) * sizeof(wchar_t);

    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy; item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 64, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 64, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
        };

    AppendItem(IDC_STATIC, L"Static", L"\x521d\x7ea7:", 10, 10, 30, 10, SS_LEFT); 
    AppendItem(101, L"Static", L"", 45, 10, 30, 10, SS_LEFT);
    AppendItem(102, L"Static", L"", 85, 10, 60, 10, SS_LEFT);
    AppendItem(IDC_STATIC, L"Static", L"\x4e2d\x7ea7:", 10, 25, 30, 10, SS_LEFT); 
    AppendItem(103, L"Static", L"", 45, 25, 30, 10, SS_LEFT);
    AppendItem(104, L"Static", L"", 85, 25, 60, 10, SS_LEFT);
    AppendItem(IDC_STATIC, L"Static", L"\x9ad8\x7ea7:", 10, 40, 30, 10, SS_LEFT); 
    AppendItem(105, L"Static", L"", 45, 40, 30, 10, SS_LEFT);
    AppendItem(106, L"Static", L"", 85, 40, 60, 10, SS_LEFT);
    AppendItem(107, L"Button", L"\x91cd\x65b0\x8ba1\x5206(&R)", 20, 70, 55, 15, BS_PUSHBUTTON); 
    AppendItem(IDOK, L"Button", L"\x786e\x5b9a", 95, 70, 45, 15, BS_DEFPUSHBUTTON); 
    return buffer;
}

void* CreateRecordScoreTemplate(int levelIdx) {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = 0;
    pDlg->cdit = 3;
    pDlg->x = 20; pDlg->y = 20; pDlg->cx = 120; pDlg->cy = 80;
    p += sizeof(DLGTEMPLATE);
    *(WORD*)p = 0; p += 2; *(WORD*)p = 0; p += 2;
    wcscpy_s((wchar_t*)p, 64, L"\x626b\x96f7"); p += (wcslen(L"\x626b\x96f7") + 1) * sizeof(wchar_t); 

    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy; item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 64, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 64, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
        };

    const wchar_t* lvN = (levelIdx == 0) ? L"\x521d\x7ea7" : (levelIdx == 1 ? L"\x4e2d\x7ea7" : L"\x9ad8\x7ea7");
    wchar_t msg[128];
    swprintf_s(msg, L"\x5df2\x7ec4 %s \x8bb0\x5f55\x3002\n\x8bf7\x7559\x5c0a\x59d3\x5927\x540d\x3002", lvN); 

    AppendItem(IDC_STATIC, L"Static", msg, 10, 10, 100, 20, SS_CENTER);
    AppendItem(101, L"Edit", L"\x533f\x540d", 20, 35, 80, 12, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP);
    AppendItem(IDOK, L"Button", L"\x786e\x5b9a", 40, 55, 40, 15, BS_DEFPUSHBUTTON);
    return buffer;
}

// --- 对话框过程 ---

INT_PTR CALLBACK CustomDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static CustomLevelData* pData = nullptr;
    switch (msg) {
    case WM_INITDIALOG: {
        pData = (CustomLevelData*)lParam;
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            HWND hChild = GetWindow(hDlg, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
        }
        SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, pData->height, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, pData->width, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_MINES, pData->mines, FALSE);
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            BOOL t;
            pData->height = GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, &t, FALSE);
            pData->width = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, &t, FALSE);
            pData->mines = GetDlgItemInt(hDlg, IDC_EDIT_MINES, &t, FALSE);
            EndDialog(hDlg, IDOK);
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
        }
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK BestTimesProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            HWND hChild = GetWindow(hDlg, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
        }
        for (int i = 0; i < 3; i++) {
            wchar_t tStr[32];
            swprintf_s(tStr, L"%d \x79d2", g_HighScores[i].time); // 秒
            SetDlgItemText(hDlg, 101 + i * 2, tStr);
            SetDlgItemText(hDlg, 102 + i * 2, g_HighScores[i].name);
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) EndDialog(hDlg, IDOK);
        if (LOWORD(wParam) == 107) {
            for (int i = 0; i < 3; i++) { g_HighScores[i].time = 999; wcscpy_s(g_HighScores[i].name, 32, L"\x533f\x540d"); }
            EndDialog(hDlg, 107);
        }
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK RecordScoreProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int levelIdx = 0;
    switch (msg) {
    case WM_INITDIALOG: {
        levelIdx = (int)lParam;
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            HWND hC = GetWindow(hDlg, GW_CHILD);
            while (hC) { SendMessage(hC, WM_SETFONT, (WPARAM)hFont, TRUE); hC = GetWindow(hC, GW_HWNDNEXT); }
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemText(hDlg, 101, g_HighScores[levelIdx].name, 32);
            EndDialog(hDlg, IDOK);
        }
        return TRUE;
    }
    return FALSE;
}

// --- 接口实现 ---

HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hGame = CreatePopupMenu();
    AppendMenuW(hGame, MF_STRING, IDM_GAME_NEW, L"\x5f00\x5c40(&N)\tF2");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_BEGINNER, L"\x521d\x7ea7(&B)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_INTERMEDIATE, L"\x4e2d\x7ea7(&I)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXPERT, L"\x9ad8\x7ea7(&E)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_CUSTOM, L"\x81ea\x5b9a\x4e49(&C)...");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_MARKS, L"\x6807\x8bb0(&M)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_COLOR, L"\x989c\x8272(&L)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_SOUND, L"\x58f0\x97f3(&S)");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_BEST_TIMES, L"\x626b\x96f7\x82f1\x96c4\x699c(&T)...");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXIT, L"\x9000\x51fa(&X)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hGame, L"\x6e38\x620f(&G)");

    HMENU hSpecial = CreatePopupMenu();
    AppendMenuW(hSpecial, MF_STRING, IDM_SPECIAL_NETWORK, L"网络游戏(&N)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSpecial, L"特色(&S)");

    return hMenu;
}

void UpdateMenuCheck(HWND hWnd) {
    HMENU hM = GetMenu(hWnd); if (!hM) return;
    HMENU hG = GetSubMenu(hM, 0); if (!hG) return;
    CheckMenuItem(hG, IDM_GAME_MARKS, MF_BYCOMMAND | (g_bMarks ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hG, IDM_GAME_COLOR, MF_BYCOMMAND | (g_bColor ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hG, IDM_GAME_SOUND, MF_BYCOMMAND | (g_bSound ? MF_CHECKED : MF_UNCHECKED));
}

int GetCurrentLevelIndex() {
    int w = g_Logic.GetWidth(), h = g_Logic.GetHeight(), m = g_Logic.GetTotalMines();
    if (w == 9 && h == 9 && m == 10) return 0;
    if (w == 16 && h == 16 && m == 40) return 1;
    if (w == 30 && h == 16 && m == 99) return 2;
    return -1;
}

void CheckHighScore(HWND hWnd) {
    int idx = GetCurrentLevelIndex();
    if (idx != -1 && g_Logic.GetTime() < g_HighScores[idx].time) {
        g_HighScores[idx].time = g_Logic.GetTime();
        void* pT = CreateRecordScoreTemplate(idx);
        DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pT, hWnd, RecordScoreProc, (LPARAM)idx);
        SendMessage(hWnd, WM_COMMAND, IDM_GAME_BEST_TIMES, 0);
    }
}
// 1. 创建连接对话框的内存模板 (修复了字段缺失和对齐问题)
void* CreateNetworkConnectTemplate() {
    static unsigned char buffer[2048]; // 稍微加大缓冲区
    memset(buffer, 0, sizeof(buffer)); // 全部清零，确保 dwExtendedStyle 等字段为 0

    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = 0; // 必须明确设置为 0
    pDlg->cdit = 6;            // 6 个控件
    pDlg->x = 20; pDlg->y = 20; pDlg->cx = 160; pDlg->cy = 80;
    p += sizeof(DLGTEMPLATE);

    // 接下来是菜单(0)、类(0)和标题
    *(WORD*)p = 0; p += 2; // 无菜单
    *(WORD*)p = 0; p += 2; // 使用默认对话框类

    const wchar_t* title = L"连接服务器";
    wcscpy_s((wchar_t*)p, 64, title);
    p += (wcslen(title) + 1) * sizeof(wchar_t);

    // 辅助函数：添加控件并确保 4 字节对齐
    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        while ((UINT_PTR)p % 4 != 0) { *(WORD*)p = 0; p += 2; } // 必须 4 字节对齐

        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0; // 必须明确设置为 0
        item->x = x; item->y = y; item->cx = cx; item->cy = cy;
        item->id = ctrlId;
        p += sizeof(DLGITEMTEMPLATE);

        // 控件类名
        wcscpy_s((wchar_t*)p, 64, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        // 控件文本
        wcscpy_s((wchar_t*)p, 64, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        // 额外数据
        *(WORD*)p = 0; p += 2;
        };

    AppendItem(IDC_STATIC, L"Static", L"服务器地址:", 10, 12, 45, 10, SS_LEFT);
    AppendItem(IDC_EDIT_SERVER_IP, L"Edit", L"127.0.0.1", 60, 10, 90, 12, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP);

    AppendItem(IDC_STATIC, L"Static", L"端口号:", 10, 32, 45, 10, SS_LEFT);
    AppendItem(IDC_EDIT_SERVER_PORT, L"Edit", L"8888", 60, 30, 40, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);

    AppendItem(IDOK, L"Button", L"连接", 40, 55, 40, 15, BS_DEFPUSHBUTTON | WS_TABSTOP);
    AppendItem(IDCANCEL, L"Button", L"取消", 90, 55, 40, 15, BS_PUSHBUTTON | WS_TABSTOP);

    return buffer;
}

// 2. 连接对话框的事件处理
INT_PTR CALLBACK NetworkConnectProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static NetworkConfig* pConfig = nullptr;
    switch (msg) {
    case WM_INITDIALOG: {
        pConfig = (NetworkConfig*)lParam;
        SetDlgItemTextW(hDlg, IDC_EDIT_SERVER_IP, pConfig->ip);
        SetDlgItemInt(hDlg, IDC_EDIT_SERVER_PORT, pConfig->port, FALSE);

        // 设置字体（可选，让界面好看点）
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            HWND hChild = GetWindow(hDlg, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            GetDlgItemTextW(hDlg, IDC_EDIT_SERVER_IP, pConfig->ip, 128);
            BOOL trans;
            pConfig->port = GetDlgItemInt(hDlg, IDC_EDIT_SERVER_PORT, &trans, FALSE);
            EndDialog(hDlg, IDOK);
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
        }
        return TRUE;
    }
    return FALSE;
}
