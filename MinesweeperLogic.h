#pragma once
#include <vector>

enum class GameStatus { Playing, Won, Lost };

// --- 经典位布局常量 ---
#define STATE_MINE      0x0010  // 地雷
#define STATE_OPEN      0x0020  // 已打开
#define STATE_FLAG      0x0040  // 已插旗
#define STATE_ERROR     0x0080  // 爆炸或标注错误
#define STATE_QUESTION  0x0100  // 增加问号状态位 (0x0100)
#define MASK_COUNT      0x000F  // 低4位存储周围雷数 (0-8)

class MinesweeperLogic {
    friend class DebugUI;
public:
    MinesweeperLogic();
    void SetLevel(int width, int height, int mines);
    void StartNewGame();
    void SaveStateToConfig();
    void LoadStateFromConfig();

    void RevealCell(int x, int y);
    void ToggleFlag(int x, int y);
    void TryChord(int x, int y);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetMinesLeft() const { return m_mines - m_flagsPlaced; }
    GameStatus GetStatus() const { return m_status; }
    int GetTime() const { return m_seconds; }
    int GetTotalMines() const { return m_mines; }
    int GetFlags() const { return m_flagsPlaced; }

    void UpdateTimer();

    // 适配渲染器的辅助函数
    bool IsMine(int x, int y) const { return m_board[y * m_width + x] & STATE_MINE; }
    bool IsRevealed(int x, int y) const { return m_board[y * m_width + x] & STATE_OPEN; }
    bool IsFlagged(int x, int y) const { return m_board[y * m_width + x] & STATE_FLAG; }
    bool IsExploded(int x, int y) const { return m_board[y * m_width + x] & STATE_ERROR; }
    bool IsQuestioned(int x, int y) const { return m_board[y * m_width + x] & STATE_QUESTION; }
    int GetNeighborCount(int x, int y) const { return m_board[y * m_width + x] & MASK_COUNT; }
    
    // 键盘选中框位置
    int GetSelX() const { return m_selX; }
    int GetSelY() const { return m_selY; }
    void MoveSelection(int dx, int dy);
    void SetSelection(int x, int y) { m_selX = x; m_selY = y; }

private:
    void PlaceMines(); // 现在不需要参数，开局即生成
    void FloodFill(int x, int y);
    void CheckWin();
    bool IsInBounds(int x, int y) const;
    int CountNeighborMines(int x, int y) const;
    int CountNeighborFlags(int x, int y) const; 

    int m_width, m_height, m_mines;
    unsigned short m_board[2500]; // 静态内存池 (最大支持 50x50)

    GameStatus m_status;
    int m_flagsPlaced;
    int m_cellsRevealed;
    int m_seconds;

    int m_selX = 0;
    int m_selY = 0;
};
