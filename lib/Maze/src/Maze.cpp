#include <Arduino.h>
#include <EEPROM.h>
#include "Maze.hpp"

Maze::Maze(int rows, int columns) : mazeRows(rows), mazeColumns(columns) {
  // Initialize the maze
  maze = new uint8_t*[mazeRows];
  for (int i = 0; i < mazeRows; i++) {
    maze[i] = new uint8_t[mazeColumns];
  }
}

int Maze::getRows() {
  return mazeRows;
}

int Maze::getColumns() {
  return mazeColumns;
}

MazePosition Maze::getStartPosition() {
  MazePosition startPosition = {0, 1};
  return startPosition;
}

MazePosition Maze::getEndPosition() {
  MazePosition endPosition = {mazeRows - 1, mazeColumns - 2};
  return endPosition;
}
  
void Maze::printToSerial() {
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      if (maze[i][j] == WALL) {
        Serial.print(WALL_CHAR);
      } else {
        Serial.print(EMPTY_CHAR);
      }
    }
    Serial.println();
  }
}

void Maze::printToSerialWithPlayer(int playerRow, int playerColumn) {
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      if (i == playerRow && j == playerColumn) {
        Serial.print(PLAYER_CHAR);
      } 
      else if (maze[i][j] == END) {
        Serial.print(END_CHAR);
      }
      else if (maze[i][j] == START) {
        Serial.print(START_CHAR);
      }
      else if (maze[i][j] == WALL) {
        Serial.print(WALL_CHAR);
      } else {
        Serial.print(EMPTY_CHAR);
      }
    }
    Serial.println();
  }
}

void Maze::printToSerialWithPlayer(MazePosition playerPosition) {
  printToSerialWithPlayer(playerPosition.row, playerPosition.column);
}

void Maze::saveToEEPROM() {
  int address = EEPROM_START_ADDRESS;
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      EEPROM.write(address++, maze[i][j]);
    }
  }
  uint8_t checksum = calculateChecksum();
  EEPROM.write(address, checksum);
}

bool Maze::loadFromEEPROM() {
  int address = EEPROM_START_ADDRESS;
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      maze[i][j] = EEPROM.read(address++);
    }
  }
  uint8_t storedChecksum = EEPROM.read(address);
  if (storedChecksum == calculateChecksum()) {
    isMazeInitialized = true;
    return true;
  } else {
    return false;
  }
}

uint8_t Maze::calculateChecksum() {
  uint8_t checksum = 0;
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      checksum ^= maze[i][j];
    }
  }
  return checksum;
}

void Maze::getSubMaze(int startRow, int startColumn, int numRows, int numColumns, uint8_t** subMaze) {
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numColumns; j++) {
      int mazeRow = startRow + i;
      int mazeCol = startColumn + j;
      if (mazeRow >= 0 && mazeRow < mazeRows && mazeCol >= 0 && mazeCol < mazeColumns && isMazeInitialized) {
        subMaze[i][j] = maze[mazeRow][mazeCol];
      } else {
        subMaze[i][j] = 0; // Pad with zeros if out of bounds
      }
    }
  }
}

bool Maze::isCollision(int row, int column) {
  if (!isMazeInitialized) {
    return true;
  }
  if (row >= 0 && row < mazeRows && column >= 0 && column < mazeColumns && isMazeInitialized) {
    return maze[row][column] == WALL;
  } else {
    return true; // Treat out of bounds as a wall
  }
}

void Maze::generateMaze() {
  // Based on the recursive backtracking algorithm implementation found here: 
  // https://github.com/professor-l/mazes/blob/master/scripts/backtracking.js

  // Seed the random number generator
  randomSeed(analogRead(0));
  
  // Fill maze with walls (1s)
  for (int i = 0; i < mazeRows; i++) {
    for (int j = 0; j < mazeColumns; j++) {
      maze[i][j] = WALL;
    }
  }
  
  // Create entrance
  MazePosition startPosition = getStartPosition();
  maze[startPosition.row][startPosition.column] = START;
  
  // Choose a random starting cell (must be odd coordinates)
  int startRow, startCol;
  do {
    startRow = random(1, mazeRows);
  } while (startRow % 2 == 0);
  
  do {
    startCol = random(1, mazeColumns);
  } while (startCol % 2 == 0);
  
  maze[startRow][startCol] = EMPTY;
  
  // Create stack for backtracking
  const int MAX_STACK_SIZE = 100; // Adjust based on expected maze size
  int stackRow[MAX_STACK_SIZE];
  int stackCol[MAX_STACK_SIZE];
  int stackSize = 0;
  
  // Push starting cell to stack
  stackRow[stackSize] = startRow;
  stackCol[stackSize] = startCol;
  stackSize++;
  
  while (stackSize > 0) {
    // Get current cell from top of stack
    int row = stackRow[stackSize - 1];
    int col = stackCol[stackSize - 1];
    
    // Find unvisited neighbors
    int neighborDirections[4] = {0}; // 0: none, 1: possible direction
    int neighborCount = 0;
    
    // Check up
    if (row >= 2 && maze[row-2][col] == WALL) {
      neighborDirections[0] = 1;
      neighborCount++;
    }
    
    // Check right
    if (col < mazeColumns-2 && maze[row][col+2] == WALL) {
      neighborDirections[1] = 1;
      neighborCount++;
    }
    
    // Check down
    if (row < mazeRows-2 && maze[row+2][col] == WALL) {
      neighborDirections[2] = 1;
      neighborCount++;
    }
    
    // Check left
    if (col >= 2 && maze[row][col-2] == WALL) {
      neighborDirections[3] = 1;
      neighborCount++;
    }
    
    // If no unvisited neighbors, backtrack
    if (neighborCount == 0) {
      stackSize--;
      continue;
    }
    
    // Choose a random direction
    int chosen = random(0, neighborCount);
    int directionIndex = -1;
    
    for (int i = 0; i < 4; i++) {
      if (neighborDirections[i] == 1) {
        if (chosen == 0) {
          directionIndex = i;
          break;
        }
        chosen--;
      }
    }
    
    // Move in the chosen direction
    int newRow = row;
    int newCol = col;
    
    switch (directionIndex) {
      case 0: // Up
        newRow -= 2;
        maze[row-1][col] = EMPTY; // Remove wall between cells
        break;
      case 1: // Right
        newCol += 2;
        maze[row][col+1] = EMPTY;
        break;
      case 2: // Down
        newRow += 2;
        maze[row+1][col] = EMPTY;
        break;
      case 3: // Left
        newCol -= 2;
        maze[row][col-1] = EMPTY;
        break;
    }
    
    // Mark the new cell as empty
    maze[newRow][newCol] = EMPTY;
    
    // Push the new cell onto the stack
    stackRow[stackSize] = newRow;
    stackCol[stackSize] = newCol;
    stackSize++;
  }
  
  // Create exit at the bottom
  MazePosition endPosition = getEndPosition();
  maze[endPosition.row][endPosition.column] = END;
  isMazeInitialized = true;
}