/*
  指纹识别模块测试程序

  更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

  Author     : YFROBOT ZL
  Website    : www.yfrobot.com.cn
  Create Time: 2023-05-30
*/

#include <SoftwareSerial.h>
#include "yfrobot_fpm383.h"

SoftwareSerial mySerial(9, 8);  //软串口引脚，RX：D9    TX：D8
YFROBOTFPM383 fpm(&mySerial);
int MODEPIN = 3;
int LEDPIN = 13;

int ENROLLID_1 = 1;  // 在此位置注册指纹
int flag_enroll = 255;

int state_MODEPIN_old = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);

  // 初始化
  while (fpm.getChipSN() == "") {
    Serial.println("等待......");
    delay(200);  //等待指纹识别模块初始化完成
  }
  Serial.println(fpm.getChipSN());

  //清空指纹
  // while (fpm.empty() != 0x00) {
  //   Serial.println("清除指纹失败");
  //   delay(1000);
  // }
  // Serial.println("清除指纹成功");
  // delay(200);
  Serial.println("开始");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(MODEPIN)) {  // 读取触摸模块引脚确定工作模式
    int id = fpm.identify();  //验证指纹并获取指纹ID
    if (id == 0) {
      Serial.println("识别到指纹0");
      digitalWrite(LEDPIN, HIGH);
    } else if (id == 1) {
      Serial.println("识别到指纹1");
      digitalWrite(LEDPIN, LOW);
    }
    delay(1000);
  } else {
    if (flag_enroll == 255) {
      Serial.print("注册指纹，在ID:");
      Serial.print(ENROLLID_1);
      Serial.println("位置");
      Serial.println("当前有10秒钟的时间，将需要注册的手指按压在模块上4次！");
      flag_enroll = fpm.enroll(ENROLLID_1);
    } else if (flag_enroll == 00) {
      Serial.print("注册ID ");
      Serial.print(ENROLLID_1);
      Serial.println("，成功！");
      flag_enroll = 01;  //需要注册，请重新进入
    }
    delay(1000);
  }
}
