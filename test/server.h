#ifndef SERVER_H_
#define SERVER_H_

#include <Arduino.h>

#include "AP.h"

extern String rx_data;

// 串口中断函数,由服务器向客户端发送数据
void Serial_callback();
// 接受客户端数据，并打印到串口
void handleUpdate();

#endif /* SERVER_H_ */
