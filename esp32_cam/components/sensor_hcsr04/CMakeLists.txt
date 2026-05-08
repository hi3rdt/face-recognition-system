idf_component_register(SRCS "sensor_hcsr04.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver freertos esp_timer websocket_client)