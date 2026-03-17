#include "GameRender.h"
#include "TextureLoader.h"

bool GameRenderer::Initialize(ID3D11Device* device) {
    if (!m_Sprites.Initialize(device)) {
        return false;
    }


    m_TexBlocks = TextureLoader::LoadTexture(device, L"assets/blocks.bmp");
    m_TexLed = TextureLoader::LoadTexture(device, L"assets/led.bmp");
    m_TexButton = TextureLoader::LoadTexture(device, L"assets/button.bmp");


}


// 借用贴图的边缘像素来画 3D 线条
void GameRenderer ::Draw3DLine(ID3D11DeviceContext* ctx, float x, float y, float w, float h, bool light) {
    // blocks 第一个方块 (0,0,16,16)
    // 左上边缘是亮的 (srcX=0, srcY=0)
    // 右下边缘是暗的 (srcX=15, srcY=15)
    float srcX = light ? 0.0f : 15.0f;
    float srcY = light ? 0.0f : 15.0f;

     m_Sprites.Draw(ctx, m_TexBlocks.srv.Get(),
        x, y, w, h,           // 目标位置和拉伸后的尺寸
        srcX, srcY, 1.0f, 1.0f // 只切 1 像素，它会被拉伸成线条
    );
}

void GameRenderer ::DrawBaseFrame(ID3D11DeviceContext* ctx, int width, int height, int boardW, int boardH) {
    // 1 & 2. 最外圈凸起边框 (左, 顶) - 宽度由 LIGHT_BOARD 控制
    Draw3DLine(ctx, 0, 0, (float)width, LIGHT_BOARD, true); // 顶
    Draw3DLine(ctx, 0, 0, LIGHT_BOARD, (float)height, true); // 左

    // --- HUD 区域绘制 ---
    // 根据 OFFSET_Y 公式堆叠：顶(LIGHT) + 下一层(DARK)
    float hudX_inner = LIGHT_BOARD + FRAME_LEFT_THICK;
    float hudY_inner = LIGHT_BOARD + FRAME_MIDDLE_TOP_THICK;
    float hudW_inner = (float)width - hudX_inner - FRAME_RIGHT_THICK;

    // 3, 4, 5, 6. HUD 框 (4条线) - 线宽由 LIGHT_BOARD / DARK_BOARD 控制
    Draw3DLine(ctx, hudX_inner, hudY_inner, hudW_inner, DARK_BOARD, false); // HUD 顶 (暗)
    Draw3DLine(ctx, hudX_inner, hudY_inner, DARK_BOARD, HUD_HEIGHT + DARK_BOARD, false); // HUD 左 (暗)
    Draw3DLine(ctx, (float)width - FRAME_RIGHT_THICK - LIGHT_BOARD, hudY_inner, LIGHT_BOARD, HUD_HEIGHT + DARK_BOARD, true); // HUD 右 (亮)
    Draw3DLine(ctx, hudX_inner, hudY_inner + HUD_HEIGHT, hudW_inner, LIGHT_BOARD, true); // HUD 底 (亮)
    // --- 棋盘区域绘制 ---
    // 棋盘内容起始坐标由 OFFSET_X / OFFSET_Y 决定
    float boardX_line = (float)OFFSET_X - DARK_BOARD;
    float boardY_line = (float)OFFSET_Y - DARK_BOARD;
    float boardW_box = (float)boardW * CELL_SIZE + DARK_BOARD + LIGHT_BOARD;
    float boardH_box = (float)boardH * CELL_SIZE + DARK_BOARD + LIGHT_BOARD;

    // 7, 8, 9, 10. 棋盘框 (4条线)
    Draw3DLine(ctx, boardX_line, boardY_line, boardW_box, DARK_BOARD, false); // 棋盘顶 (暗)
    Draw3DLine(ctx, boardX_line, boardY_line, DARK_BOARD, boardH_box, false); // 棋盘左 (暗)
    Draw3DLine(ctx, boardX_line + boardW_box - LIGHT_BOARD, boardY_line, LIGHT_BOARD, boardH_box, true); // 棋盘右 (亮)
    Draw3DLine(ctx, boardX_line, boardY_line + boardH_box - LIGHT_BOARD, boardW_box, LIGHT_BOARD, true); // 棋盘底 (亮)
}



void GameRenderer::DrawDigit(ID3D11DeviceContext* context, int value, int x, int y) {
    int imgIdx = 0;
    if (value == -1) {
        imgIdx = 0; // 负号
    }
    else if (value >= 0 && value <= 9) {
        imgIdx = 11 - value; // 倒序映射：0->11, 1->10... 9->2
    }
    else {
        imgIdx = 1; // 默认空
    }

    m_Sprites.Draw(context, m_TexLed.srv.Get(),
        (float)x, (float)y, (float)LED_W, (float)LED_H,
        0.0f, (float)(imgIdx * LED_H), (float)LED_W, (float)LED_H);
}

void GameRenderer::DrawNumber(ID3D11DeviceContext* context, int num, int x, int y) {
    // 限制范围
    num = (std::max)((std::min)(num, 999), -99);

    if (num < 0) {
        // 第一位画负号
        DrawDigit(context, -1, x, y);
        int absNum = -num;
        DrawDigit(context, (absNum / 10) % 10, x + LED_W, y);
        DrawDigit(context, absNum % 10, x + LED_W * 2, y);
    }
    else {
        // 正常的三位数逻辑
        DrawDigit(context, (num / 100) % 10, x, y);
        DrawDigit(context, (num / 10) % 10, x + LED_W, y);
        DrawDigit(context, num % 10, x + LED_W * 2, y);
    }
}



void GameRenderer::Render(ID3D11DeviceContext* context, HWND hWnd, MinesweeperLogic& logic, int width, int height,int boardW, int boardH) {

     
        // 在 Render 函数内部
        POINT pt; GetCursorPos(&pt); ScreenToClient(hWnd, &pt);
        // 计算鼠标当前在哪个格子坐标上
        int mouseGridX = (pt.x - OFFSET_X) / CELL_SIZE;
        int mouseGridY = (pt.y - OFFSET_Y) / CELL_SIZE;
        // 判断左键和中键是否按下
        bool isLDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        bool isMDown = (GetAsyncKeyState(VK_MBUTTON) & 0x8000);

        
         m_Sprites.Begin(context, width, height);
         //Draw BACKGROUND

         // 1. 绘制基础 3D 框架 (外框、HUD 框、棋盘框)
         DrawBaseFrame(context,width,height,boardW,boardH);

         // 2. Draw HUD 内容 (稍微偏离边缘一点)
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
                 const Cell& cell = logic.GetCell(x, y);
                 int blkIdx = BLK_UNTOUCHED;

                 bool shouldPush = false;
                 if (logic.GetStatus() == GameStatus::Playing) {
                     // 左键逻辑：当前鼠标指向的这个格子下陷
                     if (isLDown && x == mouseGridX && y == mouseGridY) {
                         shouldPush = true;
                     }
                     // 中键逻辑：鼠标周围 3x3 的区域全部下陷
                     if (isMDown && abs(x - mouseGridX) <= 1 && abs(y - mouseGridY) <= 1) {
                         shouldPush = true;
                     }
                 }


                 // 2. 状态判断优先级树
                 if (cell.isRevealed) {
                     // A. 已经挖开的情况 (不受鼠标按住影响)
                     if (cell.isMine) {
                         // 踩中的雷是红色背景 (BLK_EXPLODED)，其他露出来的雷是灰色背景 (BLK_MINE)
                         blkIdx = cell.isExploded ? BLK_EXPLODED : BLK_MINE;
                     }
                     else {
                         // 数字逻辑：0 为全平空白块，1-8 为带颜色数字
                         if (cell.neighborMines == 0) {
                             blkIdx = BLK_REVEALED_EMPTY;
                         }
                         else {
                             // 对应你那张倒序贴图的计算：BLK_1 是 14，减去偏移得到对应数字
                             blkIdx = BLK_1 - (cell.neighborMines - 1);
                         }
                     }
                 }
                 else if (shouldPush && !cell.isFlagged) {
                     // B. 尚未挖掘，但鼠标正在按着它（预览效果）
                     // 注意：插了旗的格子不能被按下
                     if (cell.isQuestioned) {
                         blkIdx = BLK_QUESTION_PUSHED; // 如果有问号，显示按下态问号 (索引 6)
                     }
                     else {
                         blkIdx = BLK_REVEALED_EMPTY;  // 否则显示凹陷的空白块 (索引 15)
                     }
                 }
                 else if (cell.isFlagged) {
                     // C. 插了旗的情况
                     // 如果游戏结束且旗子插错了位置，显示带 X 的雷 (BLK_WRONG_FLAG)
                     if (logic.GetStatus() == GameStatus::Lost && !cell.isMine) {
                         blkIdx = BLK_WRONG_FLAG;
                     }
                     else {
                         blkIdx = BLK_FLAG;
                     }
                 }
                 else if (cell.isQuestioned) {
                     // D. 问号状态
                     blkIdx = BLK_QUESTION;
                 }
                 else {
                     // E. 默认状态：凸起的未挖掘格子
                     blkIdx = BLK_UNTOUCHED;
                 }
                 // 3. 执行绘制
                 m_Sprites.Draw(context, m_TexBlocks.srv.Get(),
                     (float)(OFFSET_X + x * CELL_SIZE),
                     (float)(OFFSET_Y + y * CELL_SIZE),
                     (float)CELL_SIZE, (float)CELL_SIZE,
                     0.0f, (float)(blkIdx * CELL_SIZE),
                     (float)CELL_SIZE, (float)CELL_SIZE);
             }
         }
         m_Sprites.End();
}