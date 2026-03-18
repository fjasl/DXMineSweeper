#pragma once
#include <windows.h>


#define IDM_GAME_NEW          1001 
#define IDM_GAME_BEGINNER     1002
#define IDM_GAME_INTERMEDIATE 1003
#define IDM_GAME_EXPERT       1004
#define IDM_GAME_CUSTOM       1005
#define IDM_GAME_MARKS        1006 
#define IDM_GAME_COLOR        1007 
#define IDM_GAME_SOUND        1008 
#define IDM_GAME_BEST_TIMES   1009 
#define IDM_GAME_EXIT         1010 

#define IDC_EDIT_HEIGHT 2001
#define IDC_EDIT_WIDTH  2002
#define IDC_EDIT_MINES  2003

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

// --- 数据结构 ---
struct CustomLevelData {
    int width;
    int height;
    int mines;
};

struct ScoreRecord {
    int time;
    wchar_t name[32];
};

// --- 全局状态声明 (由 AppUI.cpp 定义) ---
extern ScoreRecord g_HighScores[3];
extern bool g_bMarks;
extern bool g_bColor;
extern bool g_bSound;

// --- 接口函数声明 ---

// 菜单管理
HMENU CreateAppMenu();
void UpdateMenuCheck(HWND hWnd);

// 英雄榜与破纪录
void CheckHighScore(HWND hWnd);
int GetCurrentLevelIndex();

// 对话框模板生成
void* CreateCustomDialogTemplate();
void* CreateBestTimesTemplate();
void* CreateRecordScoreTemplate(int levelIdx);

// 对话框过程
INT_PTR CALLBACK CustomDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK BestTimesProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK RecordScoreProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// 辅助功能
void ChangeLevel(HWND hWnd, int w, int h, int m); // 注意：ChangeLevel 逻辑依赖 g_Logic，通常保留在 main.cpp 或由 main 提供实现
