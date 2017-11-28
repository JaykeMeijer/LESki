#include "FS.h"
#include "SPIFFS.h"
#include "data.h"


void setup() {
  Serial.begin(115200);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  if (true) { // Change to false to NOT override the current contents
    Serial.println("FORMATTING");
    if(!SPIFFS.format()) {
      Serial.println("Failed to format");
    }
    clearDir("/");
  }

  listDir("/", 0);

  File file = SPIFFS.open("/rgb", FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }

  for (int i = 0; i < program_len; i++) {
    if(!file.print(program[i])){
      Serial.println("Write failed");
      break;
    }
  }

  file.close();
  listDir("/", 0);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void listDir(const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SPIFFS.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void clearDir(const char * dirname) {
  Serial.printf("Clearing directory: %s\n", dirname);

  File root = SPIFFS.open(dirname);
  if(!root){
    Serial.println("Failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      clearDir(file.name());
    } else {
      SPIFFS.remove(file.name());
    }
    file = root.openNextFile();
  }
}

