#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include "config.h"
#include "secrets.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Base64.h>
#include "display_manager.h" 

extern PubSubClient mqttClient;

void setupNetwork();
void connectAWS();
void checkAWSConnection();
void sendToAWS(uint8_t* buffer, size_t len);
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif