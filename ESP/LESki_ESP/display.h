#include <WString.h>
#include "SSD1306.h"

void display_init();
void display_draw(String, String, int, bool, bool);
void display_draw_servermode(String, String, bool);
void display_error(String);
