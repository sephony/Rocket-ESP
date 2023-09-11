#include "AP.h"

//  -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
char RunModeValue[NUMBER_LEN];
const char RunModeValues[][STRING_LEN] = {"Standard", "Advanced", "Debug", "Real-time Wireless Serial"};
const char RunModeNames[][STRING_LEN] = {"标准模式", "高级模式", "调试模式", "实时无线串口模式"};
char ParaMode_Value[STRING_LEN];
const char LaunchModeValues[][STRING_LEN] = {"time control", "height control"};
const char LaunchModeNames[][STRING_LEN] = {"定时开伞", "定高开伞"};
char LaunchReadyValue[STRING_LEN];

char H_ParaValue[NUMBER_LEN];     // 开伞高度
char T_ProtectValue[NUMBER_LEN];  // 保护时间
char T_DetachValue[NUMBER_LEN];   // 二级点火时间
char T_ParaValue[NUMBER_LEN];     // 开伞时间

char RGB_BrightnessValue[NUMBER_LEN];  // RGB亮度

const char thingName[] = "Rocket";  // -- Name of the Thing. Used e.g. as SSID of the own Access Point.
// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "88888888";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// DNS server
DNSServer dnsServer;
// HTTP server
WebServer server(80);
// HTTP Update Server
HTTPUpdateServer httpUpdater;

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
ParameterGroup group_RGB("_RGB灯", "RGB灯");

// Parameters.
// TextParameter FileNameParam("文件名", "FileName", FileNameValue, STRING_LEN);
SelectParameter RunMode_Param("运行模式", "RunMode_Param", RunModeValue, STRING_LEN,
                              (const char*)RunModeValues,
                              (const char*)RunModeNames,
                              sizeof(RunModeValues) / STRING_LEN,
                              STRING_LEN,
                              "Standard");

SelectParameter ParaMode_Param("发射模式", "LaunchMode", ParaMode_Value, STRING_LEN,
                               (const char*)LaunchModeValues,
                               (const char*)LaunchModeNames,
                               sizeof(LaunchModeValues) / STRING_LEN,
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
    // -- Let IotWebConf test and handle captive portal requests.
    if (iotWebConf.handleCaptivePortal()) {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<meta charset=\"UTF-8\">";
    s += "<title>火箭调参</title>";
    s += "<style>";
    s += "body {background-color: #f2f2f2; font-family: Arial, sans-serif;}";
    s += "h1 {color: #333333; text-align: center;}";
    s += "ul {list-style-type: none; margin: 0; padding: 0;}";
    s += "li {padding: 12px; background-color: #ffffff; border-bottom: 1px solid #dddddd;}";
    s += "a {color: #333333; text-decoration: none;}";
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
    s += "</ul>";
    s += "<p>前往<a href='config' style='color: #007bff; text-decoration: none; font-weight: bold;'>配置页面</a>调整参数。</p>";
    s += "</body></html>\n";
    /*
    String s =
    "<!DOCTYPE html><html lang=\"zh-CN\"><head><meta name=\"viewport\" content=\"width=device-width, "
    "initial-scale=1, user-scalable=no\"/>";
    s += "<title>火箭调参</title><style>img {float: top;margin-top: 10px;}</ style></ head><body>";
    s += "<img src="https://pic2.zhimg.com/v2-45249e82081900c9ec94cd1a3ec1ab5e_r.jpg?source=12a79843" alt="Image description" height="100%" width="100%">";
    s += "<ul>";
    s += "<li>文件名: ";
    s += FileNameValue;
    s += "<li>二级点火时间: ";
    s += atoi(T_DetachValue);
    s += "<li>开伞时间: ";
    s += atof(T_ParaValue);
    s += "<li>开伞高度: ";
    s += atof(H_ParaValue);
    s += "</ul>";
    s += "到 <a href='config'>配置页面</a> 调整参数.";
    s += "</body></html>\n";
    */

    server.send(200, "text/html", s);
}
void handleDir() {
}
void wifiConnected() {
    Serial.println("Wifi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    timeClient.begin();
    sign_beginNTPClient = true;
    Serial.println("TimeClient was started.");
}
void configSaved() {
    Serial.println("Configuration was updated.");
    Serial.printf("Detach time is changed to:");
    Serial.println(T_DetachValue);
    Serial.printf("Parachute time is changed to:");
    Serial.println(T_ParaValue);
    Serial.printf("Parachute height is changed to:");
    Serial.println(H_ParaValue);
    T_DETACH = strtof(T_DetachValue, NULL);
    T_PARACHUTE = strtof(T_ParaValue, NULL);
    HEIGHT_PARACHUTE = atoi(H_ParaValue);
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper) {
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
    return valid;
}
