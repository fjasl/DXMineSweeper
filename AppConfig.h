#pragma once

// 游戏配置与存档结构
struct GameConfig {
    // 1. 上次游戏的规模
    int lastWidth = 9;
    int lastHeight = 9;
    int lastMines = 10;

    // 2. 英雄榜（0:初级, 1:中级, 2:高级）
    int bestTime[3] = { 999, 999, 999 };
    char bestName[3][32] = { "Anonymous", "Anonymous", "Anonymous" };

    // 3. 游戏偏好
    bool useMarks = true;

    // (可选) 如果想保存进度，可以在这里增加棋盘数组
    // unsigned char savedBoard[2500];
};

// 全局配置实例声明
extern GameConfig g_Config;

// 加载与保存接口
bool LoadAppConfig();
bool SaveAppConfig();

