#include "AppConfig.h"
#include <fstream>

// 全局变量定义
GameConfig g_Config;
const char* CONFIG_FILE = "config.bin";

bool LoadAppConfig() {
    std::ifstream file(CONFIG_FILE, std::ios::binary);
    if (!file) return false;
    file.read(reinterpret_cast<char*>(&g_Config), sizeof(GameConfig));
    std::streamsize bytesRead = file.gcount();

    if (bytesRead < 50) { // Extremely corrupted or ancient config
        g_Config = GameConfig();
        return true;
    }

    // Safety checks for critical values in case struct padding shifted them
    if (g_Config.uiScale < 0.5f || g_Config.uiScale > 3.0f) {
        g_Config.uiScale = 1.0f;
    }
    if (g_Config.lastWidth <= 0 || g_Config.lastHeight <= 0) {
        g_Config.lastWidth = 9;
        g_Config.lastHeight = 9;
    }

    return true;
}

bool SaveAppConfig() {
    std::ofstream file(CONFIG_FILE, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(&g_Config), sizeof(GameConfig));
    return true;
}
