/******************************************************************************
  yfrobot_fpm383.cpp
  YFROBOT FPM383 Sensor Library Source File
  Creation Date: 05-31-2023
  @ YFROBOT

  Distributed as-is; no warranty is given.
******************************************************************************/

#include "Arduino.h"
#include "yfrobot_fpm383.h"

YFROBOTFPM383::YFROBOTFPM383(uint8_t address, uint8_t resetPin, uint8_t interruptPin, uint8_t oscillatorPin)
{
    // Store the received parameters into member variables
    deviceAddress = address;
    pinInterrupt = interruptPin;
    pinOscillator = oscillatorPin;
    pinReset = resetPin;
}

uint8_t YFROBOTFPM383::begin(uint8_t address, TwoWire &wirePort, uint8_t resetPin)
{
    // Store the received parameters into member variables
    _i2cPort = &wirePort;
    deviceAddress = address;
    pinReset = resetPin;

    return init();
}

void YFROBOTFPM383::reset(bool hardware)
{
    // if hardware bool is set
    if (hardware) {
        // Check if bit 2 of REG_MISC is set
        // if so nReset will not issue a POR, we'll need to clear that bit first
        uint8_t regMisc = readByte(REG_MISC);
        if (regMisc & (1 << 2)) {
            regMisc &= ~(1 << 2);
            writeByte(REG_MISC, regMisc);
        }
        // Reset the YFROBOTFPM383, the pin is active low
        pinMode(pinReset, OUTPUT);	  // set reset pin as output
        digitalWrite(pinReset, LOW);  // pull reset pin low
        delay(1);					  // Wait for the pin to settle
        digitalWrite(pinReset, HIGH); // pull reset pin back high
    } else {
        // Software reset command sequence:
        writeByte(REG_RESET, 0x12);
        writeByte(REG_RESET, 0x34);
    }
}

/**
  * @brief   串口发送函数
  * @param   len: 发送数组长度
  * @param   PS_Databuffer[]: 需要发送的功能协议数组，在上面已有定义
  * @return  None
  */
void YFROBOTFPM383::sendData(int len, uint8_t PS_Databuffer[]) {
    mySerial.write(PS_Databuffer, len);
    while (mySerial.read() >= 0)
        ;
    // memset(PS_ReceiveBuffer, 0xFF, sizeof(PS_ReceiveBuffer));
}

/**
  * @brief   串口接收函数
  * @param   Timeout：接收超时时间
  * @return  None
  */
void YFROBOTFPM383::receiveData(uint16_t Timeout) {
    uint8_t i = 0;
    while (mySerial.available() == 0 && (--Timeout)) {
        delay(1);
    }
    while (mySerial.available() > 0) {
        delay(2);
        PS_ReceiveBuffer[i++] = mySerial.read();
        if (i > 15) break;
    }
}

/**
  * @brief   休眠函数，只有发送休眠后，模块的TOUCHOUT引脚才会变成低电平
  * @param   None
  * @return  None
  */
void YFROBOTFPM383::sleep()
{
    FPM383C_SendData(12,PS_SleepBuffer);
}

/**
  * @brief   模块LED灯控制函数
  * @param   PS_ControlLEDBuffer[]：需要设置颜色的协议，一般定义在上面
  * @return  None
  */
void YFROBOTFPM383::controlLED(uint8_t PS_ControlLEDBuffer[])
{
    FPM383C_SendData(16,PS_ControlLEDBuffer);
}

/**
  * @brief   模块任务取消操作函数，如发送了注册指纹命令，但是不想注册了，需要发送此函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::cancel()
{
    FPM383C_SendData(12,PS_CancelBuffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   模块获取搜索指纹用的图像函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::getImage()
{
    FPM383C_SendData(12,PS_GetImageBuffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}

/**
  * @brief   模块获取图像后生成特征，存储到缓冲区1
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::getChar1()
{
    FPM383C_SendData(13,PS_GetChar1Buffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   生成特征，存储到缓冲区2
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::getChar2()
{
    FPM383C_SendData(13,PS_GetChar2Buffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   搜索指纹模板函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::searchMB()
{
    FPM383C_SendData(17,PS_SearchMBBuffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   清空指纹模板函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::empty()
{
    FPM383C_SendData(12,PS_EmptyBuffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   自动注册指纹模板函数
  * @param   PageID：注册指纹的ID号，取值0 - 59
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::autoEnroll(uint16_t PageID)
{
    PS_AutoEnrollBuffer[10] = (PageID>>8);
    PS_AutoEnrollBuffer[11] = (PageID);
    PS_AutoEnrollBuffer[15] = (0x54+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11])>>8;
    PS_AutoEnrollBuffer[16] = (0x54+PS_AutoEnrollBuffer[10]+PS_AutoEnrollBuffer[11]);
    FPM383C_SendData(17,PS_AutoEnrollBuffer);
    FPM383C_ReceiveData(10000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   删除指定指纹模板函数
  * @param   PageID：需要删除的指纹ID号，取值0 - 59
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::delete(uint16_t PageID)
{
    PS_DeleteBuffer[10] = (PageID>>8);
    PS_DeleteBuffer[11] = (PageID);
    PS_DeleteBuffer[14] = (0x15+PS_DeleteBuffer[10]+PS_DeleteBuffer[11])>>8;
    PS_DeleteBuffer[15] = (0x15+PS_DeleteBuffer[10]+PS_DeleteBuffer[11]);
    FPM383C_SendData(16,PS_DeleteBuffer);
    FPM383C_ReceiveData(2000);
    return PS_ReceiveBuffer[6] == 0x07 ? PS_ReceiveBuffer[9] : 0xFF;
}


/**
  * @brief   二次封装自动注册指纹函数，实现注册成功闪烁两次绿灯，失败闪烁两次红灯
  * @param   PageID：注册指纹的ID号，取值0 - 59
  * @return  应答包第9位确认码或者无效值0xFF
  */
/*，返回应答包的位9确认码。*/
uint8_t YFROBOTFPM383::enroll(uint16_t PageID)
{
    if(PS_AutoEnroll(PageID) == 0x00)
    {
        PS_ControlLED(PS_GreenLEDBuffer);
        return PS_ReceiveBuffer[9];
    }
    PS_ControlLED(PS_RedLEDBuffer);
    return 0xFF;
}


/**
  * @brief   分步式命令搜索指纹函数
  * @param   None
  * @return  应答包第9位确认码或者无效值0xFF
  */
uint8_t YFROBOTFPM383::identify()
{
    if(PS_GetImage() == 0x00)
    {
        if(PS_GetChar1() == 0x00)
        {
            if(PS_SearchMB() == 0x00)
            {
                if(PS_ReceiveBuffer[8] == 0x07 && PS_ReceiveBuffer[9] == 0x00)
                {
                    PS_ControlLED(PS_GreenLEDBuffer);
                    return PS_ReceiveBuffer[9];
                }
            }
        }
    }
    ErrorNum++;
    PS_ControlLED(PS_RedLEDBuffer);
    return 0xFF;
}


/**
  * @brief   搜索指纹后的应答包校验，在此执行相应的功能，如开关继电器、开关灯等等功能
  * @param   ACK：各个功能函数返回的应答包
  * @return  None
  */
void YFROBOTFPM383::SEARCH_ACK_CHECK(uint8_t ACK)
{
	if(PS_ReceiveBuffer[6] == 0x07)
	{
		switch (ACK)
		{
			case 0x00:                          //指令正确
                SearchID = (int)((PS_ReceiveBuffer[10] << 8) + PS_ReceiveBuffer[11]);
                sprintf(str,"Now Search ID: %d",(int)SearchID);
                Blinker.notify(str);
                if(SearchID == 0) WiFi_Connected_State = 0;
                digitalWrite(12,!digitalRead(12));
                if(ErrorNum < 5) ErrorNum = 0;
				break;
		}
	}
  for(int i=0;i<20;i++) PS_ReceiveBuffer[i] = 0xFF;
}


/**
  * @brief   注册指纹后返回的应答包校验
  * @param   ACK：注册指纹函数返回的应答包
  * @return  None
  */
void YFROBOTFPM383::ENROLL_ACK_CHECK(uint8_t ACK)
{
	if(PS_ReceiveBuffer[6] == 0x07)
	{
		switch (ACK)
		{
			case 0x00:                          //指令正确
                EnrollID = (int)((PS_AutoEnrollBuffer[10] << 8) + PS_AutoEnrollBuffer[11]);
                sprintf(str,"Now Enroll ID: %d",(int)EnrollID);
                Blinker.notify(str);
				break;
		}
	}
  for(int i=0;i<20;i++) PS_ReceiveBuffer[i] = 0xFF;
}