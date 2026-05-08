idf_component_register(SRCS "esp32_cam.c"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash camera_driver wifi_client websocket_client sensor_hcsr04)
