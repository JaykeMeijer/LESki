#include "strands.h"

strand_t strands[] = {
  {.rmtChannel = 1, .gpioNum = 25, .ledType = LED_WS2812B, .brightLimit = 32, .numPixels =  NUM_PIXELS},
  {.rmtChannel = 2, .gpioNum = 26, .ledType = LED_WS2812B, .brightLimit = 32, .numPixels =  NUM_PIXELS},
  {.rmtChannel = 3, .gpioNum = 0, .ledType = LED_WS2812B, .brightLimit = 32, .numPixels =  NUM_PIXELS},
  {.rmtChannel = 4, .gpioNum = 16, .ledType = LED_WS2812B, .brightLimit = 32, .numPixels =  NUM_PIXELS}
};

int strands_init() {
  // Initialize ws2812 lib
  if(ws2812_init(strands, STRANDS)) {
    return 1;
  }
  setStrip(0, makeRGBVal(255, 0, 0));
  setStrip(1, makeRGBVal(0, 255, 0));
  setStrip(2, makeRGBVal(0, 0, 255));
  setStrip(3, makeRGBVal(255, 255, 0));
  showStrips();

  return 0;
}

/* Set an entire strand with one color */
void setAll(rgbVal use_color) {
  for (int i = 0; i < STRANDS; i++) {
    setStrip(i, use_color);
  }
}

/* Set pixel x on all strands to color */
void setRow(int pixel, rgbVal use_color) {
  for (int i = 0; i < STRANDS; i++) {
    strands[i].pixels[pixel] = use_color;
  }
}

/* Set an entire strip with one color */
void setStrip(int strand, rgbVal use_color) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    strands[strand].pixels[i] = use_color;
  }
}

/* Set a single pixel */
void setPixel(int strand, int pixel, rgbVal use_color) {
  strands[strand].pixels[pixel] = use_color;
}

/* Show all strips */
void showStrips() {
  for (int i = 0; i < STRANDS; i++) {
    ws2812_setColors(&strands[i]);
  }
}
