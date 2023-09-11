//
//    FILE: MS5611.cpp
//  AUTHOR: Qiao Dong
// VERSION: 0.0.1
// PURPOSE: MS5611 (SPI) Temperature & Pressure library for ESP32 with Arduino framework
//     URL: https://github.com/QiaoDong1216/MS5611
//
//  HISTORY: see changelog.md

#include "MS5611.h"

/*********************************PRIVATE**************************************/

bool MS5611_SPI::begin() {
    //  print experimental message.
    //  Serial.println(MS5611_SPI_LIB_VERSION);
    //  SPI
    pinMode(_ss, OUTPUT);
    digitalWrite(_ss, HIGH);
    if (_sck == 255 && _miso == 255 && _mosi == 255) {
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
        _sck = 12;
        _miso = 13;
        _mosi = 11;
#endif
    }
    Serial.print("sck: ");
    Serial.println(_sck);
    Serial.print("miso: ");
    Serial.println(_miso);
    Serial.print("mosi: ");
    Serial.println(_mosi);
    if (_hwSPI) {
        mySPI = new SPIClass(FSPI);
        mySPI->begin(_sck, _miso, _mosi, _ss);
        delay(1);
    } else {
        //  Serial.println("SW_SPI");
        pinMode(_sck, OUTPUT);
        pinMode(_miso, INPUT);
        pinMode(_mosi, OUTPUT);
        pinMode(_ss, OUTPUT);
        digitalWrite(_sck, HIGH);
        digitalWrite(_mosi, LOW);
        digitalWrite(_ss, HIGH);
    }
    return reset();
}

bool MS5611_SPI::isConnected() {
    int rv = read();
    return (rv == MS5611_READ_OK);
}

bool MS5611_SPI::reset() {
    command(MS5611_CMD_RESET);
    uint32_t start = micros();
    //  while loop prevents blocking RTOS
    while (micros() - start < 3000)  //  increased as first ROM values were missed.
    {
        yield();
        delayMicroseconds(10);
    }

    //  initialize the C[] array
    // initConstants(mathMode);

    // read factory calibrations from EEPROM.
    bool ROM_OK = true;
    for (uint8_t reg = 0; reg < 8; ++reg) {
        //  used indices match datasheet.
        //  C[0] == manufacturer - read but not used;
        //  C[7] == CRC.
        uint16_t tmp = readProm(reg);
        C[reg] = tmp;
        Serial.print(reg);
        Serial.print(": ");
        Serial.println(C[reg]);
        //  _deviceID is a simple SHIFT XOR merge of PROM data
        _deviceID <<= 4;
        _deviceID ^= tmp;

        if (reg > 0) {
            ROM_OK = ROM_OK && (tmp != 0);
        }
    }
    return ROM_OK;
}

int MS5611_SPI::read() {
    //  VARIABLES NAMES BASED ON DATASHEET
    //  ALL MAGIC NUMBERS ARE FROM DATASHEET
    convert(MS5611_CMD_CONVERT_D1, _overSamplingRate);
    //  NOTE: D1 and D2 seem reserved in MBED (NANO BLE)
    D1 = readADC();
    convert(MS5611_CMD_CONVERT_D2, _overSamplingRate);
    D2 = readADC();

    //  Serial.println(_D1);
    //  Serial.println(_D2);

    //   TEST VALUES - comment lines above
    //  uint32_t _D1 = 9085466;
    //  uint32_t _D2 = 8569150;

    //  TEMP & PRESS MATH - PAGE 7/20
    dT = D2 - C[5] * 256;
    TEMP = 2000 + dT * C[6] / 8388608;

    OFF = (uint64_t)C[2] * 65536 + dT * (uint64_t)C[4] / 128;
    SENS = (uint64_t)C[1] * 32768 + dT * (uint64_t)C[3] / 256;

    if (_compensation) {
        //  SECOND ORDER COMPENSATION - PAGE 8/20
        //  COMMENT OUT < 2000 CORRECTION IF NOT NEEDED
        //  NOTE TEMPERATURE IS IN 0.01 C
        if (TEMP < 2000) {
            int32_t aux = (2000 - TEMP) * (2000 - TEMP);
            int32_t T2 = dT * dT / 2147483647;
            int64_t OFF2 = 2.5 * aux;
            int64_t SENS2 = 1.25 * aux;
            //  COMMENT OUT < -1500 CORRECTION IF NOT NEEDED
            if (TEMP < -1500) {
                aux = (TEMP + 1500) * (TEMP + 1500);
                OFF2 += 7 * aux;
                SENS2 += 5.5 * aux;
            }
            TEMP -= T2;
            OFF -= OFF2;
            SENS -= SENS2;
        }
        //  END SECOND ORDER COMPENSATION
    }

    P = (D1 * SENS / 2097152 - OFF) / 32768;

    _lastRead = millis();
    return MS5611_READ_OK;
}

void MS5611_SPI::setOversampling(osr_t overSamplingRate) {
    _overSamplingRate = overSamplingRate;
}

osr_t MS5611_SPI::getOversampling() const {
    return (osr_t)_overSamplingRate;
};

void MS5611_SPI::setPressureOffset(float offset) {
    _pressureOffset = offset;
};

float MS5611_SPI::getPressureOffset() {
    return _pressureOffset;
};

void MS5611_SPI::setTemperatureOffset(float offset) {
    _temperatureOffset = offset;
};

float MS5611_SPI::getTemperatureOffset() {
    return _temperatureOffset;
};

int MS5611_SPI::getLastResult() const {
    return _result;
};

uint32_t MS5611_SPI::lastRead() const {
    return _lastRead;
};

uint32_t MS5611_SPI::getDeviceID() const {
    return _deviceID;
};

void MS5611_SPI::setCompensation(bool flag) {
    _compensation = flag;
};

bool MS5611_SPI::getCompensation() {
    return _compensation;
};

void MS5611_SPI::setSPIspeed(uint32_t speed) {
    _SPIspeed = speed;
    _spi_settings = SPISettings(_SPIspeed, MSBFIRST, SPI_MODE0);
};

uint32_t MS5611_SPI::getSPIspeed() {
    return _SPIspeed;
};

bool MS5611_SPI::usesHWSPI() {
    return _hwSPI;
};

bool MS5611_SPI::end() {
    if (_hwSPI) {
        SPI.end();
    }
    return true;
}

/*********************************PRIVATE**************************************/

void MS5611_SPI::convert(const uint8_t addr, osr_t overSamplingRate) {
    //  values from page 3 datasheet - MAX column (rounded up)
    uint16_t del[5] = {600, 1200, 2300, 4600, 9100};

    uint8_t index = overSamplingRate;
    switch (overSamplingRate) {
    case OSR_LOW:
        index = 0;
        break;
    case OSR_ULTRA_LOW:
        index = 1;
        break;
    case OSR_STANDARD:
        index = 2;
        break;
    case OSR_HIGH:
        index = 3;
        break;
    case OSR_ULTRA_HIGH:
        index = 4;
        break;
    default:
        break;
    }
    uint8_t offset = index * 2;
    command(addr + offset);

    uint16_t waitTime = del[index];
    uint32_t start = micros();
    //  while loop prevents blocking RTOS
    while (micros() - start < waitTime) {
        yield();
        delayMicroseconds(10);
    }
}

uint16_t MS5611_SPI::readProm(uint8_t reg) {
    //  last EEPROM register is CRC - Page 13 datasheet.
    uint8_t promCRCRegister = 7;
    if (reg > promCRCRegister) return 0;

    uint16_t value = 0;
    digitalWrite(_ss, LOW);
    if (_hwSPI) {
        mySPI->beginTransaction(_spi_settings);
        mySPI->transfer(MS5611_CMD_READ_PROM + reg * 2);
        value += mySPI->transfer(0x00);
        value <<= 8;
        value += mySPI->transfer(0x00);
        mySPI->endTransaction();
    } else  //  Software SPI
    {
        swSPI_transfer(MS5611_CMD_READ_PROM + reg * 2);
        value += swSPI_transfer(0x00);
        value <<= 8;
        value += swSPI_transfer(0x00);
    }
    digitalWrite(_ss, HIGH);
    return value;
}

uint32_t MS5611_SPI::readADC() {
    //  command(MS5611_CMD_READ_ADC);

    uint32_t value = 0;

    digitalWrite(_ss, LOW);
    if (_hwSPI) {
        mySPI->beginTransaction(_spi_settings);
        mySPI->transfer(0x00);
        value += mySPI->transfer(0x00);
        value <<= 8;
        value += mySPI->transfer(0x00);
        value <<= 8;
        value += mySPI->transfer(0x00);
        mySPI->endTransaction();
    } else  //  Software SPI
    {
        swSPI_transfer(0x00);
        value += swSPI_transfer(0x00);
        value <<= 8;
        value += swSPI_transfer(0x00);
        value <<= 8;
        value += swSPI_transfer(0x00);
    }
    digitalWrite(_ss, HIGH);
    //  Serial.println(value, HEX);
    return value;
}

int MS5611_SPI::command(const uint8_t command) {
    yield();
    digitalWrite(_ss, LOW);
    if (_hwSPI) {
        mySPI->beginTransaction(_spi_settings);
        mySPI->transfer(command);
        mySPI->endTransaction();
    } else  //  Software SPI
    {
        swSPI_transfer(command);
    }
    digitalWrite(_ss, HIGH);
    return 0;
}

//  simple one mode version
uint8_t MS5611_SPI::swSPI_transfer(uint8_t val) {
    uint8_t clk = _sck;
    uint8_t dao = _mosi;
    uint8_t dai = _miso;
    uint8_t value = 0;
    for (uint8_t mask = 0x80; mask; mask >>= 1) {
        digitalWrite(dao, (val & mask));
        digitalWrite(clk, HIGH);
        value <<= 1;
        if (digitalRead(dai) != 0) value += 1;
        digitalWrite(clk, LOW);
    }
    digitalWrite(dao, LOW);
    //  Serial.print(" # ");
    //  Serial.println(value, HEX);
    return value;
}

// -- END OF FILE --
