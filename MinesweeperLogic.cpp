#include "MinesweeperLogic.h"
#include <algorithm>
#include <ctime>

MinesweeperLogic::MinesweeperLogic() 
    : m_width(0), m_height(0), m_mines(0), m_status(GameStatus::Playing), 
      m_firstClick(true), m_flagsPlaced(0), m_cellsRevealed(0), m_seconds(0) {
}

void MinesweeperLogic::SetLevel(int width, int height, int mines) {
    m_width = width;
    m_height = height;
    m_mines = mines;
    StartNewGame();
}

void MinesweeperLogic::StartNewGame() {
    m_grid.assign(m_width * m_height, Cell());
    m_status = GameStatus::Playing;
    m_firstClick = true;
    m_flagsPlaced = 0;
    m_cellsRevealed = 0;
    m_seconds = 0;
}

void MinesweeperLogic::PlaceMines(int firstX, int firstY) {
    std::vector<int> positions;
    for (int i = 0; i < m_width * m_height; ++i) {
        int x = i % m_width;
        int y = i / m_width;
        // First click protection: don't place mine on the first clicked cell
        if (x == firstX && y == firstY) continue;
        positions.push_back(i);
    }

    std::shuffle(positions.begin(), positions.end(), std::default_random_engine((unsigned int)time(nullptr)));

    for (int i = 0; i < m_mines && i < (int)positions.size(); ++i) {
        m_grid[positions[i]].isMine = true;
    }

    // Calculate neighbor counts
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (!m_grid[y * m_width + x].isMine) {
                m_grid[y * m_width + x].neighborMines = CountNeighborMines(x, y);
            }
        }
    }
}

void MinesweeperLogic::RevealCell(int x, int y) {
    if (!IsInBounds(x, y) || m_status != GameStatus::Playing) return;
    
    Cell& cell = m_grid[y * m_width + x];
    if (cell.isRevealed || cell.isFlagged) return;

    if (m_firstClick) {
        PlaceMines(x, y);
        m_firstClick = false;
    }

    cell.isRevealed = true;
    m_cellsRevealed++;

    if (cell.isMine) {
        cell.isExploded = true;
        m_status = GameStatus::Lost;
        // Reveal all mines upon loss
        for (auto& c : m_grid) {
            if (c.isMine) c.isRevealed = true;
            if (c.isFlagged && !c.isMine) c.isWrongFlag = true;
        }
        return;
    }

    if (cell.neighborMines == 0) {
        FloodFill(x, y);
    }

    CheckWin();
}

void MinesweeperLogic::FloodFill(int x, int y) {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;
            if (IsInBounds(nx, ny)) {
                Cell& neighbor = m_grid[ny * m_width + nx];
                if (!neighbor.isRevealed && !neighbor.isFlagged) {
                    neighbor.isRevealed = true;
                    m_cellsRevealed++;
                    if (neighbor.neighborMines == 0 && !neighbor.isMine) {
                        FloodFill(nx, ny);
                    }
                }
            }
        }
    }
}

void MinesweeperLogic::ToggleFlag(int x, int y) {
    if (!IsInBounds(x, y) || m_status != GameStatus::Playing) return;

    Cell& cell = m_grid[y * m_width + x];
    if (cell.isRevealed) return;
    // 状态循环逻辑：
    if (!cell.isFlagged && !cell.isQuestioned) {
        // 1. 从“普通”变为“插旗”
        cell.isFlagged = true;
        m_flagsPlaced++; // 只有插旗才会计数
    }
    else if (cell.isFlagged) {
        // 2. 从“插旗”变为“问号”
        cell.isFlagged = false;
        cell.isQuestioned = true;
        m_flagsPlaced--; // 取消旗子，计数减一
    }
    else if (cell.isQuestioned) {
        // 3. 从“问号”变回“普通”
        cell.isQuestioned = false;
        // 此时 isFlagged 和 isQuestioned 都为 false
    }
}

void MinesweeperLogic::TryChord(int x, int y) {
    if (!IsInBounds(x, y) || m_status != GameStatus::Playing) return;
    Cell& cell = m_grid[y * m_width + x];
    if (!cell.isRevealed || cell.neighborMines == 0) return;

    if (CountNeighborFlags(x, y) == cell.neighborMines) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;
                RevealCell(x + dx, y + dy);
            }
        }
    }
}

void MinesweeperLogic::CheckWin() {
    if (m_cellsRevealed == (m_width * m_height - m_mines)) {
        m_status = GameStatus::Won;
        // Flag all remaining mines
        for (auto& c : m_grid) {
            if (c.isMine && !c.isFlagged) {
                c.isFlagged = true;
            }
        }
        m_flagsPlaced = m_mines;
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
            if (IsInBounds(nx, ny) && m_grid[ny * m_width + nx].isMine) {
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
            int nx = x + dx;
            int ny = y + dy;
            if (IsInBounds(nx, ny) && m_grid[ny * m_width + nx].isFlagged) {
                count++;
            }
        }
    }
    return count;
}

void MinesweeperLogic::UpdateTimer() {
    if (m_status == GameStatus::Playing && !m_firstClick) {
        if (m_seconds < 999) m_seconds++;
    }
}
