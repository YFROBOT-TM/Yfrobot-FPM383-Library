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

初始化，成功则返回模组序列号(String)，否则返回""。

`fpm.getChipSN();`

验证指纹，并返回指纹ID

`fpm.identify()`

在ID位置注册指纹

`fpm.enroll(ID);`


删除ID位置的指纹

`fpm.deleteID(ID);`

清空指纹库

`fpm.empty();`


## 更新日志 Release Note
* V0.0.2    优化LED控制，增加多种LED状态；
            优化验证指纹，增加无指纹时，LED灯控制参数；
            优化添加指纹，增加参数，可控指纹拼接次数；
            新增LED自由控制函数。
* V0.0.1    基础功能完成，验证指纹，删除指纹，添加指纹。

## 联系我们 Contact Us
* http://www.yfrobot.com.cn/wiki/index.php?title=%E8%81%94%E7%B3%BB%E6%88%91%E4%BB%AC

## 其他扩展库 Other extension libraries

## 参考 Reference Resources