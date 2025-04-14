#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

void playEndAnimation();
void printUpArrowToLEDMatrix();

Adafruit_8x8matrix matrix = Adafruit_8x8matrix();

const int LED_MATRIX_SIZE = 8;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Maze Game");

  matrix.begin(0x70);  // Initialize with the I2C address of the matrix

  // Print up arrow initially so player knows which way is up
  printUpArrowToLEDMatrix();
  delay(2000);
  matrix.clear();
  matrix.writeDisplay();
  playEndAnimation();
}

void loop() {
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
