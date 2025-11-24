#include "cam_handler.h"

void setup() {
    Serial.begin(115200);
    delay(3000);
    
    setupCamera();
    setupWiFiAndWS();
}

void loop() {
    handleCamLoop();
}