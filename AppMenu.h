#pragma once
#include <windows.h>

// --- 菜单与控件 ID ---
#define IDM_GAME_NEW          1001 // 开局 (F2)
#define IDM_GAME_BEGINNER     1002
#define IDM_GAME_INTERMEDIATE 1003
#define IDM_GAME_EXPERT       1004
#define IDM_GAME_CUSTOM       1005
#define IDM_GAME_MARKS        1006 // 标记 (?)
#define IDM_GAME_COLOR        1007 // 颜色
#define IDM_GAME_SOUND        1008 // 声音
#define IDM_GAME_BEST_TIMES   1009 // 扫雷英雄榜
#define IDM_GAME_EXIT         1010 // 退出

#define IDC_EDIT_HEIGHT 2001
#define IDC_EDIT_WIDTH  2002
#define IDC_EDIT_MINES  2003

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

// --- 数据结构 ---
struct CustomLevelData {
    int width, height, mines;
};

struct ScoreRecord {
    int time;
    wchar_t name[32];
};

// --- 全局 UI 状态 (由 AppUI.cpp 定义) ---
extern ScoreRecord g_HighScores[3];
extern bool g_bMarks, g_bColor, g_bSound;

// --- 接口函数声明 ---
HMENU CreateAppMenu();
void UpdateMenuCheck(HWND hWnd);
void CheckHighScore(HWND hWnd);
int  GetCurrentLevelIndex();

// 对话框过程 (供 DialogBox 调用)
INT_PTR CALLBACK CustomDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BestTimesProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK RecordScoreProc(HWND, UINT, WPARAM, LPARAM);

// 模板生成函数
void* CreateCustomDialogTemplate();
void* CreateBestTimesTemplate();
void* CreateRecordScoreTemplate(int levelIdx);
