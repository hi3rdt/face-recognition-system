#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "shared_res.h"

// Component Headers
#include "wifi_manager.h"
#include "tft_display.h"
#include "websocket_server.h"
#include "aws_iot_core.h"

SemaphoreHandle_t tft_mutex;
QueueHandle_t image_upload_queue;
volatile bool displaying_capture = false;

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    tft_mutex = xSemaphoreCreateMutex();
    image_upload_queue = xQueueCreate(2, sizeof(image_payload_t));

    tft_display_init();

    wifi_manager_init();      
    aws_iot_core_init();      
    websocket_server_init();  

    vTaskDelete(NULL);
}