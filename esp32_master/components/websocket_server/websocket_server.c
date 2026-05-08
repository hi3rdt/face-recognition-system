#include "websocket_server.h"
#include "esp_http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tft_display.h"
#include "shared_res.h"
#include "string.h"
#include <stdlib.h>

static httpd_handle_t server = NULL;
static bool expecting_capture = false; 

static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) return ESP_OK; 

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
   
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) return ret;

    if (ws_pkt.len > 0) {
        uint8_t *buf = calloc(1, ws_pkt.len + 1);
        httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);

        if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
            
            if (strncmp((char*)buf, "CAPTURE", 7) == 0) {
                expecting_capture = true;
            } 
            else if (strncmp((char*)buf, "DIST:", 5) == 0) {
                char dist_str[10];
                int dist = atoi((char*)buf + 5);
                snprintf(dist_str, sizeof(dist_str), "%d", dist);
                tft_display_update_ui(NULL, NULL, dist_str, 0, COLOR_WHITE);
            }
        } 
        else if (ws_pkt.type == HTTPD_WS_TYPE_BINARY) {
           
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            static uint32_t capture_start_time = 0;

            if (displaying_capture) {
                if (now - capture_start_time >= 5000) {
                    displaying_capture = false;
                    tft_display_clear_camera_area();
                }
                free(buf);
                return ESP_OK;
            }

            if (expecting_capture) {
             
                expecting_capture = false;
                displaying_capture = true;
                capture_start_time = now;
                
                tft_display_draw_jpg(buf, ws_pkt.len);
                
                image_payload_t payload = { .buffer = buf, .length = ws_pkt.len };
                if (xQueueSend(image_upload_queue, &payload, 0) != pdPASS) {
                    free(buf); 
                }
            } else {
                
                tft_display_draw_jpg(buf, ws_pkt.len);
                free(buf);
            }
        } else {
            free(buf);
        }
    }
    return ESP_OK;
}

void websocket_server_init(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 8888;
    httpd_start(&server, &config);

    httpd_uri_t ws_uri = {
        .uri        = "/",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .is_websocket = true
    };
    httpd_register_uri_handler(server, &ws_uri);
}