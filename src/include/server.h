#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Game state tracking
char** mine_map;        // 2D array to store actual mine layout ('X' for mine, '.' for non-mine)
bool** visited;         // 2D array to track which blocks have been visited
bool** marked;          // 2D array to track which blocks have been marked as mines
int visit_count;        // Number of visited non-mine grids
int marked_mine_count;  // Number of correctly marked mines

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  
  // Allocate memory for game state arrays
  mine_map = new char*[rows];
  visited = new bool*[rows];
  marked = new bool*[rows];
  
  for (int i = 0; i < rows; i++) {
    mine_map[i] = new char[columns];
    visited[i] = new bool[columns]();  // Initialize to false
    marked[i] = new bool[columns]();  // Initialize to false
  }
  
  // Read the mine layout
  total_mines = 0;
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      mine_map[i][j] = line[j];
      if (line[j] == 'X') {
        total_mines++;
      }
    }
  }
  
  // Initialize game state
  game_state = 0;
  visit_count = 0;
  marked_mine_count = 0;
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
// Helper function to count adjacent mines
int CountAdjacentMines(int r, int c) {
  int count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int nr = r + i;
      int nc = c + j;
      if (i == 0 && j == 0) continue;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (mine_map[nr][nc] == 'X') {
          count++;
        }
      }
    }
  }
  return count;
}

// Helper function to recursively visit blocks with zero adjacent mines
void VisitRecursive(int r, int c) {
  // Check bounds
  if (r < 0 || r >= rows || c < 0 || c >= columns) return;
  
  // If already visited or marked, do nothing
  if (visited[r][c] || marked[r][c]) return;
  
  // Mark as visited
  visited[r][c] = true;
  visit_count++;
  
  // Count adjacent mines
  int adjacent_mines = CountAdjacentMines(r, c);
  
  // If this block has mines adjacent, stop recursion here
  if (adjacent_mines > 0) return;
  
  // If zero adjacent mines, recursively visit all 8 surrounding blocks
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i == 0 && j == 0) continue;
      VisitRecursive(r + i, c + j);
    }
  }
}

void VisitBlock(int r, int c) {
  // If already visited or marked, do nothing
  if (visited[r][c] || marked[r][c]) {
    return;
  }
  
  // If it's a mine, game over
  if (mine_map[r][c] == 'X') {
    visited[r][c] = true;  // Show the mine that was hit
    game_state = -1;
    return;
  }
  
  // It's a non-mine block - visit it
  VisitRecursive(r, c);
  
  // Check for win condition
  if (visit_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  // If already visited or marked, do nothing
  if (visited[r][c] || marked[r][c]) {
    return;
  }
  
  // Mark the block
  marked[r][c] = true;
  
  // If it's a mine, increment marked_mine_count
  if (mine_map[r][c] == 'X') {
    marked_mine_count++;
    
    // Check for win condition - all non-mine grids visited OR all mines marked
    // According to requirements: victory when all non-mine grids have been visited
    if (visit_count == rows * columns - total_mines) {
      game_state = 1;
    }
  } else {
    // Marking a non-mine grid causes immediate failure
    game_state = -1;
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  // Can only target visited non-mine grids
  if (!visited[r][c] || mine_map[r][c] == 'X') {
    return;
  }
  
  // Count marked mines around the target
  int marked_count = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int nr = r + i;
      int nc = c + j;
      if (i == 0 && j == 0) continue;
      if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
        if (marked[nr][nc]) {
          marked_count++;
        }
      }
    }
  }
  
  // Get the mine count of the target block
  int target_mine_count = CountAdjacentMines(r, c);
  
  // If marked count equals target mine count, auto-explore
  if (marked_count == target_mine_count) {
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        int nr = r + i;
        int nc = c + j;
        if (i == 0 && j == 0) continue;
        if (nr >= 0 && nr < rows && nc >= 0 && nc < columns) {
          // Only visit unmarked non-mine grids
          if (!marked[nr][nc] && mine_map[nr][nc] != 'X') {
            VisitRecursive(nr, nc);
          }
        }
      }
    }
  }
  
  // Check for win condition
  if (visit_count == rows * columns - total_mines) {
    game_state = 1;
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
  } else {
    std::cout << "GAME OVER!" << std::endl;
  }
  
  // Output visit_count and marked_mine_count
  // If victory, marked_mine_count should be total number of mines
  // If failure, it's the number of correctly marked mines
  if (game_state == 1) {
    std::cout << visit_count << " " << total_mines << std::endl;
  } else {
    std::cout << visit_count << " " << marked_mine_count << std::endl;
  }
  
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      // Victory condition: all non-mine grids visited
      if (game_state == 1) {
        // In victory, show all mines as '@' regardless of marked status
        if (mine_map[i][j] == 'X') {
          std::cout << '@';
        } else {
          // Show the mine count for non-mine blocks
          int count = CountAdjacentMines(i, j);
          std::cout << count;
        }
      } else {
        // Normal game state
        if (visited[i][j]) {
          if (mine_map[i][j] == 'X') {
            std::cout << 'X';
          } else {
            int count = CountAdjacentMines(i, j);
            std::cout << count;
          }
        } else if (marked[i][j]) {
          if (mine_map[i][j] == 'X') {
            std::cout << '@';
          } else {
            std::cout << 'X';
          }
        } else {
          std::cout << '?';
        }
      }
    }
    std::cout << std::endl;
  }
}

#endif
