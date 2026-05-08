#include "tft_display.h"
#include "shared_res.h"
#include "string.h"


static char current_name[20] = "Unknown";
static char current_acc[10] = "......";
static char current_dist[10] = "N/A";

void tft_display_init(void) {
}

void tft_display_draw_layout(void) {
    if (xSemaphoreTake(tft_mutex, portMAX_DELAY)) {
       
        xSemaphoreGive(tft_mutex);
    }
}

void tft_display_update_ui(const char* name, const char* acc, const char* dist, int err, uint16_t color) {
    if (xSemaphoreTake(tft_mutex, portMAX_DELAY)) {
        if (name) strncpy(current_name, name, sizeof(current_name));
        if (acc) strncpy(current_acc, acc, sizeof(current_acc));
        if (dist) strncpy(current_dist, dist, sizeof(current_dist));

        
        xSemaphoreGive(tft_mutex);
    }
}

void tft_display_draw_jpg(uint8_t *buffer, size_t len) {
    if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(50))) { 
       
        xSemaphoreGive(tft_mutex);
    }
}

void tft_display_clear_camera_area(void) {
    if (xSemaphoreTake(tft_mutex, pdMS_TO_TICKS(100))) {
        
        xSemaphoreGive(tft_mutex);
    }
}