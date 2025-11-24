#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H
#include "config.h"
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>

extern TFT_eSPI tft; 

void setupDisplay();
void drawLayout();
void updateDisplay(String name, String accuracy, String distance, int errors, uint16_t textColor);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
void clearCameraArea(); 

#endif