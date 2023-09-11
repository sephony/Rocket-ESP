#include "server.h"

String rx_data;

void Serial_callback() {
    while (Serial.available()) {
        rx_data += char(Serial.read());
        delay(5);  // 这里不能去掉，要给串口处理数据的时间
    }
    if (rx_data != "") {
        server.send(200, "text/plain", rx_data);  // 向客户端发送数据

        // Serial.print(rx_data);
        rx_data = "";
    }
}

void handleUpdate() {
    delay(1);
    String strdata = server.arg("data");
    if (strdata != "") {
        Serial.print(strdata + "\r\n");
    }
}
