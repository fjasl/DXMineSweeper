#include "DebugUI.h"
#include "imgui/imgui.h"

void DebugUI::Render(MinesweeperLogic& logic) {
    if (ImGui::Begin("Cheat & Debug Menu", &m_visible)) {

        if (ImGui::CollapsingHeader("Cheats", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Vision (Show All Mines)", &m_showMines);

            if (ImGui::Button("Instant Win")) {
                // 这里可以在逻辑层实现一个接口，或者直接手动揭开所有非雷格
                // 暂时留空，一会儿可以细化逻辑
            }
        }

        if (ImGui::CollapsingHeader("Game Stats")) {
            ImGui::Text("Time: %d s", logic.GetTime());
            ImGui::Text("Mines Left: %d", logic.GetMinesLeft());
            ImGui::Text("Status: %s",
                (logic.GetStatus() == GameStatus::Won ? "WON" :
                    (logic.GetStatus() == GameStatus::Lost ? "LOST" : "Playing")));
        }

    
    
    }
   
    ImGui::End();
}

void DebugUI::OnCharInput(wchar_t ch) {
    // 将输入的字符加入缓冲区
    m_inputBuffer += towlower(ch); // 转为小写匹配更好用

    // 只保留最近和作弊码长度相同的字符
    if (m_inputBuffer.length() > m_cheatCode.length()) {
        m_inputBuffer.erase(0, 1);
    }

    // 如果匹配成功，切换可见性
    if (m_inputBuffer == m_cheatCode) {
        m_visible = !m_visible;
        m_inputBuffer.clear(); // 匹配后清空，防止连按导致频繁切换
    }
}