#include <Arduino.h>
#ifndef MAZE_HPP
#define MAZE_HPP

const uint8_t START = 3;
const uint8_t END = 2;
const uint8_t WALL = 1;
const uint8_t EMPTY = 0;

struct MazePosition {
  int row;
  int column;
};

/**
 * @class Maze
 * @brief A class to represent and manipulate a maze.
 */
class Maze {
public:
  /**
   * @brief Constructs a Maze object with specified rows and columns.
   * @param rows Number of rows in the maze.
   * @param columns Number of columns in the maze.
   */
  Maze(int rows, int columns);

  /**
   * @brief Gets the number of rows in the maze.
   * @return The number of rows in the maze.
   */
  int getRows();

  /**
   * @brief Gets the number of columns in the maze.
   * @return The number of columns in the maze.
   */
  int getColumns();

  /**
   * @brief Gets the starting position of the maze.
   * @return The starting position of the maze.
   */
  MazePosition getStartPosition();

  /**
   * @brief Gets the ending position of the maze.
   * @return The ending position of the maze.
   */
  MazePosition getEndPosition();

  /**
   * @brief Prints the maze to the serial output.
   */
  void printToSerial();

  /**
   * @brief Prints the maze to the serial output with the player at the specified position.
   * @param playerRow The row of the player.
   * @param playerColumn The column of the player.
   */
  void printToSerialWithPlayer(int playerRow, int playerColumn);

  /**
   * @brief Prints the maze to the serial output with the player at the specified position.
   * @param playerPosition The position of the player.
   */
  void printToSerialWithPlayer(MazePosition playerPosition);

  /**
   * @brief Saves the maze to EEPROM for use after a power cycle.
   * 
   * The maze is saved to EEPROM starting at address 0. The last byte of EEPROM
   * is used to store a checksum of the maze data.
   */
  void saveToEEPROM();

  /**
   * @brief Loads the maze from EEPROM.
   * @note The maze must have been previously saved to EEPROM.
   * @note The maze must have the same dimensions as the maze that was saved.
   * @note The loaded maze is checked against its checksum stored in EEPROM to ensure data integrity.
   * @return True if the maze was successfully loaded, false otherwise.
   */
  bool loadFromEEPROM();

  /**
   * @brief Retrieves a sub-maze from the specified starting position.
   * @note The sub-maze is a 2D array of the specified dimensions.
   * @note The sub-maze is a copy of the maze data, not a reference.
   * @note The sub-maze is allocated on the heap and must be freed by the caller.
   * @note if the bounds of the sub-maze exceed the maze dimensions, the function padds the sub-maze
   *       with empty cells.
   * 
   * @param startRow The starting row of the sub-maze.
   * @param startColumn The starting column of the sub-maze.
   * @param numRows The number of rows in the sub-maze.
   * @param numColumns The number of columns in the sub-maze.
   * @param subMaze The sub-maze to populate.
   */
  void getSubMaze(int startRow, int startColumn, int numRows, int numColumns, uint8_t** subMaze);

  /**
   * @brief Checks if a cell is a collision, i.e. a wall or out of bounds.
   * @note If the maze has not been initialized, the function returns true.
   * 
   * @param row The row of the cell.
   * @param column The column of the cell.
   * @return True if the cell is a wall, false otherwise.
   */
  bool isCollision(int row, int column);

  /**
   * @brief Generates a new maze using the recursive backtracking algorithm.
   */
  void generateMaze();

private:
  int mazeRows;
  int mazeColumns;
  uint8_t** maze;
  bool isMazeInitialized = false;
  uint8_t calculateChecksum();

  const int EEPROM_START_ADDRESS = 1; // Matrix brightness is stored at address 0
  const char WALL_CHAR = '#';
  const char EMPTY_CHAR = ' ';
  const char PLAYER_CHAR = 'P';
  const char START_CHAR = 'S';
  const char END_CHAR = 'E';

};

#endif
