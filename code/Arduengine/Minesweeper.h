#include "ButtonManager.h"
#include "AbstractGame.h"
#include "Board.h"

#ifndef MINESWEEPER_H
#define MINESWEEPER_H

class Minesweeper: public AbstractGame {
  private:
    uint16_t xSize, ySize;
    uint16_t drawScale;
    uint16_t currentPos;
    uint16_t minesToMake;
    uint16_t revealed;
    uint16_t flagged;
  public:
    Board board;
    Minesweeper() {
      name = "sweep";
      changeSeed();
    }
    void quitGame() {}
    void startGame() {
      xSize = 10;
      ySize = 10;
      currentPos = 0;
      minesToMake = 15;
      drawScale = SCREEN_WIDTH / xSize;
      reset();
    }
    void clickTile(int16_t tile, bool isFlagClick, bool isHuman);
    void reset();
    void frameTick();
    void drawTile(int16_t tile, bool drawMiddle, bool drawBorder, bool borderOn);
};
#endif