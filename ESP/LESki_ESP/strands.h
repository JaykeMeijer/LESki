#include <WString.h>
#include "ws2812.h"

#ifndef STRANDS_H
#define STRANDS_H

#define NUM_PIXELS  50
#define STRANDS     4

int strands_init();

/* Set an entire strand with one color */
void setAll(rgbVal use_color);

/* Set pixel x on all strands to color */
void setRow(int pixel, rgbVal use_color);

/* Set an entire strip with one color */
void setStrip(int strand, rgbVal use_color);

/* Set a specific pixel */
void setPixel(int strand, int pixel, rgbVal use_color);

/* Show all strips */
void showStrips();

#endif
