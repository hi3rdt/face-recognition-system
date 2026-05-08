#ifndef TFT_DISPLAY_H
#define TFT_DISPLAY_H

#include <stdint.h>
#include <stddef.h>


#define COLOR_NAVY      0x000F
#define COLOR_WHITE     0xFFFF
#define COLOR_LIGHTGRAY 0xC618
#define COLOR_GREEN     0x07E0
#define COLOR_RED       0xF800
#define COLOR_BLACK     0x0000


void tft_display_init(void);


void tft_display_draw_layout(void);


void tft_display_update_ui(const char* name, const char* acc, const char* dist, int err, uint16_t color);


void tft_display_draw_jpg(uint8_t *buffer, size_t len);


void tft_display_clear_camera_area(void);

#endif 