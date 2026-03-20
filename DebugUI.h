#pragma once
#include "MinesweeperLogic.h"
#include "D3DContext.h"
#include "GameRender.h"
#include <string>
class DebugUI {
public:

    void Render(MinesweeperLogic& logic, D3DContext& d3d, GameRenderer& renderer);

    bool IsShowMinesEnabled() const { return m_showMines; }
    void OnCharInput(wchar_t ch);
    bool IsVisible() const { return m_visible; };
    int  GetFpsCap()  const { return m_fpsCap; }  // 0=无限 60=60fps 120=120fps
    std::wstring GetCursorPath() const { return m_cursorPath; }
    bool IsCursorSwitchEnabled() const { return m_cursorSwitch; }

    bool HandleKey(int vk); // Returns true if it consumed the key
    bool IsEditingKey() const { return m_keyToEdit != -1; }
private:

	bool m_visible = false; 
    bool m_showMines = false;
    std::wstring m_inputBuffer;
    const std::wstring m_cheatCode = L"funnyenough";


    bool m_autoSolve = false;      
    float m_solveFreq = 1.0f;      
    double m_lastStepTime = 0.0;   
    bool m_crossSwitch = false; 
    float m_crossColor[3] = { 1.0f, 0.0f, 0.0f };

    bool m_cursorSwitch = false;
    std::wstring m_cursorPath = L"";
    bool m_isPathSelected = false;

    int  m_fpsCap = 0;  // 0=无限 60=60fps 120=120fps
    void PerformAutoStep(MinesweeperLogic& logic);

    int m_keyToEdit = -1; // -1: None, 0: Up, 1: Down, 2: Left, 3: Right, 4: Reveal, 5: Flag
};