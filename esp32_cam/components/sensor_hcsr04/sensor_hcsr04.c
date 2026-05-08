#include "sensor_hcsr04.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "websocket_client.h"
#include <stdio.h>

#define TRIGGER_PIN GPIO_NUM_12
#define ECHO_PIN    GPIO_NUM_14
#define MAX_DISTANCE 200
#define MIN_DIST     5
#define MAX_DIST     10

static uint32_t get_distance() {
    gpio_set_level(TRIGGER_PIN, 0); ets_delay_us(2);
    gpio_set_level(TRIGGER_PIN, 1); ets_delay_us(10);
    gpio_set_level(TRIGGER_PIN, 0);

    uint32_t start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 0 && (esp_timer_get_time() - start_time) < 60000);
    start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 1 && (esp_timer_get_time() - start_time) < 60000);
    
    uint32_t time_us = esp_timer_get_time() - start_time;
    return (time_us > 60000) ? MAX_DISTANCE : (time_us * 0.034 / 2);
}

static void sensor_task(void *pvParameters) {
    uint32_t last_capture_time = 0;
    char text_buffer[32];

    while (1) {
        uint32_t dist = get_distance();
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

       
        if (dist != MAX_DISTANCE) {
            snprintf(text_buffer, sizeof(text_buffer), "DIST:%lu", dist);
            websocket_client_send_text(text_buffer);
        }

        if (dist >= MIN_DIST && dist <= MAX_DIST && (current_time - last_capture_time >= 1000)) {
            websocket_client_send_text("CAPTURE"); 
            last_capture_time = current_time;
            vTaskDelay(pdMS_TO_TICKS(5000)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(1000)); 
        }
    }
}

void sensor_hcsr04_init(void) {
    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}