#include "DebugUI.h"
#include "imgui/imgui.h"

void DebugUI::Render(MinesweeperLogic& logic, D3DContext& d3d, GameRenderer& renderer) {
  
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Global State Monitor", &m_visible)) {

        if (ImGui::BeginTabBar("Modules")) {

           
            if (ImGui::BeginTabItem("Logic")) {

                // 1. 暴露内存位置 (Memory Addresses)
                ImGui::TextColored(ImVec4(1, 0, 1, 1), "Memory Inspector:");
                ImGui::BulletText("Logic Object Address: %p", &logic);
                // 如果您在 MinesweeperLogic 中把 m_board 设为公有或添加了友元，可以打印更深层地址
                // ImGui::BulletText("Board Array Address: %p", &logic.m_board); 
                ImGui::Separator();
                // 2. 带参数的方法调用 (SetLevel)
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Call: SetLevel(w, h, m)");
                static int setW = 9, setH = 9, setM = 10;
                ImGui::SetNextItemWidth(100); ImGui::InputInt("W##set", &setW); ImGui::SameLine();
                ImGui::SetNextItemWidth(100); ImGui::InputInt("H##set", &setH); ImGui::SameLine();
                ImGui::SetNextItemWidth(100); ImGui::InputInt("M##set", &setM);

                if (ImGui::Button("Apply Level Settings")) {
                    logic.SetLevel(setW, setH, setM);
                    // 注意：修改关卡后，主窗口大小可能需要同步更新，这里可能需要调用 main.cpp 里的 UpdateSize
                }
                ImGui::Separator();
                // 3. 实时单元格操作 (Reveal/Flag/Chord)
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Cell Operations:");
                static int opX = 0, opY = 0;
                ImGui::SetNextItemWidth(100); ImGui::InputInt("X##op", &opX); ImGui::SameLine();
                ImGui::SetNextItemWidth(100); ImGui::InputInt("Y##op", &opY);
                if (ImGui::Button("RevealCell")) logic.RevealCell(opX, opY);
                ImGui::SameLine();
                if (ImGui::Button("ToggleFlag")) logic.ToggleFlag(opX, opY);
                ImGui::SameLine();
                if (ImGui::Button("TryChord"))  logic.TryChord(opX, opY);
                ImGui::Separator();
                // 4. 只读属性展示 (原有内容)
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Live Properties:");
                ImGui::BulletText("Current Board: %d x %d", logic.GetWidth(), logic.GetHeight());
                ImGui::BulletText("Mines Left: %d / %d", logic.GetMinesLeft(), logic.GetTotalMines());
                ImGui::BulletText("Status: %d (0:Playing, 1:Won, 2:Lost)", (int)logic.GetStatus());
                if (ImGui::TreeNode("Raw Memory Viewer")) {
                    static int inspectX = 0, inspectY = 0;
                    ImGui::InputInt("Inspect X", &inspectX);
                    ImGui::InputInt("Inspect Y", &inspectY);
                    if (inspectX >= 0 && inspectX < logic.GetWidth() && inspectY >= 0 && inspectY < logic.GetHeight()) {
                        unsigned char rawVal = logic.m_board[inspectY * logic.m_width + inspectX];
                        ImGui::Text("Cell Memory Address: %p", &logic.m_board[inspectY * logic.m_width + inspectX]);
                        ImGui::Text("Raw Hex Value: 0x%02X", rawVal);

                        // 解释位信息
                        ImGui::BulletText("Is Mine: %s", (rawVal & STATE_MINE) ? "YES" : "NO");
                        ImGui::BulletText("Is Open: %s", (rawVal & STATE_OPEN) ? "YES" : "NO");
                        ImGui::BulletText("Is Flag: %s", (rawVal & STATE_FLAG) ? "YES" : "NO");
                        ImGui::BulletText("Is Quest: %s", (rawVal & STATE_QUESTION) ? "YES" : "NO");
                        ImGui::BulletText("Neighbor Count: %d", rawVal & MASK_COUNT);
                    }
                    ImGui::TreePop();
                }
                ImGui::EndTabItem();
                
            }

            
            if (ImGui::BeginTabItem("Graphics")) {
               
                ImGui::Checkbox("Enable Cross Locate", &m_crossSwitch);
                if (ImGui::ColorEdit3("Clear Color",m_crossColor)) {

                }
                ImGui::Separator();
                ImGui::Text("Device Address: %p", d3d.GetDevice());
                ImGui::Text("Context Address: %p", d3d.GetDeviceContext());

                static float clearColor[3] = { 0.753f, 0.753f, 0.753f };
                if (ImGui::ColorEdit3("Clear Color", clearColor)) {
                    
                }
                ImGui::EndTabItem();
            }

            
            if (ImGui::BeginTabItem("Cheats")) {
                ImGui::Checkbox("X-Ray: Show Mines on Grid", &m_showMines);
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0, 1, 1, 1), "Auto-Solve Controller:");
                ImGui::SetNextItemWidth(100);
                if (ImGui::InputFloat("Freq (Hz)", &m_solveFreq)) {
                    
                    if (m_solveFreq <= 0.0f) m_solveFreq = 1.0f;
                }
                ImGui::Checkbox("Enable Auto-Solver", &m_autoSolve);

                if (ImGui::Button("Instant Win (Reveal All)")) {
                    for (int y = 0; y < logic.GetHeight(); ++y) {
                        for (int x = 0; x < logic.GetWidth(); ++x) {
                            if (!logic.IsMine(x, y)) logic.RevealCell(x, y);
                        }
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("CustomInfo")) {

                ImGui::Text("FPS: %.1f  (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

                // 将鼠标屏幕坐标转换为棋盘格子坐标
                ImVec2 mp = ImGui::GetMousePos();
                int cellX = (int)(mp.x - OFFSET_X) / CELL_SIZE;
                int cellY = (int)(mp.y - OFFSET_Y) / CELL_SIZE;

                bool inBoard = (cellX >= 0 && cellX < logic.GetWidth() &&
                                cellY >= 0 && cellY < logic.GetHeight());

                ImGui::Text("Mouse Screen: (%.0f, %.0f)", mp.x, mp.y);

                if (inBoard) {
                    bool hasMine = logic.IsMine(cellX, cellY);
                    // 先输出坐标文字
                    ImGui::Text("CurrentLocation: %d, %d  HasMine:", cellX, cellY);
                    ImGui::SameLine();
                    if (hasMine) {
                        // 红色 X
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "[NO]"); // ✗
                    } else {
                        // 绿色 √
                        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.3f, 1.0f), "[OK]"); // ✓
                    }
                } else {
                    ImGui::TextDisabled("CurrentLocation: -- (out of board)");
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
   
    if (m_showMines && logic.GetStatus() == GameStatus::Playing) {
      
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        for (int y = 0; y < logic.GetHeight(); ++y) {
            for (int x = 0; x < logic.GetWidth(); ++x) {
               
                if (logic.IsMine(x, y) && !logic.IsRevealed(x, y)) {
                  
                    float rect_x1 = (float)(OFFSET_X + x * CELL_SIZE);
                    float rect_y1 = (float)(OFFSET_Y + y * CELL_SIZE);
                    float rect_x2 = rect_x1 + CELL_SIZE;
                    float rect_y2 = rect_y1 + CELL_SIZE;
                    drawList->AddRectFilled(
                        ImVec2(rect_x1 + 2, rect_y1 + 2),
                        ImVec2(rect_x2 - 2, rect_y2 - 2),
                        IM_COL32(255, 0, 0, 100),
                        2.0f
                    );
                   
                    drawList->AddRect(
                        ImVec2(rect_x1 + 1, rect_y1 + 1),
                        ImVec2(rect_x2 - 1, rect_y2 - 1),
                        IM_COL32(255, 255, 0, 200) 
                    );
                }
            }
        }
    }
    if (m_autoSolve && logic.GetStatus() == GameStatus::Playing) {
        double currentTime = ImGui::GetTime();
        double interval = 1.0 / (double)m_solveFreq;
        if (currentTime - m_lastStepTime >= interval) {
            PerformAutoStep(logic);
            m_lastStepTime = currentTime;
        }
    }
    if (m_crossSwitch)
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

       
        ImU32 lineColor = IM_COL32(
            (int)(m_crossColor[0] * 255),
            (int)(m_crossColor[1] * 255),
            (int)(m_crossColor[2] * 255),
            180
        );
        float lineThickness = 1.0f;

        
        drawList->AddLine(
            ImVec2(0.0f,          mousePos.y),
            ImVec2(displaySize.x, mousePos.y),
            lineColor, lineThickness
        );

        // 垂直线：从屏幕最上到最下，X = 鼠标X
        drawList->AddLine(
            ImVec2(mousePos.x, 0.0f),
            ImVec2(mousePos.x, displaySize.y),
            lineColor, lineThickness
        );
    }
}


void DebugUI::OnCharInput(wchar_t ch) {
   
    m_inputBuffer += towlower(ch); 


    if (m_inputBuffer.length() > m_cheatCode.length()) {
        m_inputBuffer.erase(0, 1);
    }

    
    if (m_inputBuffer == m_cheatCode) {
        m_visible = !m_visible;
        m_inputBuffer.clear();
    }
}
void DebugUI::PerformAutoStep(MinesweeperLogic& logic) {
    for (int y = 0; y < logic.GetHeight(); ++y) {
        for (int x = 0; x < logic.GetWidth(); ++x) {
            
            if (!logic.IsMine(x, y) && !logic.IsRevealed(x, y)) {
                logic.RevealCell(x, y);
                return; 
            }
        }
    }
    
    m_autoSolve = false;
}