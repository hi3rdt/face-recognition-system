#ifndef CAM_CONFIG_H
#define CAM_CONFIG_H

#include <Arduino.h>


#define WIFI_SSID           "ESP32CAM_to_ESP32" 
#define WIFI_PASS           "myesp32server"
#define WS_SERVER_HOST      "192.168.1.1"       
#define WS_SERVER_PORT      8888

// --- Logic ---
#define CAPTURE_INTERVAL    100
#define RECONNECT_INTERVAL  500

// --- Camera Pins (AI THINKER) ---
#define PWDN_GPIO_NUM       32
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       0
#define SIOD_GPIO_NUM       26
#define SIOC_GPIO_NUM       27
#define Y9_GPIO_NUM         35
#define Y8_GPIO_NUM         34
#define Y7_GPIO_NUM         39
#define Y6_GPIO_NUM         36
#define Y5_GPIO_NUM         21
#define Y4_GPIO_NUM         19
#define Y3_GPIO_NUM         18
#define Y2_GPIO_NUM         5
#define VSYNC_GPIO_NUM      25
#define HREF_GPIO_NUM       23
#define PCLK_GPIO_NUM       22
#define FLASH_PIN           4

#endif