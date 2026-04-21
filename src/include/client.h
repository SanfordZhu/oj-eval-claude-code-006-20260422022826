#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

// Client's view of the map
char client_map[30][30];  // Maximum size 30x30 as per constraints

// Track which cells we've processed
bool processed[30][30];

// Helper function to check if a cell is within bounds
bool inBounds(int r, int c) {
  return r >= 0 && r < rows && c >= 0 && c < columns;
}

// Helper function to count neighbors of a certain type
void countNeighbors(int r, int c, int& unknown, int& marked, int& number) {
  unknown = 0;
  marked = 0;
  number = 0;

  if (!inBounds(r, c) || client_map[r][c] == '?' || client_map[r][c] == '@' || client_map[r][c] == 'X') {
    return;  // Not a numbered cell
  }

  // Get the number in this cell
  int cell_value = client_map[r][c] - '0';

  // Check all 8 neighbors
  for (int dr = -1; dr <= 1; dr++) {
    for (int dc = -1; dc <= 1; dc++) {
      if (dr == 0 && dc == 0) continue;
      int nr = r + dr;
      int nc = c + dc;
      if (inBounds(nr, nc)) {
        if (client_map[nr][nc] == '?') {
          unknown++;
        } else if (client_map[nr][nc] == '@') {
          marked++;
        }
      }
    }
  }

  number = cell_value;
}

// Structure for constraint solving
struct Constraint {
  int r, c;
  int required_mines;  // number - marked
  std::vector<std::pair<int, int>> unknown_neighbors;
};

// Simple constraint solver using backtracking
bool solveConstraints(const std::vector<Constraint>& constraints,
                      const std::vector<std::pair<int, int>>& variables,
                      std::vector<int>& solution,
                      int idx,
                      std::vector<bool>& is_mine) {
  if (idx == variables.size()) {
    // Check if solution satisfies all constraints
    for (const auto& constraint : constraints) {
      int mine_count = 0;
      for (const auto& neighbor : constraint.unknown_neighbors) {
        // Find index of this neighbor in variables
        for (size_t i = 0; i < variables.size(); i++) {
          if (variables[i] == neighbor) {
            if (is_mine[i]) mine_count++;
            break;
          }
        }
      }
      if (mine_count != constraint.required_mines) {
        return false;
      }
    }
    return true;
  }

  // Try this cell as not a mine
  is_mine[idx] = false;
  if (solveConstraints(constraints, variables, solution, idx + 1, is_mine)) {
    return true;
  }

  // Try this cell as a mine
  is_mine[idx] = true;
  if (solveConstraints(constraints, variables, solution, idx + 1, is_mine)) {
    return true;
  }

  return false;
}

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
  // TODO (student): Initialize all your global variables!
  int first_row, first_column;
  std::cin >> first_row >> first_column;

  // Initialize client map
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      client_map[i][j] = '?';
      processed[i][j] = false;
    }
  }

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
void ReadMap() {
  // TODO (student): Implement me!
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      std::cin >> client_map[i][j];
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // TODO (student): Implement me!

  // Strategy: Basic reasoning (Baseline1)
  // 1. First, try to find obvious mines to mark
  // 2. Then, try to find obvious safe cells to visit
  // 3. Then, try auto-explore on visited cells
  // 4. Finally, make a random move

  // Step 1: Look for cells where we can mark mines
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '1' && client_map[i][j] <= '8') {
        int unknown, marked, number;
        countNeighbors(i, j, unknown, marked, number);

        // If unknown + marked == number, all unknown neighbors are mines
        if (unknown > 0 && unknown + marked == number) {
          // Mark all unknown neighbors as mines
          for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
              if (dr == 0 && dc == 0) continue;
              int nr = i + dr;
              int nc = j + dc;
              if (inBounds(nr, nc) && client_map[nr][nc] == '?') {
                Execute(nr, nc, 1);  // Mark as mine
                return;
              }
            }
          }
        }
      }
    }
  }

  // Step 2: Look for cells where we can visit safe neighbors
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '1' && client_map[i][j] <= '8') {
        int unknown, marked, number;
        countNeighbors(i, j, unknown, marked, number);

        // If marked == number, all unknown neighbors are safe
        if (unknown > 0 && marked == number) {
          // Visit all unknown neighbors
          for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
              if (dr == 0 && dc == 0) continue;
              int nr = i + dr;
              int nc = j + dc;
              if (inBounds(nr, nc) && client_map[nr][nc] == '?') {
                Execute(nr, nc, 0);  // Visit
                return;
              }
            }
          }
        }
      }
    }
  }

  // Step 3: Try to recognize common patterns
  // Pattern: 1-2 pattern (common in Minesweeper)
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '1') {
        // Check for adjacent '2' cells that share unknowns
        for (int dr = -1; dr <= 1; dr++) {
          for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = i + dr;
            int nc = j + dc;
            if (inBounds(nr, nc) && client_map[nr][nc] == '2') {
              // Find shared unknown neighbors
              std::vector<std::pair<int, int>> unknowns1, unknowns2, shared;

              // Get unknowns around cell (i,j)
              for (int dr1 = -1; dr1 <= 1; dr1++) {
                for (int dc1 = -1; dc1 <= 1; dc1++) {
                  if (dr1 == 0 && dc1 == 0) continue;
                  int r1 = i + dr1;
                  int c1 = j + dc1;
                  if (inBounds(r1, c1) && client_map[r1][c1] == '?') {
                    unknowns1.push_back({r1, c1});
                  }
                }
              }

              // Get unknowns around cell (nr,nc)
              for (int dr2 = -1; dr2 <= 1; dr2++) {
                for (int dc2 = -1; dc2 <= 1; dc2++) {
                  if (dr2 == 0 && dc2 == 0) continue;
                  int r2 = nr + dr2;
                  int c2 = nc + dc2;
                  if (inBounds(r2, c2) && client_map[r2][c2] == '?') {
                    unknowns2.push_back({r2, c2});
                  }
                }
              }

              // Find shared unknowns
              for (const auto& u1 : unknowns1) {
                for (const auto& u2 : unknowns2) {
                  if (u1 == u2) {
                    shared.push_back(u1);
                  }
                }
              }

              // If '1' cell has exactly 2 unknowns and '2' cell has exactly 3 unknowns
              // and they share 2 unknowns, then the non-shared unknown of '2' is safe
              if (unknowns1.size() == 2 && unknowns2.size() == 3 && shared.size() == 2) {
                // Find the non-shared unknown of '2'
                for (const auto& u2 : unknowns2) {
                  bool is_shared = false;
                  for (const auto& s : shared) {
                    if (u2 == s) {
                      is_shared = true;
                      break;
                    }
                  }
                  if (!is_shared) {
                    // This cell is safe
                    Execute(u2.first, u2.second, 0);
                    return;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Step 4: Try auto-explore on visited cells where it's valid
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] >= '1' && client_map[i][j] <= '8') {
        int unknown, marked, number;
        countNeighbors(i, j, unknown, marked, number);

        // Auto-explore is valid if marked == number
        if (marked == number && unknown > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }

  // Step 4: Make the safest guess
  // Calculate mine probabilities for each unknown cell
  std::vector<std::pair<int, int>> unknown_cells;
  std::vector<double> mine_probabilities;

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (client_map[i][j] == '?') {
        unknown_cells.push_back({i, j});

        // Estimate mine probability based on adjacent numbers
        double prob = 0.0;
        int count = 0;

        for (int dr = -1; dr <= 1; dr++) {
          for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            int nr = i + dr;
            int nc = j + dc;
            if (inBounds(nr, nc) && client_map[nr][nc] >= '1' && client_map[nr][nc] <= '8') {
              int unknown, marked, number;
              countNeighbors(nr, nc, unknown, marked, number);
              if (unknown > 0) {
                // Remaining mines around this cell = number - marked
                int remaining_mines = number - marked;
                if (remaining_mines > 0) {
                  prob += (double)remaining_mines / unknown;
                  count++;
                }
              }
            }
          }
        }

        if (count > 0) {
          mine_probabilities.push_back(prob / count);
        } else {
          // Cell not adjacent to any numbers - use global mine density
          mine_probabilities.push_back((double)total_mines / (rows * columns));
        }
      }
    }
  }

  // Find cell with lowest mine probability
  if (!unknown_cells.empty()) {
    int best_idx = 0;
    double best_prob = mine_probabilities[0];
    for (size_t i = 1; i < unknown_cells.size(); i++) {
      if (mine_probabilities[i] < best_prob) {
        best_prob = mine_probabilities[i];
        best_idx = i;
      }
    }

    Execute(unknown_cells[best_idx].first, unknown_cells[best_idx].second, 0);
    return;
  }

  // Fallback (shouldn't happen)
  Execute(0, 0, 0);
}

#endif