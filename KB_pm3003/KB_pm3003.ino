#include <stdio.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define MATRIX_ADDR 0x70    // แอดเดรสของ LED Matrix 8x16
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

#define RXD2 18
#define TXD2 19

#define LM73_ADDR 0x4D

#define NUMBER_OF_BYTE 23
char line1[16], line2[16];
unsigned char buffer [NUMBER_OF_BYTE];
int PM25 = 0, PM10 = 0;
int PM1, PM1a, PM25a, PM10a;

const int led_BT    =  17;  //  ขาใช้งาน BT led
const int led_WIFI  =  2;   //  ขาใช้งาน WIFI led
const int led_NTP   =  15;  //  ขาใช้งาน NTP led
const int led_IOT   =  12;  //  ขาใช้งาน IOT led

const int button_Pin_1 = 16;    //  ขาใช้งานสวิตซ์ 1
const int button_Pin_2 = 14;    //  ขาใช้งานสวิตซ์ 2

const int buzzer_Pin =  13;     //  ขาใช้งาน Buzzer

const int ldr_Pin = 36;         //  ขาใช้งาน LDR sensor

uint32_t pevTime = 0;
const long interval = 10 * 1000;

int read_LDR() {
  const int ldr_Low = 0;      //  ค่าน้อยที่สุดที่อ่านได้จาก LDR sensor
  const int ldr_HIGH = 500;   //  ค่ามากที่สุดที่อ่านได้จาก LDR sensor
  const int ldr_Light = 100;  //  กำหนดระดับความสว่างสูงสุด
  const int ldr_Dark = 0;     //  กำหนดระดับความสว่างต่ำสุด
  // map(value, fromLow, fromHigh, toLow, toHigh)
  int light = map(analogRead(ldr_Pin), ldr_Low, ldr_HIGH, ldr_Light, ldr_Dark);
  return light;
}

float readTemperature() {
  Wire1.beginTransmission(LM73_ADDR);
  Wire1.write(0x00);
  Wire1.endTransmission();

  uint8_t count = Wire1.requestFrom(LM73_ADDR, 2);
  float temp = 0.0;
  if (count == 2) {
    byte buff[2];
    buff[0] = Wire1.read();
    buff[1] = Wire1.read();
    temp += (int)(buff[0] << 1);
    if (buff[1] & 0b10000000) temp += 1.0;
    if (buff[1] & 0b01000000) temp += 0.5;
    if (buff[1] & 0b00100000) temp += 0.25;
    if (buff[0] & 0b10000000) temp *= -1.0;
  }
  return temp;
}

bool checkValue(unsigned char *buf, int length)
{
  bool flag = 0;
  int sum = 0;

  for (int i = 0; i < (length - 2); i++)
  {
    sum += buf[i];
  }
  sum = sum + 0x42;

  if (sum == ((buf[length - 2] << 8) + buf[length - 1]))
  {
    sum = 0;
    flag = 1;
  }
  return flag;
}

void readPM3003() {
  char fel = 0x42;
  if (Serial1.find(&fel, 1)) {
    Serial1.readBytes(buffer, NUMBER_OF_BYTE);
  }

  if (buffer[0] == 0x4d)
  {
    if (checkValue(buffer, NUMBER_OF_BYTE))
    {
      PM25 = ((buffer[5] << 8) + buffer[6]);
      PM10 = ((buffer[7] << 8) + buffer[8]);

      // rest of values (if you want to use it)
      PM1 = ((buffer[3] << 8) + buffer[4]);
      PM1a = ((buffer[9] << 8) + buffer[10]);
      PM25a = ((buffer[11] << 8) + buffer[12]);
      PM10a = ((buffer[13] << 8) + buffer[14]);
    }
  }
}

void setup() {
  Wire1.begin(4, 5);
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(led_BT, OUTPUT);    //  กำหนดให้ led_BT เป็น OUTPUT
  pinMode(led_WIFI, OUTPUT);  //  กำหนดให้ led_WIFI เป็น OUTPUT
  pinMode(led_NTP, OUTPUT);   //  กำหนดให้ led_NTP เป็น OUTPUT
  pinMode(led_IOT, OUTPUT);   //  กำหนดให้ led_IOT เป็น OUTPUT

  digitalWrite(led_BT, HIGH);
  digitalWrite(led_WIFI, HIGH);
  digitalWrite(led_NTP, HIGH);
  digitalWrite(led_IOT, HIGH);

  pinMode(button_Pin_1, INPUT_PULLUP);
  pinMode(button_Pin_2, INPUT_PULLUP);

  matrix.begin(MATRIX_ADDR);
  matrix.setTextColor(LED_ON);
  matrix.setTextSize(1);
  matrix.setRotation(1);
  matrix.setTextWrap(false);

  ledcSetup(0, 5000, 13);
  ledcAttachPin(buzzer_Pin, 0);
  Serial.println("Dust sensor begin...");
}

void loop() {
  if (millis() - pevTime >= interval)
  {
    pevTime = millis();

    //    pm3003();
    readPM3003();
    
    Serial.print("pm1 atmosphere: "); Serial.println(PM1a);
    Serial.print("pm2.5 atmosphere: "); Serial.println(PM25a);
    Serial.print("pm10 atmosphere: "); Serial.println(PM10a);
    Serial.println();
    Serial.print("pm1: "); Serial.println(PM1);
    Serial.print("pm2.5: "); Serial.println(PM25);
    Serial.print("pm10: "); Serial.println(PM10);
    Serial.println();
    Serial.println();
    
    matrix.clear();
    matrix.setCursor(0, 0);
    matrix.print(PM25);
    matrix.writeDisplay();
  }
}
