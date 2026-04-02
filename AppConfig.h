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
    float uiScale = 1.0f;
    float windowScale = 1.0f;
    bool enableKeyboard = true;

    // 4. 按键设置
    int keyUp = 'K';
    int keyDown = 'J';
    int keyLeft = 'H';
    int keyRight = 'L';
    int keyReveal = 'U';
    int keyFlag = 'I';
    float selColor[3] = { 1.0f, 0.0f, 0.0f };
    bool showSelBox = true;
    bool followMouse = true;

    // 5. 进行中的游戏状态保存
    bool autoSaveProgress = false;
    bool hasSavedGame = false;
    int savedSeconds = 0;
    int savedFlagsPlaced = 0;
    int savedCellsRevealed = 0;
    int savedStatus = 0;
    unsigned short savedBoard[2500];
};

// 全局配置实例声明
extern GameConfig g_Config;

// 加载与保存接口
bool LoadAppConfig();
bool SaveAppConfig();

