#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include "D3DContext.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"
#include "GameRender.h"

#define IDM_GAME_EXIT 1001 

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


HMENU CreateAppMenu() {
    HMENU hMenu = CreateMenu();      
    HMENU hGame = CreatePopupMenu(); 
    AppendMenuW(hGame, MF_STRING, IDM_GAME_EXIT, L"退出 (&X)");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hGame, L"游戏 (&G)");

    return hMenu;
}



int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    g_Logic.SetLevel(16, 16, 40); 
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_LBUTTONDOWN: {
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
    case WM_MBUTTONDOWN: {
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
        if (LOWORD(wParam) == IDM_GAME_EXIT) {
            PostQuitMessage(0); 
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
