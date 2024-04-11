/*
  指纹识别模块测试程序
  指纹识别，指纹验证

  更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

  Author     : YFROBOT ZL
  Website    : www.yfrobot.com.cn
  update Time: 2024-04-11
*/

#include "yfrobot_fpm383.h"

YFROBOTFPM383 fpm(9, 8);  //软串口引脚，RX：D9    TX：D8
int MODEPIN = 3;
int LEDPIN = 13;

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

  Serial.println("开始");
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*验证指纹工作流程：
        1、无手指时，闪烁红绿色（黄色）灯一次
        2、搜索到未认证手指指纹，闪烁红色灯两次
        3、搜索未认证手指指纹，解析并校验数据，正常则返回指纹ID，并闪烁绿灯两次
  */
  int id = fpm.identify(false);  //验证指纹并获取指纹ID，参数true 在无手指状态是否有LED灯提示，您可以false关掉指示灯
  if (id == 0) {
    Serial.println("识别到指纹0");
    digitalWrite(LEDPIN, HIGH);
  } else if (id == 1) {
    Serial.println("识别到指纹1");
    digitalWrite(LEDPIN, LOW);
  }
  delay(1000);  // 推荐>500ms
}
