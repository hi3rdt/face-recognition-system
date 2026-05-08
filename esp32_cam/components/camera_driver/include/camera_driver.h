#ifndef CAMERA_DRIVER_H
#define CAMERA_DRIVER_H
#include "esp_camera.h"

void camera_driver_init(void);
camera_fb_t* camera_driver_get_fb(void);
void camera_driver_return_fb(camera_fb_t *fb);

#endif 