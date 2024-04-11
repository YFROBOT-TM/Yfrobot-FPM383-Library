/*
  指纹识别模块测试程序
  查询已注册指纹数量

  更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

  Author     : YFROBOT ZL
  Website    : www.yfrobot.com.cn
  update Time: 2024-04-11
*/

#include "yfrobot_fpm383.h"

YFROBOTFPM383 fpm(9, 8);  //软串口引脚，RX：D9    TX：D8

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // 初始化
  while (fpm.getChipSN() == "") {
    Serial.println("等待......");
    delay(200);  //等待指纹识别模块初始化完成
  }
  Serial.println(fpm.getChipSN());

  Serial.print("已注册指纹数量：");
  Serial.println(fpm.inquiry()); // 查询当前已注册指纹数量，最大容量50枚

  Serial.println("开始");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
}
