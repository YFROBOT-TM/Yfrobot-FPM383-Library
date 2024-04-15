/******************************************************************************
  yfrobot_fpm383.cpp
  YFROBOT FPM383 Sensor Library Source File
  Update Date: 04-11-2024
  @ YFROBOT

  Distributed as-is; no warranty is given.
******************************************************************************/

#include "yfrobot_fpm383.h"


YFROBOTFPM383::YFROBOTFPM383(int rxPin, int txPin)
{
    this->_pin_rx = rxPin;
    this->_pin_tx = txPin;

#ifdef __AVR__    // AVR 软件串口库 自定义引脚
    // 初始化软件串口通信设置
    // 这可能包括设置引脚为输入/输出，初始化通信速率等
    pinMode(this->_pin_rx, INPUT);
    pinMode(this->_pin_tx, OUTPUT);
    _ss = new SERIAL_CLASS(this->_pin_rx,  this->_pin_tx);
    _ss->begin(57600);

#elif defined(ESP32)    // ESP32 硬件串口2 自定义引脚
    // Serial.begin(9600);

    // 初始化 ESP32 的硬件串口2
    _ss = &Serial2;
    // 设置引脚为输入输出模式
    pinMode(this->_pin_rx, INPUT);
    pinMode(this->_pin_tx, OUTPUT);
    // 开始串口通信
    _ss->begin(57600, SERIAL_8N2, this->_pin_rx, this->_pin_tx);
    // Serial.println("Serial2 init success");
#endif

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
  * @brief   模块LED灯用户自由控制控制函数
  * @param   fun：LED 灯模式控制位，1-普通呼吸灯，2-闪烁灯，3-常开灯，4-常闭灯，5-渐开灯，6-渐闭灯，其他功能码不适用于此指令包格式；
  * @param   start：设置为普通呼吸灯时，由灭到亮的颜色，只限于普通呼吸灯（功能码 01）功能，其他功能时，与结束颜色保持一致。
  *                 其中，bit0 是蓝灯控制位；bit1 是绿灯控制位；bit2 是红灯控制位。置 1 灯亮，置 0 灯灭。
  *                 例如 0x01_蓝灯亮，0x02_绿灯亮，0x04_红灯亮，0x06_红绿灯亮，0x05_红蓝灯亮，0x03_绿蓝灯亮，0x07_红绿蓝灯亮，0x00_全灭；
  * @param   end：结束颜色：设置为普通呼吸灯时，由亮到灭的颜色，只限于普通呼吸灯（功能码 0x01），其他功能时，与起始颜色保持一致。设置方式与起始颜色一样；
  * @param   cycle：循环次数：表示呼吸或者闪烁灯的次数。当设为 0 时，表示无限循环，当设为其他值时，表示呼吸有限次数。
  *                 循环次数适用于呼吸、闪烁功能，其他功能中无效，例如在常开、常闭、渐开和渐闭中是无效的；
  * @return  None
  */
void YFROBOTFPM383::controlLEDC( uint8_t fun, uint8_t start, uint8_t end, uint8_t cycle )
{
    uint16_t checksum = 0x44;
    PS_CustomLEDBuffer[10] = (fun);
    checksum += fun;
    PS_CustomLEDBuffer[11] = (start);
    checksum += start;
    PS_CustomLEDBuffer[12] = (end);
    checksum += end;
    PS_CustomLEDBuffer[13] = (cycle);
    checksum += cycle;
    PS_CustomLEDBuffer[14] = checksum >> 8;
    PS_CustomLEDBuffer[15] = checksum;
    // for(int i = 10;i<=15;i++){
    //     Serial.println(PS_CustomLEDBuffer[i]);
    // }
    sendData(16, PS_CustomLEDBuffer);
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
  * @param   entriesCount：录入（拼接）次数，取值1~12，推荐4~6
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t * YFROBOTFPM383::autoEnroll(uint16_t PageID, uint8_t entriesCount)
{
    static uint8_t backData[3] = {0xFF,0xFF,0xFF};
    uint8_t eC = entriesCount > 12 ? 12 : entriesCount;
    PS_AutoEnrollBuffer[10] = (PageID>>8);
    PS_AutoEnrollBuffer[11] = (PageID);
    PS_AutoEnrollBuffer[12] = (eC);
    PS_AutoEnrollBuffer[15] = (PS_AutoEnrollBuffer_Check+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11]+PS_AutoEnrollBuffer[12])>>8;
    PS_AutoEnrollBuffer[16] = (PS_AutoEnrollBuffer_Check+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11]+PS_AutoEnrollBuffer[12]);
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
uint8_t YFROBOTFPM383::enroll(uint16_t PageID, uint8_t entriesCount)
{   
    controlLED(PS_BlueLEDBuffer); // 点亮蓝灯，注册开始
    delay(10);
    uint8_t *confirmationCode = autoEnroll(PageID, entriesCount);
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
        controlLED(PS_RedLEDLOOPBuffer);
        return 0x01; // 该ID已注册指纹循环闪烁红灯
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
uint8_t YFROBOTFPM383::identify(bool NoFingerLED)
{
    if(getImage() == 0x00) {
        if(getChar() == 0x00) {
            uint8_t sMB = searchMB();
            // Serial.println(sMB);
            if(sMB == 0x00) { // 搜索到认证手指时，解析返回数据
                // uint8_t PackageID = PS_ReceiveBuffer[6];
                if(PS_ReceiveBuffer[6] == 0x07) {
                    // static uint8_t confirmationCode = PS_ReceiveBuffer[9];
                    if(PS_ReceiveBuffer[9] == 0x00) {     // 返回数据校验正确，则识别正常，返回指纹ID并闪烁绿灯两次
                        controlLED(PS_GreenLEDBuffer);
                        // int SearchID = (int)((PS_ReceiveBuffer[10] << 8) + PS_ReceiveBuffer[11]);
                        int SearchID = (int)(PS_ReceiveBuffer[11]);
                        return SearchID; // 此模组最大支持49个指纹库，所以直接返回第11位码即可；ID码有2字节
                    }
                }
            } else if (sMB == 0x17){
                // controlLED(PS_BlueLEDBuffer);
                // return 0x17;
            } else { // 搜索到未认证手指时，闪烁红灯两次
                controlLED(PS_RedLEDBuffer);
            }
        }
    } else if(getImage() == 0x02) { // 无手指时，NoFingerLED == true 闪烁红绿色灯一次
        if ( NoFingerLED )
        {
            controlLED(PS_RGLEDBlinkBuffer);
        }
    }

    return 0xFF;
}


/**
  * @brief   读取有效模板个数，查询当前已注册指纹数量
  * @param   None
  * @return  应答包第11位有效数量或者无效值0xFF
  */
uint8_t YFROBOTFPM383::inquiry()
{
    sendData(12, PS_ValidTempleteNumBuffer);
    receiveData(2000);
    return PS_ReceiveBuffer[9] == 0x00 ? PS_ReceiveBuffer[11] : 0xFF;
}


// /**
//   * @brief   获取搜索指纹ID
//   * @param   ACK：各个功能函数返回的应答包
//   * @return  None
//   */
// uint8_t YFROBOTFPM383::getSearchID(uint8_t ACK)
// {
// 	if (ACK == 0x00)
//     {
//         int SearchID = (int)((PS_ReceiveBuffer[10] << 8) + PS_ReceiveBuffer[11]);
//         // sprintf(str,"Now Search ID: %d",(int)SearchID);
//         // Blinker.notify((int)SearchID);
//         // if(SearchID == 0) WiFi_Connected_State = 0;
//         return SearchID;
//     } else {

//     }
//     return 0xFF;
// }


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