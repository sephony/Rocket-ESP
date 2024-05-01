#ifndef ROCKET_CLIENT_H_
#define ROCKET_CLIENT_H_

#include <WiFi.h>

// #include "AP.h"

extern WiFiClient client;

extern String rx_data;

extern const char *host;    // 即将连接服务器网址/IP
extern const int httpPort;  // 即将连接服务器端口

// 向服务器发送数据data
void wifiClientRequest(String data1);
// 串口中断函数，当串口接收到数据时，将数据发送给服务器
void Serial_callback();
String Read_Tcp();
void Tcp_Handler(String data);
// 连接路由器
void getwifi();
// 连接服务器
void ycconnect();

#endif  // ROCKET_CLIENT_H_
