#include "AppConfig.h"
#include <fstream>

// 全局变量定义
GameConfig g_Config;
const char* CONFIG_FILE = "config.bin";

bool LoadAppConfig() {
    std::ifstream file(CONFIG_FILE, std::ios::binary);
    if (!file) return false;
    file.read(reinterpret_cast<char*>(&g_Config), sizeof(GameConfig));
    return true;
}

bool SaveAppConfig() {
    std::ofstream file(CONFIG_FILE, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(&g_Config), sizeof(GameConfig));
    return true;
}
