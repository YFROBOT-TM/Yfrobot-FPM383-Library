/*
  指纹识别模块测试程序
  删掉指纹

  更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

  Author     : YFROBOT ZL
  Website    : www.yfrobot.com.cn
  update Time: 2024-04-11
*/

#include "yfrobot_fpm383.h"

YFROBOTFPM383 fpm(9, 8);  //软串口引脚，RX：D9    TX：D8

int ENROLLID_1 = 2;  // 在此位置注册指纹

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // 初始化
  while (fpm.getChipSN() == "") {
    Serial.println("等待......");
    delay(200);  //等待指纹识别模块初始化完成
  }
  Serial.println(fpm.getChipSN());
  
  //删除指定ID指纹
  while (fpm.deleteID(ENROLLID_1) != 0x00) {
    Serial.print("删除ID:");
    Serial.print(ENROLLID_1);
    Serial.println("指纹失败");
    delay(1000);
  }
    Serial.print("删除ID:");
    Serial.print(ENROLLID_1);
    Serial.println("指纹成功");
  delay(200);

  Serial.println(" ");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
}
