#include <Arduino.h>

// FS
#include <FS.h>
#include <SPIFFS.h>

// Peripheral
#include <ESP32Servo.h>
#include <ICM42688.h>
#include <MS5611.h>

// basic
#include <math.h>
#include <stdlib.h>
#include <string.h>

// User header
#include "AP.h"
#include "Filter.h"
#include "PinConfig.h"
#include "RocketClient.h"
#include "UserFunction.h"
#include "Varibles.h"
#include "main.h"

/***** State Machine Definition *****/
mission_stage_t mission_stage;

/***** Class Definition *****/
MS5611_IIC ms5611_iic(pin_sda, pin_scl);  // ESP32 IIC
MS5611 ms5611(ms5611_iic);
// an ICM42688 object with the ICM42688 sensor on SPI bus 0 and chip select pin 10
// SPIClass SPI_IMU(FSPI);
// ICM42688 IMU(SPI_IMU, ICM42688_CS);

ESP32PWM pwm_para;
ESP32PWM pwm_servo_1;
ESP32PWM pwm_servo_2;

/***** Initialization Fuction Definition *****/
void setup() {
    Serial.begin(115200);
    Serial.println("****************** INITIALIZATION START ******************");

    /** init gpio **/
    pinMode(pin_fire, OUTPUT);
    pinMode(pin_parachute, OUTPUT);
    pinMode(pin_RGB, OUTPUT);
    pinMode(pin_run_sign, OUTPUT);
    pinMode(pin_fire_sign, OUTPUT);
    pinMode(pin_parachute_sign, OUTPUT);

    digitalWrite(pin_fire, LOW);
    digitalWrite(pin_parachute, LOW);
    digitalWrite(pin_fire_sign, HIGH);
    digitalWrite(pin_parachute_sign, HIGH);
    digitalWrite(pin_run_sign, HIGH);

    /* init EXTI */
    pinMode(pin_EXTI, INPUT_PULLDOWN);
    attachInterrupt(pin_EXTI, EXTI_Interrupt, FALLING);
    Serial.println("GPIO initalized!");

    /** init MS5611 **/
    if (ms5611.begin()) {
        Serial.print("MS5611 found! ID: ");
        Serial.println(ms5611.getDeviceID(), HEX);
    } else {
        Serial.println("MS5611 not found.");
    }
    float altitude_sum = 0;

    ms5611.setOversampling(OSR_STANDARD);
    for (int i = 0; i < 100; i++) {
        READ_5611(ms5611);
        altitude_sum += height;
        delay(30);  // 经验数据,不要改(标准模式下执行一次main循环需要74ms)
    }
    H0 = altitude_sum / 100.0;
    ms5611.list();
    Serial.printf("MS5611 initalized!");
    Serial.printf("The initial height is: %.4f m\n", H0);

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

    /** init IMU42688 **/
    // int status = IMU.begin();
    // if (status < 0) {
    //     Serial.println("IMU initialization unsuccessful");
    //     Serial.println("Check IMU wiring or try cycling power");
    //     Serial.print("Status: ");
    //     Serial.println(status);
    // }
    // Serial.println("ax,ay,az,gx,gy,gz,temp_C");
    // Serial.println("IMU42688 initalized!");

    /** init servo **/
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    pwm_para.attachPin(38, 50, 10);
    pwm_servo_1.attachPin(pin_servo_1, 50, 10);
    pwm_servo_2.attachPin(pin_servo_2, 50, 10);
    Serial.println("Servo initalized!");

    /** init FS **/
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
    }
    SPIFFS.mkdir("/data");
    // SPIFFS.format();//格式化文件系统，仅在清除飞行记录时使用
    Serial.println("SPIFFS mounted!");
    Serial.printf("SPIFFS total memory: %d (Byte)\n", SPIFFS.totalBytes());
    Serial.printf("SPIFFS used memory: %d (Byte)\n", SPIFFS.usedBytes());

    // TODO 列出文件系统中的目录与文件
    File root = SPIFFS.open("/");
    File file_temp = root.openNextFile();
    while (file_temp) {
        if (file_temp.isDirectory()) {
            dirs.push_back(file_temp.name());
        } else {
            files.push_back({file_temp.name(), file_temp.path()});
        }
        file_temp = root.openNextFile();
    }
    // 根据文件名输出文件内容
    // String fileName = "2023_10_05__23_45.txt";
    // File file = SPIFFS.open("/data/" + fileName, FILE_READ);
    // outputFile(file);
    Serial.println("SPIFFS file list:");
    for (auto file : files) {
        Serial.println(file[0].c_str());
    }

    // 把files第1，2个'_'替换为'-'，第四个后面的'_'替换为':'，并按时间从小到大排序
    for (auto& file : files) {
        if (std::count(file[0].begin(), file[0].end(), '_') >= 4) {
            file[0].replace(4, 1, "-");
            file[0].replace(7, 1, "-");
            file[0].replace(10, 1, " ");
            file[0].replace(11, 1, " ");
            file[0].replace(14, 1, ":");
        } else
            Serial.printf("Error: file name %s format error!", file[0].c_str());
    }

    Serial.println("SPIFFS dir list:");
    std::sort(files.begin(), files.end(), [](const std::array<std::string, 2>& a, const std::array<std::string, 2>& b) {
        return a[0] < b[0];
    });

    group_mode.addItem(&RunMode_Param);
    group_mode.addItem(&ParaMode_Param);
    group_mode.addItem(&LaunchReadyParam);
    group_time.addItem(&dateParam);
    group_time.addItem(&timeParam);
    group_heightControl.addItem(&H_ParaParam);
    group_heightControl.addItem(&T_ProtectParam);
    group_timeControl.addItem(&T_DetachParam);
    group_timeControl.addItem(&T_ParaParam);
    group_mqtt.addItem(&mqttServerParam);
    group_mqtt.addItem(&mqttUserNameParam);
    group_mqtt.addItem(&mqttUserPasswordParam);
    group_RGB.addItem(&RGB_BrightnessParam);

    iotWebConf.addParameterGroup(&group_mode);
    iotWebConf.addParameterGroup(&group_time);
    iotWebConf.addParameterGroup(&group_heightControl);
    iotWebConf.addParameterGroup(&group_timeControl);
    iotWebConf.addParameterGroup(&group_mqtt);
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
    server.on("/dir", handleDir);
    server.onNotFound([]() {
        if (!handleFileRead(server.uri())) {
            iotWebConf.handleNotFound();
        }
    });

    mqttClient.begin(mqttServerValue, net);
    mqttClient.onMessage(mqttMessageReceived);

    Serial.println("HTTP server started");

    // 读取存储在flash中的配置参数
    T_detach = atof(T_DetachValue);
    T_para = atof(T_ParaValue);
    H_para = atof(H_ParaValue);
    T_protectPara = atof(T_ProtectValue);
    rgbBrightness = atoi(RGB_BrightnessValue);

    lastRunMode = (String)RunModeValue;
    lastParaMode = (String)ParaModeValue;
    lastLaunchReady = (String)LaunchReadyValue;

    Serial.println("****************** INITIALIZATION DONE ******************");
}

/***** Loop Function Definition *****/
void loop() {
    iotWebConf.doLoop();
    // server.handleClient(); //好像iotWebConf.doLoop函数会调用这个，所以不需要

    if (sign_needReset || !digitalRead(pin_key_ap)) {
        Serial.println("Rebooting after 1 second.");
        delay(1000);
        ESP.restart();
        sign_needReset = false;
    }

    if (String(RunModeValue) == "Standard") {
        // TODO: auto create new file
        if (!sign_setTime) {
            if (sign_beginNTPClient) {
                // TODO: 获取日期，包括年月日时分秒，格式为：2004_10_28__23_14
                timeClient.update();
                auto minute = timeClient.getMinutes();
                auto hour = timeClient.getHours();

                String date = dateParam.value();
                // 将date字符串中的'-'替换为'_'
                date.replace("-", "_");
                nowTime = date + "__" + (String)(hour + 8) + "_" + (String)minute;
                fileName = "/data/" + nowTime + ".txt";
                Serial.println(nowTime);
                sign_setTime = true;
                // Serial.println("Get time from NTP server!");
            } else {
                String date = dateParam.value();
                String time = timeParam.value();
                date.replace("-", "_");
                time.replace(":", "_");
                fileName = "/data/" + date + "__" + time + ".txt";
                sign_setTime = true;
            }
        }

        static File file;
        file = SPIFFS.open(fileName, FILE_APPEND);

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
        READ_5611(ms5611);
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

        switch (mission_stage) {
        case STAND_BY:
            neopixelWrite(pin_RGB, rgbBrightness, rgbBrightness, rgbBrightness);
            if (String(LaunchReadyValue) == "selected") {
                mission_stage = PRE_LAUNCH;
                digitalWrite(pin_run_sign, HIGH);
                Serial.println("Mission start!");
            }
            break;

        case PRE_LAUNCH:
            neopixelWrite(pin_RGB, rgbBrightness, rgbBrightness, 0);
            break;

        case LAUNCHED:
            neopixelWrite(pin_RGB, rgbBrightness, 0, 0);
            // record height
            appendFile(file, InformationToPrint);
            // Serial.println(InformationToPrint);
            if ((millis() - time_launch) > T_detach * 1000) {
                digitalWrite(pin_fire, HIGH);
                digitalWrite(pin_fire_sign, LOW);
                mission_stage = DETACHED;
                Serial.println("Detached!");
                appendFile(file, "Detached!\r\n");
            }
            break;

        case DETACHED:
            neopixelWrite(pin_RGB, 0, rgbBrightness, 0);
            // record height
            appendFile(file, InformationToPrint);
            // Serial.println(InformationToPrint);

            if ((millis() - time_launch) > (T_detach + DELTA_T_DETACH) * 1000) {
                digitalWrite(pin_fire, LOW);
            }
            if (String(ParaModeValue) == "height control") {
                Serial.println("height control!");
                if (height_filter > H_para) {
                    Serial.println("height para!");
                    sign_parachute = true;
                    time_para = millis();
                    pwm_para.writeScaled(ANGLE(180));
                    digitalWrite(pin_parachute_sign, LOW);
                    mission_stage = PRELAND;
                    Serial.printf("Height Parachute on!\r\n");
                    appendFile(file, "Height Parachute on!\r\n");
                }
                if (((millis() - time_launch) > (T_protectPara * 1000)) && (sign_parachute == false)) {
                    Serial.println("time para!");
                    time_para = millis();
                    pwm_para.writeScaled(ANGLE(180));
                    digitalWrite(pin_parachute_sign, LOW);
                    mission_stage = PRELAND;
                    Serial.printf("Time Parachute on!\r\n");
                    appendFile(file, "Time Protect Parachute on!\r\n");
                }
            } else if (String(ParaModeValue) == "time control") {
                Serial.println("time control!");
                if ((millis() - time_launch) > (T_para * 1000)) {
                    time_para = millis();
                    pwm_para.writeScaled(ANGLE(180));
                    digitalWrite(pin_parachute_sign, LOW);
                    mission_stage = PRELAND;
                    Serial.printf("Time Parachute on!\r\n");
                    appendFile(file, "Time Parachute on!\r\n");
                }
            }
            break;

        case PRELAND:
            neopixelWrite(pin_RGB, 0, 0, rgbBrightness);
            appendFile(file, InformationToPrint);
            // Serial.println(InformationToPrint);
            if (height_filter < HEIGHT_LAND) {
                mission_stage = LANDED;
                Serial.printf("Preland!\r\n");
                appendFile(file, "Preland!\r\n");
            }
            break;

        case LANDED:
            neopixelWrite(pin_RGB, rgbBrightness, 0, rgbBrightness);
            appendFile(file, "Landed!\r\n");
            appendFile(file, "\n\n\n\n");
            digitalWrite(pin_fire_sign, HIGH);
            digitalWrite(pin_parachute_sign, HIGH);
            digitalWrite(pin_run_sign, HIGH);
            file.close();
            mission_stage = STAND_BY;
            // sign_initServo = false;
            break;

        default:
            break;
        }
    } else if (String(RunModeValue) == "Advanced") {
        mqttClient.loop();
        if (sign_needMqttConnect) {
            if (connectMqtt()) {
                sign_needMqttConnect = false;
            }
        } else if ((iotWebConf.getState() == iotwebconf::OnLine) && (!mqttClient.connected())) {
            Serial.println("MQTT reconnect");
            connectMqtt();
        }
        unsigned long now = millis();
        if ((500 < now - lastReport) && (mqttClient.connected())) {
            static File file = SPIFFS.open(fileName, FILE_READ);
            String data = file.readString();
            lastReport = now;
            Serial.print("Sending on MQTT channel 'test/rocket/data' :");
            Serial.println(data);
            mqttClient.publish("test/rocket/data", data);
        }
    } else if (String(RunModeValue) == "Debug") {
        // TODO: Debug
        static bool open_servo_1 = false;
        static bool open_servo = false;
        neopixelWrite(pin_RGB, rgbBrightness, rgbBrightness, rgbBrightness);

        // 舵机一直来回转
        //  for (float angle = ANGLE(0); angle <= ANGLE(180); angle += 0.001) {
        //      pwm_servo_1.writeScaled(angle);
        //      pwm_servo_2.writeScaled(angle);
        //      delay(15);
        //  }
        //  for (float angle = ANGLE(180); angle >= ANGLE(0); angle -= 0.001) {
        //      pwm_servo_1.writeScaled(angle);
        //      pwm_servo_2.writeScaled(angle);
        //      delay(15);
        //  }

        // 舵机来回只转一次
        if (!open_servo) {
            if (open_servo_1) {
                for (float angle = ANGLE(0); angle <= ANGLE(180); angle += 0.001) {
                    // Write a unit vector value from 0.0 to 1.0
                    pwm_servo_1.writeScaled(angle);
                    Serial.println(angle);
                    pwm_servo_2.writeScaled(angle);
                    delay(15);
                }
                for (float angle = ANGLE(180); angle >= ANGLE(0); angle -= 0.001) {
                    Serial.println(angle);
                    pwm_servo_1.writeScaled(angle);
                    pwm_servo_2.writeScaled(angle);
                    delay(15);
                    open_servo_1 = true;
                }
                open_servo = true;
            } else {
                for (float angle = ANGLE(0); angle <= ANGLE(180); angle += 0.001) {
                    // Write a unit vector value from 0.0 to 1.0
                    pwm_servo_1.writeScaled(angle);
                    delay(15);
                }
                for (float angle = ANGLE(180); angle >= ANGLE(0); angle -= 0.001) {
                    pwm_servo_1.writeScaled(angle);
                    delay(15);
                }
                open_servo_1 = true;
            }
        }

    } else if (String(RunModeValue) == "Real-time Wireless Serial") {
        auto start = millis();
        // TODO: Real-time Wireless Serial
        // Serial.println("Real-time Wireless Serial");
        READ_5611(ms5611);
        delay(30);
        height -= H0;
        height_filter -= H0;
        rx_data = String(height) + ',' + String(height_filter);
        Serial.println(height_filter);
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Connecting to WiFi..");
            delay(1000);
            neopixelWrite(pin_RGB, 0, 0, rgbBrightness);
        } else {
            // 如果没有连接到服务器
            if (!client.connected()) {  // Serial.println("1");
                neopixelWrite(pin_RGB, rgbBrightness, 0, 0);
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
                neopixelWrite(pin_RGB, 0, rgbBrightness, 0);
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
