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

typedef AbstractGame* (*GameCreateList)(bool*);
typedef String (*StringGameList)();

PCF8574 pcf(0x27);
int frameRate = 20;

DIYables_TFT_ILI9486_Shield display;

ButtonManager buttons;

GameCreateList createGameList[] = {
  &Minesweeper::createGame,
  &TestGame::createGame
};
StringGameList nameGameList[] = {
  &Minesweeper::getDisplayName,
  &TestGame::getDisplayName
};

AbstractGame* game;

uint8_t numberOfGames = sizeof(createGameList) / sizeof(createGameList[0]);;

bool gameWasDeleted;
uint8_t currentGameSelection;

Minesweeper m(&gameWasDeleted);

//left, right, up, down, SELECT, START, B, A 

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  delay(100);
  display.begin();
  display.setRotation(2);
  display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
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
  
    Serial.println("Test");

  delay(100);
  //game = &m;
  game = nullptr;
  currentGameSelection = 0;
  setGameSelectionText();
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

void setGameSelectionText() {
  display.setTextSize(3);  // Adjust text size as needed
  display.fillRect(10, 100, 300, 30, BLACK);
  display.setCursor(15, 100);
  display.println(String(nameGameList[currentGameSelection]()));
}
void setCurrentGame() {
  game = createGameList[currentGameSelection](&gameWasDeleted);
  game->gameSetup(&buttons, display);
}
void loop()
{
  unsigned long startFrameTime = millis();
  /*
  Serial.print(F("Free RAM: "));
  printFreeRam();
  */
  
  bool b[8];
  for(int i = 0; i < 8; i++) {
    b[7 - i] = !pcf.digitalRead(i); // set each bool to be if the corresponding button is pressed
  }
  buttons.passInputs(b);
  if(game != nullptr) {
    //game->passInputs(b);
    game->frameTick();
    if(buttons.getButtonWentDown("-")) {
      game->exitGame();
    }
    if(gameWasDeleted) {
      game = nullptr;
      setGameSelectionText();
    }
  } else {//handle main menu calculations
    if(buttons.getButtonWentDown("L")) {
      if(currentGameSelection > 0) {
        currentGameSelection--;
      } else {
        currentGameSelection = numberOfGames - 1;
      }
      Serial.println(currentGameSelection);
      setGameSelectionText();
    } else if (buttons.getButtonWentDown("R")) {
      if(currentGameSelection >= numberOfGames - 1) {
        currentGameSelection = 0;
      } else {
        currentGameSelection++;
      }
      Serial.println(currentGameSelection);
      setGameSelectionText();
    } else if (buttons.getButtonWentDown("+")) {
      setCurrentGame();
    }
  }
  unsigned long endFrameTime = millis();
  float duration = endFrameTime - startFrameTime;
  delay(max(1000 / frameRate - duration, 1));
}