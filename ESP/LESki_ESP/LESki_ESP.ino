
#if defined(ARDUINO) && ARDUINO >= 100
  // No extras
#elif defined(ARDUINO) // pre-1.0
  // No extras
#elif defined(ESP_PLATFORM)
  #include "arduinoish.hpp"
#endif

#include "display.h"
#include "programs.h"

/* ============================== ESP BASE INIT ======================================*/
#define WIFIBUTTON 36                      //If this pin is 0 during startup wifi mode will be enabled            
#define WIFIMODE  WIFI_AP_STA             //Define Which wifi mode will be enable WIFI_AP (only access point) , WIFI_STA (Only cstation/Client) WIFI_AP_STA (both AP and Station)


//Include necessary libaries (board type dependend)
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
WebServer server(80);

#include <ArduinoOTA.h>
#include <ArduinoJson.h>

const char* ap_name = "LESkiFi";
const char* ssid = "daybrayke";                                          //Wifi SSID
const char* password = "jhepem9808";                                      //Wifi Password
int timeZone = 1;                                                      // Central European Time

const char* host = "esp32fs";
bool connected = true;
bool wifiinitstarted = false;
bool servermode = false;                                                //servermode disabled 
bool otamode = false;                                                   //otamode disabled


//holds the current upload
File fsUploadFile;
/* ==============================================================================================*/

int nr_programs;
int program = 0;
int current_program = 0;
int color = 0;
int speed = 1;
bool blackout = false;
bool current_blackout = false;
bool backwards = false;

Program **programs;

String *color_names_here;
rgbVal *colors_here;

void setup() {
  Serial.begin(115200);

  pinMode(36, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  filesystemInit();
  display_init();

  if (!digitalRead(WIFIBUTTON)) {
    // WiFI mode
    wifiInit();
    servermode = true;
  } else {
    // Normal operations
    attachInterrupt(digitalPinToInterrupt(36), button_program_up_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(39), button_program_down_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(2), button_speed_up_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(14), button_speed_down_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(12), button_color_up_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(13), button_color_down_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(15), button_reverse_pushed, RISING);
    attachInterrupt(digitalPinToInterrupt(3), button_blackout_pushed, RISING);
    if(strands_init()) {
      Serial.println("WS2812 Init FAILURE: halting");
      while (true) {};
    }
  
    // Initialize colors
    initColors();
    colors_here = getColors();
    color_names_here = getColorNames();
    
    // Initialize programs
    int additional_programs = countProgramList();
    if (additional_programs < 0) {
      Serial.println("Failed to open directory");
    } else {
      Serial.printf("Programs discovered on SPIFFS: %d\n", additional_programs);    
    }

    uint8_t static_programs = 8;
    nr_programs = static_programs + additional_programs;
    programs = (Program**)malloc(sizeof(Program*) * nr_programs);
    programs[0] = new Alternate();
    programs[1] = new Rainbow();
    programs[2] = new MovingRainbow();
    programs[3] = new Run();
    programs[4] = new RunGhost();
    programs[5] = new RunMany();
    programs[6] = new ColorRide();
    programs[7] = new Pulsate();
  
    Serial.println("Loading programs: ");
    Serial.println(loadProgramList(programs, static_programs));
  
    for (int i = 0; i < nr_programs; i++) {
      programs[i]->init();
    } 
  }
}

void button_program_up_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    program = (program + 1) % nr_programs;
  }
  last_interrupt_time = interrupt_time;
}

void button_program_down_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    program = program - 1;
    if (program < 0) {
      program = nr_programs - 1;
    }
  }
  last_interrupt_time = interrupt_time;
}

void button_color_up_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    color = (color + 1) % NUM_COLORS;
  }
  last_interrupt_time = interrupt_time;
}

void button_color_down_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    color = color - 1;
    if (color < 0) {
      color = NUM_COLORS - 1;
    }
  }
  last_interrupt_time = interrupt_time;
}

void button_speed_up_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    speed = speed + 1;
    if (speed == MAX_SPEED + 1) {
      speed = 1;
    }
  }
  last_interrupt_time = interrupt_time;
}

void button_speed_down_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    speed = speed - 1;
    if (speed == 0) {
      speed = MAX_SPEED;
    }
  }
  last_interrupt_time = interrupt_time;
}

void button_blackout_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    blackout = !blackout;
  }
  last_interrupt_time = interrupt_time;
}

void button_reverse_pushed() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    backwards = !backwards;
  }
  last_interrupt_time = interrupt_time;
}

void loop() {
  if (servermode) {
    display_draw_servermode(ap_name, WiFi.softAPIP().toString(), otamode);
    server.handleClient();

    if (otamode) {
      ArduinoOTA.handle();
    }

    if (!connected && !wifiinitstarted) {                                //Lost netwerk and init not in progress the reinitialise
      Serial.println("lost Wifi REINT network; Reconnect started");
      wifiInit();
    }
  } else {
    display_draw(programs[program]->name, color_names_here[color], speed, blackout, backwards);
  
    if (current_program != program) {
      if (!programs[current_program]->unload()) {
        Serial.println("Failed to unload program!");
      }
      if (!programs[program]->load()) {
        Serial.println("Failed to load program!");
        Serial.println(((FileProgram*)programs[program])->n_frames);
      } else {
        Serial.println("Succesfull load");
      }
      current_program = program;
    }
  
    if (blackout && ! current_blackout) {
      setBlackout();
      current_blackout = true;
    } else if (!blackout) {
      current_blackout = false;
      if(backwards) {
        programs[program]->step_back(colors_here[color], speed);
      } else {
        programs[program]->step(colors_here[color], speed);
      }
      if (color == 7) {
        updateRainbow(speed);
      }
    }
    delay(20);
  }
}
