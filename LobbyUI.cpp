#include "LobbyUI.h"
#include "NetworkManager.h"
#include "AppMenu.h"
#include <shellapi.h> 
void LobbyUI::ShowConnectDialog(HWND parentWnd) {
    NetworkConfig netCfg;
    wcscpy_s(netCfg.ip, L"127.0.0.1");
    netCfg.port = 8888;

    void* pTemplate = CreateNetworkConnectTemplate();
    if (DialogBoxIndirectParamW(GetModuleHandle(NULL), (LPCDLGTEMPLATEW)pTemplate, parentWnd, NetworkConnectProc, (LPARAM)&netCfg) == IDOK) {
        if (NetworkManager::Instance().Connect(netCfg.ip, netCfg.port)) {
            NetworkManager::Instance().SendJson("JOIN", "Player1");

            // "连接成功，准备启动同步模块..."
            MessageBoxW(parentWnd, L"\x8fde\x63a5\x6210\x529f\xff0c\x51c6\x5907\x542f\x52a8\x540c\x6b65\x6a21\x5757...", L"\x63d0\x793a", MB_OK);

            // 启动 C2
            ShellExecuteW(NULL, L"open", L"C2.exe", NULL, NULL, SW_SHOWNORMAL);
        }
        else {
            // "无法连通服务器，请检查地址或端口。"
            MessageBoxW(parentWnd, L"\x65e0\x6cd5\x8fde\x901a\x670d\x52a1\x5668\xff0c\x8bf7\x68c0\x67e5\x5730\x5740\x6216\x7aef\x53e3\x3002", L"\x9519\x8bef", MB_ICONERROR);
        }
    }
}
