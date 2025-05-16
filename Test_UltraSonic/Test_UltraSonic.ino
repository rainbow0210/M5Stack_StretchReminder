#include <M5Core2.h>
#include <Unit_Sonic.h>

SONIC_I2C sensor;

void setup() {
    M5.begin();          // M5Coreの初期化
    sensor.begin();
}

void loop() {
  // 情報を取得
  M5.update();

  float dist = sensor.getDistance();
  //Serial.println(dist);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(30, 90);
  M5.Lcd.print(dist); 
}