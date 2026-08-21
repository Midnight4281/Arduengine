#include "ButtonManager.h"
#include "AbstractGame.h"

#ifndef TEST_GAME_H
#define TEST_GAME_H

class TestGame : public AbstractGame {
  private:
    int x = 0;
    int y = 0;
  public:
    uint8_t turns = 0;
    uint8_t selectedTile = 0; 
    uint8_t board[9];
    uint8_t winRows[8][3] = { //all possible win conditions
      {0, 1, 2},
      {3, 4, 5},
      {6, 7, 8},
      {0, 3, 6},
      {1, 4, 7},
      {2, 5, 8},
      {0, 4, 8},
      {2, 4, 6}
    };
    TestGame() {
      name = "ttt";
    }
    void quitGame() {

    }
    void startGame() {
      reset();
    }
    void frameTick();
    void reset();
    uint8_t getTile(uint8_t x, uint8_t y);
    void setTile(uint8_t x, uint8_t y, uint8_t v);
    uint8_t getScore(uint8_t l);
    void drawTile(uint8_t t, bool, bool);
};
#endif