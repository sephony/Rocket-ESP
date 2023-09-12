#ifndef MAIN_H_
#define MAIN_H_

#include <ESP32Servo.h>
#include <ICM42688.h>
#include <MS5611.h>

#include "PinConfig.h"

enum mission_stage_t {
    STAND_BY,
    PRE_LAUNCH,
    LAUNCHED,
    DETACHED,
    PRELAND,
    LANDED
};
extern mission_stage_t mission_stage;
extern MS5611 ms5611;  // ESP32 HW SPI
// an ICM42688 object with the ICM42688 sensor on SPI bus 0 and chip select pin 10
extern ICM42688 IMU;
extern Servo myservo1;
extern Servo myservo2;
#endif /* MAIN_H_ */
