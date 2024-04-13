#include "UserFunction.h"

#include "PinConfig.h"
#include "Varibles.h"

/***** Interrupt Function Definition *****/
void EXTI_Interrupt(void) {
    if (mission_stage == PRE_LAUNCH) {
        time_launch = millis();  // 单位ms
        digitalWrite(GPIO_RUN_SIGN, LOW);
        mission_stage = LAUNCHED;
        Serial.printf("External Interrupt Called\r\n");
        // appendFile( "Hall Interrupt Called!\r\n", file);
    }
}

float getHeight(MS5611 ms5611) {
    float height = 44330.0 * (1.0 - pow(ms5611.getPressure() / 1013.25, 1 / 5.255));  // barometric
    // float height = (pow((1013.25 / getPressure()), 1.0 / 5.257) - 1) * (getTemperature() + 273.15) / 0.65;  // hypsometric
    return height;
}
void READ_5611(MS5611& ms5611) {
    if ((ms5611.read()) != MS5611_READ_OK) {
        Serial.print("Error in read: ");
    } else {
        // Serial.printf("Preassure: %.2f\r\n", MS5611.getPressure());
        height = getHeight(ms5611);
        AltitudeLPF_50.input = height;
        height_filter = Butterworth50HzLPF(&AltitudeLPF_50);
        // 二次滤波
        // AltitudeLPF_50.input = height_filter;
        // height_filter = Butterworth50HzLPF(&AltitudeLPF_50);
    }
}

void appendFile(File file, const char* message) {
    Serial.printf("Appending to file: %s\r\n", file.path());
    if (!file) {
        Serial.println("- failed to open file for appending");
    }
    if (file.print(message)) {
        Serial.printf("- message appended: %s\r\n", message);
    } else {
        Serial.println("- append failed!");
    }
}
void appendFile(File file, String message) {
    appendFile(file, message.c_str());
}

void outputFile(File file) {
    Serial.printf("Output file: %s\r\n", file.path());
    if (!file) {
        Serial.println("- failed to open file for reading");
    }
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}
String getContentType(String filename) {
    if (server.hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    } else
        return "text/plain";
}

bool exists(String path) {
    bool status = false;
    File file = SPIFFS.open(path, "r");
    status = !file.isDirectory();
    file.close();
    return status;
}
bool handleFileRead(String path) {
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) {
        path += "index.htm";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (exists(pathWithGz) || exists(path)) {
        if (exists(pathWithGz)) {
            path += ".gz";
        }
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

std::string html_to_string(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html_string = buffer.str();

    // Escape double quotes
    size_t pos = 0;
    while ((pos = html_string.find("\"", pos)) != std::string::npos) {
        html_string.replace(pos, 1, "\\\"");
        pos += 2;
    }

    return html_string;
}
