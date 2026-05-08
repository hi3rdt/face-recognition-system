#ifndef SHARED_RES_H
#define SHARED_RES_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"


typedef struct {
    uint8_t *buffer;
    size_t length;
} image_payload_t;


extern SemaphoreHandle_t tft_mutex;
extern QueueHandle_t image_upload_queue;
extern volatile bool displaying_capture;

#endif