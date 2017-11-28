#include "programs.h"

String color_names[] = {"Red", "Green", "Blue", "Yellow", "Pink", "Cyan", "White", "Rainbow"};
rgbVal colors[NUM_COLORS];

void Program::init() {
  // To be overriden
  name = "Default";
}

void Program::step(rgbVal color, int speed) {
  // To be overriden
}

void Program::step_back(rgbVal color, int speed) {
  // To be overriden
}

bool Program::load() {
  // To be overriden
  return true;
}

bool Program::unload() {
  // To be overriden
  return true;
}

/* Manually programmed */
void FileProgram::init() {
  use_color = false;
  loaded = false;
}

void FileProgram::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  if (n_frames > 0) {
    current = (current + 1) % n_frames;
  }

  for (uint8_t j = 0; j < STRANDS; j++) {
    for (uint8_t k = 0; k < NUM_PIXELS; k++) {
      if(use_color) {
        float brightness = sframes[current][j][k] / 255.0;
        setPixel(j, k, makeRGBVal(brightness * color.r, brightness * color.g, brightness * color.b));
      } else {
        setPixel(j, k, frames[current][j][k]);
      }
    }
  }
  showStrips();
}

void FileProgram::step_back(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  if (n_frames > 0) {
    current = current > 0 ? current -1 : n_frames - 1;
  }

  for (uint8_t j = 0; j < STRANDS; j++) {
    for (uint8_t k = 0; k < NUM_PIXELS; k++) {
      if(use_color) {
        float brightness = sframes[current][j][k] / 255.0;
        setPixel(j, k, makeRGBVal(brightness * color.r, brightness * color.g, brightness * color.b));
      } else {
        setPixel(j, k, frames[current][j][k]);
      }
    }
  }
  showStrips();
}

void FileProgram::set_name(String new_name) {
  name = new_name;
}

bool FileProgram::load() {
  // Load
  File f = SPIFFS.open((String)FILE_PATH + name + ".seq", FILE_READ);

  // Parse header
  byte b;
  byte n_rows, n_pixs;
  f.readBytes((char*)&b, 1);
  use_color = ((int)b != 1);
  f.readBytes((char*)&n_frames, 1);
  f.readBytes((char*)&n_rows, 1);
  f.readBytes((char*)&n_pixs, 1);

  // Verify header is valid data
  if ((uint8_t)n_rows != STRANDS || (uint8_t)n_pixs != NUM_PIXELS) {
    return false;
  }

  f.readBytes((char*)&b, 1);
  while(b != '\n') {
    f.readBytes((char*)&b, 1);
  }

  // Allocate memory for frames
  if (use_color) {
    sframes = (uint8_t***)malloc(sizeof(uint8_t**) * n_frames);
    if (sframes == NULL) {return false;}
  } else {
    frames = (rgbVal***)malloc(sizeof(rgbVal**) * n_frames);
    if (frames == NULL) {return false;}
  }

  for (uint8_t i = 0; i < (uint8_t)n_frames; i++) {
    if (use_color) {
      sframes[i] = (uint8_t**)malloc(sizeof(uint8_t*) * STRANDS);
      if (sframes[i] == NULL) {return false;}
    } else {
      frames[i] = (rgbVal**)malloc(sizeof(rgbVal*) * STRANDS);
      if (frames[i] == NULL) {return false;}
    }
    for (uint8_t j = 0; j < STRANDS; j++) {
      if (use_color) {
        sframes[i][j] = (uint8_t*)malloc(sizeof(uint8_t) * NUM_PIXELS);
        if (sframes[i][j] == NULL) {return false;}
      } else {
        frames[i][j] = (rgbVal*)malloc(sizeof(rgbVal) * NUM_PIXELS);
        if (frames[i][j] == NULL) {return false;}
      }
    }
  }

  // Parse data into memory
  byte b_arr[3];
  for (uint8_t i = 0; i < n_frames; i++) {
    for (uint8_t j = 0; j < STRANDS; j++) {
      for (uint8_t k = 0; k < NUM_PIXELS; k++) {
        if (use_color) {
          f.readBytes((char*)b_arr, 1);
          sframes[i][j][k] = b_arr[0];
        } else {
          f.readBytes((char*)b_arr, 3);
          frames[i][j][k] = makeRGBVal(b_arr[0], b_arr[1], b_arr[2]);
        }
      }
    }
  }

  loaded = true;
  return true;
}

bool FileProgram::unload() {
  // Free used memory
  for (int i = 0; i < n_frames; i++) {
    for (int j = 0; j < STRANDS; j++) {
      if (use_color) {
        free(sframes[i][j]);
      } else {
        free(frames[i][j]);
      }
    }
    if (use_color) {
      free(sframes[i]);
    } else {
      free(frames[i]);
    }
  }
  if (use_color) {
    free(sframes);    
  } else {
    free(frames);
  }

  loaded = false;
  return true;
}

/* Alternate */
void Alternate::init() {
  name = "Alternate";
  current = 0;
  divider = 0;
}

void Alternate::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }
  if (current % 2 == 0) {
    setStrip(0, color);
    setStrip(1, color);
    setStrip(2, makeRGBVal(0, 0, 0));
    setStrip(3, makeRGBVal(0, 0, 0));
  } else {
    setStrip(0, makeRGBVal(0, 0, 0));
    setStrip(1, makeRGBVal(0, 0, 0));
    setStrip(2, color);
    setStrip(3, color);
  }
  current = (current + 1) % STRANDS;
  showStrips();
}

void Alternate::step_back(rgbVal color, int speed) {
  step(color, speed);
}

/* Static rainbow */
void Rainbow::init() {
  name = "Rainbow";
  current = 0;
  
  r = 255;
  g = 0;
  b = 255;
}

void Rainbow::step(rgbVal color, int speed) {
  recalcRainbow(&r, &g, &b, 2 * speed);
  setAll(makeRGBVal(r, g, b));
  showStrips();
}

void Rainbow::step_back(rgbVal color, int speed) {
  recalcRainbowRev(&r, &g, &b, 2 * speed);
  setAll(makeRGBVal(r, g, b));
  showStrips();
}

/* Moving rainbow */
void MovingRainbow::init() {
  name = "Rainbow 2";
  current = 0;
  
  r = 255;
  g = 0;
  b = 255;
}

void MovingRainbow::step(rgbVal color, int speed) {
  drawRainbow(r, g, b, 5 * speed);
  showStrips();
  recalcRainbow(&r, &g, &b, 5 * speed);
}

void MovingRainbow::step_back(rgbVal color, int speed) {
  drawRainbowRev(r, g, b, 5 * speed);
  showStrips();
  recalcRainbowRev(&r, &g, &b, 5 * speed);
}

/* Running PX */
void Run::init() {
  name = "Run";
  current = 0;
}

void Run::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current + 1) % NUM_PIXELS;
  setAll(makeRGBVal(0, 0, 0));
  setRow(current, color);
  showStrips();
}

void Run::step_back(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current > 0) ? current - 1 : NUM_PIXELS - 1;
  setAll(makeRGBVal(0, 0, 0));
  setRow(current, color);
  showStrips();
}


/* Running PX with Ghost */
void RunGhost::init() {
  name = "RunGhost";
  current = 0;
  ghost_length = 5;
  ghost_step = 1.0 / float(ghost_length + 1);
}

void RunGhost::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current + 1) % NUM_PIXELS;
  setAll(makeRGBVal(0, 0, 0));
  setRow(current, color);

  int g_pix;
  float g_strength;
  float out_g_strength;
  for (int i = 1; i <= ghost_length; i++) {
    g_pix = current - i;
    if (g_pix < 0) {
      g_pix += NUM_PIXELS;
    }
    g_strength = 1.0 - (i * ghost_step);
    setRow(g_pix, makeRGBVal(color.r * g_strength, color.g * g_strength, color.b * g_strength));
  }
  
  showStrips();
}

void RunGhost::step_back(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current > 0) ? current - 1 : NUM_PIXELS - 1;
  setAll(makeRGBVal(0, 0, 0));
  setRow(current, color);
  int g_pix;
  float g_strength;
  for (int i = 1; i <= ghost_length; i++) {
    g_pix = current + i;
    if (g_pix >= NUM_PIXELS) {
      g_pix -= NUM_PIXELS;
    }
    g_strength = 1.0 - (i * ghost_step);
    setRow(g_pix, makeRGBVal(uint8_t(color.r * g_strength), uint8_t(color.g * g_strength), uint8_t(color.b * g_strength)));
  }

  showStrips();
}

/* Running PX */
void RunMany::init() {
  name = "RunMany";
  current = 0;
  gap = 3;
}

void RunMany::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current + 1) % gap;
  setAll(makeRGBVal(0, 0, 0));
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i % gap == current) {
      setRow(i, color);
    }
  }
  showStrips();
}

void RunMany::step_back(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  current = (current > 0) ? current - 1 : gap - 1;
  setAll(makeRGBVal(0, 0, 0));
  for (int i = 0; i < NUM_PIXELS; i++) {
    if (i % gap == current) {
      setRow(i, color);
    }
  }
  showStrips();
}

/* Color Ride PX */
void ColorRide::init() {
  name = "ColorRide";
  current = 0;
  color_pointer = 0;
  led_pointer = 0;
}

void ColorRide::step(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  setRow(led_pointer, colors[color_pointer]);
  showStrips();

  /* Prepare next step */
  led_pointer = (led_pointer + 1) % NUM_PIXELS;
  if (led_pointer == 0) {
    color_pointer = (color_pointer + 1) % NUM_COLORS;
  }
}

void ColorRide::step_back(rgbVal color, int speed) {
  if (divider < (MAX_SPEED + 1) - speed) {
    divider++;
    return;
  } else {
    divider = 0;
  }

  setRow(led_pointer, colors[color_pointer]);
  showStrips();

  /* Prepare next step */
  led_pointer = (led_pointer == 0 ? NUM_PIXELS - 1 : led_pointer - 1);

  if (led_pointer == NUM_PIXELS - 1) {
    color_pointer = (color_pointer + 1) % NUM_COLORS;
  }
}

/* Pulsate */
void Pulsate::init() {
  name = "Pulsate";
  brightness = 1.0;
  upgoing = false;
}

void Pulsate::step(rgbVal color, int speed) {
  if (upgoing) {
    brightness += (speed / 100.0);
  } else {
    brightness -= (speed / 100.0);
  }

  if (brightness > 1) {
    brightness = 1.0;
    upgoing = false;
  } else if (brightness < 0) {
    brightness = 0.0;
    upgoing = true;
  }
  
  setAll(makeRGBVal(color.r * brightness, color.g * brightness, color.b * brightness));
  showStrips();
}

void Pulsate::step_back(rgbVal color, int speed) {
  this->step(color, speed);
}

/* Helper functions */
void drawRainbow(uint8_t r, uint8_t g, uint8_t b, uint8_t stepSize) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    setRow(i, makeRGBVal(r, g, b));
    recalcRainbow(&r, &g, &b, stepSize);
  }
}

void drawRainbowRev(uint8_t r, uint8_t g, uint8_t b, uint8_t stepSize) {
  for (int i = NUM_PIXELS - 1; i >= 0; i--) {
    setRow(i, makeRGBVal(r, g, b));
    recalcRainbowRev(&r, &g, &b, stepSize);
  }
}

void recalcRainbow(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t stepSize) {
  if (*r == 255 && *g == 0 && *b > 0) {
    // Phase 0
    *b = *b < stepSize ? 0 : *b - stepSize;
  } else if (*r == 255 && *g < 255 && *b == 0) {
    // Phase 1
    *g = *g + stepSize > 255 ? 255 : *g + stepSize; 
  } else if (*r > 0 && *g == 255 && *b == 0) {
    // Phase 2
    *r = *r < stepSize ? 0 : *r - stepSize;
  } else if (*r == 0 && *g == 255 && *b < 255) {
    // Phase 3
    *b = *b + stepSize > 255 ? 255 : *b + stepSize;
  } else if (*r == 0 && *g > 0 && *b == 255) {
    // Phase 4
    *g = *g < stepSize ? 0 : (*g - stepSize);
  } else {
    // Phase 6
    *r = *r + stepSize > 255 ? 255 : *r + stepSize;
  }
}

void recalcRainbowRev(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t stepSize) {
  if (*r == 255 && *g == 0 && *b < 255) {
    // Phase 0
    *b = *b + stepSize > 255 ? 255 : *b + stepSize;
  } else if (*r == 255 && *g > 0 && *b == 0) {
    // Phase 1
    *g = *g < stepSize ? 0 : (*g - stepSize);
  } else if (*r < 255 && *g == 255 && *b == 0) {
    // Phase 2
    *r = *r + stepSize > 255 ? 255 : *r + stepSize;
  } else if (*r == 0 && *g == 255 && *b > 0) {
    // Phase 3
    *b = *b < stepSize ? 0 : *b - stepSize;
  } else if (*r == 0 && *g < 255 && *b == 255) {
    // Phase 4
    *g = *g + stepSize > 255 ? 255 : *g + stepSize; 
  } else {
    // Phase 6
    *r = *r < stepSize ? 0 : *r - stepSize;
  }
}

int countProgramList() {
  File directory = SPIFFS.open("/");

  if(!directory){
    return -1;
  }
  
  int cnt = 0;
  File file = directory.openNextFile();
  while(file){
    if (String(file.name()).endsWith(".seq")) {
      cnt++;
    }
    file = directory.openNextFile();
  }
  return cnt;
}

String loadProgramList(Program **programs, int start) {
  File directory = SPIFFS.open(FILE_PATH);

  if(!directory){
    return "Failed to open directory";
  }
  
  String fl = "";
  File file = directory.openNextFile();
  while(file){
    if (((String)file.name()).endsWith(".seq")) {
      fl += file.name();
  
      // Create program object
      programs[start] = new FileProgram();
      String programname = (String)(file.name());
      programname.remove(0, 1);
      programname.remove(programname.length() - 4);
      ((FileProgram*)programs[start])->set_name(programname);
  
      start++;
    }
    file = directory.openNextFile();
  }
  return fl;
}

void initColors() {
  // Create colors
  colors[0] = makeRGBVal(255, 0, 0);
  colors[1] = makeRGBVal(0, 255, 0);
  colors[2] = makeRGBVal(0, 0, 255);
  colors[3] = makeRGBVal(255, 255, 0);
  colors[4] = makeRGBVal(255, 0, 255);
  colors[5] = makeRGBVal(0, 255, 255);
  colors[6] = makeRGBVal(255, 255, 255);
  colors[7] = makeRGBVal(255, 0, 0);
}

void updateRainbow(uint8_t stepSize) {
  recalcRainbow(&colors[7].r, &colors[7].g, &colors[7].b, stepSize);
}

rgbVal* getColors() {
  return colors;
}

String* getColorNames() {
  return color_names;
}

void setBlackout() {
    setAll(makeRGBVal(0, 0, 0));
    showStrips();
}

