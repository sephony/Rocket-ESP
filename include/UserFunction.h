#ifndef USERFUNCTION_H_
#define USERFUNCTION_H_

#include <Arduino.h>
#include <FS.h>
#include <MS5611.h>
#include <SPIFFS.h>

#include <fstream>
#include <iostream>
#include <sstream>

#include "AP.h"
#include "Filter.h"
#include "main.h"

void EXTI_Interrupt(void);
float getHeight(MS5611 ms5611);
void READ_5611(MS5611& ms5611);                   // 读取MS5611
void appendFile(File file, const char* message);  // 追加文件
void appendFile(File file, String message);
void outputFile(File file);              // 输出文件
String getContentType(String filename);  // 获取文件类型
bool exists(String path);                // 判断文件是否存在
bool handleFileRead(String path);        // 处理文件读取
std::string html_to_string(const std::string& file_path);
#endif /* USERFUNCTION_H_ */
