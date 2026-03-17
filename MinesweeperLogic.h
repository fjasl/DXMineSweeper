#pragma once
#include <vector>
#include <random>

enum class GameStatus {
    Playing,
    Won,
    Lost
};

struct Cell {
    bool isMine = false;
    bool isRevealed = false;
    bool isFlagged = false;
    bool isQuestioned = false;
    int neighborMines = 0;
    bool isExploded = false; // The mine that caused the loss
    bool isWrongFlag = false; // Incorrectly flagged mine
};

class MinesweeperLogic {
public:
    MinesweeperLogic();
    void SetLevel(int width, int height, int mines);
    void StartNewGame();

    // Interaction
    void RevealCell(int x, int y);
    void ToggleFlag(int x, int y);
    void TryChord(int x, int y);

    // Getters
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetMinesLeft() const { return m_mines - m_flagsPlaced; }
    const Cell& GetCell(int x, int y) const { return m_grid[y * m_width + x]; }
    GameStatus GetStatus() const { return m_status; }
    int GetTime() const { return m_seconds; }

    void UpdateTimer();

private:
    void PlaceMines(int firstX, int firstY);
    void FloodFill(int x, int y);
    void CheckWin();
    bool IsInBounds(int x, int y) const;
    int CountNeighborMines(int x, int y) const;
    int CountNeighborFlags(int x, int y) const;

    int m_width, m_height, m_mines;
    std::vector<Cell> m_grid;
    GameStatus m_status;
    bool m_firstClick;
    int m_flagsPlaced;
    int m_cellsRevealed;
    int m_seconds;
};
