#include "display.h"
SSD1306 display(0x3c, 5, 4);

void display_init() {
  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_24);
}

void display_draw(String program_name, String color_name, int speed, bool blackout, bool backwards) {
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, program_name);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 28, color_name);
  display.drawString(0, 46, String(speed));
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  if (backwards) {
    display.drawString(128, 25, "Rev");
  }
  display.setFont(ArialMT_Plain_24);
  if (blackout) {
    display.drawString(128, 40, "B/O");
  }
    
  display.display();
}

void display_draw_servermode(String ssid, String ip, bool otamode) {
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  
  if (otamode) {
    display.drawString(0, 0, "OTA active");
  } else {
    display.drawString(0, 0, "Server active");
  }

  display.drawString(0, 18, "SSID: " + ssid);
  display.drawString(0, 36, "IP: " + ip);

  display.display();
}

void display_error(String error) {
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Error");
  display.drawString(0, 18, error);
  display.display();
}

