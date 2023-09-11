#include <Arduino.h>
#include <ESP32Servo.h>

#include <string>
#include <vector>

#include "AP.h"
#include "Filter.h"
#include "PinConfig.h"
#include "RocketClient.h"
#include "Varibles.h"

// FS
#include <FS.h>
#include <SPIFFS.h>

// Sensors
#include <ICM42688.h>
#include <MS5611.h>
#include <Wire.h>

// #include "MS5611.h"

// basic
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/***** Function Declaration *****/
float getHeight(MS5611 ms5611);
void READ_5611();                                                           // 读取MS5611
void appendFile(FS& fs, const char* path, const char* message, File file);  // 追加文件
String getContentType(String filename);                                     // 获取文件类型
bool exists(String path);                                                   // 判断文件是否存在
bool handleFileRead(String path);                                           // 处理文件读取

/* FSBroswer Varibles */
// const char* ssid = "wifi-233";
// const char* password = "wifi-password";
// const char* host = "rocket";

/***** State Machine Definition *****/
enum mission_stage_t {
    STAND_BY,
    PRE_LAUNCH,
    LAUNCHED,
    DETACHED,
    PRELAND,
    LANDED
};
mission_stage_t MISSION_STAGE;

/***** Interrupt Function Definition *****/
void EXTI_Interrupt(void) {
    if (MISSION_STAGE == PRE_LAUNCH) {
        time_launch = millis();  // 单位ms
        digitalWrite(GPIO_RUN_SIGN, HIGH);
        MISSION_STAGE = LAUNCHED;
        Serial.printf("External Interrupt Called\r\n");
        // appendFile(SPIFFS, "/a.txt", "Hall Interrupt Called!\r\n", file);
    }
}

bool handleFileRead(String path) {
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) {
        path += "index.htm";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (exists(pathWithGz) || exists(path)) {
        if (exists(pathWithGz)) {
            path += ".gz";
        }
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

/***** Class Definition *****/
MS5611 ms5611(0x77);  // ESP32 HW SPI
// an ICM42688 object with the ICM42688 sensor on SPI bus 0 and chip select pin 10
ICM42688 IMU(SPI, ICM42688_CS);
Servo myservo1;
Servo myservo2;

/***** Initialization Fuction Definition *****/
void setup() {
    Serial.begin(115200);
    Serial.println("Program Started!");

    /** init gpio **/
    pinMode(GPIO_FIRE, OUTPUT);
    pinMode(GPIO_PARACHUTE, OUTPUT);
    pinMode(GPIO_RGB, OUTPUT);
    pinMode(GPIO_FIRE_SIGN, OUTPUT);
    pinMode(GPIO_PARACHUTE_SIGN, OUTPUT);
    pinMode(GPIO_RUN_SIGN, OUTPUT);

    digitalWrite(GPIO_FIRE, LOW);
    digitalWrite(GPIO_PARACHUTE, LOW);
    digitalWrite(GPIO_FIRE_SIGN, HIGH);
    digitalWrite(GPIO_PARACHUTE_SIGN, HIGH);
    digitalWrite(GPIO_RUN_SIGN, HIGH);

    /* init EXTI */
    pinMode(GPIO_EXTI, INPUT_PULLDOWN);
    attachInterrupt(GPIO_EXTI, EXTI_Interrupt, FALLING);
    Serial.println("GPIO initalized!");

    /** init MS5611 **/
    if (ms5611.begin(IIC_SDA, IIC_SCL)) {
        Serial.print("MS5611 found! ID: ");
        Serial.println(ms5611.getDeviceID(), HEX);
    } else {
        Serial.println("MS5611 not found.");
    }
    float altitude_sum = 0;

    for (int i = 0; i < 100; i++) {
        READ_5611();
        altitude_sum += height;
        delay(50);  // 经验数据,不要改(标准模式下执行一次main循环需要74ms)
    }
    H0 = altitude_sum / 100.0;
    Serial.printf("Initial height is:%.4f/r/n", H0);

    // start communication with IMU
    // int status = IMU.begin();
    // if (status < 0) {
    //     Serial.println("IMU initialization unsuccessful");
    //     Serial.println("Check IMU wiring or try cycling power");
    //     Serial.print("Status: ");
    //     Serial.println(status);
    //     while (1) {
    //     }
    // }
    // Serial.println("ax,ay,az,gx,gy,gz,temp_C");

    /** init servo **/
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    myservo1.setPeriodHertz(50);
    myservo2.setPeriodHertz(50);
    myservo1.attach(GPIO_SERVO_1, 1000, 2000);
    myservo2.attach(GPIO_SERVO_2, 1000, 2000);
    myservo1.write(0);
    myservo2.write(0);
    Serial.println("Servo initalized!");

    /** init FS **/
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
    }
    Serial.println("SPIFFS mounted!");
    Serial.printf("SPIFFS total memory: %d (Byte)\n", SPIFFS.totalBytes());
    Serial.printf("SPIFFS used memory: %d (Byte)\n", SPIFFS.usedBytes());

    // TODO 列出文件系统中的目录与文件
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.println(file.path());
        file = root.openNextFile();
    }
    group_mode.addItem(&RunMode_Param);
    group_mode.addItem(&ParaMode_Param);
    group_mode.addItem(&LaunchReadyParam);
    group_time.addItem(&dateParam);
    group_time.addItem(&timeParam);
    group_heightControl.addItem(&H_ParaParam);
    group_heightControl.addItem(&T_ProtectParam);
    group_timeControl.addItem(&T_DetachParam);
    group_timeControl.addItem(&T_ParaParam);
    group_RGB.addItem(&RGB_BrightnessParam);

    iotWebConf.addParameterGroup(&group_mode);
    iotWebConf.addParameterGroup(&group_time);
    iotWebConf.addParameterGroup(&group_heightControl);
    iotWebConf.addParameterGroup(&group_timeControl);
    iotWebConf.addParameterGroup(&group_RGB);

    iotWebConf.setConfigPin(CONFIG_PIN);
    // iotWebConf.setStatusPin(STATUS_PIN);
    // -- Note: multipleWifiAddition.init() calls setFormValidator, that
    // overwrites existing formValidator setup. Thus setFormValidator
    // should be called _after_ multipleWifiAddition.init() .
    multipleWifiAddition.init();
    // -- Define how to handle updateServer calls.
    iotWebConf.setupUpdateServer(
        [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
        [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
    iotWebConf.setWifiConnectionCallback(&wifiConnected);
    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setFormValidator(&formValidator);
    iotWebConf.setHtmlFormatProvider(&optionalGroupHtmlFormatProvider);
    iotWebConf.getApTimeoutParameter()->visible = true;
    iotWebConf.init();

    server.on("/", handleRoot);
    server.on("/config", [] { iotWebConf.handleConfig(); });
    // server.on("/dir", handleDir);
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            iotWebConf.handleNotFound();
        }
    });
    Serial.println("HTTP server started");
    Serial.println("initialize done!");

    T_DETACH = strtof(T_DetachValue, NULL);
    T_PARACHUTE = strtof(T_ParaValue, NULL);
    HEIGHT_PARACHUTE = atoi(H_ParaValue);
    Serial.print("T_DETACH is");
    Serial.println(T_DETACH);
    Serial.print("T_PARACHUTE is");
    Serial.println(T_PARACHUTE);
    Serial.print("HEIGHT_PARACHUTE is");
    Serial.println(HEIGHT_PARACHUTE);

    auto start = millis();
    ms5611.read();
    auto stop = millis();
    Serial.print("Temperature: ");
    Serial.print(ms5611.getTemperature(), 2);
    Serial.print(" C, Pressure: ");
    Serial.print(ms5611.getPressure(), 2);
    Serial.print(" mBar, Duration: ");
    Serial.print(stop - start);
    Serial.println(" ms");
    // writeFile(SPIFFS, "/a.txt", "Start Of Program!\r\n");
    // TODO:LED STATE
}

/***** Loop Function Definition *****/
void loop() {
    iotWebConf.doLoop();
    // server.handleClient();

    if (String(RunModeValue) == "Standard") {
        // TODO: auto create new file
        if (!sign_setTime) {
            // Serial.println("Get time from NTP server!");
            if (sign_beginNTPClient) {
                // TODO: 获取日期，包括年月日时分秒，格式为：2004-10-28  23:14:05
                timeClient.update();
                auto second = timeClient.getSeconds();
                auto minute = timeClient.getMinutes();
                auto hour = timeClient.getHours();

                String date = dateParam.value();
                nowTime = date + "  " + (String)(hour + 8) + ":" + (String)minute + ":" + (String)second;
                fileName = "/data/" + nowTime + ".txt";
                Serial.println(nowTime);
                sign_setTime = true;
            } else {
                String date = dateParam.value();
                String time = timeParam.value();
                fileName = "/data/" + date + " " + time + ".txt";
                sign_setTime = true;
            }
        }
        SPIFFS.mkdir("/data");
        static File file = SPIFFS.open(fileName, FILE_APPEND);

        static auto t_start = millis();
        // // read the sensor
        // IMU.getAGT();
        // // display the data
        // Serial.print(IMU.accX(), 6);
        // Serial.print("\t");
        // Serial.print(IMU.accY(), 6);
        // Serial.print("\t");
        // Serial.print(IMU.accZ(), 6);
        // Serial.print("\t");
        // Serial.print(IMU.gyrX(), 6);
        // Serial.print("\t");
        // Serial.print(IMU.gyrY(), 6);
        // Serial.print("\t");
        // Serial.print(IMU.gyrZ(), 6);
        // Serial.print("\t");
        // Serial.println(IMU.temp(), 6);
        // delay(100);
        READ_5611();
        height -= H0;
        height_filter -= H0;

        current_time = (millis() - time_launch) / 1000.0;
        sprintf(str_time, "%.4f", current_time);
        sprintf(str_height, "%.2f", height);
        sprintf(str_height_filter, "%.2f", height_filter);

        strcpy(InformationToPrint, "");
        strcat(InformationToPrint, str_time);
        strcat(InformationToPrint, "\t ");
        strcat(InformationToPrint, str_height);
        strcat(InformationToPrint, "\t ");
        strcat(InformationToPrint, str_height_filter);
        strcat(InformationToPrint, "\t ");
        strcat(InformationToPrint, "\r\n");

        switch (MISSION_STAGE) {
        case STAND_BY:
            neopixelWrite(GPIO_RGB, atoi(RGB_BrightnessValue), 0, 0);
            if (atoi(LaunchReadyValue) == true) {
                MISSION_STAGE = PRE_LAUNCH;
                digitalWrite(GPIO_RUN_SIGN, LOW);
                Serial.println("Mission start!");
                appendFile(SPIFFS, "/a.txt", fileName.c_str(), file);
                appendFile(SPIFFS, "/a.txt", "KEY IS PRESSED!\r\n", file);
            }
            break;

        case PRE_LAUNCH:
            neopixelWrite(GPIO_RGB, atoi(RGB_BrightnessValue), atoi(RGB_BrightnessValue), 0);
            break;

        case LAUNCHED:
            neopixelWrite(GPIO_RGB, 0, atoi(RGB_BrightnessValue), 0);
            // record height
            appendFile(SPIFFS, "/a.txt", InformationToPrint, file);
            // Serial.println(InformationToPrint);
            if ((millis() - time_launch) > T_DETACH * 1000) {
                digitalWrite(GPIO_FIRE, HIGH);
                digitalWrite(GPIO_FIRE_SIGN, LOW);
                MISSION_STAGE = DETACHED;
                // timerAlarmDisable(timer);
                Serial.println("Detached!");
                appendFile(SPIFFS, "/a.txt", "Detached!\r\n", file);
            }

            break;

        case DETACHED:
            neopixelWrite(GPIO_RGB, 0, atoi(RGB_BrightnessValue), atoi(RGB_BrightnessValue));
            // record height
            appendFile(SPIFFS, "/a.txt", InformationToPrint, file);
            // Serial.println(InformationToPrint);

            if ((millis() - time_launch) > (T_DETACH + DELTA_T_DETACH) * 1000) {
                digitalWrite(GPIO_FIRE, LOW);
            }
            if (String(ParaMode_Value) == "height control") {
                if (height_filter > HEIGHT_PARACHUTE) {
                    Sign_Parachute = true;
                    time_para = millis();
                    myservo1.write(180);
                    myservo2.write(180);
                    digitalWrite(GPIO_PARACHUTE_SIGN, LOW);
                    MISSION_STAGE = PRELAND;
                    Serial.printf("Height Parachute on!\r\n");
                    appendFile(SPIFFS, "/a.txt", "Height Parachute on!\r\n", file);
                }
                if (((millis() - time_launch) > (atof(T_ProtectValue) * 1000)) && (Sign_Parachute == false)) {
                    time_para = millis();
                    myservo1.write(180);
                    myservo2.write(180);
                    digitalWrite(GPIO_PARACHUTE_SIGN, LOW);
                    MISSION_STAGE = PRELAND;
                    Serial.printf("Time Parachute on!\r\n");
                    appendFile(SPIFFS, "/a.txt", "Time Parachute on!\r\n", file);
                }
            } else if (String(ParaMode_Value) == "time control") {
                if ((millis() - time_launch) > (T_PARACHUTE * 1000)) {
                    time_para = millis();
                    myservo1.write(180);
                    myservo2.write(180);
                    digitalWrite(GPIO_PARACHUTE_SIGN, LOW);
                    MISSION_STAGE = PRELAND;
                    Serial.printf("Time Parachute on!\r\n");
                    appendFile(SPIFFS, "/a.txt", "Time Parachute on!\r\n", file);
                }
                break;
            }
        case PRELAND:
            neopixelWrite(GPIO_RGB, 0, 0, atoi(RGB_BrightnessValue));
            // record height
            appendFile(SPIFFS, "/a.txt", InformationToPrint, file);
            // Serial.println(InformationToPrint);
            if (height_filter < HEIGHT_LAND) {
                MISSION_STAGE = LANDED;
                Serial.printf("Preland!\r\n");
                appendFile(SPIFFS, "/a.txt", "Preland!\r\n", file);
            }
            break;

        case LANDED:
            neopixelWrite(GPIO_RGB, atoi(RGB_BrightnessValue), atoi(RGB_BrightnessValue), atoi(RGB_BrightnessValue));
            // stop record
            appendFile(SPIFFS, "/a.txt", "Landed!\r\n", file);
            file.close();
            digitalWrite(GPIO_FIRE_SIGN, HIGH);
            digitalWrite(GPIO_PARACHUTE_SIGN, HIGH);
            digitalWrite(GPIO_RUN_SIGN, HIGH);
            // myservo.write(0);
            MISSION_STAGE = STAND_BY;
            break;

        default:
            break;
        }
    } else if (String(RunModeValue) == "Advanced") {
        // TODO: Advanced
    } else if (String(RunModeValue) == "debug") {
        // TODO: Debug
    } else if (String(RunModeValue) == "Real-time Wireless Serial") {
        auto start = millis();
        // TODO: Real-time Wireless Serial
        // Serial.println("Real-time Wireless Serial");
        READ_5611();
        delay(30);
        height -= H0;
        height_filter -= H0;
        rx_data = String(height) + ',' + String(height_filter);
        Serial.println(height_filter);
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Connecting to WiFi..");
            delay(1000);
            neopixelWrite(GPIO_RGB, 0, 0, atoi(RGB_BrightnessValue));
        } else {
            // 如果没有连接到服务器
            if (!client.connected()) {  // Serial.println("1");
                neopixelWrite(GPIO_RGB, atoi(RGB_BrightnessValue), 0, 0);
                Serial.println("Waiting for reconnection with server.");
                client.stop();
                delay(1);
                if (client.connect(host, httpPort)) {  // Serial.println("3");
                    Serial.println("Reconnection successful with server.");
                    // Tcp_Handler(Read_Tcp());
                    // Serial_callback();
                    wifiClientRequest(rx_data);  // 向客户端发送数据
                }
            } else {
                neopixelWrite(GPIO_RGB, 0, atoi(RGB_BrightnessValue), 0);
                // Tcp_Handler(Read_Tcp());
                // Serial_callback();
                wifiClientRequest(rx_data);  // 向客户端发送数据
            }
        }
        auto stop = millis();
        Serial.print("Duration: ");
        Serial.print(stop - start);
        Serial.println(" ms");
    }
}
float getHeight(MS5611 ms5611) {
    float height = 44330.0 * (1.0 - pow(ms5611.getPressure() / 1013.25, 1 / 5.255));  // barometric
    // float height = (pow((1013.25 / getPressure()), 1.0 / 5.257) - 1) * (getTemperature() + 273.15) / 0.65;  // hypsometric
    return height;
}
void READ_5611() {
    if ((ms5611.read()) != MS5611_READ_OK) {
        Serial.print("Error in read: ");
    } else {
        // Serial.printf("Preassure: %.2f\r\n", MS5611.getPressure());
        height = getHeight(ms5611);
        AltitudeLPF_50.input = height;
        height_filter = Butterworth50HzLPF(&AltitudeLPF_50);
        // AltitudeLPF_50.input = height_filter;
        // height_filter = Butterworth50HzLPF(&AltitudeLPF_50);
    }
}

void appendFile(FS& fs, const char* path, const char* message, File file) {
    Serial.printf("Appending to file: %s\r\n", path);
    if (!file) {
        Serial.println("- failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
}

String getContentType(String filename) {
    if (server.hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    } else
        return "text/plain";
}

bool exists(String path) {
    bool status = false;
    File file = SPIFFS.open(path, "r");
    status = !file.isDirectory();
    file.close();
    return status;
}
