#include <WString.h>
#include "strands.h"
#include "FS.h"
#include "SPIFFS.h"

#ifndef PROGRAMS_H
#define PROGRAMS_H

#define MAX_SPEED 10
#define FILE_PATH "/"

#define NUM_COLORS  8

class Program {
  public:
    String name;
    virtual void init();
    virtual void step(rgbVal color, int speed);
    virtual void step_back(rgbVal color, int speed);
    virtual bool load();
    virtual bool unload();
  protected:
    int current;
};

class FileProgram : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);
    bool load();
    bool unload();
    void set_name(String name);
    byte n_frames;

  private:
    int divider;
    bool loaded;
    bool use_color;
    rgbVal ***frames;
    uint8_t ***sframes;
};

class Alternate : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    int divider;
};

class Rainbow : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    uint8_t r, g, b;
};

class MovingRainbow : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    uint8_t r, g, b;
};

class Run : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    int divider;
};

class RunGhost : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    int divider;
    int ghost_length;
    float ghost_step;
};

class RunMany : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    int divider;
    int gap;
};

class ColorRide : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    int divider;
    int color_pointer;
    int led_pointer;
};

class Pulsate : public Program {
  public:
    void init();
    void step(rgbVal color, int speed);
    void step_back(rgbVal color, int speed);

  private:
    float brightness;
    bool upgoing;
};

/* Helper functions */
void drawRainbow(uint8_t r, uint8_t g, uint8_t b, uint8_t stepSize);
void drawRainbowRev(uint8_t r, uint8_t g, uint8_t b, uint8_t stepSize);
void recalcRainbow(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t stepSize);
void recalcRainbowRev(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t stepSize);

int countProgramList();
String loadProgramList(Program **programs, int start);

void initColors();
rgbVal* getColors();
String* getColorNames();
void updateRainbow(uint8_t stepSize);
void setBlackout();

#endif
