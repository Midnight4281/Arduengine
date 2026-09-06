#include "ButtonManager.h"
#include "AbstractGame.h"
#include "Board.h"

#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H

class SnakeGame : public AbstractGame {
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
    SnakeGame(bool* flag) : AbstractGame(flag) {
      name = getName();
      frameRate = 20;
    }

    void quitGame() {
      setRecord();
    }
    void startGame() {
      reset();

    }
    static String getName() {
      return "snake";
    }
    static String getDisplayName() {
      return "Snake";
    }
    static AbstractGame* createGame(bool* flag) {
      return new SnakeGame(flag);
    }
    int8_t getNextTile(int8_t xd, int8_t yd);
    void frameTick();
    void reset();
    void drawTile(uint16_t i);
    bool canMoveSnake(int8_t xd, int8_t yd);
    void moveSnake(int8_t xd, int8_t yd);
    uint16_t getHeadPos();
    uint16_t getNextTailPos();
    uint16_t getTailPos();
    void lose();
    void win();
    void addApple();
    void setRecord();
};
#endif