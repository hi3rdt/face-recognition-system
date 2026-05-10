#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "camera_driver.h"
#include "wifi_client.h"
#include "websocket_client.h"
#include "sensor_hcsr04.h"

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

   
    camera_driver_init();

    
    wifi_client_init("ESP32CAM_to_ESP32", "myesp32server");

    
    websocket_client_init("ws://192.168.1.1:8888");

    
    sensor_hcsr04_init();

    vTaskDelete(NULL);
}