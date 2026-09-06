#include "Minesweeper.h"
#include <DIYables_TFT_Shield.h>
void Minesweeper::frameTick() {
  int16_t x = board.getXAtNumeric(currentPos);
  int16_t y = board.getYAtNumeric(currentPos);
  Serial.println("Started frame tick");
  if(buttons->getButtonWentDown("L")) {
    if(board.movementIsOnBoard(x, y, -1, 0)) {
      drawTile(currentPos, false, true, false);
      currentPos = currentPos - 1;
      drawTile(currentPos, false, true, true);
    }
  } else if(buttons->getButtonWentDown("R")) {
    if(board.movementIsOnBoard(x, y, 1, 0)) {
      drawTile(currentPos, false, true, false);
      currentPos = currentPos + 1;
      drawTile(currentPos, false, true, true);
    }
  } else if(buttons->getButtonWentDown("U")) {
    if(board.movementIsOnBoard(x, y, 0, -1)) {
      drawTile(currentPos, false, true, false);
      currentPos = currentPos - board.getXSize();
      drawTile(currentPos, false, true, true);
    }
  } else if(buttons->getButtonWentDown("D")) {
    if(board.movementIsOnBoard(x, y, 0, 1)) {
      drawTile(currentPos, false, true, false);
      currentPos = currentPos + board.getXSize();
      drawTile(currentPos, false, true, true);
    }
  }
  if(buttons->getButtonWentDown("A")) {
    clickTile(currentPos, false, true);
  }
  if(buttons->getButtonWentDown("B")) {
    clickTile(currentPos, true, true);
  }
}
void Minesweeper::clickTile(int16_t tile, bool isFlagClick, bool isHuman) {
  if(!isFlagClick) {
    if(board.getVal(tile) < 10) {
      board.setVal(tile, board.getVal(tile) + 10);
      if(isHuman) {
        drawTile(tile, true, true, true);
      } else {
        drawTile(tile, true, false, false);
      }
      revealed++;
      if (board.getVal(tile) == 19) {
        //lose
        display.setTextColor(DIYables_TFT::colorRGB(255, 255, 255));
        display.setTextSize(3);  // Adjust text size as needed
        display.setCursor(20, 325);    // Set cursor position (x, y)
        display.println("YOU DIED :(");
        delay(2000);
        reset();
      } else if (board.getVal(tile) == 10) {
        //zero spreading
        for(uint8_t x = 0; x < 3; x++) {
          for(uint8_t y = 0; y < 3; y++) {
            int8_t xd = board.getXAtNumeric(tile) + x - 1;
            int8_t yd = board.getYAtNumeric(tile) + y - 1;
            if(board.isOnBoard(xd, yd)) {
              clickTile(board.getNumericAtPos(xd, yd), false, false);
            }
          } 
        }
      } else {
        if(revealed == xSize * ySize - minesToMake) {
          //win
          display.setTextColor(DIYables_TFT::colorRGB(255, 255, 255));
          display.setTextSize(3);  // Adjust text size as needed
          display.setCursor(20, 325);    // Set cursor position (x, y)
          display.println("YOU WON :)");
          delay(2000);
          reset();
        }
      }
    }
  } else {
    if(board.getVal(tile) < 10) {
      board.setVal(tile, board.getVal(tile) + 30);
      flagged++;
      drawTile(tile, true, true, true);
      display.fillRect(20, 390, 300, 25, DIYables_TFT::colorRGB(0, 0, 0));
      display.setTextColor(DIYables_TFT::colorRGB(255, 255, 255));
      display.setTextSize(3);  // Adjust text size as needed
      display.setCursor(20, 390);    // Set cursor position (x, y)
      display.println(minesToMake - flagged);
    } else if (board.getVal(tile) >= 30) {
      board.setVal(tile, board.getVal(tile) - 30);
      drawTile(tile, true, true, true);
      flagged--;
      display.fillRect(20, 390, 300, 25, DIYables_TFT::colorRGB(0, 0, 0));
      display.setTextColor(DIYables_TFT::colorRGB(255, 255, 255));
      display.setTextSize(3);  // Adjust text size as needed
      display.setCursor(20, 390);    // Set cursor position (x, y)
      display.println(minesToMake - flagged);
    }
    
  }
}
void Minesweeper::drawTile(int16_t tile, bool drawMiddle, bool drawBorder, bool borderOn) {
  int16_t x = board.getXAtNumeric(tile);
  int16_t y = board.getYAtNumeric(tile);
  if(board.isOnBoard(x, y)) {
    int16_t xDraw = x * drawScale;
    int16_t yDraw = y * drawScale;
    int8_t v = board.getVal(tile);
    //tiles are stored as: 0-8 (hidden tile), 9 (hidden mine), 10-18 (revealed tile), 19 (revealed mine), 30-39 (flagged)
    if(drawMiddle) {
      //draw the center of the tile
      if(v >= 0) {
        if(v >= 30) {
          drawBMP("flag", xDraw, yDraw);
        } else if(v > 18) {
          drawBMP("mine", xDraw, yDraw);
        } else if (v > 9){
          drawBMP(("tile_" + String(v - 10)).c_str(), xDraw, yDraw);
        } else {
          drawBMP("hidden", xDraw, yDraw);
        }
      }
    }
    if(drawBorder) {
      if(borderOn) {
        //draw the border of a selection
        drawBorderBMP("border", xDraw, yDraw, 4);
      } else {
        //draw the border of the respective empty sprite
        if(v >= 0) {
          if(v >= 30) {
            drawBorderBMP("flag", xDraw, yDraw, 4);
          } else if(v > 18) {
            drawBorderBMP("mine", xDraw, yDraw, 4);
          } else if (v >= 10){
            if(v == 10) {
              drawBorderBMP("tile_0", xDraw, yDraw, 4);
            } else {
              drawBorderBMP(("tile_" + String(v - 10)).c_str(), xDraw, yDraw, 4);
            }
          } else {
            drawBorderBMP("hidden", xDraw, yDraw, 4);
          }
        }
      }
    }
  }
}
void Minesweeper::reset() {
  display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
  changeSeed();
  currentPos = 0;
  revealed = 0;
  flagged = 0;
  board.reset(xSize, ySize);
  uint16_t m = 0;
  while (m < minesToMake) {
    uint16_t r = rand() % (xSize * ySize);
    if(board.getVal(r) == 0) {
      board.setVal(r, 9);
      m++;
    }
  }
  for(uint16_t i = 0; i < xSize * ySize; i++) {
    if(board.getVal(i) != 9) {
      uint8_t f = 0;
      for(uint8_t x = 0; x < 3; x++) {
        for(uint8_t y = 0; y < 3; y++) {
          int8_t xd = board.getXAtNumeric(i) + x - 1;
          int8_t yd = board.getYAtNumeric(i) + y - 1;
          if(board.isOnBoard(xd, yd)) {
            if(board.getVal(board.getNumericAtPos(xd, yd)) == 9) {
              f++;
            }
          }
        } 
      }
      board.setVal(i, f);
    }
  }
  drawBMPArea("hidden", 0, 0, xSize, ySize);
  
  drawTile(currentPos, false, true, true);
  display.fillRect(20, 390, 300, 25, DIYables_TFT::colorRGB(0, 0, 0));
  display.setTextColor(DIYables_TFT::colorRGB(255, 255, 255));
  display.setTextSize(3);  // Adjust text size as needed
  display.setCursor(20, 390);    // Set cursor position (x, y)
  display.println(minesToMake - flagged);
}