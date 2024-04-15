/******************************************************************************
  yfrobot_fpm383.h
  YFROBOT FPM383 Sensor Library Source File
  Update Date: 04-11-2024
  @ YFROBOT

  Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef YFROBOTFPM383_H
#define YFROBOTFPM383_H

#include "Arduino.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#include <SoftwareSerial.h>
#define SERIAL_CLASS SoftwareSerial
#elif defined(ESP32)
#include <pgmspace.h>
#include <HardwareSerial.h>
#define SERIAL_CLASS HardwareSerial
#endif

#define RECEIVE_TIMEOUT_VALUE 1000 // Timeout for I2C receive

class YFROBOTFPM383
{
  private:
    
    uint8_t PS_ReceiveBuffer[50];  //串口接收数据的临时缓冲数组

    /********************************************** 指纹模块 指令集 ************************************************/
    //指令/命令包格式：                  包头0xEF01  设备地址4bytes 包标识1byte 包长度2bytes 指令码1byte  参数1......参数N  校验和2bytes
    
    //获取芯片唯一序列号 0x34。确认码=00H 表示 OK；确认码=01H 表示收包有错。
    uint8_t PS_GetChipSN[13]        = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x34, 0x00, 0x00, 0x39 };
    //获取模组基本参数 0x0F，读取模组的基本参数（波特率，包大小等）。参数表前 16 个字节存放了模组的基本通讯和配置信息，称为模组的基本参数。
    uint8_t PS_ReadSysPara[12]      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0F, 0x00, 0x13 };

    //验证用获取图像 0x01，验证指纹时，探测手指，探测到后录入指纹图像存于图像缓冲区。返回确认码表示：录入成功、无手指等。
    uint8_t PS_GetImageBuffer[12]   = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05 };
    // 生成特征 0x02，将图像缓冲区中的原始图像生成指纹特征文件存于模板缓冲区。
    uint8_t PS_GetCharBuffer[13]    = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08 };
    // uint8_t PS_GetChar2Buffer[13]   = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09 };
    // 搜索指纹 0x04，以模板缓冲区中的特征文件搜索整个或部分指纹库。若搜索到，则返回页码。加密等级设置为 0 或 1 情况下支持此功能。
    uint8_t PS_SearchMBBuffer[17]   = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0x0C };
    // 删除指纹 0x0C，删除 flash 数据库中指定 ID 号开始的1 个指纹模板。//PageID: bit 10:11，SUM: bit 14:15
    uint8_t PS_DeleteBuffer[16]     = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, '\0', '\0', 0x00, 0x01, '\0', '\0' };
    // 清空指纹库 0x0D，删除 flash 数据库中所有指纹模板。
    uint8_t PS_EmptyBuffer[12]      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0D, 0x00, 0x11 };
    // 读模组参数 0x0F，读取模组的基本参数（波特率，包大小等）。参数表前 16 个字节存放了模组的基本通讯和配置信息，称为模组的基本参数。
    uint8_t PS_ReadSysParaBuffer[12]      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0F, 0x00, 0x13 };
    // 读有效模板个数 0x1D，读有效模板个数。
    uint8_t PS_ValidTempleteNumBuffer[12] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D, 0x00, 0x21 };

    // 取消指令 0x30，取消自动注册模板和自动验证指纹。加密等级设置为 0 或 1 情况下支持此功能。
    uint8_t PS_CancelBuffer[12]     = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x30, 0x00, 0x34 };
    // 自动注册 0x31，一站式注册指纹，包含采集指纹、生成特征、组合模板、存储模板等功能。加密等级设置为 0 或 1 情况下支持此功能。
    // 指令说明                                                              标识  长      度  指令   i   d   号  次数   参     数   校     验
    uint8_t PS_AutoEnrollBuffer[17] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x31, '\0', '\0', '\0', 0x00, 0x17, '\0', '\0' };
    // 自动注册，
    // 录入次数：1~12，推荐值4~6
    // 参数说明：最低位为 bit0。
    //     1) bit0：采图背光灯控制位，0-LED 长亮，1-LED 获取图像成功后灭；
    //     2) bit1：采图预处理控制位，0-关闭预处理，1-打开预处理；
    //     3) bit2：注册过程中，是否要求模组在关键步骤，返回当前状态，0-要求返回，1-不要求返回；
    //     4) bit3：是否允许覆盖 ID 号，0-不允许，1-允许；
    //     5) bit4：允许指纹重复注册控制位，0-允许，1-不允许；
    //     6) bit5：注册时，多次指纹采集过程中，是否要求手指离开才能进入下一次指纹图 像采集， 0-要求离开；1-不要求离开；
    //     7) bit6~bit15：预留。
    // 当前值为 0x17，0001 0111；灯光获取成功后熄灭，打开预处理，不要求返回状态，不允许覆盖ID，不允许重复注册，多次采集手指需要离开


    // 校验和预处理
    uint8_t PS_AutoEnrollBuffer_Check = PS_AutoEnrollBuffer[6] +PS_AutoEnrollBuffer[7] +PS_AutoEnrollBuffer[8] +PS_AutoEnrollBuffer[9] +
    PS_AutoEnrollBuffer[12] +PS_AutoEnrollBuffer[13] +PS_AutoEnrollBuffer[14];

    // 休眠指令 0x33，设置传感器进入休眠模式。
    uint8_t PS_SleepBuffer[12]      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x33, 0x00, 0x37 };
    // LED控制灯指令  0x3C，控制灯指令主要分为两类：一般指示灯和七彩编程呼吸灯。
    
    // 指令说明
    // 功能码：LED 灯模式控制位，1-普通呼吸灯，2-闪烁灯，3-常开灯，4-常闭灯，5-渐开灯，6-渐闭灯，其他功能码不适用于此指令包格式；
    // 起始颜色：设置为普通呼吸灯时，由灭到亮的颜色，只限于普通呼吸灯（功能码 01）功能，其他功能时，与结束颜色保持一致。
    //      其中，bit0 是蓝灯控制位；bit1 是绿灯控制位；bit2 是红灯控制位。置 1 灯亮，置 0 灯灭。
    //      例如 0x01_蓝灯亮，0x02_绿灯亮，0x04_红灯亮，0x06_红绿灯亮，0x05_红蓝灯亮，0x03_绿蓝灯亮，0x07_红绿蓝灯亮，0x00_全灭；
    // 结束颜色：设置为普通呼吸灯时，由亮到灭的颜色，只限于普通呼吸灯（功能码 0x01），其他功能时，与起始颜色保持一致。设置方式与起始颜色一样；
    // 循环次数：表示呼吸或者闪烁灯的次数。当设为 0 时，表示无限循环，当设为其他值时，
    // 表示呼吸有限次数。循环次数适用于呼吸、闪烁功能，其他功能中无效，例如在常开、常闭、渐开和渐闭中是无效的；
    // 指令说明                                                                  标识  长      度   指令  功能   起始  结束  循环   校     验
    uint8_t PS_COLORLEDBuffer[16]       = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x01, 0x02, 0x01, 0x02, 0x00, 0x4A }; // 呼吸灯 绿到蓝色
    uint8_t PS_BlueLEDBuffer[16]        = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x03, 0x01, 0x01, 0x00, 0x00, 0x49 }; // 常开 蓝色
    uint8_t PS_RGLEDBlinkBuffer[16]     = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x06, 0x06, 0x01, 0x00, 0x53 }; // 闪烁1次 红绿色
    uint8_t PS_RedLEDBuffer[16]         = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x04, 0x04, 0x02, 0x00, 0x50 }; // 闪烁2次 红色
    uint8_t PS_RedLEDLOOPBuffer[16]     = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x04, 0x04, 0x00, 0x00, 0x4E }; // 循环闪烁闪烁 红色
    uint8_t PS_GreenLEDBuffer[16]       = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x02, 0x02, 0x02, 0x00, 0x4C }; // 闪烁2次 绿色

    uint8_t PS_OFFLEDBuffer[16]         = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x48 }; // 全灭

    uint8_t PS_CustomLEDBuffer[16]      = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, '\0', '\0', '\0', '\0', '\0', '\0' }; // 自由控制LED
    // 更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

    void sendData(int len, uint8_t PS_Databuffer[]);
    void receiveData(uint16_t Timeout);
    String HexToString(uint8_t* data, uint8_t length);

  public:
    // -----------------------------------------------------------------------------
    // Constructor - YFROBOTFPM383
    // -----------------------------------------------------------------------------
    // YFROBOTFPM383(SERIAL_CLASS *serial, int rxPin, int txPin);
    YFROBOTFPM383(int rxPin, int txPin);
    SERIAL_CLASS *_ss;
    // SoftwareSerial *_ss;
    int _pin_rx;			//RX pin
    int _pin_tx;			//TX pin

    bool begin();
    String getChipSN();
    void sleep();
    void controlLED(uint8_t PS_ControlLEDBuffer[]);
    void controlLEDC( uint8_t fun, uint8_t start, uint8_t end, uint8_t cycle );
    uint8_t cancel();
    uint8_t getImage();
    uint8_t getChar();
    uint8_t searchMB();
    uint8_t empty();
    uint8_t * autoEnroll(uint16_t PageID, uint8_t entriesCount);
    uint8_t deleteID(uint16_t PageID);
    uint8_t enroll(uint16_t PageID, uint8_t entriesCount);
    uint8_t identify(bool NoFingerLED);
    // uint8_t getSearchID(uint8_t ACK);
    // void ENROLL_ACK_CHECK(uint8_t ACK);
    uint8_t inquiry(); // 查询已注册数量

};

#endif // YFROBOTFPM383_H
