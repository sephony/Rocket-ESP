#include "MS5611.h"

/////////////////////////////////////////////////////
//
//  PUBLIC
//
MS5611::MS5611(uint8_t deviceAddress) {
    _address = deviceAddress;
    _overSamplingRate = OSR_ULTRA_LOW;
    _temperature = MS5611_NOT_READ;
    _pressure = MS5611_NOT_READ;
    _result = MS5611_NOT_READ;
    _lastRead = 0;
    _deviceID = 0;
    _pressureOffset = 0;
    _temperatureOffset = 0;
    _compensation = true;
}

#if defined(ESP8266) || defined(ESP32)
bool MS5611::begin(uint8_t dataPin, uint8_t clockPin, TwoWire* wire) {
    if ((_address < 0x76) || (_address > 0x77)) return false;

    _wire = wire;
    if ((dataPin < 255) && (clockPin < 255)) {
        _wire->begin(dataPin, clockPin);
    } else {
        _wire->begin();
    }
    if (!isConnected()) return false;

    return reset();
}
#endif

bool MS5611::begin(TwoWire* wire) {
    if ((_address < 0x76) || (_address > 0x77)) return false;
    _wire = wire;
    _wire->begin();
    if (!isConnected()) return false;

    return reset();
}

bool MS5611::isConnected() {
    _wire->beginTransmission(_address);
#ifdef ARDUINO_ARCH_NRF52840
    //  needed for NANO 33 BLE
    _wire->write(0);
#endif
    return (_wire->endTransmission() == 0);
}

bool MS5611::reset(uint8_t mathMode) {
    command(MS5611_CMD_RESET);
    uint32_t start = micros();

    //  while loop prevents blocking RTOS
    while (micros() - start < 2800) {
        yield();
        delayMicroseconds(10);
    }

    //  read factory calibrations from EEPROM.
    bool ROM_OK = true;
    for (uint8_t reg = 0; reg < 8; ++reg) {
        //  used indices match datasheet.
        //  C[0] == manufacturer - read but not used;
        //  C[7] == CRC.
        uint16_t tmp = readProm(reg);
        C[reg] = tmp;
        //  _deviceID is a SHIFT XOR merge of 7 PROM registers, reasonable unique
        _deviceID <<= 4;
        _deviceID ^= tmp;
        //  Serial.println(readProm(reg));

        if (reg > 0) {
            ROM_OK = ROM_OK && (tmp != 0);
        }
    }
    return ROM_OK;
}

int MS5611::read(osr_t overSamplingRate) {
    //  VARIABLES NAMES BASED ON DATASHEET
    //  ALL MAGIC NUMBERS ARE FROM DATASHEET

    convert(MS5611_CMD_CONVERT_D1, overSamplingRate);
    if (_result) return _result;
    //  NOTE: D1 and D2 seem reserved in MBED (NANO BLE)
    D1 = readADC();
    if (_result) return _result;

    convert(MS5611_CMD_CONVERT_D2, overSamplingRate);
    if (_result) return _result;
    D2 = readADC();
    if (_result) return _result;

    // Serial.println(D1);
    // Serial.println(D2);

    //  TEST VALUES - comment lines above
    //  uint32_t D1 = 9085466;
    //  uint32_t D2 = 8569150;

    //  TEMP & PRESS MATH - PAGE 7/20
    dT = D2 - C[5] * 256;
    TEMP = 2000 + dT * C[6] / 8388608;

    OFF = (int64_t)C[2] * 65536 + dT * (int64_t)C[4] / 128;
    SENS = (int64_t)C[1] * 32768 + dT * (int64_t)C[3] / 256;

    if (_compensation) {
        //  second order tempreture compensation - PAGE 8/20
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

    P = (int32_t)((D1 * SENS / 2097152 - OFF) / 32768);

    _temperature = (float)TEMP * 0.01 + _temperatureOffset;
    _pressure = (float)P * 0.01 + _pressureOffset;

    _lastRead = millis();
    return MS5611_READ_OK;
}

float MS5611::getHeight(h_mode mode) const {
    switch (mode) {
    case ONLY_PRESSURE:
        return 44330.0 * (1.0 - pow(_pressure / 1013.25, 1 / 5.255));  // barometric
    case MIXED:
        return (pow((1013.25 / _pressure), 1.0 / 5.257) - 1) * (_temperature + 273.15) / 0.65;  // hypsometric
    default:
        Serial.println("getHeight() - unknown mode(the mode must be ONLY_PRESSURE or MIXED)");
        return -1;
    }
}
//       EXPERIMENTAL
uint16_t MS5611::getManufacturer() {
    return readProm(0);
}

//       EXPERIMENTAL
uint16_t MS5611::getSerialCode() {
    return readProm(7) >> 4;
}

void MS5611::list() {
    for (uint8_t reg = 0; reg < 8; ++reg) {
        Serial.printf("C%d", reg);
        Serial.print(": ");
        Serial.println(C[reg]);
    }
    Serial.printf("D1: %lu\n", D1);
    Serial.printf("D2: %lu\n", D2);
    Serial.printf("dT: %ld\n", dT);
    Serial.printf("TEMP: %ld\n", TEMP);
    Serial.printf("OFF: %ld\n", OFF);
    Serial.printf("SENS: %ld\n", SENS);
    Serial.printf("P: %ld\n", P);
    Serial.printf("Temperature: %.2f\n", _temperature);
    Serial.printf("Pressure: %.2f\n", _pressure);
}

/////////////////////////////////////////////////////
//
//  PRIVATE
//
void MS5611::convert(const uint8_t addr, osr_t overSamplingRate) {
    //  values from page 3 datasheet - MAX column (rounded up)
    uint16_t del[5] = {600, 1200, 2300, 4600, 9100};

    uint8_t index = 0;
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

uint16_t MS5611::readProm(uint8_t reg) {
    //  last EEPROM register is CRC - Page 13 datasheet.
    uint8_t promCRCRegister = 7;
    if (reg > promCRCRegister) return 0;

    uint8_t offset = reg * 2;
    command(MS5611_CMD_READ_PROM + offset);
    if (_result == 0) {
        uint8_t length = 2;
        int bytes = _wire->requestFrom(_address, length);
        if (bytes >= length) {
            uint16_t value = _wire->read() * 256;
            value += _wire->read();
            return value;
        }
        return 0;
    }
    return 0;
}

uint32_t MS5611::readADC() {
    command(MS5611_CMD_READ_ADC);
    if (_result == 0) {
        uint8_t length = 3;
        int bytes = _wire->requestFrom(_address, length);
        if (bytes >= length) {
            uint32_t value = _wire->read() * 65536UL;
            value += _wire->read() * 256UL;
            value += _wire->read();
            return value;
        }
        return 0UL;
    }
    return 0UL;
}

int MS5611::command(const uint8_t command) {
    yield();
    _wire->beginTransmission(_address);
    _wire->write(command);
    _result = _wire->endTransmission();
    return _result;
}

void MS5611::initConstants(uint8_t mathMode) {
    //  constants that were multiplied in read() - datasheet page 8
    //  do this once and you save CPU cycles
    //
    //                               datasheet ms5611               |    appNote
    //                                mode = 0;                     |    mode = 1
    if (mathMode == 0) {              //  datasheet version for temperature.
        C_multi[1] = C[1] * 32768;    //  SENSt1   = C[1] * 2^15    |    * 2^16
        C_multi[2] = C[2] * 65536;    //  OFFt1    = C[2] * 2^16    |    * 2^17
        C_multi[3] = C[3] / 256;      //  TCS      = C[3] / 2^8     |    / 2^7
        C_multi[4] = C[4] / 128;      //  TCO      = C[4] / 2^7     |    / 2^6
        C_multi[5] = C[5] * 256;      //  Tref     = C[5] * 2^8     |    * 2^8
        C_multi[6] = C[6] / 8388608;  //  TEMPSENS = C[6] / 2^23    |    / 2^23
    } else {                          //  Appnote version for pressure.

        C_multi[1] = C[1] * 65536;   //  SENSt1
        C_multi[2] = C[2] * 131072;  //  OFFt1
        C_multi[3] = C[3] / 128;     //  TCS
        C_multi[4] = C[3] / 64;      //  TCO
        C_multi[5] = C[5] * 256;
        C_multi[6] = C[6] / 8388608;
    }
}

// -- END OF FILE --
