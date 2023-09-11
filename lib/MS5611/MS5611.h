#pragma once
//
//    FILE: MS5611.cpp
//  AUTHOR: Qiao Dong
// VERSION: 0.0.1
// PURPOSE: MS5611 (SPI) Temperature & Pressure library for ESP32 with Arduino framework
//     URL: https://github.com/QiaoDong1216/MS5611

#include "Arduino.h"
#include "SPI.h"

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
#define MS5611_SPI_LIB_VERSION (F("0.0.1 EXPERIMENTAL"))

#define MS5611_READ_OK 0
#define MS5611_ERROR_2 2  //  TODO ??
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
class MS5611 {
public:
protected:
};
class MS5611_SPI : MS5611 {
public:
    MS5611_SPI(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
        : _sck(sck), _miso(miso), _mosi(mosi), _ss(ss), _overSamplingRate(OSR_ULTRA_HIGH), _hwSPI(true), _SPIspeed(1000000), _spi_settings(1000000, MSBFIRST, SPI_MODE0), _compensation(true) {}

    MS5611_SPI(int8_t ss) : MS5611_SPI(255, 255, 255, ss) {}

    bool begin();
    bool isConnected();

    //       reset command + get constants
    //       mathMode = 0 (default), 1 = factor 2 fix.
    //       returns false if ROM constants == 0;
    bool reset();

    //  the actual reading of the sensor;
    //  returns MS5611_READ_OK upon success
    int read();

    //  sets oversampling to a value between 8 and 12
    void setOversampling(osr_t overSamplingRate);

    //  oversampling rate is in osr_t
    osr_t getOversampling() const;

    //  temperature is in ²C
    inline float getTemperature();

    //  pressure is in mBar
    inline float getPressure();

    //  height is in meters
    inline float getHeight(float seaLevelPressure = 1013.25);
    //  OFFSET
    void setPressureOffset(float offset = 0);
    float getPressureOffset();
    void setTemperatureOffset(float offset = 0);
    float getTemperatureOffset();

    //  to check for failure
    int getLastResult() const;

    //  last time in millis() when the sensor has been read.
    uint32_t lastRead() const;

    uint32_t getDeviceID() const;

    void setCompensation(bool flag = true);
    bool getCompensation();

    //  develop functions.
    /*
    void     setAddress(uint8_t address) { _address = address; };  // RANGE CHECK
    uint8_t  getAddress() const          { return _address; };
    uint8_t  detectAddress() { todo };  // works with only one on the bus?
    */

    //       speed in Hz
    void setSPIspeed(uint32_t speed);
    uint32_t getSPIspeed();

    //  debugging
    bool usesHWSPI();
    bool end();

private:
    int command(const uint8_t command);
    void convert(const uint8_t addr, osr_t overSamplingRate);
    uint16_t readProm(uint8_t reg);
    uint32_t readADC();

    // enum class OSR : int {
    //     OSR_ULTRA_LOW = 256,
    //     OSR_LOW = 512,
    //     OSR_STANDARD = 1024,
    //     OSR_HIGH = 2048,
    //     OSR_ULTRA_HIGH = 4096
    // };
    uint8_t _address;
    osr_t _overSamplingRate;
    float _temperature;
    float _pressure;
    float _height;
    float _pressureOffset;
    float _temperatureOffset;
    int _result;
    uint16_t C[8];
    uint32_t D1;
    uint32_t D2;
    int64_t dT;
    int64_t TEMP;
    int64_t OFF;
    int64_t SENS;
    int32_t P;
    uint32_t _lastRead;
    uint32_t _deviceID;
    bool _compensation;

    //  SPI
    uint8_t _sck;
    uint8_t _miso;
    uint8_t _mosi;
    uint8_t _ss;
    bool _hwSPI = true;
    uint32_t _SPIspeed = 1000000;
    uint8_t swSPI_transfer(uint8_t value);

    SPIClass *mySPI;
    SPISettings _spi_settings;
    bool _useHSPI = false;
};
inline float MS5611_SPI::getTemperature() {
    // _temperature = TEMP * 0.01 + _temperatureOffset;
    // return _temperature;
    if (_temperatureOffset == 0) return TEMP * 0.01;
    return TEMP * 0.01 + _temperatureOffset;
};

inline float MS5611_SPI::getPressure() {
    // _pressure = P * 0.01 + _pressureOffset;
    // return _pressure;
    if (_pressureOffset == 0) return P * 0.01;
    return P * 0.01 + _pressureOffset;
};

inline float MS5611_SPI::getHeight(float seaLevelPressure) {
    //  pow() will be replaced with faster LUT or polynomial.
    //  altitude difference in meters per hPa
    float height = 44330.0 * (1.0 - pow(getPressure() / seaLevelPressure, 1 / 5.255));  // barometric
    // float height = (pow((seaLevelPressure / getPressure()), 1.0 / 5.257) - 1) * (getTemperature() + 273.15) / 0.65;  // hypsometric
    return height;
};
// -- END OF FILE --
