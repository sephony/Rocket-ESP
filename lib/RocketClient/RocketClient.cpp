#include "rocketClient.h"

#include <Arduino.h>

const char *ssid = "WiFi-Serial";   // 请将您需要连接的WiFi名填入引号中
const char *password = "88888888";  // 请将您需要连接的WiFi密码填入引号中

const char *host = "192.168.4.1";  // 即将连接服务器网址/IP
const int httpPort = 80;           // 即将连接服务器端口

float clientFloatValue;  // 存储客户端发送的浮点型测试数据
int clientIntValue;      // 存储客户端发送的整数型测试数据
bool buttonState;        // 存储客户端按键控制数据

WiFiClient client;

String rx_data;
// 向服务器发送数据data
void wifiClientRequest(String data1);
// 串口中断函数，当串口接收到数据时，将数据发送给服务器
void Serial_callback() {
    // Serial.println("read;");
    while (Serial.available()) {
        rx_data += char(Serial.read());
        delay(3);  // 这里不能去掉，要给串口处理数据的时间
    }
    if (rx_data != "") {
        wifiClientRequest(rx_data);  //

        rx_data = "";
    }
}

String Read_Tcp() {
    String data = "";
    while (client.available() > 0) {
        data += char(client.read());
        delay(1);
    }
    return data;
}
void Tcp_Handler(String data) {
    if (data != "") {
        // Serial.print("收到服务器信息：");
        int maxIndex = data.length();
        int ipos = 0;

        for (int i = maxIndex; i >= 0; --i) {
            if (data.charAt(i) == 0x0D) {
                ipos = i;
                break;
            }
        }
        String data2 = "";
        for (int i = ipos; i < maxIndex; ++i) {
            char sterp = data.charAt(i);
            if (sterp != 0x0D && sterp != 0x0A) {
                data2 += sterp;
            }
        }
        Serial.print(data2);
    }
}

// 连接路由器
void getwifi() {
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}
// 连接服务器
void ycconnect() {
    Serial.println("client try connect");
    client.connect(host, httpPort);
    delay(1);
}

void wifiClientRequest2() {
    String url = "/update?float=" + String(clientFloatValue) +
                 "&int=" + String(clientIntValue) +
                 "&button=" + String(buttonState) +
                 "&data=" + "kk";

    // 建立字符串，用于HTTP请求
    String httpRequest = String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n" +
                         "\r\n";

    client.print(httpRequest);  // 向服务器发送HTTP请求
}

void wifiClientRequest(String data1) {
    String url = "/update?data=" + data1;
    String httpRequest = String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         //"Connection: close\r\n" +
                         "\r\n";
    client.print(httpRequest);  // 向服务器发送HTTP请求
}
