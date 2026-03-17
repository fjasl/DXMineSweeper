#pragma once

// 基础尺寸常量
const int CELL_SIZE = 16;
const int LED_W = 13;
const int LED_H = 23;
const int FACE_SIZE = 24;

// 布局边框厚度常量
const float LIGHT_BOARD = 2.0f;
const float DARK_BOARD = 2.0f;
const float FRAME_MIDDLE_TOP_THICK = 6.0f;
const float FRAME_MIDDLE_CENTER_THICK = 2.0f;
const float FRAME_MIDDLE_BOTTOM_THICK = 4.0f;
const float FRAME_LEFT_THICK = 6.0f;
const float FRAME_RIGHT_THICK = 6.0f;
const float HUD_HEIGHT = 34.0f;

// 棋盘偏移量计算逻辑 (基于上述常量)
const int OFFSET_X = (int)(LIGHT_BOARD + DARK_BOARD + FRAME_LEFT_THICK);
const int OFFSET_Y = (int)(LIGHT_BOARD + DARK_BOARD + FRAME_MIDDLE_TOP_THICK + HUD_HEIGHT + FRAME_MIDDLE_BOTTOM_THICK + FRAME_MIDDLE_CENTER_THICK + FRAME_MIDDLE_TOP_THICK);


// Mapping based on the verified 16-tile vertical strip (blocks.bmp)
enum BlockIdx {
    BLK_UNTOUCHED = 0,       // 凸起的空白格子
    BLK_FLAG = 1,            // 插旗
    BLK_QUESTION = 2,        // 问号
    BLK_EXPLODED = 3,        // 踩到的炸弹 (红色背景)
    BLK_WRONG_FLAG = 4,      // 错误的旗子 (插错地雷)
    BLK_MINE = 5,            // 炸弹 (游戏结束时的展示)
    BLK_QUESTION_PUSHED = 6, // 按下的问号
    BLK_8 = 7,               // 数字 8
    BLK_7 = 8,               // 数字 7
    BLK_6 = 9,               // 数字 6
    BLK_5 = 10,              // 数字 5
    BLK_4 = 11,              // 数字 4
    BLK_3 = 12,              // 数字 3
    BLK_2 = 13,              // 数字 2
    BLK_1 = 14,              // 数字 1
    BLK_REVEALED_EMPTY = 15  // 挖开后的空白 (凹下的灰色)
};
