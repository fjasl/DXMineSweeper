#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "winmm.lib")
#include "NetworkManager.h" // 必须放在第一行！
#include <windows.h>
#include <ctime>
#include <mmsystem.h>
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
#include "AppConfig.h"
#include "LobbyUI.h"
#include "SoundManager.h"


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

D3DContext g_D3D;
MinesweeperLogic g_Logic;
GameRenderer g_Renderer;
DebugUI g_DebugUI;

int g_Width = 0;
int g_Height = 0;
int g_ClientWidth = 0;
int g_ClientHeight = 0;



void UpdateSize() {
  
    g_Width = g_Logic.GetWidth() * CELL_SIZE + OFFSET_X + (int)(DARK_BOARD + FRAME_RIGHT_THICK);
    g_Height = g_Logic.GetHeight() * CELL_SIZE + OFFSET_Y + (int)(DARK_BOARD + FRAME_MIDDLE_BOTTOM_THICK);
    
    if (g_Config.windowScale < 0.2f || g_Config.windowScale > 5.0f) g_Config.windowScale = 1.0f;

    g_ClientWidth = (int)(g_Width * g_Config.windowScale);
    g_ClientHeight = (int)(g_Height * g_Config.windowScale);
}

void ChangeLevel(HWND hWnd, int w, int h, int mines) {

    g_Logic.SetLevel(w, h, mines);

    UpdateSize();

    RECT rc = { 0, 0, g_ClientWidth, g_ClientHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);

    SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

    g_D3D.Resize(g_ClientWidth, g_ClientHeight); 
}




int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    SetProcessDPIAware();
    timeBeginPeriod(1);
    srand((unsigned int)time(nullptr));
    
    LoadAppConfig(); // 尝试读取存档
    // 使用存档中的值初始化（如果没有存档则使用默认值）
    g_Width = g_Config.lastWidth;
    g_Height = g_Config.lastHeight;
    int mines = g_Config.lastMines;
    
    g_Logic.SetLevel(g_Width, g_Height, mines);
    if (g_Config.hasSavedGame && g_Config.autoSaveProgress) {
        g_Logic.LoadStateFromConfig();
    }
    UpdateSize();
    NetworkManager::Instance().Init();
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON)LoadImage(nullptr, L"assets/winmine.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"MineSweepDX11";
    RegisterClassExW(&wcex);

    RECT rc = { 0, 0, g_ClientWidth, g_ClientHeight };
   
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    HMENU hMenu = CreateAppMenu();

    HWND hWnd = CreateWindowW(L"MineSweepDX11", L"MineSweepDX11", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, hMenu, hInstance, nullptr);

    if (!hWnd) return FALSE;

    if (!g_D3D.Initialize(hWnd, g_Width, g_Height)) return FALSE;
	if (!g_Renderer.Initialize(g_D3D.GetDevice())) return FALSE;

    // 在 g_Renderer.Initialize 之后插入
    if (!g_D3D.InitImGui(hWnd)) return FALSE;
    // 在循环处理 WM_QUIT 之前或程序退出前插入清理
    // 注意在程序结束前调用以下清理代码
   

    ShowWindow(hWnd, nCmdShow);
    UpdateMenuCheck(hWnd);
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
            ImGui::GetIO().FontGlobalScale = g_Config.uiScale;
            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            auto context = g_D3D.GetDeviceContext();
			g_Renderer.Render(context, hWnd, g_Logic, g_ClientWidth, g_ClientHeight, g_Width, g_Height, g_Logic.GetWidth(), g_Logic.GetHeight());
            //ImGui::ShowDemoWindow();
            if (g_DebugUI.IsVisible())
            {
                float scaleX = (float)g_ClientWidth / g_Width;
                float scaleY = (float)g_ClientHeight / g_Height;
                g_DebugUI.Render(g_Logic, g_D3D, g_Renderer, scaleX, scaleY);
            }
          
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            g_D3D.Present();

            // FPS Cap ———————————————————————————————
            int fpsCap = g_DebugUI.GetFpsCap();
            if (fpsCap > 0) {
                static LARGE_INTEGER s_freq = {};
                static LARGE_INTEGER s_last = {};
                if (s_freq.QuadPart == 0) {
                    QueryPerformanceFrequency(&s_freq);
                    QueryPerformanceCounter(&s_last);
                }
                LARGE_INTEGER now;
                QueryPerformanceCounter(&now);
                double targetMs  = 1000.0 / fpsCap;
                double elapsedMs = (double)(now.QuadPart - s_last.QuadPart) * 1000.0 / s_freq.QuadPart;
                double sleepMs   = targetMs - elapsedMs;
                if (sleepMs > 1.0) Sleep((DWORD)(sleepMs - 0.5));
                QueryPerformanceCounter(&s_last);
            }
        }
    }

    timeEndPeriod(1);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    // 【修改点】将光标逻辑提前，拦截 ImGui 的默认行为
    if (message == WM_SETCURSOR) {
        // 只有当：选了自定义光标、在客户区、且鼠标没被 ImGui 占用时才处理
        if (g_DebugUI.IsCursorSwitchEnabled() &&
            LOWORD(lParam) == HTCLIENT &&
            (ImGui::GetCurrentContext() == nullptr || !ImGui::GetIO().WantCaptureMouse))
        {
            static std::wstring lastLoadedPath = L"";
            static HCURSOR hCurrentCustom = NULL;
            std::wstring currentPath = g_DebugUI.GetCursorPath();
            if (currentPath != lastLoadedPath || (hCurrentCustom == NULL && !currentPath.empty())) {
                if (!currentPath.empty()) {
                    hCurrentCustom = LoadCursorFromFileW(currentPath.c_str());
                }
                else {
                    hCurrentCustom = NULL;
                }
                lastLoadedPath = currentPath;
            }
            if (hCurrentCustom != NULL) {
                SetCursor(hCurrentCustom);
                return TRUE;
            }
            else {
                SetCursor(NULL);
                return TRUE;
            }
        }
    }
    if (ImGui::GetCurrentContext() != nullptr) {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
    }
  
    switch (message) {
    case WM_SIZING: {
        RECT* pRect = (RECT*)lParam;
        int edge = (int)wParam;
        float targetRatio = (float)g_Width / (float)g_Height;
        int width = pRect->right - pRect->left;
        int height = pRect->bottom - pRect->top;
        if (edge == WMSZ_LEFT || edge == WMSZ_RIGHT || edge == WMSZ_BOTTOMLEFT || edge == WMSZ_TOPLEFT) {
            height = (int)(width / targetRatio);
            if (edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT) pRect->top = pRect->bottom - height;
            else pRect->bottom = pRect->top + height;
        } else {
            width = (int)(height * targetRatio);
            if (edge == WMSZ_TOPLEFT || edge == WMSZ_LEFT || edge == WMSZ_BOTTOMLEFT) pRect->left = pRect->right - width;
            else pRect->right = pRect->left + width;
        }
        return TRUE;
    }
    case WM_SIZE: {
        if (wParam != SIZE_MINIMIZED && g_D3D.GetDevice() != nullptr) {
            g_ClientWidth = LOWORD(lParam);
            g_ClientHeight = HIWORD(lParam);
            if (g_Width > 0 && wParam != SIZE_MAXIMIZED) {
                g_Config.windowScale = (float)g_ClientWidth / (float)g_Width;
            }
            g_D3D.Resize(g_ClientWidth, g_ClientHeight);
        }
        break;
    }
    case WM_CHAR: {
		g_DebugUI.OnCharInput((wchar_t)wParam);
        break;
    }
    case WM_KEYDOWN:
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        if (g_DebugUI.HandleKey((int)wParam)) break;
        
        if (wParam == VK_F2) {
            SendMessage(hWnd, WM_COMMAND, IDM_GAME_NEW, 0);
        }
        else if (g_Config.enableKeyboard) {
            if (wParam == g_Config.keyUp) {
                g_Logic.MoveSelection(0, -1);
            }
            else if (wParam == g_Config.keyDown) {
                g_Logic.MoveSelection(0, 1);
            }
            else if (wParam == g_Config.keyLeft) {
                g_Logic.MoveSelection(-1, 0);
            }
            else if (wParam == g_Config.keyRight) {
                g_Logic.MoveSelection(1, 0);
            }
            else if (wParam == g_Config.keyReveal) {
                if (g_Logic.GetStatus() == GameStatus::Playing) {
                    int selX = g_Logic.GetSelX();
                    int selY = g_Logic.GetSelY();
                    g_Logic.RevealCell(selX, selY);
                    if (g_Logic.GetStatus() == GameStatus::Lost) {
                        PlayGameSound(SoundEvent::Explode);
                    }
                    else if (g_Logic.GetStatus() == GameStatus::Won) {
                        PlayGameSound(SoundEvent::Win);
                        CheckHighScore(hWnd);
                    }
                }
            }
            else if (wParam == g_Config.keyFlag) {
                if (g_Logic.GetStatus() == GameStatus::Playing) {
                    g_Logic.ToggleFlag(g_Logic.GetSelX(), g_Logic.GetSelY());
                }
            }
        }
        break;
    case WM_MOUSEMOVE: {
        if (g_Config.followMouse) {
            int mappedX = (int)(LOWORD(lParam) * ((float)g_Width / g_ClientWidth));
            int mappedY = (int)(HIWORD(lParam) * ((float)g_Height / g_ClientHeight));
            int gridX = (mappedX - OFFSET_X) / CELL_SIZE;
            int gridY = (mappedY - OFFSET_Y) / CELL_SIZE;
            if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
                g_Logic.SetSelection(gridX, gridY);
            }
        }
        break;
    }
    case WM_LBUTTONUP: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        int mouseX = (int)(LOWORD(lParam) * ((float)g_Width / g_ClientWidth));
        int mouseY = (int)(HIWORD(lParam) * ((float)g_Height / g_ClientHeight));
        
        int gridX = (mouseX - OFFSET_X) / CELL_SIZE;
        int gridY = (mouseY - OFFSET_Y) / CELL_SIZE;
        
        int faceX = g_Width / 2 - FACE_SIZE / 2;
        if (mouseX >= faceX && mouseX < faceX + FACE_SIZE && mouseY >= 15 && mouseY < 15 + FACE_SIZE) {
            g_Logic.StartNewGame();
        } else if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            if (g_Logic.GetStatus() == GameStatus::Playing) {
                g_Logic.RevealCell(gridX, gridY);

                if (g_Logic.GetStatus() == GameStatus::Lost) {
                    PlayGameSound(SoundEvent::Explode);
                } else if (g_Logic.GetStatus() == GameStatus::Won) {
                    PlayGameSound(SoundEvent::Win);
                    CheckHighScore(hWnd);
                }
            }
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        int mappedX = (int)(LOWORD(lParam) * ((float)g_Width / g_ClientWidth));
        int mappedY = (int)(HIWORD(lParam) * ((float)g_Height / g_ClientHeight));
        int gridX = (mappedX - OFFSET_X) / CELL_SIZE;
        int gridY = (mappedY - OFFSET_Y) / CELL_SIZE;
        if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            g_Logic.ToggleFlag(gridX, gridY);
            
        }
        break;
    }
    case WM_MBUTTONUP: {
        if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard) break;
        int mappedX = (int)(LOWORD(lParam) * ((float)g_Width / g_ClientWidth));
        int mappedY = (int)(HIWORD(lParam) * ((float)g_Height / g_ClientHeight));
        int gridX = (mappedX - OFFSET_X) / CELL_SIZE;
        int gridY = (mappedY - OFFSET_Y) / CELL_SIZE;
        if (gridX >= 0 && gridX < g_Logic.GetWidth() && gridY >= 0 && gridY < g_Logic.GetHeight()) {
            if (g_Logic.GetStatus() == GameStatus::Playing) {
                g_Logic.TryChord(gridX, gridY);
                if (g_Logic.GetStatus() == GameStatus::Lost) {
                    PlayGameSound(SoundEvent::Explode);
                }
            }
        }
        break;
    }
    case WM_TIMER:
        if (g_Logic.GetStatus() == GameStatus::Playing) {
            PlayGameSound(SoundEvent::Tick);
        }
        g_Logic.UpdateTimer();
        break;
    case WM_DESTROY:
        // 退出前记录当前的关卡信息
        g_Config.lastWidth = g_Logic.GetWidth();
        g_Config.lastHeight = g_Logic.GetHeight();
        g_Config.lastMines = g_Logic.GetTotalMines();

        if (g_Config.autoSaveProgress) {
            g_Logic.SaveStateToConfig();
        } else {
            g_Config.hasSavedGame = false;
        }

        SaveAppConfig(); // 保存到磁盘
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
        case IDM_SPECIAL_NETWORK: {
            LobbyUI::ShowConnectDialog(hWnd);
            break;
        }
        } 
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
