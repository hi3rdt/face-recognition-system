#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- WiFi ---
#define WIFI_SSID           "WIFI 5"
#define WIFI_PASS           "0987654322"
#define AP_SSID             "ESP32CAM_to_ESP32"
#define AP_PASS             "myesp32server"
#define WS_PORT             8888

// --- AWS IoT ---
#define AWS_IOT_ENDPOINT    "xxxxxxxxxxxxx-ats.iot.ap-southeast-1.amazonaws.com" 
#define AWS_IOT_TOPIC_PUB   "esp32/capture"
#define AWS_IOT_TOPIC_SUB   "esp32/result"
#define AWS_THING_NAME      "ESP32-CAM"
#define MQTT_PORT           8883
#define MQTT_BUFFER_SIZE    30720 

// --- Hardware Pins ---
#define PIN_TRIGGER         12
#define PIN_ECHO            14

// --- Display ---
#define SCREEN_W            320
#define SCREEN_H            240
#define CAM_X               0
#define CAM_Y               50
#define CAM_W               176
#define CAM_H               144
#define INFO_X              177
#define INFO_Y              40
#define INFO_W              150
#define INFO_H              200
#define HEADER_H            49

// --- Colors ---
#define CLR_NAVY            0x000F
#define CLR_WHITE           0xFFFF
#define CLR_GRAY            0xC618
#define CLR_GREEN           0x07E0
#define CLR_RED             0xF800
#define CLR_BLACK           0x0000

// --- Logic ---
#define DIST_MAX            200
#define DIST_MIN_THRESH     5
#define DIST_MAX_THRESH     10
#define INTERVAL_CAPTURE    5000 
#define INTERVAL_SENSOR     1000
#define DISPLAY_TIMEOUT     5000
#define MAX_SENSOR_ERRORS   10


struct AppState {
    String currentName = "Unknown";
    String currentAccuracy = "...";
    String currentDistance = "N/A";
    int sensorErrorCount = 0;
    unsigned long lastCaptureTime = 0;
    unsigned long lastSensorTime = 0;
    unsigned long captureDisplayStart = 0;
    bool displayingCapture = false;
    uint8_t* captureBuffer = nullptr;
    size_t captureBufferLen = 0;
};

extern AppState state; 

#endif