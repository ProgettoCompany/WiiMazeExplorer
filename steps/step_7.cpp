#include <Arduino.h>
#include <EEPROM.h>
#include <Maze.hpp>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <NintendoExtensionCtrl.h>

// Comment the line below to disable player position debug output, which slows down the game
#define DEBUG_PLAYER_POSITION

void printSubMazeToLEDMatrix(uint8_t** subMaze, int width, int height, bool playerBlinkState, bool endBlinkState = false);
void playEndAnimation();
void printUpArrowToLEDMatrix();

Maze maze(16, 16); // max size depends on EEPROM storage and RAM, feel free to experiment
Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
Nunchuk nunchuck;

const int LED_MATRIX_SIZE = 8;
const int PLAYER_BLINK_FREQUENCY = 500; // In milliseconds
const int END_BLINK_FREQUENCY = 1000; // In milliseconds
const int PLAYER_MATRIX_POSITION_Y = 3;
const int PLAYER_MATRIX_POSITION_X = 3;

const int NUNCHUCK_CHECK_FREQUENCY = 100; // Frequency to check nunchuck in milliseconds
const int JOYSTICK_DEADZONE = 55; // Deadzone for joystick
const int MIN_MOVE_DELAY = 100; // Minimum delay between player movements
const int MAX_MOVE_DELAY = 500; // Maximum delay between player movements

MazePosition playerPosition = maze.getStartPosition();

uint8_t** subMaze8x8;  // Declare globally

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Maze Game");

  matrix.begin(0x70);  // Initialize with the I2C address of the matrix
  nunchuck.begin(); // Initialize the nunchuck

  while (!nunchuck.connect()) {
    Serial.println("Nunchuk not detected!");
    delay(1000);
  }

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
  static uint64_t lastEndBlinkTime = 0;
  static bool playerBlinkState = false;
  static bool endBlinkState = false;
  static uint64_t lastNunchuckCheckTime = 0;
  static uint64_t lastPlayerMoveTime = 0;

  uint32_t currentTime = millis();
  if (currentTime - lastPlayerBlinkTime >= PLAYER_BLINK_FREQUENCY) {
    playerBlinkState = !playerBlinkState;
    lastPlayerBlinkTime = currentTime;
  }
  if (currentTime - lastEndBlinkTime >= END_BLINK_FREQUENCY) {
    endBlinkState = !endBlinkState;
    lastEndBlinkTime = currentTime;
  }

  if (currentTime - lastNunchuckCheckTime >= NUNCHUCK_CHECK_FREQUENCY) {
    lastNunchuckCheckTime = currentTime;

    if (!nunchuck.update()) {
      Serial.println("Failed to poll nunchuck, attempting reconnection...");
      if (nunchuck.connect()) {
          Serial.println("Reconnected to nunchuck!");
      } else {
          delay(500);  // Don't spam reconnection attempts
      }
      return;
    }
  }

  // Move maze based on joystick input
  int newMazeX = playerPosition.column;
  int newMazeY = playerPosition.row;

  // Get joystick input and center values
  int joyX = nunchuck.joyX();
  int joyY = nunchuck.joyY();
  int joyXCentered = joyX - 128;  // Center at 0
  int joyYCentered = joyY - 128;  // Center at 0
  int moveSpeedX = 1;
  int moveSpeedY = 1;

  // Calculate joystick direction and magnitude
  int joyMagnitude = sqrt(joyXCentered*joyXCentered + joyYCentered*joyYCentered);
 

  // Adaptive movement delay - faster response for stronger joystick tilt
  unsigned long playerMovementDelay = map(constrain(joyMagnitude, JOYSTICK_DEADZONE, 127),
                              JOYSTICK_DEADZONE, 127, 
                              MAX_MOVE_DELAY, MIN_MOVE_DELAY);


  if (joyMagnitude > JOYSTICK_DEADZONE && currentTime - lastPlayerMoveTime >= playerMovementDelay) {
    lastPlayerMoveTime = currentTime;
    if (joyX > 128 + JOYSTICK_DEADZONE) {
      newMazeX = min(newMazeX + moveSpeedX, maze.getColumns() - 1);
      Serial.println("Joystick moved right");
    } else if (joyX < 128 - JOYSTICK_DEADZONE) {
      newMazeX = max(newMazeX - moveSpeedX, 0);
      Serial.println("Joystick moved left");
    }
    // Need to invert Y axis to account for different coordinate systems
    if (joyY > 128 + JOYSTICK_DEADZONE) {
      newMazeY = max(newMazeY - moveSpeedY, 0);
      Serial.println("Joystick moved up");
    } else if (joyY < 128 - JOYSTICK_DEADZONE) {
      newMazeY = min(newMazeY + moveSpeedY, maze.getRows() - 1);
      Serial.println("Joystick moved down");
    }

    if (newMazeX != playerPosition.column || newMazeY != playerPosition.row) {
      if (!maze.isCollision(newMazeY, newMazeX)) {
        playerPosition.column = newMazeX;
        playerPosition.row = newMazeY;
        #ifdef DEBUG_PLAYER_POSITION
          Serial.print("Moved to new position: ");
          Serial.print("X = ");
          Serial.print(playerPosition.column);
          Serial.print(", Y = ");
          Serial.println(playerPosition.row);
          maze.printToSerialWithPlayer(playerPosition);
        #endif
      } else {
        Serial.println("Collision detected, position not updated");
      }
    }
  }

  MazePosition endPosition = maze.getEndPosition();
  if (playerPosition.row == endPosition.row && playerPosition.column == endPosition.column) {
    Serial.println("Congratulations! You have reached the end of the maze!");
    delay(500); // Delay to prevent accidental restart
    playEndAnimation();
    maze.generateMaze();
    playerPosition = maze.getStartPosition();
    Serial.println("New maze generated and saved to EEPROM.");
    maze.printToSerialWithPlayer(playerPosition);
  }

  maze.getSubMaze(playerPosition.row - PLAYER_MATRIX_POSITION_Y, playerPosition.column - PLAYER_MATRIX_POSITION_X, LED_MATRIX_SIZE, LED_MATRIX_SIZE, subMaze8x8);
  printSubMazeToLEDMatrix(subMaze8x8, LED_MATRIX_SIZE, LED_MATRIX_SIZE, playerBlinkState, endBlinkState);
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

/**
 * @brief Plays an animation when the player reaches the end of the maze.
 */
void playEndAnimation() {
  for (int i = 0; i < 3; i++) {
    matrix.clear();
    for (int size = 1; size <= LED_MATRIX_SIZE / 2; size++) {
      matrix.clear();
      for (int x = LED_MATRIX_SIZE / 2 - size; x <= LED_MATRIX_SIZE / 2 + size - 1; x++) {
      for (int y = LED_MATRIX_SIZE / 2 - size; y <= LED_MATRIX_SIZE / 2 + size - 1; y++) {
        if (size == 1 || x == LED_MATRIX_SIZE / 2 - size || x == LED_MATRIX_SIZE / 2 + size - 1 || y == LED_MATRIX_SIZE / 2 - size || y == LED_MATRIX_SIZE / 2 + size - 1) {
          matrix.drawPixel(x, y, LED_ON);
        }
      }
      }
      matrix.writeDisplay();
      delay(200);
    }
  }
  matrix.clear();
  matrix.writeDisplay();
}
