
#include "config.h"
#include "display_manager.h"
#include "network_manager.h"
#include <ArduinoWebsockets.h>

using namespace websockets;
WebsocketsServer server;
WebsocketsClient client;


AppState state;



void handleWebSocket() {
    if (client.available()) {
        client.poll();
        WebsocketsMessage msg = client.readBlocking();

        if (msg.isBinary()) {
            
            if (state.displayingCapture) {
                
                if (millis() - state.captureDisplayStart >= DISPLAY_TIMEOUT) {
                    Serial.println("Timeout. Reset state.");
                    if (state.captureBuffer) { 
                        free(state.captureBuffer); 
                        state.captureBuffer = nullptr; 
                        state.captureBufferLen = 0; 
                    }
                    state.displayingCapture = false;
                    clearCameraArea();
                }
                return; 
            }

            
            uint32_t now = millis();
            
            if (state.lastCaptureTime > 0 && now - state.lastCaptureTime < INTERVAL_CAPTURE * 2) {
                
                if (state.captureBuffer) free(state.captureBuffer);
                state.captureBufferLen = msg.length();
                state.captureBuffer = (uint8_t*)malloc(state.captureBufferLen);

                if (state.captureBuffer) {
                    memcpy(state.captureBuffer, msg.c_str(), state.captureBufferLen);
                    
                   
                    state.displayingCapture = true;
                    state.captureDisplayStart = millis();
                    
                    
                    TJpgDec.drawJpg(CAM_X, CAM_Y, state.captureBuffer, state.captureBufferLen);
                    sendToAWS((uint8_t*)msg.c_str(), msg.length());
                    
                    state.lastCaptureTime = 0; 
                }
            } else {
                
                if (!state.displayingCapture) {
                    TJpgDec.drawJpg(CAM_X, CAM_Y, (const uint8_t*)msg.c_str(), msg.length());
                }
            }
        }
    }
}

void handleSensor() {
    
    if (state.displayingCapture) {
        return; 
    }

    unsigned long now = millis();
    if (now - state.lastSensorTime >= INTERVAL_SENSOR) {
        // Đo khoảng cách
        digitalWrite(PIN_TRIGGER, LOW); delayMicroseconds(2);
        digitalWrite(PIN_TRIGGER, HIGH); delayMicroseconds(10);
        digitalWrite(PIN_TRIGGER, LOW);
        long duration = pulseIn(PIN_ECHO, HIGH, 60000);
        unsigned int distance = (duration == 0) ? DIST_MAX : duration * 0.034 / 2;

        if (duration == 0) { state.sensorErrorCount++; state.currentDistance = "Error"; }
        else { state.sensorErrorCount = 0; state.currentDistance = String(distance); }
        
        updateDisplay(state.currentName, state.currentAccuracy, state.currentDistance, state.sensorErrorCount, CLR_WHITE);

        
        if (distance >= DIST_MIN_THRESH && distance <= DIST_MAX_THRESH && distance != DIST_MAX) {
            if (client.available()) {
                Serial.println("Detect -> Capture");
                client.send("capture");
                state.lastCaptureTime = now; 
            }
        }

        if (state.sensorErrorCount >= MAX_SENSOR_ERRORS) state.sensorErrorCount = 0;
        state.lastSensorTime = now;
    }
}



void setup() {
    Serial.begin(115200);
    
    
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    digitalWrite(PIN_TRIGGER, LOW);

   
    setupDisplay();
    drawLayout();
    setupNetwork();
    connectAWS();

    server.listen(WS_PORT);
}

void loop() {
    checkAWSConnection(); 

    if (server.poll()) {
        client = server.accept();
    }

    
    handleWebSocket();
    handleSensor();

    delay(50);
}