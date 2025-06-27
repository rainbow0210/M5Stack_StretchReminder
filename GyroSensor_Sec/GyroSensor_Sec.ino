// https://logikara.blog/3_axis_monitor/
// 振動センサのデータをシリアル出力するプログラム。ArduinoIDEのシリアルプロッタでグラフとして確認します。※液晶表示は何も表示されません。
#define M5STACK_MPU6886     // ヘッダーファイルをincludeする前に、IMUモジュールを#defineしておく
#include <M5Unified.h>   // ヘッダーファイル　※AtomMatrixは <M5Atom.h> ／ CPlusは <M5StickCPlus.h> ／ GRAYは <M5Stack.h>
#include <M5Core2.h>
// 変数宣言
float pitch, roll, yaw;     // 姿勢角格納用

unsigned long millisec;
unsigned long now_millisec;

int volume = 128;
bool sound_mode = false;

float first_pitch, first_roll, first_yaw;
int first_num = 0;

// 初期設定 ----------------------------------------------------------
void setup(){
  auto config = M5.config();
  M5.begin(config);// M5Coreの初期化
  M5.Speaker.begin();
  Serial.begin(9600); // シリアル出力初期化
  M5.IMU.Init();                      // 6軸センサ初期化
  M5.IMU.SetAccelFsr(M5.IMU.AFS_8G);  // 加速度センサースケール初期値設定 ±8G(2,4,8,16) ※GRAYは「setAccelFsr」（先頭のsが小文字）
}
// メイン処理 --------------------------------------------------------
void loop() {
  M5.update();  // ボタン状態更新

  M5.IMU.getAhrsData(&pitch, &roll, &yaw);    // 姿勢角データ取得

  float x_angle = atan2(accX, accZ) * 180.0 / PI; // X-Z加速度から角度に換算
  float y_angle = atan2(accY, accZ) * 180.0 / PI; // Y-Z加速度から角度に換算

  if(first_num == 0){
    first_pitch = pitch;
    first_roll = roll;
    first_yaw = yaw;

    first_num = 1;
  }

  Serial.println("pitch, roll, yaw"); // 姿勢角
  Serial.printf("%7.2f, %7.2f, %7.2f\n", pitch, roll, yaw);   // 姿勢角

  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("%7.2f, %7.2f, %7.2f\n", pitch, roll, yaw);  

  if(pitch <= first_pitch + 50 && pitch >= first_pitch - 50 || roll <= first_roll + 50 && roll >= first_roll - 50 || yaw <= first_yaw + 50 && yaw >= first_yaw - 50){
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
      first_num = 0;
    }
  }

  delay(100);
}