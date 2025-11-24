#include "display_manager.h"

TFT_eSPI tft = TFT_eSPI();

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (y >= CAM_Y + CAM_H || x >= CAM_X + CAM_W) return 0;
    tft.pushImage(x, y, w, h, bitmap);
    return 1;
}

void setupDisplay() {
    tft.begin();
    tft.setRotation(1);
    tft.setSwapBytes(true);
    TJpgDec.setJpgScale(1);
    TJpgDec.setCallback(tft_output);
}

void drawLayout() {
    tft.fillScreen(CLR_BLACK);
    tft.fillRect(0, HEADER_Y, SCREEN_W, HEADER_H, CLR_NAVY);
    tft.setTextColor(CLR_WHITE);
    tft.drawCentreString("FACE RECOGNITION SYSTEM", 150, HEADER_Y + HEADER_H / 2 - 8, 2);
    tft.fillRect(0, 194, SCREEN_W, 50, CLR_NAVY);
    tft.fillRect(CAM_X, CAM_Y, CAM_W, CAM_H, CLR_GRAY);
    tft.drawRect(CAM_X, CAM_Y, CAM_W, CAM_H, CLR_GRAY);
    tft.fillRect(INFO_X, 240, INFO_W, INFO_H, CLR_BLACK);
    tft.drawCentreString("INFO", INFO_X + INFO_W / 2, INFO_Y + 20, 2);
}

void updateDisplay(String name, String accuracy, String distance, int errors, uint16_t textColor) {
    tft.setTextColor(textColor);
    tft.fillRect(INFO_X + 10, INFO_Y + 50, INFO_W - 20, 100, CLR_BLACK);
    tft.drawString("NAME: " + name, INFO_X + 10, INFO_Y + 50, 2);
    tft.drawString("SIMILI: " + accuracy, INFO_X + 10, INFO_Y + 70, 2);
    tft.drawString("DIST: " + distance + " cm", INFO_X + 10, INFO_Y + 90, 2);
    tft.drawString("ERR: " + String(errors), INFO_X + 10, INFO_Y + 110, 2);
}

void clearCameraArea() {
    tft.fillRect(CAM_X, CAM_Y, CAM_W, CAM_H, CLR_GRAY);
}