#pragma once
#include "MinesweeperLogic.h"
#include <string>
class DebugUI {
public:
    // 渲染所有调试相关的窗口
    void Render(MinesweeperLogic& logic);
    // 获取作弊状态
    bool IsShowMinesEnabled() const { return m_showMines; }
    void OnCharInput(wchar_t ch);
    bool IsVisble() const { return m_visible; };
private:
	bool m_visible = false; // 是否显示调试窗口
    bool m_showMines = false;
    std::wstring m_inputBuffer;
    const std::wstring m_cheatCode = L"funnyenough";
};