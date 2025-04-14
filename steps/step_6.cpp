#include <Arduino.h>
#include <EEPROM.h>
#include <Maze.hpp>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

void printSubMazeToLEDMatrix(uint8_t** subMaze, int width, int height, bool playerBlinkState, bool endBlinkState = false);
void printUpArrowToLEDMatrix();

Maze maze(16, 16); // max size depends on EEPROM storage and RAM, feel free to experiment
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

const int LED_MATRIX_SIZE = 8;
const int PLAYER_BLINK_FREQUENCY = 500; // In milliseconds
const int END_BLINK_FREQUENCY = 1000; // In milliseconds
const int PLAYER_MATRIX_POSITION_Y = 3;
const int PLAYER_MATRIX_POSITION_X = 3;

MazePosition playerPosition = maze.getStartPosition();

uint8_t** subMaze8x8;  // Declare globally

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Maze Game");

  matrix.begin(0x70);  // Initialize with the I2C address of the matrix

  Serial.println("Maze:");
  maze.generateMaze();
  maze.printToSerialWithPlayer(playerPosition);
  
  // Allocate once
  subMaze8x8 = new uint8_t*[LED_MATRIX_SIZE];
  for (int i = 0; i < LED_MATRIX_SIZE; i++) {
    subMaze8x8[i] = new uint8_t[LED_MATRIX_SIZE];
  }

  // Print up arrow initially so player knows which way is up
  printUpArrowToLEDMatrix();
  delay(2000);
}

void loop() {
  static uint64_t lastPlayerBlinkTime = 0;
  static bool playerBlinkState = false;

  uint32_t currentTime = millis();
  if (currentTime - lastPlayerBlinkTime >= PLAYER_BLINK_FREQUENCY) {
    playerBlinkState = !playerBlinkState;
    lastPlayerBlinkTime = currentTime;
  }

  maze.getSubMaze(playerPosition.row - PLAYER_MATRIX_POSITION_Y, playerPosition.column - PLAYER_MATRIX_POSITION_X, LED_MATRIX_SIZE, LED_MATRIX_SIZE, subMaze8x8);
  printSubMazeToLEDMatrix(subMaze8x8, LED_MATRIX_SIZE, LED_MATRIX_SIZE, playerBlinkState, false);
}

/**
 * @brief Prints an up arrow to the LED matrix.
 */
void printUpArrowToLEDMatrix() {
  matrix.clear();
  matrix.drawPixel(3, 0, LED_ON);
  matrix.drawPixel(4, 0, LED_ON);
  matrix.drawPixel(2, 1, LED_ON);
  matrix.drawPixel(3, 1, LED_ON);
  matrix.drawPixel(4, 1, LED_ON);
  matrix.drawPixel(5, 1, LED_ON);
  matrix.drawPixel(1, 2, LED_ON);
  matrix.drawPixel(2, 2, LED_ON);
  matrix.drawPixel(3, 2, LED_ON);
  matrix.drawPixel(4, 2, LED_ON);
  matrix.drawPixel(5, 2, LED_ON);
  matrix.drawPixel(6, 2, LED_ON);
  matrix.drawPixel(3, 3, LED_ON);
  matrix.drawPixel(4, 3, LED_ON);
  matrix.drawPixel(3, 4, LED_ON);
  matrix.drawPixel(4, 4, LED_ON);
  matrix.drawPixel(3, 5, LED_ON);
  matrix.drawPixel(4, 5, LED_ON);
  matrix.drawPixel(3, 6, LED_ON);
  matrix.drawPixel(4, 6, LED_ON);
  matrix.writeDisplay();
}

/**
 * @brief Prints a sub-maze to the LED matrix.
 * 
 * @param subMaze The sub-maze to print.
 * @param width The width of the sub-maze.
 * @param height The height of the sub-maze.
 * @param playerBlinkState The state of the player blink effect.
 * @param endBlinkState The state of the end blink effect.
 */
void printSubMazeToLEDMatrix(uint8_t** subMaze, int width, int height, bool playerBlinkState, bool endBlinkState) {
  matrix.clear();
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (i == PLAYER_MATRIX_POSITION_Y && j == PLAYER_MATRIX_POSITION_X) {
        matrix.drawPixel(j, i, playerBlinkState ? LED_ON : LED_OFF);
      } else {
        if (subMaze[i][j] == WALL) {
          matrix.drawPixel(j, i, LED_ON);
        } else if (subMaze[i][j] == END) {
          matrix.drawPixel(j, i, endBlinkState ? LED_ON : LED_OFF);
        } else {
          matrix.drawPixel(j, i, LED_OFF);
        }
      }
    }
  }
  matrix.writeDisplay();
}