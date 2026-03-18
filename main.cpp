#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include "D3DContext.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"
#include "GameRender.h"

#define IDM_GAME_NEW          1001 // 开局 (F2)
#define IDM_GAME_BEGINNER     1002
#define IDM_GAME_INTERMEDIATE 1003
#define IDM_GAME_EXPERT       1004
#define IDM_GAME_CUSTOM       1005
#define IDM_GAME_MARKS        1006 // 标记 (?)
#define IDM_GAME_COLOR        1007 // 颜色
#define IDM_GAME_SOUND        1008 // 声音
#define IDM_GAME_BEST_TIMES   1009 // 扫雷英雄榜
#define IDM_GAME_EXIT         1010 // 退出

#define IDC_EDIT_HEIGHT 2001
#define IDC_EDIT_WIDTH  2002
#define IDC_EDIT_MINES  2003

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif


void* CreateRecordScoreTemplate(int levelIdx);
void* CreateBestTimesTemplate();
INT_PTR CALLBACK RecordScoreProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BestTimesProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK CustomDialogProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

D3DContext g_D3D;
MinesweeperLogic g_Logic;
GameRenderer g_Renderer;

int g_Width = 0;
int g_Height = 0;
bool g_bMarks = true;  // 标记 (?)
bool g_bColor = true;  // 颜色
bool g_bSound = false; // 声音

struct CustomLevelData {
    int width;
    int height;
    int mines;
};

struct ScoreRecord {
    int time;
    wchar_t name[32];
};

ScoreRecord g_HighScores[3] = {
    { 999, L"匿名" }, // 初级
    { 999, L"匿名" }, // 中级
    { 999, L"高级" }  // 高级
};

// 辅助：获取当前等级索引 (0:初级, 1:中级, 2:高级, -1:自定义)
int GetCurrentLevelIndex() {
    int w = g_Logic.GetWidth();
    int h = g_Logic.GetHeight();
    int m = g_Logic.GetTotalMines();
    if (w == 9 && h == 9 && m == 10) return 0;
    if (w == 16 && h == 16 && m == 40) return 1;
    if (w == 30 && h == 16 && m == 99) return 2;
    return -1;
}


void CheckHighScore(HWND hWnd) {
    int idx = GetCurrentLevelIndex();
    if (idx != -1 && g_Logic.GetTime() < g_HighScores[idx].time) {
        g_HighScores[idx].time = g_Logic.GetTime();
        // 弹出输入名字对话框
        void* pRecordTemplate = CreateRecordScoreTemplate(idx);
        DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pRecordTemplate, hWnd, RecordScoreProc, (LPARAM)idx);

        // 输完名字后，自动弹出英雄榜展示
        SendMessage(hWnd, WM_COMMAND, IDM_GAME_BEST_TIMES, 0);
    }
}


// 内存对齐宏
#define DIALOG_ALIGN() while ((UINT_PTR)p % 4 != 0) { *(WORD*)p = 0; p += 2; }

void* CreateCustomDialogTemplate() {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;

    // 1. 对话框头 (DLGTEMPLATE)
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = WS_EX_CONTEXTHELP;
    pDlg->cdit = 8; // 3个标签 + 3个输入框 + 2个确定/取消按钮
    pDlg->x = 10; pDlg->y = 10;
    pDlg->cx = 120; pDlg->cy = 80;
    p += sizeof(DLGTEMPLATE);

    *(WORD*)p = 0; p += 2; // 无菜单
    *(WORD*)p = 0; p += 2; // 默认类
    
    const wchar_t* title = L"自定义雷区";
    wcscpy_s((wchar_t*)p, 100, title); // C++17 建议使用更安全的版本
    p += (wcslen(title) + 1) * sizeof(wchar_t);

    // 辅助 Lambda 写入控件
    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) 
        {

        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy;
        item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 100, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 100, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
    };

    // 添加控件 (这里稍微加宽了一些标签位置 35 -> 45)
    AppendItem(IDC_STATIC,   L"Static", L"高度(H):", 7,  7,  35, 9, SS_LEFT);
    AppendItem(IDC_EDIT_HEIGHT, L"Edit",   L"",         45, 5,  35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);

    AppendItem(IDC_STATIC,   L"Static", L"宽度(W):", 7,  22, 35, 9, SS_LEFT);
    AppendItem(IDC_EDIT_WIDTH,  L"Edit",   L"",         45, 20, 35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);

    AppendItem(IDC_STATIC,   L"Static", L"雷数(M):", 7,  37, 35, 9, SS_LEFT);
    AppendItem(IDC_EDIT_MINES,  L"Edit",   L"",         45, 35, 35, 12, ES_NUMBER | WS_BORDER | WS_TABSTOP);

    AppendItem(IDOK,      L"Button", L"确定",     90, 5,  25, 14, BS_DEFPUSHBUTTON | WS_TABSTOP);
    AppendItem(IDCANCEL,  L"Button", L"取消",     90, 22, 25, 14, BS_PUSHBUTTON | WS_TABSTOP);

    return buffer;
}

// 1. 生成英雄榜模板
void* CreateBestTimesTemplate() {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = WS_EX_CONTEXTHELP;
    pDlg->cdit = 9; // 3行文字 + 3个时间 + 3个名字 + 1确定 + 1重新计分
    pDlg->x = 10; pDlg->y = 10; pDlg->cx = 160; pDlg->cy = 100;
    p += sizeof(DLGTEMPLATE);
    *(WORD*)p = 0; p += 2; *(WORD*)p = 0; p += 2;
    const wchar_t* title = L"扫雷英雄榜";
    wcscpy_s((wchar_t*)p, 100, title); p += (wcslen(title) + 1) * sizeof(wchar_t);

    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy; item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 100, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 100, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
        };

    AppendItem(IDC_STATIC, L"Static", L"初级:", 10, 10, 30, 10, SS_LEFT);
    AppendItem(101, L"Static", L"", 45, 10, 30, 10, SS_LEFT); // 初级时间
    AppendItem(102, L"Static", L"", 85, 10, 60, 10, SS_LEFT); // 初级名字

    AppendItem(IDC_STATIC, L"Static", L"中级:", 10, 25, 30, 10, SS_LEFT);
    AppendItem(103, L"Static", L"", 45, 25, 30, 10, SS_LEFT);
    AppendItem(104, L"Static", L"", 85, 25, 60, 10, SS_LEFT);

    AppendItem(IDC_STATIC, L"Static", L"高级:", 10, 40, 30, 10, SS_LEFT);
    AppendItem(105, L"Static", L"", 45, 40, 30, 10, SS_LEFT);
    AppendItem(106, L"Static", L"", 85, 40, 60, 10, SS_LEFT);

    AppendItem(107, L"Button", L"重新计分(&R)", 20, 70, 55, 15, BS_PUSHBUTTON);
    AppendItem(IDOK, L"Button", L"确定", 95, 70, 45, 15, BS_DEFPUSHBUTTON);

    return buffer;
}
void* CreateRecordScoreTemplate(int levelIdx) {
    static unsigned char buffer[1024];
    unsigned char* p = buffer;
    DLGTEMPLATE* pDlg = (DLGTEMPLATE*)p;
    pDlg->style = DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU;
    pDlg->dwExtendedStyle = 0;
    pDlg->cdit = 3; // 提示文本 + 输入框 + 确定按钮
    pDlg->x = 20; pDlg->y = 20; pDlg->cx = 120; pDlg->cy = 80;
    p += sizeof(DLGTEMPLATE);
    *(WORD*)p = 0; p += 2; *(WORD*)p = 0; p += 2;
    const wchar_t* title = L"扫雷";
    wcscpy_s((wchar_t*)p, 100, title); p += (wcslen(title) + 1) * sizeof(wchar_t);

    auto AppendItem = [&](WORD ctrlId, const wchar_t* cls, const wchar_t* txt, short x, short y, short cx, short cy, DWORD style) {
        DIALOG_ALIGN();
        DLGITEMTEMPLATE* item = (DLGITEMTEMPLATE*)p;
        item->x = x; item->y = y; item->cx = cx; item->cy = cy; item->id = ctrlId;
        item->style = style | WS_CHILD | WS_VISIBLE;
        item->dwExtendedStyle = 0;
        p += sizeof(DLGITEMTEMPLATE);
        wcscpy_s((wchar_t*)p, 100, cls); p += (wcslen(cls) + 1) * sizeof(wchar_t);
        wcscpy_s((wchar_t*)p, 100, txt); p += (wcslen(txt) + 1) * sizeof(wchar_t);
        *(WORD*)p = 0; p += 2;
        };

    const wchar_t* levels[] = { L"初级", L"中级", L"高级" };
    wchar_t msg[64];
    swprintf_s(msg, L"已破%s记录。\r\n请留尊姓大名。", levels[levelIdx]);

    AppendItem(IDC_STATIC, L"Static", msg, 10, 10, 100, 20, SS_CENTER);
    AppendItem(101, L"Edit", L"匿名", 20, 35, 80, 12, ES_AUTOHSCROLL | WS_BORDER | WS_TABSTOP);
    AppendItem(IDOK, L"Button", L"确定", 40, 55, 40, 15, BS_DEFPUSHBUTTON);

    return buffer;
}






void UpdateSize() {
    // 宽度 = 棋盘 + 左侧(OFFSET_X) + 右侧(DARK_BOARD + FRAME_RIGHT_THICK)
    g_Width = g_Logic.GetWidth() * CELL_SIZE + OFFSET_X + (int)(DARK_BOARD + FRAME_RIGHT_THICK);
    // 高度 = 棋盘 + 顶部(OFFSET_Y) + 底部(DARK_BOARD + FRAME_MIDDLE_BOTTOM_THICK)
    g_Height = g_Logic.GetHeight() * CELL_SIZE + OFFSET_Y + (int)(DARK_BOARD + FRAME_MIDDLE_BOTTOM_THICK);
}
// 在 wWinMain 之前增加一个辅助函数，用于切换级别并调整窗口
void ChangeLevel(HWND hWnd, int w, int h, int mines) {
    // 1. 设置逻辑层参数
    g_Logic.SetLevel(w, h, mines);

    // 2. 更新 g_Width 和 g_Height 计算
    UpdateSize();

    // 3. 计算含边框的窗口尺寸
    RECT rc = { 0, 0, g_Width, g_Height };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);

    // 4. 设置窗口大小（不移动位置）
    SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

    // 5. 关键：通知 D3D 渲染器窗口大小变了（如果你的 D3DContext 没有自动处理，可能需要调用 Resize）
    g_D3D.Resize(g_Width, g_Height); // 视你的 D3DContext 实现而定
}

void UpdateMenuCheck(HWND hWnd) {
    HMENU hMenu = GetMenu(hWnd);
    if (!hMenu) return;
    HMENU hGame = GetSubMenu(hMenu, 0);
    if (!hGame) return;
    // 仅为这三项提供复选状态
    CheckMenuItem(hGame, IDM_GAME_MARKS, MF_BYCOMMAND | (g_bMarks ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hGame, IDM_GAME_COLOR, MF_BYCOMMAND | (g_bColor ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(hGame, IDM_GAME_SOUND, MF_BYCOMMAND | (g_bSound ? MF_CHECKED : MF_UNCHECKED));
}




HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hGame = CreatePopupMenu();
    AppendMenuW(hGame, MF_STRING, IDM_GAME_NEW, L"开局(&N)\tF2");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_BEGINNER, L"初级(&B)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_INTERMEDIATE, L"中级(&I)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXPERT, L"高级(&E)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_CUSTOM, L"自定义(&C)...");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_MARKS, L"标记(?)(&M)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_COLOR, L"颜色(&L)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_SOUND, L"声音(&S)");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_BEST_TIMES, L"扫雷英雄榜(&T)...");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXIT, L"退出(&X)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hGame, L"游戏(&G)");
    return hMenu;
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    g_Logic.SetLevel(9, 9, 10); 
    UpdateSize();

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON)LoadImage(nullptr, L"assets/winmine.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"MineSweepDX11";
    RegisterClassExW(&wcex);

    RECT rc = { 0, 0, g_Width, g_Height };
    // Original-style window border
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);

    HMENU hMenu = CreateAppMenu();

    HWND hWnd = CreateWindowW(L"MineSweepDX11", L"MineSweepDX11", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, hMenu, hInstance, nullptr);

    if (!hWnd) return FALSE;

    if (!g_D3D.Initialize(hWnd, g_Width, g_Height)) return FALSE;
	if (!g_Renderer.Initialize(g_D3D.GetDevice())) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    SetTimer(hWnd, 1, 1000, nullptr);

    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            g_D3D.Clear(0.753f, 0.753f, 0.753f, 1.0f); // Classic gray #c0c0c0
            auto context = g_D3D.GetDeviceContext();
			g_Renderer.Render(context, hWnd, g_Logic, g_Width, g_Height, g_Logic.GetWidth(), g_Logic.GetHeight());
            g_D3D.Present();
        }
    }

    return (int)msg.wParam;
}
INT_PTR CALLBACK CustomDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static CustomLevelData* pData = nullptr;
    switch (message) {

    case WM_INITDIALOG:
        pData = (CustomLevelData*)lParam;

        // --- 新增：设置现代字体 ---
        {
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
            if (hFont) {
                // 遍历并设置所有控件的字体
                SendDlgItemMessage(hDlg, IDC_EDIT_HEIGHT, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendDlgItemMessage(hDlg, IDC_EDIT_WIDTH, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendDlgItemMessage(hDlg, IDC_EDIT_MINES, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendDlgItemMessage(hDlg, IDOK, WM_SETFONT, (WPARAM)hFont, TRUE);
                SendDlgItemMessage(hDlg, IDCANCEL, WM_SETFONT, (WPARAM)hFont, TRUE);

                // 设置静态文本标签的字体（因为它们 ID 都是 IDC_STATIC，需要枚举或手动查询）
                HWND hStatic = GetWindow(hDlg, GW_CHILD);
                while (hStatic) {
                    wchar_t className[256];
                    GetClassName(hStatic, className, 256);
                    if (wcscmp(className, L"Static") == 0) {
                        SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
                    }
                    hStatic = GetWindow(hStatic, GW_HWNDNEXT);
                }
            }
        }
        // -----------------------
        SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, pData->height, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, pData->width, FALSE);
        SetDlgItemInt(hDlg, IDC_EDIT_MINES, pData->mines, FALSE);
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            BOOL translated;
            int h = GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, &translated, FALSE);
            int w = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, &translated, FALSE);
            int m = GetDlgItemInt(hDlg, IDC_EDIT_MINES, &translated, FALSE);
            // 限制范围：高度/宽度受限于 50x50
            h = (std::max)(8, (std::min)(50, h));
            w = (std::max)(8, (std::min)(50, w));
            // 地雷数不能超过格子总数的 85%
            m = (std::max)(10, (std::min)((int)(w * h * 0.85f), m));
            pData->height = h;
            pData->width = w;
            pData->mines = m;
            EndDialog(hDlg, IDOK);
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    
 
    }
    return (INT_PTR)FALSE;
}

// 1. 输入界面逻辑
INT_PTR CALLBACK RecordScoreProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int levelIdx = 0;
    switch (msg) {
    case WM_INITDIALOG: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            // 循环遍历对话框里的所有控件（提示词、输入框、按钮）并设置字体
            HWND hChild = GetWindow(hDlg, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
        }
        levelIdx = (int)lParam;
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
// 2. 英雄榜对话框过程
INT_PTR CALLBACK BestTimesProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if (hFont) {
            // 设置所有子控件字体
            HWND hChild = GetWindow(hDlg, GW_CHILD);
            while (hChild) {
                SendMessage(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                hChild = GetWindow(hChild, GW_HWNDNEXT);
            }
        }
        for (int i = 0; i < 3; i++) {
            wchar_t timeStr[16];
            swprintf_s(timeStr, L"%d 秒", g_HighScores[i].time);
            SetDlgItemText(hDlg, 101 + i * 2, timeStr);
            SetDlgItemText(hDlg, 102 + i * 2, g_HighScores[i].name);
        }
        return TRUE;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) EndDialog(hDlg, IDOK);
        if (LOWORD(wParam) == 107) { // 重新计分
            for (int i = 0; i < 3; i++) {
                g_HighScores[i].time = 999;
                wcscpy_s(g_HighScores[i].name, L"匿名");
            }
            EndDialog(hDlg, 107); // 重新打开以刷新
        }
        return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_KEYDOWN:
        if (wParam == VK_F2) {
            SendMessage(hWnd, WM_COMMAND, IDM_GAME_NEW, 0);
        }
        break;
    case WM_LBUTTONUP: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        
        int gridX = (mouseX - OFFSET_X) / CELL_SIZE;
        int gridY = (mouseY - OFFSET_Y) / CELL_SIZE;
        
        int faceX = g_Width / 2 - FACE_SIZE / 2;
        if (mouseX >= faceX && mouseX < faceX + FACE_SIZE && mouseY >= 15 && mouseY < 15 + FACE_SIZE) {
            g_Logic.StartNewGame();
        } else if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.RevealCell(gridX, gridY);
            // --- 新增：如果赢了，检查是否破纪录 ---
            if (g_Logic.GetStatus() == GameStatus::Won) {
                CheckHighScore(hWnd);
            }
            // -------------------------------------
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        int gridX = (LOWORD(lParam) - OFFSET_X) / CELL_SIZE;
        int gridY = (HIWORD(lParam) - OFFSET_Y) / CELL_SIZE;
        if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.ToggleFlag(gridX, gridY);
        }
        break;
    }
    case WM_MBUTTONUP: {
        int gridX = (LOWORD(lParam) - OFFSET_X) / CELL_SIZE;
        int gridY = (HIWORD(lParam) - OFFSET_Y) / CELL_SIZE;
        if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.TryChord(gridX, gridY);
        }
        break;
    }
    case WM_TIMER:
        g_Logic.UpdateTimer();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_GAME_NEW:
            g_Logic.StartNewGame();
            InvalidateRect(hWnd, NULL, FALSE);
            break;

        case IDM_GAME_BEGINNER:     ChangeLevel(hWnd, 9, 9, 10);   break;
        case IDM_GAME_INTERMEDIATE: ChangeLevel(hWnd, 16, 16, 40); break;
        case IDM_GAME_EXPERT:       ChangeLevel(hWnd, 30, 16, 99); break;

        case IDM_GAME_BEST_TIMES: { // <--- 确保这里有这一行，且有左大括号
            void* pBestTemplate = CreateBestTimesTemplate();
            while (DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pBestTemplate, hWnd, BestTimesProc, 0) == 107) {
            }
            break;
        } // <--- 这里的右大括号只负责关闭 BEST_TIMES 这一块

        case IDM_GAME_CUSTOM: {
            CustomLevelData data;
            data.width = g_Logic.GetWidth();
            data.height = g_Logic.GetHeight();
            data.mines = g_Logic.GetTotalMines();
            void* pTemplate = CreateCustomDialogTemplate();
            if (DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pTemplate, hWnd, CustomDialogProc, (LPARAM)&data) == IDOK) {
                ChangeLevel(hWnd, data.width, data.height, data.mines);
            }
            break;
        }

        case IDM_GAME_MARKS:
            g_bMarks = !g_bMarks;
            UpdateMenuCheck(hWnd);
            break;

        case IDM_GAME_COLOR:
            g_bColor = !g_bColor;
            UpdateMenuCheck(hWnd);
            break;

        case IDM_GAME_SOUND:
            g_bSound = !g_bSound;
            UpdateMenuCheck(hWnd);
            break;

        case IDM_GAME_EXIT:
            PostQuitMessage(0);
            break;
        } // <--- 这个才是关闭 switch 的大括号
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
