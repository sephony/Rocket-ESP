#

## SPIFFS

### 类定义函数

```C++
#include <FS.h>
#include <SPIFFS.h>
```

```C++
/*
 * 初始化SPIFFS文件系统，如果文件系统不存在，则会格式化文件系统
 *
 * @formatOnFail: 如果文件系统初始化失败，是否格式化文件系统
 * @basePath: 文件系统的根目录
 * @maxOpenFiles: 最大打开文件数
 * @partitionLabel: 分区标签
 */
bool SPIFFS::begin(bool formatOnFail=false, const char * basePath="/spiffs", uint8_t maxOpenFiles=10, const char * partitionLabel=NULL);
```

```C++
/*
 * 卸载文件系统
 */
bool SPIFFS::end();
```

```C++
/*
 * 格式化文件系统
 */
bool SPIFFS::format();
```

```C++
/*
 * SPIFFS文件系统的总大小
 */
size_t SPIFFS::totalBytes();
```

```C++
/*
 * SPIFFS文件系统的已使用大小
 */
size_t SPIFFS::usedBytes();
```

## FS

```C++
#include <FS.h>
```

```C++
/*
 * 打开文件
 *
 * @path: 文件路径
 * @mode: 打开模式
 *      FILE_READ: 读模式
 *      FILE_WRITE: 写模式
 *      FILE_APPEND: 追加模式
 * @create: 如果文件不存在，是否创建文件(貌似不管用，都会创建，默认值就好)
 */
File FS::open(const char * path, const char * mode = FILE_READ, const bool create = false);
File FS::open(const String& path, const char* mode = FILE_READ, const bool create = false);
```

```C++
/*
 * 检查文件是否存在
 *
 * @path: 文件路径
 */
bool FS::exists(const char * path);
bool FS::exists(const String& path);
```

```C++
/*
 * 重命名文件
 *
 * @pathFrom: 源文件路径
 * @pathTo: 目标文件路径
 */
bool FS::rename(const char * pathFrom, const char * pathTo);
bool FS::rename(const String& pathFrom, const String& pathTo);
```

```C++
/*
 * 删除文件
 *
 * @path: 文件路径
 */
bool FS::remove(const char * path);
bool FS::remove(const String& path);
```

```C++
/*
 * 创建目录
 *
 * @path: 目录路径
 */
bool FS::mkdir(const char * path);
bool FS::mkdir(const String& path);
```

```C++
/*
 * 删除目录
 *
 * @path: 目录路径
 */
bool FS::rmdir(const char * path);
bool FS::rmdir(const String& path);
```

## File

```C++
write(uint8_t)：向文件中写入一个字节。
write(const uint8_t *buf, size_t size)：向文件中写入一个字节数组。
available()：返回文件中可用的字节数。
read()：从文件中读取一个字节。
peek()：查看文件中下一个可用的字节，但不移动文件指针。
flush()：将缓冲区中的数据写入文件。
read(uint8_t* buf, size_t size)：从文件中读取一个字节数组。
readBytes(char *buffer, size_t length)：从文件中读取指定长度的字节数组。
seek(uint32_t pos, SeekMode mode)：将文件指针移动到指定位置。
seek(uint32_t pos)：将文件指针移动到指定位置，使用默认的SeekSet模式。
position() const：返回当前文件指针的位置。
size() const：返回文件的大小。
setBufferSize(size_t size)：设置文件读写缓冲区的大小。
close()：关闭文件。
operator bool() const：检查文件是否可用。
getLastWrite()：返回文件的最后修改时间。
path() const：返回文件的路径。
name() const：返回文件的名称。
isDirectory(void)：检查文件是否是目录。
seekDir(long position)：将目录指针移动到指定位置。
openNextFile(const char* mode = FILE_READ)：打开目录中的下一个文件。
getNextFileName(void)：返回目录中的下一个文件名。
getNextFileName(boolean *isDir)：返回目录中的下一个文件名，并指示该文件是否是目录。
rewindDirectory(void)：将目录指针重置为目录的开头。
#include <FS.h>
```

```C++
/*
 * 向文件中写入一个字节
 */
size_t File::write(uint8_t) override;
size_t File::write(const uint8_t *buf, size_t size) override;
```

```C++
/*
 * 返回文件中可用的字节数
 */
int File::available() override;
```

```C++
/*
 * 从文件中读取一个字节
 */
int File::read() override;
```

```C++
/*
 * 查看文件中下一个可用的字节，但不移动文件指针
 */
int File::peek() override;
```

```C++
/*
 * 将缓冲区中的数据写入文件
 */
void File::flush() override;
```

```C++
/*
 * 从文件中读取一个字节数组
 */
size_t File::read(uint8_t* buf, size_t size) override;
```

```C++
/*
 * 从文件中读取指定长度的字节数组
 */
size_t File::readBytes(char *buffer, size_t length);
```

```C++
/*
 * 将文件指针移动到指定位置
 */
bool File::seek(uint32_t pos, SeekMode mode) override;
bool File::seek(uint32_t pos) override;
```

```C++
/*
 * 返回当前文件指针的位置
 */
uint32_t File::position() const override;
```

```C++
/*
 * 返回文件的大小
 */
uint32_t File::size() const override;
```

```C++
/*
 * 设置文件读写缓冲区的大小
 */
void File::setBufferSize(size_t size);
```

```C++
/*
 * 关闭文件
 */
void File::close() override;
```

```C++
/*
 * 检查文件是否可用
 */
operator bool() const override;
```

```C++
/*
 * 返回文件的最后修改时间
 */
time_t File::getLastWrite();
```

```C++
/*
 * 返回文件的路径
 */
String File::path() const;
```

```C++
/*
 * 返回文件的名称
 */
String File::name() const;
```

```C++
/*
 * 检查文件是否是目录
 */
bool File::isDirectory(void) override;
```

```C++
/*
 * 将目录指针移动到指定位置
 */
void File::seekDir(long position) override;
```

```C++
/*
 * 打开目录中的下一个文件
 */
File File::openNextFile(const char* mode = FILE_READ) override;
```

```C++
/*
 * 返回目录中的下一个文件名
 */
String File::getNextFileName(void);
```

```C++
/*
 * 返回目录中的下一个文件名，并指示该文件是否是目录
 */
String File::getNextFileName(boolean *isDir);
```

```C++
/*
 * 将目录指针重置为目录的开头
 */
void File::rewindDirectory(void);
```

```C++
//在ESP32的文件系统中，目录是一种特殊的文件类型，用于存储其他文件和目录。使用isDirectory()函数可以判断当前文件是否是一个目录，从而进行相应的处理。例如，可以使用以下代码判断当前文件是否是一个目录：

File file = SPIFFS.open("/mydir", FILE_READ);
if (file.isDirectory()) {
  Serial.println("This is a directory.");
} else {
  Serial.println("This is not a directory.");
}

//在上述代码中，首先使用SPIFFS.open()函数打开一个名为/mydir的文件，然后使用isDirectory()函数判断该文件是否是一个目录。如果是目录，则输出This is a directory.，否则输出This is not a directory.。

//在ESP32的文件系统库中，可以使用SPIFFS.mkdir()函数来创建一个目录。该函数的参数为要创建的目录的路径，例如/dir。

//以下是一个示例代码，用于创建一个名为/data的目录：
if (SPIFFS.mkdir("/data")) {
  Serial.println("Directory created");
} else {
  Serial.println("Failed to create directory");
}
//在上述代码中，首先调用SPIFFS.mkdir()函数来创建一个名为/data的目录。如果创建成功，将输出Directory created；否则，将输出Failed to create directory。

//注意，如果要在目录中创建文件，需要先打开该目录，然后使用File类的open()函数来创建文件。例如，可以使用以下代码在/data目录中创建一个名为test.txt的文件：
File file = SPIFFS.open("/data/test.txt", FILE_WRITE);
if (file) {
  file.println("Hello, world!");
  file.close();
}
//在上述代码中，首先使用SPIFFS.open()函数打开/data/test.txt文件，并使用FILE_WRITE模式打开文件。然后，使用println()函数将字符串"Hello, world!"写入文件。最后，使用close()函数关闭文件。
```

## 配置页面

```C++
ParameterGroup(const char* id, const char* label = nullptr);
/**
 * Parameters is a configuration item of the config portal.
 * The parameter will have its input field on the configuration page,
 * and the provided value will be saved to the EEPROM.
 */
/**
 * Create a parameter for the config portal.
 *
 * @label - Displayable label at the config portal.
 * @id - Identifier used for HTTP queries and as configuration key. Must not
 *   contain spaces nor other special characters.
 * @valueBuffer - Configuration value will be loaded to this buffer from the
 *   EEPROM.
 * @length - The buffer should have a length provided here.
 * @defaultValue - Defalt value set on startup, when no configuration ever saved
 *   with the current config-version.
 */
Parameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue = nullptr);
/**
 * TexParameters is to store text based parameters.
 */
/**
 * Create a text parameter for the config portal.
 *
 * @placeholder (optional) - Text appear in an empty input box.
 * @customHtml (optional) - The text of this parameter will be added into
 *   the HTML INPUT field.
 * (See Parameter for arguments!)
 */
TextParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue = nullptr,
    const char* placeholder = nullptr,
    const char* customHtml = nullptr);
/**
 * The Password parameter has a special handling, as the value will be
 * overwritten in the EEPROM only if value was provided on the config portal.
 * Because of this logic, "password" type field with length more then
 * IOTWEBCONF_PASSWORD_LEN characters are not supported.
 */
/**
 * Create a password parameter for the config portal.
 *
 * (See TextParameter for arguments!)
 */
PasswordParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue = nullptr,
    const char* placeholder = nullptr,
    const char* customHtml = "ondblclick=\"pw(this.id)\"");
/**
 * This is just a text parameter, that is rendered with type 'number'.
 */
/**
 * Create a numeric parameter for the config portal.
 *
 * (See TextParameter for arguments!)
 */
NumberParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* defaultValue = nullptr,
    const char* placeholder = nullptr,
    const char* customHtml = nullptr);
/**
 * Checkbox parameter is represended as a text parameter but has a special
 * handling. As the value is either empty or has the word "selected".
 * Note, that form post will not send value if checkbox was not selected.
 */
/**
 * Create a checkbox parameter for the config portal.
 *
 * (See TextParameter for arguments!)
 */
CheckboxParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    bool defaultValue = false);
/**
 * Options parameter is a structure, that handles multiple values when rendering
 * the HTML representation.
 */
/**
 * @optionValues - List of values to choose from with, where each value
 *   can have a maximal size of 'length'. Contains 'optionCount' items.
 * @optionNames - List of names to render for the values, where each
 *   name can have a maximal size of 'nameLength'. Contains 'optionCount'
 *   items.
 * @optionCount - Size of both 'optionValues' and 'optionNames' lists.
 * @nameLength - Size of any item in optionNames list.
 * (See TextParameter for arguments!)
 */
OptionsParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t nameLength,
    const char* defaultValue = nullptr);
/**
 * Select parameter is an option parameter, that rendered as HTML SELECT.
 * Basically it is a dropdown combobox.
 */
/**
 * Create a select parameter for the config portal.
 *
 * (See OptionsParameter for arguments!)
 */
SelectParameter(
    const char* label, const char* id, char* valueBuffer, int length,
    const char* optionValues, const char* optionNames, size_t optionCount, size_t namesLenth,
    const char* defaultValue = nullptr);
/**
 * This class is here just to make some nice indents on debug output
 *   for group tree.
 */
PrefixStreamWrapper(
    Stream* originalStream,
    std::function<size_t(Stream* stream)> prefixWriter);
```
