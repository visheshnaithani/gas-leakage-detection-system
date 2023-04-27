#include <MQ2.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
//I2C pins declaration
LiquidCrystal_I2C lcd(0x27, 16, 2);
int Analog_Input = A0;
int lpg, co, smoke, smokeper;
String data = "";
const int buzzer = 9;
MQ2 mq2(Analog_Input);
void setup() {
  Serial.begin(9600);
  lcd.begin();  //setup for the 16x2 LCD display
  lcd.backlight();
  mq2.begin();
  pinMode(buzzer, OUTPUT);
}
void loop() {
  float* values = mq2.read(false);
  lpg = mq2.readLPG();
  co = mq2.readCO();
  smoke = mq2.readSmoke();
  smokeper = (smoke * 100) / 1000000;
  data = String(lpg) + "," + String(co) + "," + String(smokeper) + ",";
  if (lpg > 30 || lpg < 0) {
    tone(buzzer, 2000, 6000);
    data += "1";
  }
  else{
    data += "0";
  }
  lcd.setCursor(0, 0);
  lcd.print("LPG:");
  lcd.print(lpg);
  lcd.print(" CO:");
  lcd.print(co);
  lcd.setCursor(0, 1);
  lcd.print("SMOKE:");
  lcd.print((smoke * 100) / 1000000);
  lcd.print(" %");
  Serial.println(data);
  delay(1000);
}
