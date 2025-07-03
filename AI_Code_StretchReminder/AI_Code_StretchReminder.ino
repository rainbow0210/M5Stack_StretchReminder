#include <M5Unified.h>
#include <Unit_Sonic.h> // For the Ultrasonic Sensor

// ### 1. Global Settings & Variables ###

// Sensor object
SONIC_I2C sonic_sensor;

// Mode management
enum Mode {
  DESK_REMINDER,
  POSTURE_REMINDER,
  ALARM_TRIGGERED
};
Mode current_mode = DESK_REMINDER; // Start in Desk Reminder mode

// Timer settings (default 30 minutes)
unsigned long reminder_duration_ms = 0.01 * 60 * 1000; 
// For testing, you can set a shorter duration, e.g., 15 seconds:
// unsigned long reminder_duration_ms = 15 * 1000; 

// Timer logic variables
unsigned long state_start_time_ms = 0;
bool is_timer_running = false;

// Gyroscope variables
float gyroX, gyroY, gyroZ;
const float MOVEMENT_THRESHOLD = 20.0; // How much gyro change counts as "movement"

// ### 2. UI Drawing Functions ###
// Functions to draw the screen for each mode

void drawDeskUI() {
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Mode: Desk Reminder");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("Status:");
  // More UI elements will be drawn in the main loop
}

void drawPostureUI() {
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Mode: Posture Reminder");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("Status:");
  // More UI elements will be drawn in the main loop
}

void drawAlarmUI() {
  M5.Lcd.clear();
  M5.Lcd.fillScreen(TFT_RED);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(30, 50);
  M5.Lcd.print("Time for a");
  M5.Lcd.setCursor(60, 90);
  M5.Lcd.print("Stretch!");
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(60, 180);
  M5.Lcd.print("Press BtnA to Stop");
}

// ### 3. Core Logic for Each Mode ###

void handleDeskReminder() {
  float dist = sonic_sensor.getDistance();
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(100, 40);
  M5.Lcd.printf("Dist: %0.1f cm", dist/10.0); // Show distance in cm

  // Condition: User is present (distance < 50 cm)
  if (dist > 0 && dist < 500) {
    if (!is_timer_running) {
      // Start the timer
      is_timer_running = true;
      state_start_time_ms = millis();
    }
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("User is present. Timer running.");
  } else {
    // User is not present, reset timer
    is_timer_running = false;
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("No user detected. Timer paused.");
  }
}

void handlePostureReminder() {
  static float last_gyroX = 0, last_gyroY = 0, last_gyroZ = 0;
  M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(100, 40);
  M5.Lcd.printf("G: %.1f,%.1f,%.1f", gyroX, gyroY, gyroZ);

  // Check for significant movement
  if (abs(gyroX - last_gyroX) > MOVEMENT_THRESHOLD ||
      abs(gyroY - last_gyroY) > MOVEMENT_THRESHOLD ||
      abs(gyroZ - last_gyroZ) > MOVEMENT_THRESHOLD) {
    // Movement detected, reset timer
    is_timer_running = false;
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("Movement detected. Timer paused. ");
  } else {
    // No significant movement (still)
    if (!is_timer_running) {
      // Start the timer
      is_timer_running = true;
      state_start_time_ms = millis();
    }
    M5.Lcd.setCursor(10, 70);
    M5.Lcd.print("Posture stable. Timer running.");
  }
  // Update last known gyro values
  last_gyroX = gyroX;
  last_gyroY = gyroY;
  last_gyroZ = gyroZ;
}

// ### 4. Setup and Main Loop ###

void setup() {
  auto config = M5.config();
  M5.begin(config);
  M5.Speaker.begin();
  sonic_sensor.begin(); // Initialize I2C Sonic sensor
  M5.Imu.begin();       // Initialize built-in Gyro/IMU

  drawDeskUI(); // Draw the initial screen
}

void loop() {
  M5.update(); // Update button states

  // --- Mode Switching Logic ---
  // Use Button B to cycle between modes
  if (M5.BtnB.wasPressed()) {
    if (current_mode == DESK_REMINDER) {
      current_mode = POSTURE_REMINDER;
      drawPostureUI();
    } else if (current_mode == POSTURE_REMINDER) {
      current_mode = DESK_REMINDER;
      drawDeskUI();
    }
    // Reset timer when switching modes
    is_timer_running = false; 
  }

  // --- Main State Machine ---
  switch (current_mode) {
    case DESK_REMINDER:
      handleDeskReminder();
      break;
    case POSTURE_REMINDER:
      handlePostureReminder();
      break;
    case ALARM_TRIGGERED:
      // Play a sound
      M5.Speaker.tone(880, 500); 
      // Check if Button A is pressed to dismiss the alarm
      if (M5.BtnA.wasPressed()) {
        M5.Speaker.stop();
        is_timer_running = false;
        // Go back to the previous mode (you could store this, but for now we'll default to Desk)
        current_mode = DESK_REMINDER;
        drawDeskUI();
      }
      return; // Skip the rest of the loop while in alarm state
  }

  // --- Universal Timer & Alarm Check ---
  if (is_timer_running) {
    unsigned long elapsed_time = millis() - state_start_time_ms;
    
    // Draw a progress bar
    int progress = (int)(((float)elapsed_time / reminder_duration_ms) * 300); // 300px wide bar
    if(progress > 300) progress = 300;
    M5.Lcd.fillRect(10, 200, progress, 20, TFT_GREEN);
    M5.Lcd.drawRect(10, 200, 300, 20, TFT_WHITE);


    if (elapsed_time >= reminder_duration_ms) {
      current_mode = ALARM_TRIGGERED;
      drawAlarmUI();
    }
  } else {
    // Draw an empty progress bar if timer is not running
    M5.Lcd.drawRect(10, 200, 300, 20, TFT_WHITE);
    M5.Lcd.fillRect(11, 201, 298, 18, TFT_BLACK);
  }
  
  delay(100); // Small delay to prevent flickering and debounce sensors
}