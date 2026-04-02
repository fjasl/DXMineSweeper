#include "MinesweeperLogic.h"
#include "AppConfig.h"
#include <algorithm>
#include <ctime>
#include <queue>
#include <cstring>



MinesweeperLogic::MinesweeperLogic()
    : m_width(0), m_height(0), m_mines(0), m_status(GameStatus::Playing),
    m_flagsPlaced(0), m_cellsRevealed(0), m_seconds(0) {
    memset(m_board, 0, sizeof(m_board));
}

void MinesweeperLogic::SetLevel(int width, int height, int mines) {
    m_width = width;
    m_height = height;
    m_mines = mines;
    StartNewGame();
}

void MinesweeperLogic::StartNewGame() {
    memset(m_board, 0, sizeof(m_board)); // 清空内存池
    m_status = GameStatus::Playing;
    m_flagsPlaced = 0;
    m_cellsRevealed = 0;
    m_seconds = 0;
    m_selX = 0;
    m_selY = 0;

    PlaceMines(); // 立即布雷，不再等待第一次点击
}

void MinesweeperLogic::SaveStateToConfig() {
    g_Config.hasSavedGame = true;
    g_Config.savedSeconds = m_seconds;
    g_Config.savedFlagsPlaced = m_flagsPlaced;
    g_Config.savedCellsRevealed = m_cellsRevealed;
    g_Config.savedStatus = (int)m_status;
    memcpy(g_Config.savedBoard, m_board, sizeof(m_board));
}

void MinesweeperLogic::LoadStateFromConfig() {
    if (!g_Config.hasSavedGame) return;
    
    m_seconds = g_Config.savedSeconds;
    m_flagsPlaced = g_Config.savedFlagsPlaced;
    m_cellsRevealed = g_Config.savedCellsRevealed;
    m_status = static_cast<GameStatus>(g_Config.savedStatus);
    memcpy(m_board, g_Config.savedBoard, sizeof(m_board));
}


void MinesweeperLogic::PlaceMines() {
    int placed = 0;
    while (placed < m_mines) {
        int r = rand() % (m_width * m_height);
        if (!(m_board[r] & STATE_MINE)) {
            m_board[r] |= STATE_MINE;
            placed++;
        }
    }

    // 计算数字
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (!(m_board[y * m_width + x] & STATE_MINE)) {
                int count = CountNeighborMines(x, y);
                m_board[y * m_width + x] |= (count & MASK_COUNT);
            }
        }
    }
}

void MinesweeperLogic::RevealCell(int x, int y) {
    if (!IsInBounds(x, y) || m_status != GameStatus::Playing) return;

    unsigned short& cell = m_board[y * m_width + x];
    if ((cell & STATE_OPEN) || (cell & STATE_FLAG)) return;

    cell |= STATE_OPEN;
    m_cellsRevealed++;

    if (cell & STATE_MINE) {
        cell |= STATE_ERROR; // 标记爆炸点
        m_status = GameStatus::Lost;
        // 游戏结束：展示所有地雷
        for (int i = 0; i < m_width * m_height; ++i) {
            if (m_board[i] & STATE_MINE) m_board[i] |= STATE_OPEN;
            // 如果标错了旗，也标记为错误
            if ((m_board[i] & STATE_FLAG) && !(m_board[i] & STATE_MINE)) m_board[i] |= STATE_ERROR;
        }
        return;
    }

    if ((cell & MASK_COUNT) == 0) {
        FloodFill(x, y);
    }
    CheckWin();
}


void MinesweeperLogic::FloodFill(int startX, int startY) {
    std::queue<std::pair<int, int>> q;
    q.push({startX, startY});

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (IsInBounds(nx, ny)) {
                    unsigned short& neighbor = m_board[ny * m_width + nx];
                    if (!(neighbor & STATE_OPEN) && !(neighbor & STATE_FLAG)) {
                        neighbor |= STATE_OPEN;
                        m_cellsRevealed++;
                        if ((neighbor & MASK_COUNT) == 0 && !(neighbor & STATE_MINE)) {
                            q.push({nx, ny});
                        }
                    }
                }
            }
        }
    }
}

extern bool g_bMarks; // 引用外部的标记开关
void MinesweeperLogic::ToggleFlag(int x, int y) {
    if (!IsInBounds(x, y) || m_status != GameStatus::Playing) return;
    unsigned short& cell = m_board[y * m_width + x];
    if (cell & STATE_OPEN) return; // 已经打开的不能标记
    // --- 三态循环逻辑 ---
    if (cell & STATE_FLAG) {
        // 当前是旗子 -> 变为问号 (如果开启了标记功能) 或 变为恢复为空
        cell &= ~STATE_FLAG;
        m_flagsPlaced--;
        if (g_bMarks) {
            cell |= STATE_QUESTION;
        }
    }
    else if (cell & STATE_QUESTION) {
        // 当前是问号 -> 恢复为空
        cell &= ~STATE_QUESTION;
    }
    else {
        // 当前为空 -> 变为旗子
        cell |= STATE_FLAG;
        m_flagsPlaced++;
    }
}

void MinesweeperLogic::TryChord(int x, int y) {
    if (!IsInBounds(x, y) || !(m_board[y * m_width + x] & STATE_OPEN)) return;
    int mines = m_board[y * m_width + x] & MASK_COUNT;
    if (mines > 0 && mines == CountNeighborFlags(x, y)) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                RevealCell(x + dx, y + dy);
            }
        }
    }
}

void MinesweeperLogic::CheckWin() {
    // 如果“总格子数 - 雷数 == 已打开格子数”，则获胜
    if (m_width * m_height - m_mines == m_cellsRevealed) {
        m_status = GameStatus::Won;
        // 获胜后，所有的雷都自动标记为旗子 (经典做法)
        for (int i = 0; i < m_width * m_height; ++i) {
            if (m_board[i] & STATE_MINE) m_board[i] |= STATE_FLAG;
        }
    }
}


bool MinesweeperLogic::IsInBounds(int x, int y) const {
    return x >= 0 && x < m_width && y >= 0 && y < m_height;
}

int MinesweeperLogic::CountNeighborMines(int x, int y) const {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (IsInBounds(nx, ny) && (m_board[ny * m_width + nx] & STATE_MINE)) {
                count++;
            }
        }
    }
    return count;
}

int MinesweeperLogic::CountNeighborFlags(int x, int y) const {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            if (IsInBounds(x + dx, y + dy) && (m_board[(y + dy) * m_width + (x + dx)] & STATE_FLAG)) {
                count++;
            }
        }
    }
    return count;
}

void MinesweeperLogic::UpdateTimer() {
    if (m_status == GameStatus::Playing && m_cellsRevealed > 0) {
        m_seconds++;
        if (m_seconds > 999) m_seconds = 999;
    }
}

void MinesweeperLogic::MoveSelection(int dx, int dy) {
    m_selX += dx;
    m_selY += dy;

    if (m_selX < 0) m_selX = 0;
    if (m_selX >= m_width) m_selX = m_width - 1;
    if (m_selY < 0) m_selY = 0;
    if (m_selY >= m_height) m_selY = m_height - 1;
}
