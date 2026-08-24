#include "TestGame.h"
#include <DIYables_TFT_Shield.h>
void TestGame::frameTick() {
  if(buttons->getButtonWentDown("L")) {
    if(selectedTile % 3 > 0) {
      selectedTile -= 1;
      drawTile(selectedTile, true, false);
      drawTile(selectedTile + 1, true, false);
    }
  }
  if(buttons->getButtonWentDown("R")) {
    if(selectedTile % 3 < 2) {
      selectedTile += 1;
      drawTile(selectedTile, true, false);
      drawTile(selectedTile - 1, true, false);
    }
  }
  if(buttons->getButtonWentDown("U")) {
    Serial.println(floor(selectedTile / 3));
    if(floor(selectedTile / 3) > 0) {
      selectedTile -= 3;
      drawTile(selectedTile, true, false);
      drawTile(selectedTile + 3, true, false);
    }
  }
  if(buttons->getButtonWentDown("D")) {
    if(floor(selectedTile / 3) < 2) {
      selectedTile += 3;
      drawTile(selectedTile, true, false);
      drawTile(selectedTile - 3, true, false);
    }
  }
  //Serial.println(selectedTile);
  if(buttons->getButtonWentDown("A")) {
    if(board[selectedTile] == 0) {
      if(turns % 2 == 0) {
        board[selectedTile] = 1;
      } else {
        board[selectedTile] = 4;
      }
      display.setTextColor(DIYables_TFT::colorRGB(0, 0, 0));
      display.setTextSize(3);  // Adjust text size as needed
      display.setCursor(20, 325);    // Set cursor position (x, y)
      drawTile(selectedTile, true, true);
      for(int i = 0; i < 8; i++) {
        int s = getScore(i);
        
        if(s == 12) {
          display.println("X WINS");
          delay(1000);
          reset();
        } else if (s == 3) {
          display.println("O WINS");
          delay(1000);
          reset();
        }
      }
      turns++;
      if(turns > 9) {
        display.println("TIE");
        delay(1000);
        reset();
      }
    }
  }
}
void TestGame::reset() {
  display.fillScreen(DIYables_TFT::colorRGB(255, 255, 255));
  selectedTile = 4;
  for(int i = 0; i < 9; i++) {
    board[i] = 0;
    //drawTile(i, true, false);
  }
  drawBMPArea("empty", 10, 10, 3, 3);
  turns = 0;
}
uint8_t TestGame::getTile(uint8_t x, uint8_t y) {
  return board[x + y * 3];
}
void TestGame::setTile(uint8_t x, uint8_t y, uint8_t v) {
  board[x + y * 3] = v;
}
uint8_t TestGame::getScore(uint8_t l) {
  int t = 0;
  for(int i = 0; i < 3; i++) {
    t += board[winRows[l][i]];//loop through all positions in the row; add the value from the board
  }
  return t;
}
void TestGame::drawTile(uint8_t t, bool checkBorder, bool checkMark) {
  int xDraw = t % 3 * 100 + 10;
  int yDraw = floor(t / 3) * 100 + 10;
  int v = board[t];

  //display.drawCircle(xDraw + 50, yDraw + 50, 40, DIYables_TFT::colorRGB(255, 255, 255));
  //display.drawLine(xDraw + 10, yDraw + 10, xDraw + 90, yDraw + 90, DIYables_TFT::colorRGB(255, 255, 255));
  //display.drawLine(xDraw + 10, yDraw + 90, xDraw + 90, yDraw + 10, DIYables_TFT::colorRGB(255, 255, 255));
  //display.drawRect(xDraw + 1, yDraw + 1, 98, 98, DIYables_TFT::colorRGB(255, 255, 255));

  //display.drawRect(xDraw, yDraw, 100, 100, DIYables_TFT::colorRGB(0, 0, 0));
  if(checkMark) {
    if(v == 1) {
      drawBMP("o", xDraw + 10, yDraw + 10);
    } else if(v == 4) {
      drawBMP("x", xDraw + 10, yDraw + 10);
    } else {
      drawBMP("empty", xDraw, yDraw);
    }
  }
  if(checkBorder) {
    if(t == selectedTile) {
      drawBorderBMP("border", xDraw, yDraw, 10);
    } else {
      drawBorderBMP("empty", xDraw, yDraw, 10);
    }
  }
}