#include <Arduino.h>
#include <Wire.h>
#include <NintendoExtensionCtrl.h>

Nunchuk nunchuck;

const int NUNCHUCK_CHECK_FREQUENCY = 100; // Frequency to check nunchuck in milliseconds
const int JOYSTICK_DEADZONE = 55; // Deadzone for joystick
const int MIN_MOVE_DELAY = 100; // Minimum delay between player movements
const int MAX_MOVE_DELAY = 500; // Maximum delay between player movements

int mazeX = 0; // Current maze X position
int mazeY = 0; // Current maze Y position

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Maze Game");

  nunchuck.begin(); // Initialize the nunchuck

  while (!nunchuck.connect()) {
    Serial.println("Nunchuk not detected!");
    delay(1000);
  }
}

void loop() {
  static uint64_t lastPlayerBlinkTime = 0;
  static uint64_t lastEndBlinkTime = 0;
  static bool playerBlinkState = false;
  static bool endBlinkState = false;
  static uint64_t lastNunchuckCheckTime = 0;
  static uint64_t lastPlayerMoveTime = 0;

  uint32_t currentTime = millis();
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
  int newMazeX = mazeX;
  int newMazeY = mazeY;

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
      newMazeX = newMazeX + moveSpeedX;
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
      newMazeY = newMazeY + moveSpeedY;
      Serial.println("Joystick moved down");
    }
  }
}