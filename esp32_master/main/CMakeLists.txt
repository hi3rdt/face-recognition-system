idf_component_register(SRCS "esp32_master.c"
                       INCLUDE_DIRS "."
                       REQUIRES wifi_manager tft_display websocket_server aws_iot_core nvs_flash esp_wifi)
