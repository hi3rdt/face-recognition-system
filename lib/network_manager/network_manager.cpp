#include "network_manager.h"

WiFiClientSecure net;
PubSubClient mqttClient(net);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) message += (char)payload[i];
    Serial.println("AWS Msg: " + message);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, message);
    String status = doc["status"].as<String>();
    String name = doc["name"].as<String>();
    String accuracy = String(doc["similarity"].as<float>(), 1) + "%";

    if (status == "Success" || status == "Face matched!") {
        updateDisplay(name, accuracy, state.currentDistance, state.sensorErrorCount, CLR_GREEN);
    } else {
        updateDisplay("Unknown", "N/A", state.currentDistance, state.sensorErrorCount, CLR_RED);
    }
}

void setupNetwork() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected");

    WiFi.softAP(AP_SSID, AP_PASS);
    delay(100);
    WiFi.softAPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));

    configTime(7 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    while (time(nullptr) < 100000) { delay(100); }

    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    mqttClient.setServer(AWS_IOT_ENDPOINT, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
}

void connectAWS() {
    if (WiFi.status() != WL_CONNECTED) return;
    while (!mqttClient.connected()) {
        Serial.print("Connecting AWS...");
        if (mqttClient.connect(AWS_THING_NAME)) {
            Serial.println("Connected!");
            mqttClient.subscribe(AWS_IOT_TOPIC_SUB);
        } else {
            delay(1000);
        }
    }
}

void checkAWSConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected()) {
            
        } else {
            mqttClient.loop();
        }
    }
}

void sendToAWS(uint8_t* buffer, size_t len) {
    if (WiFi.status() != WL_CONNECTED) {
        updateDisplay("No WiFi", "Err", state.currentDistance, state.sensorErrorCount, CLR_RED);
        return;
    }
    if (!mqttClient.connected()) connectAWS();
    
    String base64String = base64::encode(buffer, len);
    DynamicJsonDocument doc(32000);
    doc["device_id"] = "CAM-MASTER";
    doc["image"] = base64String;
    String jsonBuffer;
    serializeJson(doc, jsonBuffer);

    if (mqttClient.publish(AWS_IOT_TOPIC_PUB, jsonBuffer.c_str())) {
        Serial.println("Sent to AWS");
        updateDisplay("Processing...", "Cloud", state.currentDistance, state.sensorErrorCount, CLR_WHITE);
    } else {
        updateDisplay("AWS Fail", "Err", state.currentDistance, state.sensorErrorCount, CLR_RED);
    }
}