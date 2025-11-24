#ifndef CAM_HANDLER_H
#define CAM_HANDLER_H

#include <ArduinoWebsockets.h>
#include "esp_camera.h"

extern websockets::WebsocketsClient client;

void setupCamera();
void setupWiFiAndWS();
void connectWebSocket();
void captureAndSend(bool flashOn);
void handleCamLoop();

#endif