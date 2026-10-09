#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <functional>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // Initialize all global variables
  
  // Initialize game_map if it exists
  if (game_map != nullptr) {
    for (int i = 0; i < rows; i++) {
      delete[] game_map[i];
    }
    delete[] game_map;
    game_map = nullptr;
  }
  
  // Read the first move and execute it
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
struct Cell {
  char state;  // '?', '0'-'8', 'X', '@'
  bool is_visited;
  bool is_marked;
  int adjacent_mines;  // -1 for unknown, 0-8 for known
};

Cell** game_map;  // 2D array to store the current game state

void ReadMap() {
  // Allocate memory for game_map if not already done
  if (game_map == nullptr) {
    game_map = new Cell*[rows];
    for (int i = 0; i < rows; i++) {
      game_map[i] = new Cell[columns];
      for (int j = 0; j < columns; j++) {
        game_map[i][j].state = '?';
        game_map[i][j].is_visited = false;
        game_map[i][j].is_marked = false;
        game_map[i][j].adjacent_mines = -1;
      }
    }
  }
  
  // Read the current game state
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      char c = line[j];
      game_map[i][j].state = c;
      
      // Update cell properties based on state
      if (c == '?') {
        game_map[i][j].is_visited = false;
        game_map[i][j].is_marked = false;
        game_map[i][j].adjacent_mines = -1;
      } else if (c == 'X') {
        // This could be a marked non-mine (failure) or just part of the output
        game_map[i][j].is_visited = true;  // Since we can see it, it must be visited
        game_map[i][j].is_marked = true;
        game_map[i][j].adjacent_mines = -1;
      } else if (c == '@') {
        game_map[i][j].is_visited = true;
        game_map[i][j].is_marked = true;
        game_map[i][j].adjacent_mines = -1;
      } else if (c >= '0' && c <= '8') {
        game_map[i][j].is_visited = true;
        game_map[i][j].is_marked = false;
        game_map[i][j].adjacent_mines = c - '0';
      }
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
// Helper function to count neighboring cells of a certain type
int CountNeighbors(int r, int c, std::function<bool(const Cell&)> predicate) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int nr = r + i;
      int nc = c + j;
      if (i == 0 && j == 0) continue;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (predicate(game_map[nr][nc])) {
          count++;
        }
      }
    }
  }
  return count;
}

// Helper function to get neighboring cells of a certain type
std::vector<std::pair<int, int>> GetNeighbors(int r, int c, std::function<bool(const Cell&)> predicate) {
  std::vector<std::pair<int, int>> neighbors;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int nr = r + i;
      int nc = c + j;
      if (i == 0 && j == 0) continue;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (predicate(game_map[nr][nc])) {
          neighbors.push_back({nr, nc});
        }
      }
    }
  }
  return neighbors;
}

// Check for basic mine deduction: if a numbered cell has exactly N unknown neighbors
// and its number is N, then all unknown neighbors must be mines
bool FindBasicMineDeduction(int& row, int& col) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j].adjacent_mines > 0) {
        int unknown_count = CountNeighbors(i, j, [](const Cell& cell) {
          return cell.state == '?';
        });
        
        if (unknown_count == game_map[i][j].adjacent_mines) {
          // Found a cell where all unknown neighbors must be mines
          // Return the first unknown neighbor
          auto neighbors = GetNeighbors(i, j, [](const Cell& cell) {
            return cell.state == '?';
          });
          if (!neighbors.empty()) {
            row = neighbors[0].first;
            col = neighbors[0].second;
            return true;
          }
        }
      }
    }
  }
  return false;
}

// Check for auto-explore opportunity: if a numbered cell has all mines marked,
// then we can auto-explore to reveal all safe neighbors
bool FindAutoExploreOpportunity(int& row, int& col) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j].adjacent_mines > 0) {
        int marked_count = CountNeighbors(i, j, [](const Cell& cell) {
          return cell.is_marked;
        });
        
        if (marked_count == game_map[i][j].adjacent_mines) {
          // All mines are marked, so we can auto-explore
          row = i;
          col = j;
          return true;
        }
      }
    }
  }
  return false;
}

// Find an unvisited safe cell to visit
bool FindSafeCellToVisit(int& row, int& col) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j].state == '?' && !game_map[i][j].is_marked) {
        // Check if this cell is adjacent to any numbered cell
        bool is_adjacent_to_number = false;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            int ni = i + di;
            int nj = j + dj;
            if (di == 0 && dj == 0) continue;
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (game_map[ni][nj].adjacent_mines >= 0) {
                is_adjacent_to_number = true;
                break;
              }
            }
          }
          if (is_adjacent_to_number) break;
        }
        
        // Prefer cells adjacent to numbers (more information)
        if (is_adjacent_to_number) {
          row = i;
          col = j;
          return true;
        }
      }
    }
  }
  
  // If no cell adjacent to numbers, pick any unvisited cell
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j].state == '?' && !game_map[i][j].is_marked) {
        row = i;
        col = j;
        return true;
      }
    }
  }
  
  return false;
}

void Decide() {
  int row, col;
  
  // First, check if we can mark a mine using basic deduction
  if (FindBasicMineDeduction(row, col)) {
    Execute(row, col, 1);  // Mark as mine
    return;
  }
  
  // Next, check if we can auto-explore
  if (FindAutoExploreOpportunity(row, col)) {
    Execute(row, col, 2);  // Auto-explore
    return;
  }
  
  // Finally, find a safe cell to visit
  if (FindSafeCellToVisit(row, col)) {
    Execute(row, col, 0);  // Visit block
    return;
  }
  
  // If no move found (should not happen in normal gameplay)
  // This would only happen if the game is already over
  // But to be safe, try the top-left corner
  Execute(0, 0, 0);
}

#endif