# Yfrobot-FPM383-Library
YFROBOT FPM383 Sensor Library for Arduino

![](./assets/Fingerprint_Identification_Sensor.png)

## 相关链接 Links

* 产品购买链接: [YFROBOT商城](https://www.yfrobot.com/)、[YFROBOT淘宝商城](https://yfrobot.taobao.com/).

#### 库使用说明：
调用库：

`#include <yfrobot_fpm383.h>   // 包含头文件`

创建对象：

`SoftwareSerial mySerial(9, 8);  //软串口引脚，RX：D9    TX：D8
YFROBOTFPM383 fpm(&mySerial);`

Methods：

初始化，成功则返回1。

`fpm.begin();`

读取传感器，lineSensor值可为 0、1、2、3、4；分别对应5路传感器；lineSensor值可为 0、1、2、3、4、5、6；分别对应7路传感器；
有反射返回值为1，无反射返回值0。

`fpm.identify()`

禁用/使能传感器，默认使能。

`YFLS.enableSensor();`

`YFLS.disableSensor();`


## 更新日志 Release Note
* V0.0.1  基础功能完成

## 联系我们 Contact Us
* http://www.yfrobot.com.cn/wiki/index.php?title=%E8%81%94%E7%B3%BB%E6%88%91%E4%BB%AC

## 其他扩展库 Other extension libraries

## 参考 Reference Resources