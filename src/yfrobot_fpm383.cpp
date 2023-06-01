/******************************************************************************
  yfrobot_fpm383.cpp
  YFROBOT FPM383 Sensor Library Source File
  Creation Date: 05-31-2023
  @ YFROBOT

  Distributed as-is; no warranty is given.
******************************************************************************/

#include "yfrobot_fpm383.h"

YFROBOTFPM383::YFROBOTFPM383(SoftwareSerial *softSerial)
{
    _ss = softSerial;
    _ss->begin(57600);
}

/**
  * @brief   串口发送函数
  * @param   len: 发送数组长度
  * @param   PS_Databuffer[]: 需要发送的功能协议数组，在上面已有定义
  * @return  None
  */
void YFROBOTFPM383::sendData(int len, uint8_t PS_Databuffer[]) {
    _ss->write(PS_Databuffer, len);
    while (_ss->read() >= 0)
        ;
}

/**
  * @brief   串口接收函数
  * @param   Timeout：接收超时时间
  * @return  None
  */
void YFROBOTFPM383::receiveData(uint16_t Timeout) {
    memset(PS_ReceiveBuffer, 0xFF, sizeof(PS_ReceiveBuffer));  // 清空接收数据变量
    uint8_t i = 0;
    while (_ss->available() == 0 && (--Timeout)) {
        delay(1);
    }
    while (_ss->available() > 0) {
        delay(1);
        PS_ReceiveBuffer[i++] = _ss->read();
        if (i > 50) break;  // 超出50字节数据不接收
    }
}

/**
  * @brief   初始化，接收到模组数据0x55则表示初始化完成 (无用，模块只有在首次上电时会发送0x55，实际使用延时等待模组初始化即可)
  * @param   None
  * @return  bool
  */
bool YFROBOTFPM383::begin()
{
    receiveData(1000);
    if(PS_ReceiveBuffer[0] == 0x55){
        return true;
    }
    return false;
}

/**
  * @brief   等待初始化，并获取模组型号，建议在setup中使用
  * @param   None
  * @return  bool
  */
String YFROBOTFPM383::getChipSN()
{
    delay(200);  //等待指纹识别模块初始化完成，不可去掉，此期间不能响应命令
    sendData(13, PS_GetChipSN);
    receiveData(1000);
    if( PS_ReceiveBuffer[6] == 0x07 && PS_ReceiveBuffer[9] == 0x00 ) {
        uint8_t CSN[10];
        for(int i = 0; i < 9; i++){
            CSN[i] =  PS_ReceiveBuffer[i+10];
        }
        return HexToString(CSN, 9);
    }
    return "";
}

String YFROBOTFPM383::HexToString(uint8_t* data, uint8_t length) {
    if (!data || !length) { return ""; }
    String result = "";
    for (uint8_t i = 0; i < length; i++) {
        // uint8_t my_hex = 0x50;
        // String my_char = String((char)my_hex) //output "P"
        result += String((char)data[i]) ;
    }
    return result;
}

/**
  * @brief   休眠函数，只有发送休眠后，模块的TOUCHOUT引脚才会变成低电平
  * @param   None
  * @return  None
  */
void YFROBOTFPM383::sleep()
{
    sendData(12, PS_SleepBuffer);
}

/**
  * @brief   模块LED灯控制函数
  * @param   PS_ControlLEDBuffer[]：需要设置颜色的协议，一般定义在上面
  * @return  None
  */
void YFROBOTFPM383::controlLED( uint8_t PS_ControlLEDBuffer[] )
{
    sendData(16, PS_ControlLEDBuffer);
}

/**
  * @brief   模块任务取消操作函数，如发送了注册指纹命令，但是不想注册了，需要发送此函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::cancel()
{
    sendData(12, PS_CancelBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   模块获取搜索指纹用的图像函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::getImage()
{
    sendData(12, PS_GetImageBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   模块获取图像后生成特征，存储到缓冲区1
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::getChar()
{
    sendData(13, PS_GetCharBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   搜索指纹模板函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::searchMB()
{
    sendData(17, PS_SearchMBBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   删除指定指纹模板函数
  * @param   PageID：需要删除的指纹ID号，取值0 - 49（FPM383F）
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::deleteID(uint16_t PageID)
{
    PS_DeleteBuffer[10] = (PageID>>8);
    PS_DeleteBuffer[11] = (PageID);
    PS_DeleteBuffer[14] = (0x15+PS_DeleteBuffer[10]+PS_DeleteBuffer[11])>>8;
    PS_DeleteBuffer[15] = (0x15+PS_DeleteBuffer[10]+PS_DeleteBuffer[11]);
    sendData(16, PS_DeleteBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   清空指纹模板函数 
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::empty()
{
    sendData(12, PS_EmptyBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   自动注册指纹模板函数, 默认采集4次
  * @param   PageID：注册指纹的ID号，取值0 - 49（FPM383F）
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t * YFROBOTFPM383::autoEnroll(uint16_t PageID)
{
    static uint8_t backData[3] = {0xFF,0xFF,0xFF};
    PS_AutoEnrollBuffer[10] = (PageID>>8);
    PS_AutoEnrollBuffer[11] = (PageID);
    PS_AutoEnrollBuffer[15] = (PS_AutoEnrollBuffer_Check+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11])>>8;
    PS_AutoEnrollBuffer[16] = (PS_AutoEnrollBuffer_Check+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11]);
    sendData(17, PS_AutoEnrollBuffer);
    receiveData(10000);
    // return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
    if(PS_ReceiveBuffer[6] == 0x07){
        backData[0] = PS_ReceiveBuffer[9];
        backData[1] = PS_ReceiveBuffer[10];
        backData[2] = PS_ReceiveBuffer[11];
    }
    return backData;
}

/**
  * @brief   二次封装自动注册指纹函数，实现注册成功闪烁两次绿灯，失败闪烁两次红灯
  * @param   PageID：注册指纹的ID号，取值0 - 49（FPM383F）
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::enroll(uint16_t PageID)
{   
    controlLED(PS_BlueLEDBuffer); // 点亮蓝灯，注册开始
    delay(10);
    uint8_t *confirmationCode = autoEnroll(PageID);
    // Serial.print(confirmationCode[0]);
    // Serial.print(" ");
    // Serial.print(confirmationCode[1]);
    // Serial.print(" ");
    // Serial.println(confirmationCode[2]);
    if(confirmationCode[0] == 0x00 &&confirmationCode[1] == 0x06 &&confirmationCode[2] == 0xf2  ){
        controlLED(PS_GreenLEDBuffer); // 绿灯闪烁，注册成功
        Serial.println("ok");
        return 0x00;
    }else if(confirmationCode[0] == 0x22 &&confirmationCode[1] == 0x00 &&confirmationCode[2] == 0x00  ){
        controlLED(PS_OFFLEDBuffer);
        return 0x01;
    } else {
        controlLED(PS_OFFLEDBuffer);
        return 0xff;
    }
}

/**
  * @brief   分步式命令搜索指纹函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::identify()
{
    if(getImage() == 0x00) {
        if(getChar() == 0x00) {
            uint8_t sMB = searchMB();
            // Serial.println(sMB);
            if(sMB == 0x00) {
                // uint8_t PackageID = PS_ReceiveBuffer[6];
                if(PS_ReceiveBuffer[6] == 0x07) {
                    // static uint8_t confirmationCode = PS_ReceiveBuffer[9];
                    if(PS_ReceiveBuffer[9] == 0x00) {
                        controlLED(PS_GreenLEDBuffer);
                        // int SearchID = (int)((PS_ReceiveBuffer[10] << 8) + PS_ReceiveBuffer[11]);
                        int SearchID = (int)(PS_ReceiveBuffer[11]);
                        return SearchID; // 此模组最大支持49个指纹库，所以直接返回第11位码即可；ID码有2字节
                    }
                }
            } else if (sMB == 0x17){
                controlLED(PS_BlueLEDBuffer);
                // return 0x17;
            } else {
                controlLED(PS_RedLEDBuffer);
            }
        }
    } else if(getImage() == 0x02) { // 无手指
        controlLED(PS_COLORLEDBuffer);
    }

    return 0xFF;
}


/**
  * @brief   获取搜索指纹ID
  * @param   ACK：各个功能函数返回的应答包
  * @return  None
  */
uint8_t YFROBOTFPM383::getSearchID(uint8_t ACK)
{
	if (ACK == 0x00)
    {
        int SearchID = (int)((PS_ReceiveBuffer[10] << 8) + PS_ReceiveBuffer[11]);
        // sprintf(str,"Now Search ID: %d",(int)SearchID);
        // Blinker.notify((int)SearchID);
        // if(SearchID == 0) WiFi_Connected_State = 0;
        return SearchID;
    } else {

    }
    return 0xFF;
}


// /**
//   * @brief   注册指纹后返回的应答包校验
//   * @param   ACK：注册指纹函数返回的应答包
//   * @return  None
//   */
// void YFROBOTFPM383::ENROLL_ACK_CHECK(uint8_t ACK)
// {
//     if(PS_ReceiveBuffer[6] == 0x07)
//     {
//         switch (ACK)
//         {
//             case 0x00:                          //指令正确
//                 EnrollID = (int)((PS_AutoEnrollBuffer[10] << 8) + PS_AutoEnrollBuffer[11]);
//                 // sprintf(str,"Now Enroll ID: %d",(int)EnrollID);
//                 // Blinker.notify((int)EnrollID);
//                 break;
//         }
//     }
//     for(int i=0;i<20;i++) PS_ReceiveBuffer[i] = 0xFF;
// }