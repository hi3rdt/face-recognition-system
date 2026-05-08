#include "websocket_client.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "camera_driver.h"
#include "esp_log.h"

static esp_websocket_client_handle_t client;
static EventGroupHandle_t ws_events;
#define WS_CONNECTED_BIT BIT0

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            xEventGroupSetBits(ws_events, WS_CONNECTED_BIT);
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            xEventGroupClearBits(ws_events, WS_CONNECTED_BIT);
            break;
    }
}

void websocket_client_send_text(const char *text) {
    if (xEventGroupGetBits(ws_events) & WS_CONNECTED_BIT) {
        esp_websocket_client_send_text(client, text, strlen(text), portMAX_DELAY);
    }
}

static void camera_stream_task(void *pvParameters) {

    xEventGroupWaitBits(ws_events, WS_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    while (1) {
        if (xEventGroupGetBits(ws_events) & WS_CONNECTED_BIT) {
            camera_fb_t *fb = camera_driver_get_fb();
            if (fb) {
                esp_websocket_client_send_bin(client, (const char *)fb->buf, fb->len, portMAX_DELAY);
                camera_driver_return_fb(fb);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}

void websocket_client_init(const char *uri) {
    ws_events = xEventGroupCreate();
    
    esp_websocket_client_config_t websocket_cfg = {
        .uri = uri,
    };
    client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL);
    esp_websocket_client_start(client);

    xTaskCreate(camera_stream_task, "stream_task", 8192, NULL, 4, NULL);
}