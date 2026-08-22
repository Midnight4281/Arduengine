#include "Arduino.h"
#include "PCF8574.h"
#include "AbstractGame.h"
#include "TestGame.h"
#include "Minesweeper.h"
#include "ButtonManager.h"
#include <DIYables_TFT_Shield.h>

//REMINDER: All images must be stored in BMP format

#define BLACK     DIYables_TFT::colorRGB(0, 0, 0)
#define BLUE      DIYables_TFT::colorRGB(0, 0, 255)
#define RED       DIYables_TFT::colorRGB(255, 0, 0)
#define GREEN     DIYables_TFT::colorRGB(0, 255, 0)
#define ORANGE    DIYables_TFT::colorRGB(255, 165, 0)
#define PINK      DIYables_TFT::colorRGB(255, 192, 203)
#define VIOLET    DIYables_TFT::colorRGB(148, 0, 211)
#define TURQUOISE DIYables_TFT::colorRGB(64, 224, 208)
#define WHITE     DIYables_TFT::colorRGB(255, 255, 255)

PCF8574 pcf(0x27);
int frameRate = 20;
String keyCodes[] = {"L", "R", "U", "D", "-", "+", "B", "A"};

DIYables_TFT_ILI9486_Shield display;

ButtonManager buttons;

AbstractGame* game;
Minesweeper g;

//left, right, up, down, SELECT, START, B, A 

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  delay(100);
  display.begin();
  display.setRotation(0);
  //display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
  delay(100);
  
  pcf.pinMode(0, INPUT);
  pcf.pinMode(1, INPUT);
  pcf.pinMode(2, INPUT);
  pcf.pinMode(3, INPUT);
  pcf.pinMode(4, INPUT);
  pcf.pinMode(5, INPUT);
  pcf.pinMode(6, INPUT);
  pcf.pinMode(7, INPUT);
  
  Serial.print("Initializing PCF8574...");
  if (pcf.begin()){
    Serial.println("OK");
  } else {
    Serial.println("KO");
  }
  
  
  delay(100);
  game = &g;
  game->gameSetup(buttons, display);
  delay(1000);
  
}

extern "C" char* sbrk(int incr);

int getFreeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void printFreeRam() {
  Serial.print("SRAM Left (Bytes): ");
  Serial.println(getFreeRam());
}


void loop()
{
  /*
  Serial.print(F("Free RAM: "));
  printFreeRam();
  */
  
  
  bool b[8];
  for(int i = 0; i < 8; i++) {
    b[7 - i] = !pcf.digitalRead(i); // set each bool to be if the corresponding button is pressed
  }
  
  
  game->passInputs(b);
  game->frameTick();
  delay(1000 / frameRate);
}