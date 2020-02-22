#include <Adafruit_SSD1306.h>

#include <DFRobotDFPlayerMini.h>
#include <Keypad.h>
#include "SoftwareSerial.h"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     5 // Reset pin # (or -1 if sharing Arduino reset pin)


SoftwareSerial mySerial(10, 11);

const int interrupt0 = 0; 

boolean isPlaying = false;

int currVolume = 10;
int lastVolume = currVolume;

int CLK = 2;//CLK->D2
int DT = 3;//DT->D3 
int SW = 4;//SW->D4 

int lastCLK = 0; 

int number_folders = 1;
int current_folder = 1;
int current_song = 1;
int current_folder_count = 1;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
const char hexaKeys[ROWS][COLS] PROGMEM = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 12, 13}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {17, 16, 15, 14}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

DFRobotDFPlayerMini myDFPlayer;

void setup () {
  Serial.begin(9600);
  mySerial.begin (9600);
  delay(1000);
  Serial.println("go");

  pinMode(SW, INPUT); 
  digitalWrite(SW, HIGH); 
  pinMode(CLK, INPUT); 
  pinMode(DT, INPUT);
  attachInterrupt(interrupt0, ClockChanged, CHANGE); 

  isPlaying = true;
  myDFPlayer.begin(mySerial);
  delay(1000);
  number_folders = myDFPlayer.readFolderCounts() - 1;
  current_folder_count = myDFPlayer.readFileCountsInFolder(1);
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(currVolume);
  delay(300);
  randomSeed(analogRead(0));
  randomize_folder();
}

int digit_to_int(char d)
{
 char str[2];

 str[0] = d;
 str[1] = '\0';
 return (int) strtol(str, NULL, 10);
}

void loop () { 
 // handle volume
  if (currVolume != lastVolume) {
    setVolume(currVolume);
    lastVolume = currVolume;
    delay(100);
  }
  if (!digitalRead(SW)) {
    Serial.println("Pause");
    if(isPlaying) {
      myDFPlayer.pause();
      isPlaying = false;
    } else {
      isPlaying = true;
      myDFPlayer.start();
    }
    delay(600);
  }
  char customKey = customKeypad.getKey();
  int customKeyInt = digit_to_int(customKey);
  if (customKeyInt > 0) {
//    Serial.print("Playing sound: ");
    Serial.println(customKeyInt);
    myDFPlayer.advertise(customKeyInt);
  }

  if (customKey == 'A') {
    if(isPlaying) {
      myDFPlayer.pause();
      isPlaying = false;
    } else {
      isPlaying = true;
      myDFPlayer.start();
    }
  }


  if (customKey == 'B') {
    if(isPlaying) {
      prev_song();
      //myDFPlayer.previous();
    }
  }
 
  if (customKey == 'C') {
    if(isPlaying) {
      next_song();
      //myDFPlayer.next();
    }
  }

  if (customKey == 'D') {
    if (number_folders > 1) {
      current_folder++;
      if (current_folder >= number_folders) {
        current_folder = 1;
      }
      randomize_folder();
    }
  }

  if (myDFPlayer.available()) {
    myDFPlayer.read();
    uint8_t type = myDFPlayer.readType();
    int value = myDFPlayer.read();
    if (isPlaying && type == DFPlayerPlayFinished) {
      next_random();
    }
  }

}

void prev_song() {
  current_song--;
  if (current_song == 0) {
    current_song = current_folder_count;
  }
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;  
}

void next_song() {
  current_song++;
  if (current_song > current_folder_count) {
    current_song = 1;
  }
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;  
}

void next_random() {
  current_song = random(current_folder_count) + 1;
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;
}

void randomize_folder() {
  current_folder_count = myDFPlayer.readFileCountsInFolder(current_folder);
  delay(100);
  current_song = random(current_folder_count) + 1;
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;
}

void ClockChanged() { 
  //Read the CLK pin level 
  int clkValue = digitalRead(CLK); 
  //Read the DT pin level 
  int dtValue = digitalRead(DT); 
  if (lastCLK != clkValue) { 
    lastCLK = clkValue; 
    //CLK and inconsistent DT + 1, otherwise - 1 
    currVolume += (clkValue != dtValue ? -1 : 1); 
    if (currVolume < 0) {
      currVolume = 0;
    }
    if (currVolume > 23) {
      currVolume = 23;
    }
  } 
} 

void setVolume(int volume)
{
  myDFPlayer.volume(volume);
}
