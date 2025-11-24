#include "cam_handler.h"
#include "cam_config.h"
#include <WiFi.h>

using namespace websockets;
WebsocketsClient client;

unsigned long lastCaptureTime = 0;
unsigned long lastReconnectTime = 0;

void setupCamera() {
    pinMode(FLASH_PIN, OUTPUT);
    digitalWrite(FLASH_PIN, LOW);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QCIF;
    config.jpeg_quality = 25;
    config.fb_count = 2;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("Camera init failed!");
        while (true);
    }
}

void connectWebSocket() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconnecting WiFi...");
        WiFi.reconnect();
        return;
    }

    Serial.print("Connecting WS: "); Serial.println(WS_SERVER_HOST);
    if (client.connect(WS_SERVER_HOST, WS_SERVER_PORT, "/")) {
        Serial.println("WS Connected!");
    } else {
        Serial.println("WS Connect Failed!");
    }
}

void setupWiFiAndWS() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500); Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    client.onMessage([&](WebsocketsMessage message) {
        if (message.data() == "capture") {
            captureAndSend(true); 
        }
    });

    connectWebSocket();
}

void captureAndSend(bool flashOn) {
    if (!client.available()) return;

    if (flashOn) digitalWrite(FLASH_PIN, HIGH);
    
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Capture Fail");
        if (flashOn) digitalWrite(FLASH_PIN, LOW);
        return;
    }

    client.sendBinary((const char*)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    
    if (flashOn) digitalWrite(FLASH_PIN, LOW);
}

void handleCamLoop() {
    if (client.available()) {
        client.poll();
        
        // Stream liên tục (không flash)
        unsigned long now = millis();
        if (now - lastCaptureTime >= CAPTURE_INTERVAL) {
            captureAndSend(false);
            lastCaptureTime = now;
        }
    } else {
        // Reconnect logic
        unsigned long now = millis();
        if (now - lastReconnectTime >= RECONNECT_INTERVAL) {
            connectWebSocket();
            lastReconnectTime = now;
        }
    }
}