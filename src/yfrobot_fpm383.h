/******************************************************************************
  yfrobot_fpm383.h
  YFROBOT FPM383 Sensor Library Source File
  Creation Date: 05-31-2023
  @ YFROBOT

  Distributed as-is; no warranty is given.
******************************************************************************/

#include "Arduino.h"
#include "SoftwareSerial.h"

#ifndef YFROBOTFPM383_H
#define YFROBOTFPM383_H

#define RECEIVE_TIMEOUT_VALUE 1000 // Timeout for I2C receive

class YFROBOTFPM383
{
  private:
    uint8_t PS_ReceiveBuffer[20];  //串口接收数据的临时缓冲数组

    /********************************************** 指纹模块 指令集 ************************************************/
    //指令/命令包格式：包头0xEF01  设备地址4bytes  包标识1byte  包长度2bytes  指令码1byte  参数1......参数N  校验和2bytes
    //验证用获取图像 0x01，验证指纹时，探测手指，探测到后录入指纹图像存于图像缓冲区。返回确认码表示：录入成功、无手指等。
    uint8_t PS_GetImageBuffer[12] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x05 };
    // 生成特征 0x02，将图像缓冲区中的原始图像生成指纹特征文件存于模板缓冲区。
    uint8_t PS_GetChar1Buffer[13] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x01, 0x00, 0x08 };
    uint8_t PS_GetChar2Buffer[13] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, 0x02, 0x00, 0x09 };
    // 搜索指纹 0x04，以模板缓冲区中的特征文件搜索整个或部分指纹库。若搜索到，则返回页码。加密等级设置为 0 或 1 情况下支持此功能。
    uint8_t PS_SearchMBBuffer[17] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x02, 0x0C };
    // 删除指纹 0x0C，删除 flash 数据库中指定 ID（此处0） 号开始的N 个指纹模板。//PageID: bit 10:11，SUM: bit 14:15
    uint8_t PS_DeleteBuffer[16] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, '\0', '\0', 0x00, 0x01, '\0', '\0' };
    // 清空指纹库 0x0D，删除 flash 数据库中所有指纹模板。
    uint8_t PS_EmptyBuffer[12] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0D, 0x00, 0x11 };
    // 取消指令 0x30，取消自动注册模板和自动验证指纹。加密等级设置为 0 或 1 情况下支持此功能。
    uint8_t PS_CancelBuffer[12] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x30, 0x00, 0x34 };
    // 自动注册 0x31，一站式注册指纹，包含采集指纹、生成特征、组合模板、存储模板等功能。加密等级设置为 0 或 1 情况下支持此功能。
    uint8_t PS_AutoEnrollBuffer[17] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x31, '\0', '\0', 0x04, 0x00, 0x16, '\0', '\0' };
    // 休眠指令 0x33，设置传感器进入休眠模式。
    uint8_t PS_SleepBuffer[12] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x33, 0x00, 0x37 };
    // LED控制灯指令  0x3C，控制灯指令主要分为两类：一般指示灯和七彩编程呼吸灯。
    // 指令说明                                                                                指令  功能   起始  结束  循环   校     验
    uint8_t PS_BlueLEDBuffer[16] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x03, 0x01, 0x01, 0x00, 0x00, 0x49 };
    uint8_t PS_RedLEDBuffer[16] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x04, 0x04, 0x02, 0x00, 0x50 };
    uint8_t PS_GreenLEDBuffer[16] = { 0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x3C, 0x02, 0x02, 0x02, 0x02, 0x00, 0x4C };
    // 更多指令集请参见：http://file.yfrobot.com.cn/datasheet/FPM383C%E6%A8%A1%E7%BB%84%E9%80%9A%E4%BF%A1%E5%8D%8F%E8%AE%AE_V1.2.pdf

    void sendData(int len, uint8_t PS_Databuffer[]);
    void receiveData(uint16_t Timeout);
    
  public:
    // -----------------------------------------------------------------------------
    // Constructor - YFROBOTFPM383: This function sets up the pins connected to the
    //		YFROBOTFPM383, and sets up the private deviceAddress variable.
    // -----------------------------------------------------------------------------
    YFROBOTFPM383();
    // Legacy below. Use 0-parameter constructor, and set these parameters in the
    // begin function:
    YFROBOTFPM383(uint8_t address, uint8_t resetPin = 255, uint8_t interruptPin = 255, uint8_t oscillatorPin = 255);

    // -----------------------------------------------------------------------------
    // begin(uint8_t address, uint8_t resetPin): This function initializes the YFROBOTFPM383.
    //  	It requires wire to already be begun (previous versions did not do this), resets the IC, and tries to read some
    //  	registers to prove it's connected.
    // Inputs:
    //		- address: should be the 7-bit address of the YFROBOTFPM383. This should be
    //		 one of four values - 0x3E, 0x3F, 0x70, 0x71 - all depending on what the
    //		 ADDR0 and ADDR1 pins ar se to. This variable is required.
    //		- resetPin: This is the Arduino pin tied to the YFROBOTFPM383 RST pin. This
    //		 pin is optional. If not declared, the library will attempt to
    //		 software reset the YFROBOTFPM383.
    // Output: Returns a 1 if communication is successful, 0 on error.
    // -----------------------------------------------------------------------------
    uint8_t begin(uint8_t address = 0x3E, TwoWire &wirePort = Wire, uint8_t resetPin = 0xFF);
    uint8_t init(void); // Legacy -- use begin now

    // -----------------------------------------------------------------------------
    // reset(bool hardware): This function resets the YFROBOTFPM383 - either a hardware
    //		reset or software. A hardware reset (hardware parameter = 1) pulls the
    //		reset line low, pausing, then pulling the reset line high. A software
    //		reset writes a 0x12 then 0x34 to the REG_RESET as outlined in the
    //		datasheet.
    //
    //  Input:
    //	 	- hardware: 0 executes a software reset, 1 executes a hardware reset
    // -----------------------------------------------------------------------------
    void reset(bool hardware);

    // -----------------------------------------------------------------------------
    // pinMode(uint8_t pin, uint8_t inOut): This function sets one of the YFROBOTFPM383's 16
    //		outputs to either an INPUT or OUTPUT.
    //
    //	Inputs:
    //	 	- pin: should be a value between 0 and 15
    //	 	- inOut: The Arduino INPUT and OUTPUT constants should be used for the
    //		 inOut parameter. They do what they say!
    // -----------------------------------------------------------------------------
    void pinMode(uint8_t pin, uint8_t inOut, uint8_t initialLevel = HIGH);
    void pinDir(uint8_t pin, uint8_t inOut, uint8_t initialLevel = HIGH); // Legacy - use pinMode

    // -----------------------------------------------------------------------------
    // digitalWrite(uint8_t pin, uint8_t highLow): This function writes a pin to either high
    //		or low if it's configured as an OUTPUT. If the pin is configured as an
    //		INPUT, this method will activate either the PULL-UP	or PULL-DOWN
    //		resistor (HIGH or LOW respectively).
    //
    //	Inputs:
    //		- pin: The YFROBOTFPM383 pin number. Should be a value between 0 and 15.
    //		- highLow: should be Arduino's defined HIGH or LOW constants.
    // -----------------------------------------------------------------------------
    bool digitalWrite(uint8_t pin, uint8_t highLow);
    bool writePin(uint8_t pin, uint8_t highLow); // Legacy - use digitalWrite

    // -----------------------------------------------------------------------------
    // digitalRead(uint8_t pin): This function reads the HIGH/LOW status of a pin.
    //		The pin should be configured as an INPUT, using the pinDir function.
    //
    //	Inputs:
    //	 	- pin: The YFROBOTFPM383 pin to be read. should be a value between 0 and 15.
    //  Outputs:
    //		This function returns a 1 if HIGH, 0 if LOW
    // -----------------------------------------------------------------------------
    uint8_t digitalRead(uint8_t pin);
    bool digitalRead(uint8_t pin, bool *value);
    uint8_t readPin(uint8_t pin); // Legacy - use digitalRead
    bool readPin(const uint8_t pin, bool *value);

    // -----------------------------------------------------------------------------
    // ledDriverInit(uint8_t pin, uint8_t freq, bool log): This function initializes LED
    //		driving on a pin. It must be called if you want to use the pwm or blink
    //		functions on that pin.
    //
    //	Inputs:
    //		- pin: The YFROBOTFPM383 pin connected to an LED. Should be 0-15.
    //   	- freq: Sets LED clock frequency divider.
    //		- log: selects either linear or logarithmic mode on the LED drivers
    //			- log defaults to 0, linear mode
    //			- currently log sets both bank A and B to the same mode
    //	Note: this function automatically decides to use the internal 2MHz osc.
    // -----------------------------------------------------------------------------
    void ledDriverInit(uint8_t pin, uint8_t freq = 1, bool log = false);

    // -----------------------------------------------------------------------------
    // analogWrite(uint8_t pin, uint8_t iOn):	This function can be used to control the intensity
    //		of an output pin connected to an LED.
    //
    //	Inputs:
    //		- pin: The YFROBOTFPM383 pin connecte to an LED.Should be 0-15.
    //		- iOn: should be a 0-255 value setting the intensity of the LED
    //			- 0 is completely off, 255 is 100% on.
    //
    //	Note: ledDriverInit should be called on the pin before calling this.
    // -----------------------------------------------------------------------------
    void analogWrite(uint8_t pin, uint8_t iOn);
    void pwm(uint8_t pin, uint8_t iOn); // Legacy - use analogWrite

    // -----------------------------------------------------------------------------
    // setupBlink(uint8_t pin, uint8_t tOn, uint8_t tOff, uint8_t offIntensity, uint8_t tRise, uint8_t
    //		tFall):  blink performs both the blink and breath LED driver functions.
    //
    // 	Inputs:
    //  	- pin: the YFROBOTFPM383 pin (0-15) you want to set blinking/breathing.
    //		- tOn: the amount of time the pin is HIGH
    //			- This value should be between 1 and 31. 0 is off.
    //		- tOff: the amount of time the pin is at offIntensity
    //			- This value should be between 1 and 31. 0 is off.
    //		- offIntensity: How dim the LED is during the off period.
    //			- This value should be between 0 and 7. 0 is completely off.
    //		- onIntensity: How bright the LED will be when completely on.
    //			- This value can be between 0 (0%) and 255 (100%).
    //		- tRise: This sets the time the LED takes to fade in.
    //			- This value should be between 1 and 31. 0 is off.
    //			- This value is used with tFall to make the LED breath.
    //		- tFall: This sets the time the LED takes to fade out.
    //			- This value should be between 1 and 31. 0 is off.
    // 	 Notes:
    //		- The breathable pins are 4, 5, 6, 7, 12, 13, 14, 15 only. If tRise and
    //			tFall are set on 0-3 or 8-11 those pins will still only blink.
    // 		- ledDriverInit should be called on the pin to be blinked before this.
    // -----------------------------------------------------------------------------
    void setupBlink(uint8_t pin, uint8_t tOn, uint8_t toff, uint8_t onIntensity = 255, uint8_t offIntensity = 0, uint8_t tRise = 0, uint8_t tFall = 0, bool log = false);

    // -----------------------------------------------------------------------------
    // blink(uint8_t pin, unsigned long tOn, unsigned long tOff, uint8_t onIntensity, uint8_t offIntensity);
    //  	Set a pin to blink output for estimated on/off millisecond durations.
    //
    // 	Inputs:
    //  	- pin: the YFROBOTFPM383 pin (0-15) you want to set blinking
    //   	- tOn: estimated number of milliseconds the pin is LOW (LED sinking current will be on)
    //   	- tOff: estimated number of milliseconds the pin is HIGH (LED sinking current will be off)
    //   	- onIntensity: 0-255 value determining LED on brightness
    //   	- offIntensity: 0-255 value determining LED off brightness
    // 	 Notes:
    //		- The breathable pins are 4, 5, 6, 7, 12, 13, 14, 15 only. If tRise and
    //			tFall are set on 0-3 or 8-11 those pins will still only blink.
    // 		- ledDriverInit should be called on the pin to be blinked before this.
    // -----------------------------------------------------------------------------
    void blink(uint8_t pin, unsigned long tOn, unsigned long tOff, uint8_t onIntensity = 255, uint8_t offIntensity = 0);

    // -----------------------------------------------------------------------------
    // breathe(uint8_t pin, unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, uint8_t onInt, uint8_t offInt, bool log);
    //  	Set a pin to breathe output for estimated on/off millisecond durations, with
    //  	estimated rise and fall durations.
    //
    // 	Inputs:
    //  	- pin: the YFROBOTFPM383 pin (0-15) you want to set blinking
    //   	- tOn: estimated number of milliseconds the pin is LOW (LED sinking current will be on)
    //   	- tOff: estimated number of milliseconds the pin is HIGH (LED sinking current will be off)
    //   	- rise: estimated number of milliseconds the pin rises from LOW to HIGH
    //   	- falll: estimated number of milliseconds the pin falls from HIGH to LOW
    //   	- onIntensity: 0-255 value determining LED on brightness
    //   	- offIntensity: 0-255 value determining LED off brightness
    // 	 Notes:
    //		- The breathable pins are 4, 5, 6, 7, 12, 13, 14, 15 only. If tRise and
    //			tFall are set on 0-3 or 8-11 those pins will still only blink.
    // 		- ledDriverInit should be called on the pin to be blinked before this,
    //  	  Or call pinMode(<pin>, ANALOG_OUTPUT);
    // -----------------------------------------------------------------------------
    void breathe(uint8_t pin, unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, uint8_t onInt = 255, uint8_t offInt = 0, bool log = LINEAR);

    // -----------------------------------------------------------------------------
    // sync(void): this function resets the PWM/Blink/Fade counters, syncing any
    //		blinking LEDs. Bit 2 of REG_MISC is set, which alters the functionality
    //		of the nReset pin. The nReset pin is toggled low->high, which should
    //		reset all LED counters. Bit 2 of REG_MISC is again cleared, returning
    //		nReset pin to POR functionality
    // -----------------------------------------------------------------------------
    void sync(void);

    // -----------------------------------------------------------------------------
    // debounceConfig(uint8_t configValue): This method configures the debounce time of
    //		every input.
    //
    //	Input:
    //		- configValue: A 3-bit value configuring the debounce time.
    //			000: 0.5ms * 2MHz/fOSC
    //			001: 1ms * 2MHz/fOSC
    //			010: 2ms * 2MHz/fOSC
    //			011: 4ms * 2MHz/fOSC
    //			100: 8ms * 2MHz/fOSC
    //			101: 16ms * 2MHz/fOSC
    //			110: 32ms * 2MHz/fOSC
    //			111: 64ms * 2MHz/fOSC
    //
    //	Note: fOSC is set with the configClock function. It defaults to 2MHz.
    // -----------------------------------------------------------------------------
    void debounceConfig(uint8_t configVaule);

    // -----------------------------------------------------------------------------
    // debounceTime(uint8_t configValue): This method configures the debounce time of
    //		every input to an estimated millisecond time duration.
    //
    //	Input:
    //		- time: A millisecond duration estimating the debounce time. Actual
    //		  debounce time will depend on fOSC. Assuming it's 2MHz, debounce will
    //		  be set to the 0.5, 1, 2, 4, 8, 16, 32, or 64 ms (whatever's closest)
    //
    //	Note: fOSC is set with the configClock function. It defaults to 2MHz.
    // -----------------------------------------------------------------------------
    void debounceTime(uint8_t time);

    // -----------------------------------------------------------------------------
    // debouncePin(uint8_t pin): This method enables debounce on YFROBOTFPM383 input pin.
    //
    //	Input:
    //		- pin: The YFROBOTFPM383 pin to be debounced. Should be between 0 and 15.
    // -----------------------------------------------------------------------------
    void debouncePin(uint8_t pin);
    void debounceEnable(uint8_t pin); // Legacy, use debouncePin

    // -----------------------------------------------------------------------------
    // debounceKeypad(uint8_t pin): This method enables debounce on all pins connected
    //  to a row/column keypad matrix.
    //
    //	Input:
    //		- time: Millisecond time estimate for debounce (see debounceTime()).
    //		- numRows: The number of rows in the keypad matrix.
    //		- numCols: The number of columns in the keypad matrix.
    // -----------------------------------------------------------------------------
    void debounceKeypad(uint8_t time, uint8_t numRows, uint8_t numCols);

    // -----------------------------------------------------------------------------
    // enableInterrupt(uint8_t pin, uint8_t riseFall): This function sets up an interrupt
    //		on a pin. Interrupts can occur on all YFROBOTFPM383 pins, and can be generated
    //		on rising, falling, or both.
    //
    //	Inputs:
    //		-pin: YFROBOTFPM383 input pin that will generate an input. Should be 0-15.
    //		-riseFall: Configures if you want an interrupt generated on rise fall or
    //			both. For this param, send the pin-change values previously defined
    //			by Arduino:
    //			#define CHANGE 1	<-Both
    //			#define FALLING 2	<- Falling
    //			#define RISING 3	<- Rising
    //
    //	Note: This function does not set up a pin as an input, or configure	its
    //		pull-up/down resistors! Do that before (or after).
    // -----------------------------------------------------------------------------
    void enableInterrupt(uint8_t pin, uint8_t riseFall);

    // -----------------------------------------------------------------------------
    // interruptSource(void): Returns an uint16_t representing which pin caused
    //		an interrupt.
    //
    //	Output: 16-bit value, with a single bit set representing the pin(s) that
    //		generated an interrupt. E.g. a return value of	0x0104 would mean pins 8
    //		and 3 (bits 8 and 3) have generated an interrupt.
    //  Input:
    //  	- clear: boolean commanding whether the interrupt should be cleared
    //  	  after reading or not.
    // -----------------------------------------------------------------------------
    uint8_t interruptSource(bool clear = true);

    // -----------------------------------------------------------------------------
    // checkInterrupt(void): Checks if a single pin generated an interrupt.
    //
    //	Output: Boolean value. True if the requested pin has triggered an interrupt/
    //  Input:
    //  	- pin: Pin to be checked for generating an input.
    // -----------------------------------------------------------------------------
    bool checkInterrupt(uint8_t pin);

    // -----------------------------------------------------------------------------
    // configClock(uint8_t oscSource, uint8_t oscPinFunction, uint8_t oscFreqOut, uint8_t oscDivider)
    //		This function configures the oscillator source/speed
    //		and the clock, which is used to drive LEDs and time debounces.
    //
    //	Inputs:
    //	- oscSource: Choose either internal 2MHz oscillator or an external signal
    //		applied to the OSCIO pin.
    //		- INTERNAL_CLOCK and EXTERNAL_CLOCK are defined in the header file.
    //			Use those.
    //		- This value defaults to internal.
    //	- oscDivider: Sets the clock divider in REG_MISC.
    //		- ClkX = fOSC / (2^(RegMisc[6:4] -1))
    //		- This value defaults to 1.
    //	- oscPinFunction: Allows you to set OSCIO as an input or output.
    //		- You can use Arduino's INPUT, OUTPUT defines for this value
    //		- This value defaults to input
    //	- oscFreqOut: If oscio is configured as an output, this will set the output
    //		frequency
    //		- This should be a 4-bit value. 0=0%, 0xF=100%, else
    //			fOSCOut = FOSC / (2^(RegClock[3:0]-1))
    //		- This value defaults to 0.
    // -----------------------------------------------------------------------------
    void configClock(uint8_t oscSource = 2, uint8_t oscPinFunction = 0, uint8_t oscFreqOut = 0, uint8_t oscDivider = 1); // Legacy, use clock();

    // -----------------------------------------------------------------------------
    // clock(uint8_t oscSource, uint8_t oscDivider, uint8_t oscPinFunction, uint8_t oscFreqOut)
    //		This function configures the oscillator source/speed
    //		and the clock, which is used to drive LEDs and time debounces.
    //  	This is just configClock in a bit more sane order.
    //
    // -----------------------------------------------------------------------------
    void clock(uint8_t oscSource = 2, uint8_t oscDivider = 1, uint8_t oscPinFunction = 0, uint8_t oscFreqOut = 0);
};

#endif // YFROBOTFPM383_H
