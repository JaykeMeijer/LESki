#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

#include "programs.h"


SSD1306  display(0x3c, 5, 4);
Program programs[1];
int active_program = 0;
int frame_duration = 1337;
int time_since_frameupdate = millis();


void setup() {
  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);

  // Init programs
  programs[0] = Rainbow();
  programs[0].init();
}

void drawDisplay() {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, programs[active_program].name);
  display.setFont(ArialMT_Plain_16);
  String var = "S: " + frame_duration;
  var = var + " ms";
  display.drawString(0, 40, var);
}

void loop() {
  // Update frame if necessary
  if (millis() - time_since_frameupdate > frame_duration) {
    // Update frame
    time_since_frameupdate = millis();
    programs[active_program].step();
  }

  // Detect input to see if variables need updating

  // Update display
  display.clear();
  drawDisplay();
  display.display();
  delay(10);
}

