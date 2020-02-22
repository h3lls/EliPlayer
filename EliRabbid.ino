// Adafruit_ImageReader test for Adafruit ST7789 320x240 TFT Breakout for Arduino.
// Demonstrates loading images to the screen, to RAM, and how to query
// image file dimensions.
// Requires three BMP files in root directory of SD card:
// parrot.bmp, miniwoof.bmp and wales.bmp.
// As written, this uses the microcontroller's SPI interface for the screen
// (not 'bitbang') and must be wired to specific pins (e.g. for Arduino Uno,
// MOSI = pin 11, MISO = 12, SCK = 13). Other pins are configurable below.

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions


// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// TFT display and SD card share the hardware SPI interface, using
// 'select' pins for each to identify the active device on the bus.

#define SD_CS    4 // SD card select pin
#define TFT_CS  10 // TFT select pin
#define TFT_DC   8 // TFT display/command pin
#define TFT_RST  9 // Or set to -1 and connect to Arduino RESET pin

  SdFat                SD;         // SD card filesystem
  SdFile file;
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys


Adafruit_ST7789      tft    = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;


int seconds_30 = 5 * 1000;
unsigned long last_time = millis();
int current_image = 1;

char buf[6];
int fileCount = 0;

void setup(void) {

  ImageReturnCode stat; // Status from image-reading functions

  Serial.begin(9600);
#if !defined(ESP32)
  while(!Serial);       // Wait for Serial Monitor before continuing
#endif

  tft.init(135, 240);           // Init ST7789 320x240

  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatFileSystem object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));

  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(10))) { // Breakouts require 10 MHz limit due to longer wires
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }

  Serial.println(F("OK!"));

  Serial.print(F("Loading purple.bmp to screen..."));
  stat = reader.drawBMP("/01.bmp", tft, 0, 0);
  reader.printStatus(stat);   // How'd we do?
  SD.vwd()->rewind();
  while (file.openNext(SD.vwd(), O_READ)) {
    if (!file.isHidden()) {
      fileCount++;
    }
    file.close();
  }
  Serial.print(fileCount);
}

void loop() {
  if (millis() > last_time + seconds_30) {
    last_time = millis();
    sprintf(buf, "/%02d.bmp", current_image);
    reader.drawBMP(buf, tft, 0, 0);
    if (current_image < fileCount + 1) {
      current_image++;
    } else {
      current_image = 1;
    }
  }
}
