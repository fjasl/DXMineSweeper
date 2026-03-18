#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include "D3DContext.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"
#include "GameRender.h"

#define IDM_GAME_EXIT 1001 
#define IDM_GAME_BEGINNER     1002
#define IDM_GAME_INTERMEDIATE 1003
#define IDM_GAME_EXPERT       1004
#define IDM_GAME_CUSTOM       1005 // 用于 50x50 等自定义


#define IDC_EDIT_HEIGHT 2001
#define IDC_EDIT_WIDTH  2002
#define IDC_EDIT_MINES  2003

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif


struct CustomLevelData {
    int width;
    int height;
    int mines;
};

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



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

D3DContext g_D3D;

MinesweeperLogic g_Logic;
GameRenderer g_Renderer;

int g_Width = 0; 
int g_Height = 0;


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





HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();
    HMENU hGame = CreatePopupMenu();
    AppendMenuW(hGame, MF_STRING, IDM_GAME_BEGINNER, L"初级 (9x9, 10雷)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_INTERMEDIATE, L"中级 (16x16, 40雷)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXPERT, L"高级 (30x16, 99雷)");
    AppendMenuW(hGame, MF_STRING, IDM_GAME_CUSTOM, L"自定义");
    AppendMenuW(hGame, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXIT, L"退出 (&X)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hGame, L"游戏 (&G)");
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
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
        case IDM_GAME_BEGINNER:     ChangeLevel(hWnd, 9, 9, 10);   break;
        case IDM_GAME_INTERMEDIATE: ChangeLevel(hWnd, 16, 16, 40); break;
        case IDM_GAME_EXPERT:       ChangeLevel(hWnd, 30, 16, 99); break;
            // --- 放在这里 ---
        case IDM_GAME_CUSTOM: {
            CustomLevelData data;
            data.width = g_Logic.GetWidth();
            data.height = g_Logic.GetHeight();
            data.mines = 40;
            void* pTemplate = CreateCustomDialogTemplate();
            if (DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pTemplate, hWnd, CustomDialogProc, (LPARAM)&data) == IDOK) {
                ChangeLevel(hWnd, data.width, data.height, data.mines);
            }
            break;
        }
                            // ----------------
        case IDM_GAME_EXIT:         PostQuitMessage(0);            break;
        }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
