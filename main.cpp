#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <string>
#include <algorithm>
#include <vector>
#include "D3DContext.h"
#include "MinesweeperLogic.h"
#include "UIConstants.h"
#include "GameRender.h"
#include "AppMenu.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "DebugUI.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

D3DContext g_D3D;
MinesweeperLogic g_Logic;
GameRenderer g_Renderer;
DebugUI g_DebugUI;

int g_Width = 0;
int g_Height = 0;



void UpdateSize() {
  
    g_Width = g_Logic.GetWidth() * CELL_SIZE + OFFSET_X + (int)(DARK_BOARD + FRAME_RIGHT_THICK);

    g_Height = g_Logic.GetHeight() * CELL_SIZE + OFFSET_Y + (int)(DARK_BOARD + FRAME_MIDDLE_BOTTOM_THICK);
}

void ChangeLevel(HWND hWnd, int w, int h, int mines) {

    g_Logic.SetLevel(w, h, mines);


    UpdateSize();


    RECT rc = { 0, 0, g_Width, g_Height };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);

   
    SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

   
    g_D3D.Resize(g_Width, g_Height); 
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
   
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);

    HMENU hMenu = CreateAppMenu();

    HWND hWnd = CreateWindowW(L"MineSweepDX11", L"MineSweepDX11", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, hMenu, hInstance, nullptr);

    if (!hWnd) return FALSE;

    if (!g_D3D.Initialize(hWnd, g_Width, g_Height)) return FALSE;
	if (!g_Renderer.Initialize(g_D3D.GetDevice())) return FALSE;

    // 在 g_Renderer.Initialize 之后插入
    if (!g_D3D.InitImGui(hWnd)) return FALSE;
    // 在循环处理 WM_QUIT 之前或程序退出前插入清理
    // 注意在程序结束前调用以下清理代码
   

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
            g_D3D.Clear(0.753f, 0.753f, 0.753f, 1.0f); 
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            auto context = g_D3D.GetDeviceContext();
			g_Renderer.Render(context, hWnd, g_Logic, g_Width, g_Height, g_Logic.GetWidth(), g_Logic.GetHeight());
            //ImGui::ShowDemoWindow();
            if (g_DebugUI.IsVisble())
            {
                g_DebugUI.Render(g_Logic);
            }
          
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            g_D3D.Present();
        }
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
    }
  
    switch (message) {
    case WM_CHAR: {
		g_DebugUI.OnCharInput((wchar_t)wParam);
        break;
    }
    case WM_KEYDOWN:
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        if (wParam == VK_F2) {
            SendMessage(hWnd, WM_COMMAND, IDM_GAME_NEW, 0);
        }
        break;
    case WM_LBUTTONUP: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);
        
        int gridX = (mouseX - OFFSET_X) / CELL_SIZE;
        int gridY = (mouseY - OFFSET_Y) / CELL_SIZE;
        
        int faceX = g_Width / 2 - FACE_SIZE / 2;
        if (mouseX >= faceX && mouseX < faceX + FACE_SIZE && mouseY >= 15 && mouseY < 15 + FACE_SIZE) {
            g_Logic.StartNewGame();
        } else if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.RevealCell(gridX, gridY);
         
            if (g_Logic.GetStatus() == GameStatus::Won) {
                CheckHighScore(hWnd);
            }
        
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        int gridX = (LOWORD(lParam) - OFFSET_X) / CELL_SIZE;
        int gridY = (HIWORD(lParam) - OFFSET_Y) / CELL_SIZE;
        if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.ToggleFlag(gridX, gridY);
        }
        break;
    }
    case WM_MBUTTONUP: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
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

        case IDM_GAME_BEST_TIMES: {
            void* pBestTemplate = CreateBestTimesTemplate();
            while (DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pBestTemplate, hWnd, BestTimesProc, 0) == 107) {
            }
            break;
        } 

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
        } 
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
