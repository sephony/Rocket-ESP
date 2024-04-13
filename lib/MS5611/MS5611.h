#pragma once

#include "Arduino.h"
#include "Wire.h"

//  BREAKOUT  MS5611  aka  GY63 - see datasheet
//
//  SPI    I2C
//              +--------+
//  VCC    VCC  | o      |
//  GND    GND  | o      |
//         SCL  | o      |
//  SDI    SDA  | o      |
//  CSO         | o      |
//  SDO         | o L    |   L = led
//          PS  | o    O |   O = opening  PS = protocol select
//              +--------+
//
//  PS to VCC  ==>  I2C  (GY-63 board has internal pull up, so not needed)
//  PS to GND  ==>  SPI
//  CS to VCC  ==>  0x76
//  CS to GND  ==>  0x77

/*
 ******************************************************************************
 *	从PROM读取工厂的出厂数据
 *	C0	为0正常，为1说明IIC通信有问题或者硬件复位后延时不够
 *	C1  压力敏感度 SENS_T1				uint16_t
 *	C2  压力补偿  OFF_T1				uint16_t
 *	C3	温度压力灵敏度系数 TCS          uint16_t
 *	C4	温度系数的压力补偿 TCO          uint16_t
 *	C5	参考温度 T_REF                  uint16_t
 *	C6 	温度系数 TEMPSENS				uint16_t
 *	C7	CRC校验
 ******************************************************************************
 *	读取数字压力和温度数据
 *
 *	D1	数字压力值						uint32_t
 *	D2	数字温度值						uint32_t
 ******************************************************************************
 *	计算温度
 *
 *	dT		实际温度与参考温度之差	dT=D2-C5*2^8			int64_t
 *	TEMP	实际温度				TEMP=2000+dT*C6/2^23	int32_t	2007(20.07℃)
 ******************************************************************************
 *	计算温度补偿压力
 *
 *	OFF		实际温度补偿		OFF=C2*2^16+(C4*dT)/2^7		int64_t
 *	SENS	实际温度下的灵敏度	SENS=C1*2^15+(C3*dT)/2^8	int64_t
 *	P		温度补偿压力		P=(D1*SENS/2^21-OFF)/2^15	int32_t	100009(1000.09mbar=100kPa)
 ******************************************************************************
 */
#define MS5611_LIB_VERSION (F("0.3.9"))

#ifndef MS5611_DEFAULT_ADDRESS
#define MS5611_DEFAULT_ADDRESS 0x77
#endif

#define MS5611_READ_OK 0
#define MS5611_ERROR_2 2  //  low level I2C error
#define MS5611_NOT_READ -999

#define MS5611_CMD_READ_ADC 0x00
#define MS5611_CMD_READ_PROM 0xA0
#define MS5611_CMD_RESET 0x1E
#define MS5611_CMD_CONVERT_D1 0x40
#define MS5611_CMD_CONVERT_D2 0x50

enum osr_t : int {
    OSR_ULTRA_LOW = 256,   //  1 millis    Default = backwards compatible
    OSR_LOW = 512,         //  2 millis
    OSR_STANDARD = 1024,   //  3 millis
    OSR_HIGH = 2048,       //  5 millis
    OSR_ULTRA_HIGH = 4096  // 10 millis
};

enum h_mode : int {
    ONLY_PRESSURE = 0,
    MIXED = 1
};

class MS5611 {
public:
    explicit MS5611(uint8_t deviceAddress = MS5611_DEFAULT_ADDRESS);

#if defined(ESP8266) || defined(ESP32)
    bool begin(uint8_t sda, uint8_t scl, TwoWire *wire = &Wire);
#endif
    bool begin(TwoWire *wire = &Wire);
    bool isConnected();

    //       reset command + get constants
    //       mathMode = 0 (default), 1 = factor 2 fix.
    //       returns false if ROM constants are 0;
    bool reset(uint8_t mathMode = 0);

    //  the actual reading of the sensor;
    //  returns MS5611_READ_OK upon success
    int read(osr_t oversamplingRate);
    //  wrapper, uses the preset oversampling rate.
    inline int read() { return read(_overSamplingRate); };

    //  sets oversampling to a value between 8 and 12
    void setOversampling(osr_t overSamplingRate) { _overSamplingRate = overSamplingRate; };

    //  oversampling rate is in osr_t
    osr_t getOversampling() const { return _overSamplingRate; };

    //  temperature is in ²C
    float getTemperature() const { return _temperature; };

    //  pressure is in mBar
    float getPressure() const { return _pressure; };

    float getHeight(h_mode mode = ONLY_PRESSURE) const;
    //  OFFSET - 0.3.6
    void setPressureOffset(float offset = 0) { _pressureOffset = offset; };
    float getPressureOffset() { return _pressureOffset; };
    void setTemperatureOffset(float offset = 0) { _temperatureOffset = offset; };
    float getTemperatureOffset() { return _temperatureOffset; };

    //  to check for failure
    int getLastResult() const { return _result; };

    //  last time in millis() when the sensor has been read.
    uint32_t lastRead() const { return _lastRead; };

    //       _deviceID is a SHIFT XOR merge of 7 PROM registers, reasonable unique
    uint32_t getDeviceID() const { return _deviceID; };

    void setCompensation(bool flag = true) { _compensation = flag; };
    bool getCompensation() { return _compensation; };

    //  develop functions.
    /*
    void     setAddress(uint8_t address) { _address = address; };  // RANGE CHECK + isConnected() !
    uint8_t  getAddress() const          { return _address; };
    uint8_t  detectAddress() { todo };  // works with only one on the bus?
    */

    //       EXPERIMENTAL
    uint16_t getManufacturer();
    uint16_t getSerialCode();
    void list();

protected:
    int command(const uint8_t command);
    uint16_t readProm(uint8_t reg);
    uint32_t readADC();
    void convert(const uint8_t addr, osr_t overSamplingRate);
    void initConstants(uint8_t mathMode);

    uint8_t _address;
    osr_t _overSamplingRate;
    float _temperature;
    float _pressure;
    float _pressureOffset;
    float _temperatureOffset;
    float _height;
    int _result;
    uint16_t C[8];
    double C_multi[8];
    uint32_t D1;
    uint32_t D2;
    int32_t dT;
    int32_t TEMP;
    int64_t OFF;
    int64_t SENS;
    int32_t P;
    uint32_t _lastRead;
    uint32_t _deviceID;
    bool _compensation;

    TwoWire *_wire;
};

// -- END OF FILE --
