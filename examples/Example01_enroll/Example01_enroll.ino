/*
  指纹识别模块测试程序
  添加注册指纹
   模块闪烁红灯，表示此为止已注册过指纹，请更换位置
   模块常亮蓝灯，表示可以注册指纹，默认拼接4次（可更改）
   绿灯闪烁即灭，表示注册完成

  更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

  Author     : YFROBOT ZL
  Website    : www.yfrobot.com.cn
  update Time: 2024-04-11
*/

#include "yfrobot_fpm383.h"

YFROBOTFPM383 fpm(9, 8);  //软串口引脚，RX：D9    TX：D8

int ENROLLID_1 = 0;  // 在此位置注册指纹
int flag_enroll = 255;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // 初始化
  while (fpm.getChipSN() == "") {
    Serial.println("等待......");
    delay(200);  //等待指纹识别模块初始化完成
  }
  Serial.println(fpm.getChipSN());

  Serial.println("开始");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (flag_enroll == 255) {
    Serial.print("注册指纹，在ID:");
    Serial.print(ENROLLID_1);
    Serial.println("位置");
    Serial.println("当前有10秒钟的时间，将需要注册的手指按压在模块上4次！");
    flag_enroll = fpm.enroll(ENROLLID_1);
    // flag_enroll = fpm.enroll(ENROLLID_1, 6); // 或者添加第二个参数，6（范围1~12） 拼接6次
  } else if (flag_enroll == 01) {
    Serial.println("该ID已注册指纹，请更换ID再注册！");
  } else if (flag_enroll == 00) {
    Serial.print("注册ID ");
    Serial.print(ENROLLID_1);
    Serial.println("，成功！");
    flag_enroll = 02;  // 需要注册，请重新进入
  }
  delay(1000);
}
