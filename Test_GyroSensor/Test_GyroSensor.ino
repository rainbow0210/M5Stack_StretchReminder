// https://logikara.blog/3_axis_monitor/
// 振動センサのデータをシリアル出力するプログラム。ArduinoIDEのシリアルプロッタでグラフとして確認します。※液晶表示は何も表示されません。
#define M5STACK_MPU6886     // ヘッダーファイルをincludeする前に、IMUモジュールを#defineしておく
#include <M5Core2.h>   // ヘッダーファイル　※AtomMatrixは <M5Atom.h> ／ CPlusは <M5StickCPlus.h> ／ GRAYは <M5Stack.h>
// 変数宣言
float accX, accY, accZ;     // 加速度格納用
float gyroX, gyroY, gyroZ;  // 角速度格納用
float pitch, roll, yaw;     // 姿勢角格納用
int mode = 0;               // 測定モード選択用
// 初期設定 ----------------------------------------------------------
void setup(){
  M5.begin();         // 本体初期化
  Serial.begin(9600); // シリアル出力初期化
  M5.IMU.Init();                      // 6軸センサ初期化
  M5.IMU.SetAccelFsr(M5.IMU.AFS_8G);  // 加速度センサースケール初期値設定 ±8G(2,4,8,16) ※GRAYは「setAccelFsr」（先頭のsが小文字）
}
// メイン処理 --------------------------------------------------------
void loop() {
  M5.update();  // ボタン状態更新

  M5.IMU.getAccelData(&accX, &accY, &accZ);   // 加速度データ取得
  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ); // 角速度データ取得
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);    // 姿勢角データ取得

  float x_angle = atan2(accX, accZ) * 180.0 / PI; // X-Z加速度から角度に換算
  float y_angle = atan2(accY, accZ) * 180.0 / PI; // Y-Z加速度から角度に換算

  // ボタンONで測定モード切り替え
  if (M5.BtnA.wasPressed()) { mode++; } // ※Atom Matrixは「M5.Btn.」
  if (mode == 4) { mode = 0; }
  switch (mode) { // modeによって出力データを切り替え
  case 0:
    Serial.println("accelX, Y, Z");     // 加速度項目
    Serial.printf("%7.2f, %7.2f, %7.2f\n", accX, accY, accZ);   // 加速度

    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("%7.2f, %7.2f, %7.2f\n", accX, accY, accZ);

    break;
  case 1:
    Serial.println("gyroX, Y, Z");      // ジャイロ項目
    Serial.printf("%7.2f, %7.2f, %7.2f\n", gyroX, gyroY, gyroZ);// ジャイロ

    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("%7.2f, %7.2f, %7.2f\n", gyroX, gyroY, gyroZ);
    
    break;
  case 2:
    Serial.println("pitch, roll, yaw"); // 姿勢角
    Serial.printf("%7.2f, %7.2f, %7.2f\n", pitch, roll, yaw);   // 姿勢角

    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("%7.2f, %7.2f, %7.2f\n", pitch, roll, yaw);  

    break;
  case 3:
    Serial.println("AngleX,Y");         // 角度項目
    Serial.printf("%5.1f, %5.1f\n", x_angle, y_angle);  // 加速度から換算した角度

    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("%5.1f, %5.1f\n", x_angle, y_angle);

    break;
  }
  delay(100);
}