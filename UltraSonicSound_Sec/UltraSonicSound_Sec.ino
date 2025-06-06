#include <M5Unified.h>
#include <Unit_Sonic.h>
#include <Wire.h>

unsigned long millisec;
unsigned long now_millisec;

SONIC_I2C sensor;

int volume = 128;
bool sound_mode = false;

void setup() {
    auto config = M5.config();
    M5.begin(config);// M5Coreの初期化
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

  if(dist <= 500){
    now_millisec = millis();

    if(millisec == 0){
      millisec = millis();
    }
    else if(now_millisec - millisec >= 10000){
      sound_mode = true;
    }
  }

  if(sound_mode == true){
    M5.Speaker.setVolume(volume);
    M5.Speaker.tone(440,200);
  } 

  if(M5.BtnA.wasPressed()){
    if(sound_mode == true){
      sound_mode = false;
      millisec = 0;
    }
  }
}