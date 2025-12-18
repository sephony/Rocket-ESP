#ifndef USERFUNCTION_H_
#define USERFUNCTION_H_

#include <Arduino.h>
#include <FS.h>
#include <MS5611.h>
#include <SPIFFS.h>

#include "AP.h"
#include "Filter.h"
#include "main.h"

void EXTI_Interrupt();
// void READ_5611(MS5611& ms5611);                    // 读取MS5611
void appendFile(File& file, const char* message);  // 追加文件
void appendFile(File& file, const String& message);
void outputFile(File& file);          // 输出文件
String getFileType(String filename);  // 获取文件类型
bool exists(String path);             // 判断文件是否存在
bool handleFileRead(String path);     // 处理文件读取
String html_to_string(const std::string& file_path);
#endif /* USERFUNCTION_H_ */
