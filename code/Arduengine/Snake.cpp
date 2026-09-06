#include "Snake.h"
#include <DIYables_TFT_Shield.h>
void SnakeGame::frameTick() {
  if(buttons->getButtonPressed("L")) {
    xDir = -1;
    yDir = 0;
  } else if(buttons->getButtonPressed("R")) {
    xDir = 1;
    yDir = 0;
  } else if(buttons->getButtonPressed("U")) {
    xDir = 0;
    yDir = -1;
  } else if(buttons->getButtonPressed("D")) {
    xDir = 0;
    yDir = 1;
  }
  if(frameCount > 0) {
    frameCount--;
  } else {
    if(xDir != 0 || yDir != 0) {
      moveSnake(xDir, yDir);
    }
    frameCount = 4;
  }
}
void SnakeGame::lose() {
  setRecord();
  Serial.println("LOST");
  delay(2000);
  reset();
}
void SnakeGame::win() {
  setRecord();
}
void SnakeGame::setRecord() {
  //writeToSave(0, (snakeLength - tailPauseFrames));
  if(oldRecord < snakeLength - tailPauseFrames) {
    writeToSave(0, snakeLength - tailPauseFrames);
  }
}
void SnakeGame::reset() {
  clearScreen();
  display.fillRect(0, 320, 320, 160, DIYables_TFT::colorRGB(128, 128, 128));
  snakeLength = 4;
  board.reset(20, 20);
  headPos = 210;
  tailPos = 210;
  board.setVal(210, 5);
  drawTile(210);
  xDir = 0;
  yDir = 0;
  tailPauseFrames = snakeLength - 1;
  addApple();
  display.setTextColor(DIYables_TFT::colorRGB(0, 0, 0));
  display.setTextSize(3);  // Adjust text size as needed
  display.setCursor(20, 390);
  display.println("Score:");
  display.setCursor(20, 420);    // Set cursor position (x, y)
  //display.println("Highscore");
  oldRecord = readSaveFile(0);
  display.println("Highscore " + String(oldRecord));
}
bool SnakeGame::canMoveSnake(int8_t xd, int8_t yd) {
  uint16_t x = board.getXAtNumeric(getHeadPos());
  uint16_t y = board.getYAtNumeric(getHeadPos());
  bool f1 = board.movementIsOnBoard(x, y, xd, yd);
  uint8_t v = board.getVal(board.moveNumericByDir(getHeadPos(), xd, yd));
  bool f2 = !(v <= 4 && v >= 1);
  return f1 && f2;
}
int8_t SnakeGame::getNextTile(int8_t xd, int8_t yd) {
  return board.getVal(board.moveNumericByDir(getHeadPos(), xd, yd));
}
void SnakeGame::moveSnake(int8_t xd, int8_t yd) {
  if(getNextTile(xd, yd) == 9) {
      snakeLength += 4;
      tailPauseFrames += 4;
      addApple();
    }
    if(tailPauseFrames == 0) {
      if(snakeLength > 0) {
        uint16_t t = getNextTailPos();
        board.setVal(getTailPos(), 0);
        drawTile(getTailPos());
        tailPos = t;
      }
    } else {
      tailPauseFrames--;
      display.fillRect(130, 390, 50, 30, DIYables_TFT::colorRGB(128, 128, 128));
      display.setTextColor(DIYables_TFT::colorRGB(0, 0, 0));
      display.setTextSize(3);  // Adjust text size as needed
      display.setCursor(130, 390);    // Set cursor position (x, y)
      display.println(String(snakeLength - tailPauseFrames));
    }
  if(!canMoveSnake(xd, yd)) {
    lose();
    return;
  } else {
    uint16_t p = board.moveNumericByDir(getHeadPos(), xd, yd);
    Serial.println(p);
    uint16_t d = 0;
    if(xd == -1 && yd == 0) {
      d = 1;
    } else if (xd == 1 && yd == 0) {
      d = 2;
    } else if (xd == 0 && yd == -1) {
      d = 3;
    } else if (xd == 0 && yd == 1) {
      d = 4;
    }
    board.setVal(getHeadPos(), d);
    board.setVal(p, d + 4);
    drawTile(p);
    drawTile(getHeadPos());
    headPos = p;
    
  }
}
void SnakeGame::drawTile(uint16_t i) {
  int8_t v = board.getVal(i);
  uint16_t x = board.getXAtNumeric(i) * 16;
  uint16_t y = board.getYAtNumeric(i) * 16;
  uint16_t color;
  switch(v) {
    case 1://snake going left
      color = DIYables_TFT::colorRGB(0, 255, 0);
      break;
    case 2://snake going right
      color = DIYables_TFT::colorRGB(0, 255, 0);
      break;
    case 3://snake going up
      color = DIYables_TFT::colorRGB(0, 255, 0);
      break;
    case 4://snake going down
      color = DIYables_TFT::colorRGB(0, 255, 0);
      break;
    case 5://snake head going left
      color = DIYables_TFT::colorRGB(255, 255, 0);
      break;
    case 6://snake head going right
      color = DIYables_TFT::colorRGB(255, 255, 0);
      break;
    case 7://snake head going up
      color = DIYables_TFT::colorRGB(255, 255, 0);
      break;
    case 8://snake head going down
      color = DIYables_TFT::colorRGB(255, 255, 0);
      break;
    case 9://apple
      color = DIYables_TFT::colorRGB(255, 0, 0);
      break;
    default://empty tile
      color = DIYables_TFT::colorRGB(0, 0, 0);
      break;
  }
  display.fillRect(x, y, 16, 16, color);
}
uint16_t SnakeGame::getHeadPos() {
  return headPos;
}
uint16_t SnakeGame::getTailPos() {
  return tailPos;
}
uint16_t SnakeGame::getNextTailPos() {
  uint16_t p = board.getVal(getTailPos());
  uint16_t x = board.getXAtNumeric(getTailPos());
  uint16_t y = board.getYAtNumeric(getTailPos());
  switch(p) {
    case 1:
      return board.getNumericAtPos(x - 1, y);
    case 2:
      return board.getNumericAtPos(x + 1, y);
    case 3:
      return board.getNumericAtPos(x, y - 1);
    case 4:
      return board.getNumericAtPos(x, y + 1);
    default:
      return getTailPos();
  }
}
void SnakeGame::addApple() {
  if(snakeLength == 400) {
    win();
    return;
  }
  bool f = true;
  while(f) {
    uint16_t p = rand() % (board.getXSize() * board.getYSize());
    if(board.getVal(p) == 0) {
      f = false;
      board.setVal(p, 9);
      drawTile(p);
    }
  }
}