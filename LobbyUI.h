#pragma once
#define WIN32_LEAN_AND_MEAN  // 必须在 windows.h 之前
#include <windows.h>

class LobbyUI {
public:
    static void ShowConnectDialog(HWND parentWnd);
};
