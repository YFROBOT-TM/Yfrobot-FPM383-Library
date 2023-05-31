/*************************************************************
  lineFollowSensor_i2c.ino
  5路巡线传感器模块测试传感器，模块基于SX1508（I/O Expander），使用IIC
  通信读取5路传感器状态，实现巡线功能。
  Creation Date: 01-18-2022
  @ YFROBOT

  Hardware Hookup:
  LineFollow Breakout ------ Arduino
              GND -------------- GND
              VCC -------------- 5V
              SDA -------------- SDA (A4)
              SCL -------------- SCL (A5)

*************************************************************/

#include "YFLineFollow.h"

YFLINEFOLLOW YFLS;

// Pin definition:
const byte lineSensor1 = 0; // ir sensor to SX1508's pin 0
const byte lineSensor2 = 1; // ir sensor to SX1508's pin 1
const byte lineSensor3 = 2; // ir sensor to SX1508's pin 2
const byte lineSensor4 = 3; // ir sensor to SX1508's pin 3
const byte lineSensor5 = 4; // ir sensor to SX1508's pin 4

void setup()
{
  Serial.begin(115200);
  Serial.println("Line Follow Example");

  // Call xx.begin() to initialize the line follow sensor. If it successfully communicates, it'll return 1.
  if (YFLS.begin() == false) {
    Serial.println("Failed to communicate. Check wiring.");
    while (1) ; // If we fail to communicate, loop forever.
  }

  YFLS.enableSensor(); // 使能传感器，默认使能
}

void loop()
{
  uint8_t ls1 = YFLS.readSensor(lineSensor1);
  uint8_t ls2 = YFLS.readSensor(lineSensor2);
  uint8_t ls3 = YFLS.readSensor(lineSensor3);
  uint8_t ls4 = YFLS.readSensor(lineSensor4);
  uint8_t ls5 = YFLS.readSensor(lineSensor5);
  Serial.print(ls1);
  Serial.print("  ");
  Serial.print(ls2);
  Serial.print("  ");
  Serial.print(ls3);
  Serial.print("  ");
  Serial.print(ls4);
  Serial.print("  ");
  Serial.print(ls5);
//
  Serial.println();
  delay(200);
}
