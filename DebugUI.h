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
private:
	bool m_visible = false; 
    bool m_showMines = false;
    std::wstring m_inputBuffer;
    const std::wstring m_cheatCode = L"funnyenough";


    bool m_autoSolve = false;      
    float m_solveFreq = 1.0f;      
    double m_lastStepTime = 0.0;   
    void PerformAutoStep(MinesweeperLogic& logic);
};