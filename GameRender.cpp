#include "GameRender.h"
#include "TextureLoader.h"
#include "imgui/imgui.h"
#include "AppConfig.h"

bool GameRenderer::Initialize(ID3D11Device* device) {
    if (!m_Sprites.Initialize(device)) {
        return false;
    }


    m_TexBlocks = TextureLoader::LoadTexture(device, L"assets/blocks.bmp");
    m_TexLed = TextureLoader::LoadTexture(device, L"assets/led.bmp");
    m_TexButton = TextureLoader::LoadTexture(device, L"assets/button.bmp");


}


void GameRenderer ::Draw3DLine(ID3D11DeviceContext* ctx, float x, float y, float w, float h, bool light) {
    
    float srcX = light ? 0.0f : 15.0f;
    float srcY = light ? 0.0f : 15.0f;

     m_Sprites.Draw(ctx, m_TexBlocks.srv.Get(),
        x, y, w, h,        
        srcX, srcY, 1.0f, 1.0f
    );
}

void GameRenderer ::DrawBaseFrame(ID3D11DeviceContext* ctx, int width, int height, int boardW, int boardH) {

    Draw3DLine(ctx, 0, 0, (float)width, LIGHT_BOARD, true); 
    Draw3DLine(ctx, 0, 0, LIGHT_BOARD, (float)height, true); 


    float hudX_inner = LIGHT_BOARD + FRAME_LEFT_THICK;
    float hudY_inner = LIGHT_BOARD + FRAME_MIDDLE_TOP_THICK;
    float hudW_inner = (float)width - hudX_inner - FRAME_RIGHT_THICK;


    Draw3DLine(ctx, hudX_inner, hudY_inner, hudW_inner, DARK_BOARD, false); 
    Draw3DLine(ctx, hudX_inner, hudY_inner, DARK_BOARD, HUD_HEIGHT + DARK_BOARD, false); 
    Draw3DLine(ctx, (float)width - FRAME_RIGHT_THICK - LIGHT_BOARD, hudY_inner, LIGHT_BOARD, HUD_HEIGHT + DARK_BOARD, true); 
    Draw3DLine(ctx, hudX_inner, hudY_inner + HUD_HEIGHT, hudW_inner, LIGHT_BOARD, true); 

    float boardX_line = (float)OFFSET_X - DARK_BOARD;
    float boardY_line = (float)OFFSET_Y - DARK_BOARD;
    float boardW_box = (float)boardW * CELL_SIZE + DARK_BOARD + LIGHT_BOARD;
    float boardH_box = (float)boardH * CELL_SIZE + DARK_BOARD + LIGHT_BOARD;


    Draw3DLine(ctx, boardX_line, boardY_line, boardW_box, DARK_BOARD, false);
    Draw3DLine(ctx, boardX_line, boardY_line, DARK_BOARD, boardH_box, false); 
    Draw3DLine(ctx, boardX_line + boardW_box - LIGHT_BOARD, boardY_line, LIGHT_BOARD, boardH_box, true); 
    Draw3DLine(ctx, boardX_line, boardY_line + boardH_box - LIGHT_BOARD, boardW_box, LIGHT_BOARD, true); 
}



void GameRenderer::DrawDigit(ID3D11DeviceContext* context, int value, int x, int y) {
    int imgIdx = 0;
    if (value == -1) {
        imgIdx = 0; 
    }
    else if (value >= 0 && value <= 9) {
        imgIdx = 11 - value; 
    }
    else {
        imgIdx = 1;
    }

    m_Sprites.Draw(context, m_TexLed.srv.Get(),
        (float)x, (float)y, (float)LED_W, (float)LED_H,
        0.0f, (float)(imgIdx * LED_H), (float)LED_W, (float)LED_H);
}

void GameRenderer::DrawNumber(ID3D11DeviceContext* context, int num, int x, int y) {
 
    num = (std::max)((std::min)(num, 999), -99);

    if (num < 0) {
       
        DrawDigit(context, -1, x, y);
        int absNum = -num;
        DrawDigit(context, (absNum / 10) % 10, x + LED_W, y);
        DrawDigit(context, absNum % 10, x + LED_W * 2, y);
    }
    else {

        DrawDigit(context, (num / 100) % 10, x, y);
        DrawDigit(context, (num / 10) % 10, x + LED_W, y);
        DrawDigit(context, num % 10, x + LED_W * 2, y);
    }
}



void GameRenderer::Render(ID3D11DeviceContext* context, HWND hWnd, MinesweeperLogic& logic, int width, int height,int boardW, int boardH) {

     
        
        POINT pt; GetCursorPos(&pt); ScreenToClient(hWnd, &pt);
       
        int mouseGridX = (pt.x - OFFSET_X) / CELL_SIZE;
        int mouseGridY = (pt.y - OFFSET_Y) / CELL_SIZE;
        
        bool isLDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        bool isMDown = (GetAsyncKeyState(VK_MBUTTON) & 0x8000);

        
         m_Sprites.Begin(context, width, height);
  

        
         DrawBaseFrame(context,width,height,boardW,boardH);

   
         float hudX_draw = LIGHT_BOARD + FRAME_LEFT_THICK;
         float hudY_draw = LIGHT_BOARD + DARK_BOARD + FRAME_MIDDLE_TOP_THICK;
         DrawNumber(context, logic.GetMinesLeft(), (int)(hudX_draw + 5), (int)(hudY_draw + 5));
         DrawNumber(context, logic.GetTime(), (int)(width - hudX_draw - LED_W * 3 - 5), (int)(hudY_draw + 5));

         // 2. Draw Face
         int faceIdx = 4;
         if (logic.GetStatus() == GameStatus::Won) faceIdx = 1;
         else if (logic.GetStatus() == GameStatus::Lost) faceIdx = 2;
         else if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
             // Determine if mouse is over the grid or the face
             POINT pt; GetCursorPos(&pt); ScreenToClient(hWnd, &pt);
             int faceX = width / 2 - 12;
             if (pt.x >= faceX && pt.x < faceX + 24 && pt.y >= 15 && pt.y < 15 + 24)
                 faceIdx = 0; // Pushed
             else
                 faceIdx = 3; // Surprise
         }
         m_Sprites.Draw(context, m_TexButton.srv.Get(), (float)(width / 2 - 12), 15.0f, 24.0f, 24.0f, 0.0f, (float)(faceIdx * 24), 24.0f, 24.0f);

         // 3. Draw Grid
         for (int y = 0; y < logic.GetHeight(); ++y) {
             for (int x = 0; x < logic.GetWidth(); ++x) {
        
                 bool isRevealed = logic.IsRevealed(x, y);
                 bool isMine = logic.IsMine(x, y);
                 bool isFlagged = logic.IsFlagged(x, y);
                 bool isExploded = logic.IsExploded(x, y);
                 int count = logic.GetNeighborCount(x, y);
                 // ---------------------------------------------
                 int blkIdx = BLK_UNTOUCHED;
                 bool shouldPush = false;
                 if (logic.GetStatus() == GameStatus::Playing) {
                 
                     if (isLDown && x == mouseGridX && y == mouseGridY) {
                         shouldPush = true;
                     }
                     
                     if (isMDown && abs(x - mouseGridX) <= 1 && abs(y - mouseGridY) <= 1) {
                         shouldPush = true;
                     }
                 }
                
                 if (isRevealed) {
                     if (isMine) {
                  
                         blkIdx = isExploded ? BLK_EXPLODED : BLK_MINE;
                     }
                     else {
                       
                         if (count == 0) blkIdx = BLK_REVEALED_EMPTY;
                         else blkIdx = BLK_1 - (count - 1);
                     }
                 }
                 else if (shouldPush && !isFlagged) {
                  
                     blkIdx = BLK_REVEALED_EMPTY;
                 }
                 else if (isFlagged) {
                     // 渲染旗子逻辑 (保持不变)
                     blkIdx = (logic.GetStatus() == GameStatus::Lost && !isMine) ? BLK_WRONG_FLAG : BLK_FLAG;
                 }
                 else if (logic.IsQuestioned(x, y)) {
                     // --- 新增：渲染问号 ---
                     // 根据是否按下左键显示不同的问号样式
                     blkIdx = shouldPush ? BLK_QUESTION_PUSHED : BLK_QUESTION;
                 }
                 else {
                     blkIdx = BLK_UNTOUCHED;
                 }
               
                 m_Sprites.Draw(context, m_TexBlocks.srv.Get(),
                     (float)(OFFSET_X + x * CELL_SIZE),
                     (float)(OFFSET_Y + y * CELL_SIZE),
                     (float)CELL_SIZE, (float)CELL_SIZE,
                     0.0f, (float)(blkIdx * CELL_SIZE),
                     (float)CELL_SIZE, (float)CELL_SIZE);
             }
         }

          // 4. Draw Selection Indicator (Refactored: Prominent border with customizable color)
          if (logic.GetStatus() == GameStatus::Playing && g_Config.showSelBox) {
              int selX = logic.GetSelX();
              int selY = logic.GetSelY();
              float drawX = (float)(OFFSET_X + selX * CELL_SIZE);
              float drawY = (float)(OFFSET_Y + selY * CELL_SIZE);

              // Use ImGui background draw list to support customizable colors easily
              ImDrawList* drawList = ImGui::GetBackgroundDrawList();
              ImU32 col = IM_COL32(
                  (int)(g_Config.selColor[0] * 255),
                  (int)(g_Config.selColor[1] * 255),
                  (int)(g_Config.selColor[2] * 255),
                  255
              );
              
              drawList->AddRect(
                  ImVec2(drawX, drawY),
                  ImVec2(drawX + CELL_SIZE, drawY + CELL_SIZE),
                  col,
                  0.0f,
                  0,
                  2.0f // 2px thickness
              );
          }

          m_Sprites.End();
}