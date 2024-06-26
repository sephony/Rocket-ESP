#ifndef PINCONFIG_H_
#define PINCONFIG_H_

/****************************** Macro Definition ******************************/
/* GPIO Definition */
#define GPIO_F1 4  // 点火
#define GPIO_F2 5  // 开伞
#define GPIO_F3 6  // 舵机
#define GPIO_S1 47  // 舵机1
#define GPIO_S2 48  // 舵机2

// #define GPIO_S1 2  // 舵机1
// #define GPIO_S2 6  // 舵机2

#define GPIO_F1_SIGN 15  // 点火指示灯
#define GPIO_F2_SIGN 16  // 开伞指示灯
#define GPIO_F3_SIGN 17  // 舵机指示灯

#define GPIO_EXTI 1  // 外部中断
#define GPIO_RGB 9   // RGB灯，用于烧录到火箭板子
// 当build_flags =
//      - D ARDUINO_USB_CDC_ON_BOOT = 1时选用
// #define GPIO_RGB LED_BUILTIN  // RGB灯，用于烧录到开发板

#define GPIO_KEY_BOOT 21  // 按键
#define GPIO_KEY_AP 35
#define GPIO_KEY_SIGN 18  // 按键指示灯

/* Sensor's GPIO */
#define IIC_SCL 36
#define IIC_SDA 37

#define SPI_SCK 13
#define SPI_MISO 11
#define SPI_MOSI 12

// MS5611
#define MS5611_CS 10

// ICM42688
#define ICM42688_CS 14

/* 复用 */
#define GPIO_FIRE GPIO_F1
#define GPIO_PARACHUTE GPIO_F2

#define GPIO_RUN_SIGN GPIO_F1_SIGN
#define GPIO_FIRE_SIGN GPIO_F2_SIGN
#define GPIO_PARACHUTE_SIGN GPIO_F3_SIGN

#define GPIO_SERVO_1 GPIO_S1
#define GPIO_SERVO_2 GPIO_S2

/* RGB LED Definition */

#define RGB_BRIGHTNESS_USER 64
#define RGB_BRIGHTNESS_MAX 255
#define RGB_LOOP_TIME 8
#define RGB_TWINKLE_TIME ((int)((RGB_LOOP_TIME * 1000) / (RGB_BRIGHTNESS_USER * 3)))

/* State Definition */
#define KEY_IS_PRESSED 0    // 按键按下
#define HALL_IS_LAUNCHED 1  // 霍尔触发

/* Time Definition */
// #define T_DETACH 1 // 二级点火时间1s
#define DELTA_T_DETACH 1  // 分离间隔1s
// #define T_PARACHUTE 6 // 开伞时间6s
#define DELTA_T_PARACHUTE 1  // 开伞间隔1s

/* Height Definition */
// #define HEIGHT_PARACHUTE 60
#define HEIGHT_LAND 5  // 着陆高度5m

/****************************** Peripheral Pins *******************************/
// GPIO
constexpr uint8_t pin_fire = GPIO_FIRE;
constexpr uint8_t pin_parachute = GPIO_PARACHUTE;
constexpr uint8_t pin_run_sign = GPIO_RUN_SIGN;
constexpr uint8_t pin_fire_sign = GPIO_FIRE_SIGN;
constexpr uint8_t pin_parachute_sign = GPIO_PARACHUTE_SIGN;
constexpr uint8_t pin_servo_1 = GPIO_SERVO_1;
constexpr uint8_t pin_servo_2 = GPIO_SERVO_2;

constexpr uint8_t pin_EXTI = GPIO_EXTI;
constexpr uint8_t pin_RGB = GPIO_RGB;

constexpr uint8_t pin_key_boot = GPIO_KEY_BOOT;
constexpr uint8_t pin_key_ap = GPIO_KEY_AP;
constexpr uint8_t pin_key_sign = GPIO_KEY_SIGN;

constexpr uint8_t pin_scl = IIC_SCL;
constexpr uint8_t pin_sda = IIC_SDA;

constexpr uint8_t pin_sck = SPI_SCK;
constexpr uint8_t pin_miso = SPI_MISO;
constexpr uint8_t pin_mosi = SPI_MOSI;

constexpr uint8_t pin_ms5611 = MS5611_CS;
constexpr uint8_t pin_icm42688 = ICM42688_CS;

#endif /* PINCONFIG_H_ */
