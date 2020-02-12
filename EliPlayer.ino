#include <DFRobotDFPlayerMini.h>


#include <LiquidCrystal.h>

#include <Keypad.h>

///              MP3 PLAYER PROJECT
/// http://educ8s.tv/arduino-mp3-player/
//////////////////////////////////////////


#include "SoftwareSerial.h"
SoftwareSerial mySerial(10, 11);
# define Start_Byte 0x7E
# define Version_Byte 0xFF
# define Command_Length 0x06
# define End_Byte 0xEF
# define Acknowledge 0x00 //Returns info with command 0x41 [0x01: info, 0x00: no info]

# define ACTIVATED LOW

void printDetail(uint8_t type, int value);

boolean isPlaying = false;

int currVolume = 10;
int lastVolume = currVolume;

int CLK = 2;//CLK->D2
int DT = 3;//DT->D3 
int SW = 4;//SW->D4 

const int interrupt0 = 0; 

int lastCLK = 0; 

const int DRAC_SOUND = 1;
const int SCREAM_SOUND = 2;
const int GHOST_SOUND = 3;
const int WOLF_SOUND = 4;
const int LAZER_SOUND = 5;
const int TARDIS_SOUND = 6;
const int HEHEHE_SOUND = 7;
const int SCARYHOUSE_SOUND = 8;
const int SHOTGUN_SOUND = 9;


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
  Serial.print("Folder Counts:");
  Serial.println(myDFPlayer.readFolderCounts());
  Serial.print("File Coutns in Folder 1:");
  Serial.println(myDFPlayer.readFileCountsInFolder(1));
  Serial.print("File Counts:");
  Serial.println(myDFPlayer.readFileCounts());
  myDFPlayer.setTimeOut(500);
  myDFPlayer.volume(currVolume);
  delay(300);
  myDFPlayer.loopFolder(1);
  delay(100);
  myDFPlayer.start();

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
    Serial.print("Pause");
    if(isPlaying) {
      myDFPlayer.pause();
      isPlaying = false;
    } else {
      isPlaying = true;
      myDFPlayer.play();
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
      myDFPlayer.play();
    }
  }


  if (customKey == 'B') {
    if(isPlaying) {
      myDFPlayer.previous();
    }
  }
 
  if (customKey == 'C') {
    if(isPlaying) {
      myDFPlayer.next();
    }
  }

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }

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
