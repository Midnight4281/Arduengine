#include "ButtonManager.h"
#include "AbstractGame.h"
#include "Board.h"

#ifndef DEBUG_GAME_H
#define DEBUG_GAME_H

class DebugGame : public AbstractGame {
  private:
    int x = 0;
    int y = 0;
  public:
    Board board;
    uint16_t snakeLength = 1;
    uint16_t headPos = 0;
    uint16_t lastHeadPos = 0;
    uint16_t tailPos = 0;
    uint16_t tailPauseFrames = 0;
    uint16_t oldRecord = 0;
    int8_t xDir;
    int8_t yDir;
    uint8_t frameCount = 0;
    DebugGame(bool* flag) : AbstractGame(flag) {
      name = getName();
      frameRate = 20;
    }

    void quitGame() {
    }
    void startGame() {
      reset();
    }
    static String getName() {
      return "debug";
    }
    static String getDisplayName() {
      return "Debug";
    }
    static AbstractGame* createGame(bool* flag) {
      return new DebugGame(flag);
    }
    void frameTick() {
      
    }
    void reset();
};
#endif