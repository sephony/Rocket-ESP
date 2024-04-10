#include "AP.h"

std::string html_file_path = "/test/Rocket.html";  // html文件路径,暂时没用
//  -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.

char RunModeValue[NUMBER_LEN];
const char RunModeValues[][STRING_LEN] = {"Standard", "Advanced", "Debug", "Real-time Wireless Serial"};
const char RunModeNames[][STRING_LEN] = {"标准模式", "高级模式", "调试模式", "实时无线串口模式"};
char ParaModeValue[STRING_LEN];
const char ParaModeValues[][STRING_LEN] = {"time control", "height control"};
const char ParaModeNames[][STRING_LEN] = {"定时开伞", "定高开伞"};
char LaunchReadyValue[STRING_LEN];

char H_ParaValue[NUMBER_LEN];     // 开伞高度
char T_ProtectValue[NUMBER_LEN];  // 保护时间
char T_DetachValue[NUMBER_LEN];   // 二级点火时间
char T_ParaValue[NUMBER_LEN];     // 开伞时间

char RGB_BrightnessValue[NUMBER_LEN];  // RGB亮度

char mqttServerValue[STRING_LEN];
char mqttUserNameValue[STRING_LEN];
char mqttUserPasswordValue[STRING_LEN];

const char thingName[] = "Rocket";  // -- Name of the Thing. Used e.g. as SSID of the own Access Point.
// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "88888888";

// DNS server
DNSServer dnsServer;
// HTTP server
WebServer server(80);
// HTTP Update Server
HTTPUpdateServer httpUpdater;
// MQTT 远程发送
WiFiClient net;
MQTTClient mqttClient;
// 实时获取时间
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Web configuration instance.
IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
// WiFi 设置链(三组)
iotwebconf::ChainedWifiParameterGroup chainedWifiParameterGroups[] = {
    iotwebconf::ChainedWifiParameterGroup("wifi1"),
    iotwebconf::ChainedWifiParameterGroup("wifi2"),
    iotwebconf::ChainedWifiParameterGroup("wifi3")};

iotwebconf::MultipleWifiAddition multipleWifiAddition(
    &iotWebConf,
    chainedWifiParameterGroups,
    sizeof(chainedWifiParameterGroups) / sizeof(chainedWifiParameterGroups[0]));

iotwebconf::OptionalGroupHtmlFormatProvider optionalGroupHtmlFormatProvider;

// Parameter group.
ParameterGroup group_mode("_模式选择", "模式选择");
ParameterGroup group_time("_日期与时间", "日期与时间");
ParameterGroup group_heightControl("_高度控制", "高度控制");
ParameterGroup group_timeControl("_时间控制", "时间控制");
ParameterGroup group_mqtt("mqtt", "MQTT configuration");
ParameterGroup group_RGB("_RGB灯", "RGB灯");

// Parameters.
// TextParameter FileNameParam("文件名", "FileName", FileNameValue, STRING_LEN);
SelectParameter RunMode_Param("运行模式", "RunMode_Param", RunModeValue, STRING_LEN,
                              (const char *)RunModeValues,
                              (const char *)RunModeNames,
                              sizeof(RunModeValues) / STRING_LEN,
                              STRING_LEN,
                              "Standard");

SelectParameter ParaMode_Param("开伞模式", "LaunchMode", ParaModeValue, STRING_LEN,
                               (const char *)ParaModeValues,
                               (const char *)ParaModeNames,
                               sizeof(ParaModeValues) / STRING_LEN,
                               STRING_LEN,
                               "height control");

CheckboxParameter LaunchReadyParam("是否准备发射", "LaunchReadyParam", LaunchReadyValue, NUMBER_LEN,
                                   false);

NumberParameter T_DetachParam("二级点火时间", "T_DetachParam", T_DetachValue, NUMBER_LEN,
                              "1",
                              "e.g. 23.4",
                              "step='0.1'");

NumberParameter T_ParaParam("开伞时间", "T_ParaParam", T_ParaValue, NUMBER_LEN,
                            "6",
                            "e.g. 23.4",
                            "step='0.1'");

NumberParameter H_ParaParam("开伞高度", "H_ParaParam", H_ParaValue, NUMBER_LEN,
                            "60",
                            "1..100",
                            "min='0' max='100' step='1'");

NumberParameter T_ProtectParam("保护开伞时间", "T_ProtectParam", T_ProtectValue, NUMBER_LEN,
                               "8",
                               "e.g. 23.4",
                               "step='0.1'");

iotwebconf::DateTParameter dateParam =
    iotwebconf::Builder<iotwebconf::DateTParameter>("dateParam")
        .label("选择日期")
        .defaultValue("2023-09-05")
        .build();

iotwebconf::TimeTParameter timeParam =
    iotwebconf::Builder<iotwebconf::TimeTParameter>("timeParam")
        .label("选择时间")
        .defaultValue("12:00")
        .build();

TextParameter mqttServerParam("MQTT server", "mqttServer", mqttServerValue, STRING_LEN,
                              "public.cloud.shiftr.io");

TextParameter mqttUserNameParam("MQTT user", "mqttUser", mqttUserNameValue, STRING_LEN,
                                "public");

PasswordParameter mqttUserPasswordParam("MQTT password", "mqttPass", mqttUserPasswordValue, STRING_LEN,
                                        "public");

NumberParameter RGB_BrightnessParam("RGB亮度", "RGB_BrightnessParam", RGB_BrightnessValue, NUMBER_LEN,
                                    "64",
                                    "0..255",
                                    "min='0' max='255' step='1'");
/* 模板定义 */
// iotwebconf::TextTParameter<STRING_LEN> FileNameParam =
//     iotwebconf::Builder<iotwebconf::TextTParameter<STRING_LEN>>("FileNameParam")
//         .label("文件名")
//         .defaultValue("")
//         .build();

// iotwebconf::FloatTParameter T_DetachParam =
//     iotwebconf::Builder<iotwebconf::FloatTParameter>("T_DetachParam")
//         .label("二级点火时间")
//         .defaultValue(1.0)
//         .step(0.1)
//         .placeholder("e.g. 23.4").
//         .build();

// iotwebconf::FloatTParameter T_ParaParam =
//     iotwebconf::Builder<iotwebconf::FloatTParameter>("T_ParaParam")
//         .label("开伞时间")
//         .defaultValue(6.0)
//         .step(0.1)
//         .placeholder("e.g. 23.4")
//         .build();

// iotwebconf::IntTParameter<int16_t> H_ParaParam =
//     iotwebconf::Builder<iotwebconf::IntTParameter<int16_t>>("H_ParaParam")
//         .label("开伞高度")
//         .defaultValue(20)
//         .min(1)
//         .max(100)
//         .step(1)
//         .placeholder("1..100")
//         .build();

// iotwebconf::SelectTParameter<STRING_LEN> RunMode_Param =
//     iotwebconf::Builder<iotwebconf::SelectTParameter<STRING_LEN>>("chooseParam")
//         .label("选择状态")
//         .optionValues((const char*)StateValues)
//         .optionNames((const char*)StateNames)
//         .optionCount(sizeof(StateValues) / STRING_LEN)
//         .nameLength(STRING_LEN)
//         .defaultValue("Parachute")
//         .build();

// iotwebconf::CheckboxTParameter LaunchReadyParam =
//     iotwebconf::Builder<iotwebconf::CheckboxTParameter>("isLaunchReady")
//         .label("是否准备发射")
//         .defaultValue(false)  // 不知为何，这里的默认值无效
//         .build();

void handleRoot() {
    // -- Let  test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal()) {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\">";
    s += "<meta charset=\"UTF-8\">";
    s += "<title>火箭调参</title>";
    s += "<style>";
    s += "body {margin: 0; padding: 0; background-color: #f2f2f2; font-family: Arial, sans-serif;}";
    s += "h1 {color: #333333; text-align: center;}";
    s += "ul {list-style-type: none; margin: 0; padding: 0;}";
    s += "li {padding: 12px; background-color: #ffffff; border-bottom: 1px solid #dddddd;}";
    s += "a {color: #333333; text-decoration: none;}";
    s += "p.bottom {position: fixed; bottom: 10px; left: 10px;}";
    s += "img {float: top; margin-top: 10px; /* margin-bottom: 10px; */}";
    s += ".sidebar {position: fixed; top: 0; left: -250px; width: 250px; height: 100%; background-color: #f2f2f2; box-shadow: 0 0 10px rgba(0, 0, 0, 0.5); transition: left 0.5s ease-in-out;}";
    s += ".sidebar.open {left: 0;}";
    s += ".sidebar ul {list-style: none; padding: 0; margin: 60px 0 0 0;}";
    s += ".sidebar li {padding: 10px; border-bottom: 1px solid #dddddd;}";
    s += ".sidebar li:last-child {border-bottom: none;}";
    s += ".sidebar li a {text-decoration: none; color: inherit;}";
    s += ".sidebar p {position: absolute;}";
    s += ".sidebar p.bottom {position: fixed; bottom: 10px; left: 10px;}";
    s += ".button {position: fixed; top: 10px; left: 10px; width: 50px; height: 50px; background-color: #f2f2f2; border-radius: 50%; box-shadow: 0 0 10px rgba(0, 0, 0, 0.5); cursor: pointer; transition: transform 0.1s ease-in-out;}";
    s += ".button:active {transform: scale(0.8);}";
    s += "</style>";
    s += "</head><body>";
    s += "<h1>火箭调参</h1>";
    s += "<ul>";
    s += "<li><strong>二级点火时间:</strong> " + String(atoi(T_DetachValue)) + "</li>";
    s += "<li><strong>开伞时间:</strong> " + String(atof(T_ParaValue)) + "</li>";
    s += "<li><strong>开伞高度:</strong> " + String(atof(H_ParaValue)) + "</li>";
    s += "<li><strong>保护开伞时间:</strong> " + String(atof(T_ProtectValue)) + "</li>";
    s += "<li><strong>日期:</strong> " + String(dateParam.value()) + "</li>";
    s += "<li><strong>时间:</strong> " + String(timeParam.value()) + "</li>";
    s += "<p class=\"bottom\">Made By <em><strong>HITMA Rocket Team</strong></p>";
    s += "</ul>";
    s += "<p>前往<a href='config' style='color: #007bff; text-decoration: none; font-weight: bold;'>配置页面</a>调整参数。</p>";
    s += "<div class=\"sidebar\"><ul>";
    s += "<li><a href='' style='color: #007bff; text-decoration: none; font-weight: bold;'>首页</a></li>";
    s += "<li><a href='dir' style='color: #007bff; text-decoration: none; font-weight: bold;'>高度数据</a></li>";
    s += "<li><a href='config' style='color: #007bff; text-decoration: none; font-weight: bold;'>配置</a></li>";
    s += "<li><a href='firmware' style='color: #007bff; text-decoration: none; font-weight: bold;'>固件更新</a></li><ul>";
    s += "<p>version 1.0</p></div>";
    s += "<div class=\"button\" onclick=\"toggleSidebar()\"></div>";
    s += "<script>";
    s += "function toggleSidebar() {";
    s += "var sidebar = document.querySelector('.sidebar');";
    s += "var button = document.querySelector('.button');";
    s += "sidebar.classList.toggle('open');}";
    s += "</script>";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}
void handleDir() {
    // 将files[0]依次列表形式输出，名字可点击为file[1]对应的目录
    String s = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\">";
    s += "<meta charset=\"UTF-8\">";
    s += "<title>高度数据</title>";
    s += "<style>";
    s += "body {margin: 0; padding: 0; background-color: #f2f2f2; font-family: Arial, sans-serif;}";
    s += "h1 {color: #333333; text-align: center;}";
    s += "ul {list-style-type: none; margin: 0; padding: 0;}";
    s += "li {padding: 12px; background-color: #ffffff; border-bottom: 1px solid #dddddd;}";
    s += "a {color: #333333; text-decoration: none;}";
    s += "</style>";
    s += "</head><body>";
    s += "<h1>高度数据目录</h1>";
    s += "<ul>";
    std::vector<String> name;
    std::vector<String> url;
    for (int i = 0; i < files.size(); i++) {
        name.push_back(files[i][0].c_str());
        url.push_back(files[i][1].c_str());
        s += "<li><a href='" + url[i] + "' style='color: #007bff; text-decoration: none; font-weight: bold;'>" + name[i] + "</a></li>";
    }
    s += "</ul>";
    s += "<p>前往<a href='/' style='color: #007bff; text-decoration: none; font-weight: bold;'>首页</a>。</p>";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}
void wifiConnected() {
    Serial.println("Wifi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    if (String(RunModeValue) == "Standard") {
        timeClient.begin();
        sign_beginNTPClient = true;
        Serial.println("TimeClient was started.");
    } else if (String(RunModeValue) == "Advanced") {
        sign_needMqttConnect = true;
    } else if (String(RunModeValue) == "Debug") {
    } else if (String(RunModeValue) == "Real-time Wireless Serial") {
    }
}
void configSaved() {
    Serial.println("Configuration was updated.");
    Serial.printf("Detach time is changed to:");
    Serial.println(T_DetachValue);
    Serial.printf("Parachute time is changed to:");
    Serial.println(T_ParaValue);
    Serial.printf("Parachute height is changed to:");
    Serial.println(H_ParaValue);

    T_detach = atof(T_DetachValue);
    T_para = atof(T_ParaValue);
    H_para = atof(H_ParaValue);
    T_protectPara = atof(T_ProtectValue);
    rgbBrightness = atoi(RGB_BrightnessValue);

    if (((String)RunModeValue != lastRunMode)) {
        Serial.printf("Run mode is changed to:");
        Serial.println(RunModeValue);
        lastRunMode = (String)RunModeValue;
        ESP.restart();
        // sign_needReset = true;
    }
}
bool formValidator(iotwebconf::WebRequestWrapper *webRequestWrapper) {
    Serial.println("Validating form.");
    bool valid = true;
    // -- Note: multipleWifiAddition.formValidator() should be called, as
    // we have override this setup.
    valid = multipleWifiAddition.formValidator(webRequestWrapper);
    /*
      int l = webRequestWrapper->arg(stringParam.getId()).length();
      if (l < 3)
      {
        stringParam.errorMessage = "Please provide at least 3 characters for this test!";
        valid = false;
      }
    */
    int l = webRequestWrapper->arg(mqttServerParam.getId()).length();
    if (l < 3) {
        mqttServerParam.errorMessage = "Please provide at least 3 characters!";
        valid = false;
    }

    return valid;
}

bool connectMqtt() {
    unsigned long now = millis();
    if (1000 > now - lastMqttConnectionAttempt) {
        // Do not repeat within 1 sec.
        return false;
    }
    Serial.println("Connecting to MQTT server...");
    if (!connectMqttOptions()) {
        lastMqttConnectionAttempt = now;
        return false;
    }
    Serial.println("Connected!");

    mqttClient.subscribe("test/data");
    return true;
}

/*
// -- This is an alternative MQTT connection method.
bool connectMqtt() {
  Serial.println("Connecting to MQTT server...");
  while (!connectMqttOptions()) {
    iotWebConf.delay(1000);
  }
  Serial.println("Connected!");

  mqttClient.subscribe("test/action");
  return true;
}
*/

bool connectMqttOptions() {
    bool result;
    if (mqttUserPasswordValue[0] != '\0') {
        result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue, mqttUserPasswordValue);
    } else if (mqttUserNameValue[0] != '\0') {
        result = mqttClient.connect(iotWebConf.getThingName(), mqttUserNameValue);
    } else {
        result = mqttClient.connect(iotWebConf.getThingName());
    }
    return result;
}

void mqttMessageReceived(String &topic, String &payload) {
    Serial.println("Incoming: " + topic + " - " + payload);
}
