#include "aws_iot_core.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "mbedtls/base64.h"
#include "shared_res.h"
#include "tft_display.h"
#include "secret.h"

static esp_mqtt_client_handle_t client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, "esp32/result", 0);
            break;
        case MQTT_EVENT_DATA:
           
            cJSON *root = cJSON_Parse(event->data);
            if (root) {
                cJSON *status = cJSON_GetObjectItem(root, "status");
                cJSON *name = cJSON_GetObjectItem(root, "name");
                cJSON *similarity = cJSON_GetObjectItem(root, "similarity");
                
                if (status && strcmp(status->valuestring, "Success") == 0) {
                    char acc_str[10];
                    snprintf(acc_str, sizeof(acc_str), "%.1f%%", similarity->valuedouble);
                    tft_display_update_ui(name->valuestring, acc_str, NULL, 0, COLOR_GREEN);
                } else {
                    tft_display_update_ui("Unknown", "N/A", NULL, 0, COLOR_RED);
                }
                cJSON_Delete(root);
            }
            break;
        default: break;
    }
}

static void aws_task(void *pvParameters) {
    image_payload_t payload;
    while (1) {
        if (xQueueReceive(image_upload_queue, &payload, portMAX_DELAY)) {
            tft_display_update_ui("Processing...", "Cloud", NULL, 0, COLOR_WHITE);
            
          
            size_t olen;
            mbedtls_base64_encode(NULL, 0, &olen, payload.buffer, payload.length);
            unsigned char *base64_buf = malloc(olen);
            mbedtls_base64_encode(base64_buf, olen, &olen, payload.buffer, payload.length);
            
            
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "device_id", "CAM-MASTER");
            cJSON_AddStringToObject(root, "image", (char*)base64_buf);
            char *json_str = cJSON_PrintUnformatted(root);
            
            esp_mqtt_client_publish(client, "esp32/capture", json_str, 0, 1, 0);
            
            
            free(json_str);
            cJSON_Delete(root);
            free(base64_buf);
            free(payload.buffer); 
        }
    }
}

void aws_iot_core_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://a23g3mzejkxt5a-ats.iot.ap-southeast-1.amazonaws.com:8883",
        .broker.verification.certificate = AWS_CERT_CA,
        .credentials.client_id = "ESP32-CAM",
        .credentials.authentication.certificate = AWS_CERT_CRT,
        .credentials.authentication.key = AWS_CERT_PRIVATE,
        .network.reconnect_timeout_ms = 10000,
    };
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    
    xTaskCreate(aws_task, "aws_task", 16384, NULL, 3, NULL);
}