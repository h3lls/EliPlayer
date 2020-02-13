#include <DFRobotDFPlayerMini.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include "SoftwareSerial.h"

void printDetail(uint8_t type, int value);

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
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 19, 18}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {17, 16, 15, 14}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//LiquidCrystal lcd(1, 5, 6, 7, 12, 13);
//char array1[]=" Eli Sound                ";
//int lcdTime = 500;
DFRobotDFPlayerMini myDFPlayer;

void setup () {
  //lcd.begin(16, 2);
  pinMode(SW, INPUT); 
  digitalWrite(SW, HIGH); 
  pinMode(CLK, INPUT); 
  pinMode(DT, INPUT);
  attachInterrupt(interrupt0, ClockChanged, CHANGE); 

  Serial.begin(9600);
  mySerial.begin (9600);
  delay(1000);
  
  //playFirst();
  isPlaying = true;
  myDFPlayer.begin(mySerial);
  delay(1000);
  number_folders = myDFPlayer.readFolderCounts() - 1;
  Serial.print("Folder Counts:");
  Serial.println(myDFPlayer.readFolderCounts());
  Serial.print("File Coutns in Folder 1:");
  current_folder_count = myDFPlayer.readFileCountsInFolder(1);
  Serial.println(current_folder_count);
  Serial.print("File Counts:");
  Serial.println(myDFPlayer.readFileCounts());
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
// // Display
// lcd.setCursor(15,0);
// for ( int positionCounter1 = 0; positionCounter1 < 26; positionCounter1++)
//    {
//      lcd.scrollDisplayLeft();  //Scrolls the contents of the display one space to the left.
//      lcd.print(array1[positionCounter1]);  // Print a message to the LCD.
//      delay(lcdTime);  //wait for 250 ms
//    }
// lcd.clear();
  
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
    Serial.print("Playing sound: ");
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
    printDetail(type, value); //Print the detail message from DFPlayer to handle different errors and states.
  }

}

void prev_song() {
  current_song--;
  if (current_song == 0) {
    current_song = current_folder_count;
  }
  Serial.print("Playing Folder: ");
  Serial.print(current_folder);
  Serial.print(" Playing Song: ");
  Serial.println(current_song);
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;  
}

void next_song() {
  current_song++;
  if (current_song > current_folder_count) {
    current_song = 1;
  }
  Serial.print("Playing Folder: ");
  Serial.print(current_folder);
  Serial.print(" Playing Song: ");
  Serial.println(current_song);
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;  
}

void next_random() {
  current_song = random(current_folder_count) + 1;
  Serial.print("Playing Folder: ");
  Serial.print(current_folder);
  Serial.print(" Playing Song: ");
  Serial.println(current_song);
  myDFPlayer.playFolder(current_folder, current_song);
  isPlaying = true;
}

void randomize_folder() {
  current_folder_count = myDFPlayer.readFileCountsInFolder(current_folder);
  delay(100);
  current_song = random(current_folder_count) + 1;
  Serial.print("Playing Folder: ");
  Serial.print(current_folder);
  Serial.print(" Playing Song: ");
  Serial.println(current_song);
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
  //execute_CMD(0x06, 0, volume); // Set the volume (0x00~0x30)
  //delay(2000);
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
