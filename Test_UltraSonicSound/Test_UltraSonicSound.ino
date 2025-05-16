#include <M5Unified.h>
#include <Unit_Sonic.h>

SONIC_I2C sensor;

int volume = 128;

void setup() {
    auto config = M5.config();
    M5.begin(config);          // M5Coreの初期化
    M5.Speaker.begin();
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

  if(dist <= 50){
    M5.Speaker.setVolume(volume);
    M5.Speaker.tone(440,200);
  } 
}