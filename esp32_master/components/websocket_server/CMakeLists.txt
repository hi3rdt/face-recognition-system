idf_component_register(SRCS "websocket_server.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver freertos esp_http_server tft_display) 