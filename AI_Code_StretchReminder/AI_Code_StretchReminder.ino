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
// unsigned long reminder_duration_ms = 30 * 60 * 1000; 
// For testing, you can set a shorter duration, e.g., 15 seconds:
unsigned long reminder_duration_ms = 15 * 1000; 

// Timer logic variables
unsigned long state_start_time_ms = 0;
bool is_timer_running = false;

// Gyroscope variables
float gyroX, gyroY, gyroZ;
const float MOVEMENT_THRESHOLD = 20.0; // How much gyro change counts as "movement"

// ### 2. UI Drawing Functions ###

void drawDeskUI() {
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Mode: Desk Reminder");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("Status:");
}

void drawPostureUI() {
  M5.Lcd.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Mode: Posture Reminder");
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.print("Status:");
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
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // Reset text color for next draw
}


// ### 3. Core Logic for Each Mode ###

void handleDeskReminder() {
  float dist = sonic_sensor.getDistance();
  
  // --- Draw dynamic text ---
  M5.Lcd.setTextSize(2);

  // Clear distance display area before drawing
  M5.Lcd.fillRect(95, 40, 220, 20, TFT_BLACK); 
  M5.Lcd.setCursor(100, 40);
  M5.Lcd.printf("Dist: %0.1f cm", dist/10.0);

  // Clear status display area before drawing
  M5.Lcd.fillRect(10, 70, 310, 40, TFT_BLACK);
  M5.Lcd.setCursor(10, 70);

  // --- Update timer based on condition ---
  // Condition: User is present (distance > 0 and < 50 cm)
  if (dist > 0 && dist < 500) {
    if (!is_timer_running) {
      // Start the timer
      is_timer_running = true;
      state_start_time_ms = millis();
    }
    M5.Lcd.print("User is present. Timer running.");
  } else {
    // User is not present, reset timer
    is_timer_running = false;
    M5.Lcd.print("No user detected. Timer paused.");
  }
}

void handlePostureReminder() {
  static float last_gyroX = 0, last_gyroY = 0, last_gyroZ = 0;
  M5.Imu.getGyroData(&gyroX, &gyroY, &gyroZ);

  // --- Draw dynamic text ---
  M5.Lcd.setTextSize(1);
  // Clear gyro display area before drawing
  M5.Lcd.fillRect(95, 40, 220, 20, TFT_BLACK); 
  M5.Lcd.setCursor(100, 40);
  M5.Lcd.printf("G: %.1f,%.1f,%.1f", gyroX, gyroY, gyroZ);

  M5.Lcd.setTextSize(2);
  // Clear status display area before drawing
  M5.Lcd.fillRect(10, 70, 310, 40, TFT_BLACK);
  M5.Lcd.setCursor(10, 70);
  
  // --- Update timer based on condition ---
  // Check for significant movement
  if (abs(gyroX - last_gyroX) > MOVEMENT_THRESHOLD ||
      abs(gyroY - last_gyroY) > MOVEMENT_THRESHOLD ||
      abs(gyroZ - last_gyroZ) > MOVEMENT_THRESHOLD) {
    // Movement detected, reset timer
    is_timer_running = false;
    M5.Lcd.print("Movement detected. Timer paused.");
  } else {
    // No significant movement (still)
    if (!is_timer_running) {
      // Start the timer
      is_timer_running = true;
      state_start_time_ms = millis();
    }
    M5.Lcd.print("Posture stable. Timer running.");
  }
  // Update last known gyro values for the next loop
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
  M5.update(); // Update button states and other M5Stack components

  // --- Mode Switching Logic (Button B) ---
  if (M5.BtnB.wasPressed() && current_mode != ALARM_TRIGGERED) {
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
      M5.Speaker.tone(880, 500); // Play alarm sound
      if (M5.BtnA.wasPressed()) { // Check for dismiss button
        M5.Speaker.stop();
        is_timer_running = false;
        // Default back to Desk Reminder mode after alarm
        current_mode = DESK_REMINDER; 
        drawDeskUI();
      }
      delay(200); // Small delay to prevent sound from being too harsh
      return; // Skip the rest of the loop while in alarm state
  }

  // --- Universal Timer & Progress Bar ---
  if (is_timer_running) {
    unsigned long elapsed_time = millis() - state_start_time_ms;
    
    // Calculate progress for the bar (max width 298px inside the border)
    int progress_width = (int)(((float)elapsed_time / reminder_duration_ms) * 298.0);
    if(progress_width > 298) progress_width = 298;
    if(progress_width < 0) progress_width = 0;

    // 1. Clear the inside of the bar
    M5.Lcd.fillRect(11, 201, 298, 18, TFT_BLACK);
    // 2. Draw the new progress
    if (progress_width > 0) {
      M5.Lcd.fillRect(11, 201, progress_width, 18, TFT_GREEN);
    }
    // 3. Draw the border on top
    M5.Lcd.drawRect(10, 200, 300, 20, TFT_WHITE);

    // Check if it's time to trigger the alarm
    if (elapsed_time >= reminder_duration_ms) {
      current_mode = ALARM_TRIGGERED;
      drawAlarmUI();
    }
  } else {
    // If timer is not running, show an empty bar
    M5.Lcd.fillRect(11, 201, 298, 18, TFT_BLACK);
    M5.Lcd.drawRect(10, 200, 300, 20, TFT_WHITE);
  }
  
  delay(100); // Main loop delay to prevent flickering
}