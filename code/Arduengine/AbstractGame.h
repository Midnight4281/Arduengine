#include <Arduino.h>
#include "ButtonManager.h"
#include <EEPROM.h>
#include <DIYables_TFT_Shield.h>
#include <SD.h>

#define BUFFPIXEL 20  // Buffer size remains the same
#define SD_CS 10

#ifndef ABSTRACT_GAME_H
#define ABSTRACT_GAME_H
class AbstractGame {
  protected:
    int frameRate;
    const int seed_addr = 0;
    unsigned long seed;
    bool* _isDeletedFlag;
  public:
    AbstractGame(bool* flag) {
      _isDeletedFlag = flag;
      *_isDeletedFlag = false; 
      //name = getName();
    }
    ~AbstractGame() {
        if (_isDeletedFlag != nullptr) {
            *_isDeletedFlag = true; // Alert the system it's gone
        }
    }
    ButtonManager* buttons; //reference this in children to use

    String name;

    DIYables_TFT_ILI9486_Shield display;
    File bmpFile;
    File scFile;
    uint16_t SCREEN_WIDTH;
    uint16_t SCREEN_HEIGHT;
    
    virtual void quitGame() = 0;//run by code before deleting game instance, use for storing highscores/settings
    void exitGame() {//run by code when game ends, do not edit
      quitGame();
      display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
      delete this;
      return;
    }
    virtual void startGame() = 0;
    static String getName() {};
    static String getDisplayName() {};
    void gameSetup(ButtonManager *b, const DIYables_TFT_ILI9486_Shield& d) {
      buttons = b;
      display = d;
      display.setRotation(2);  // Rotate screen 180 degrees

      // After rotation, update screen dimensions
      SCREEN_WIDTH = display.width();
      SCREEN_HEIGHT = display.height();
      if (!SD.begin(SD_CS)) {
        Serial.println("SD card initialization failed!");
        return;
      }
      Serial.println("SD card initialized successfully.");
      startGame();
    }
    void changeSeed() {
      seed = EEPROM.read(seed_addr);
      EEPROM.write(seed_addr, seed+1);
      randomSeed(seed);
    }
    float randomFloatRange(float minVal, float maxVal) {
      return minVal + (random(0, 1000000) / 1000000.0) * (maxVal - minVal);
    }
    virtual void frameTick() = 0;
    int getFrameRate() {return frameRate;}
    uint16_t read16(File &f) {
    uint16_t result;
    result = f.read();
    result |= (f.read() << 8);
    return result;
  }

  uint32_t read32(File &f) {
    uint32_t result;
    result = f.read();
    result |= ((uint32_t)f.read() << 8);
    result |= ((uint32_t)f.read() << 16);
    result |= ((uint32_t)f.read() << 24);
    return result;
  }

  // Function to read a signed 32-bit integer
  int32_t readS32(File &f) {
    int32_t result;
    result = f.read();
    result |= ((uint32_t)f.read() << 8);
    result |= ((uint32_t)f.read() << 16);
    result |= ((uint32_t)f.read() << 24);
    return result;
  }
  void drawPixel(int16_t x, int16_t y, uint8_t xs, uint8_t ys, uint16_t color) {
    for(int k = 0; k < xs; k++) {
      for(int j = 0; j < ys; j++) {
        if ((x + k) < SCREEN_WIDTH && (y + j) < SCREEN_HEIGHT) {
          display.drawPixel(x + k, y + j, color);
        }
      }
    }
  }
  void drawPixelInImage(int16_t x, int16_t y, uint8_t xd, uint8_t yd, uint8_t xs, uint8_t ys, uint16_t color) {
    drawPixel(x + (xd * xs), y + (yd * ys), xs, ys, color);
  }

  // Function to draw BMP from SD card
  void drawBMP(const char *filename, int x, int y) {
    unsigned long startTime = millis();
    uint16_t xScale;

    uint16_t yScale;

    scFile = SD.open(name + "/" + String(filename) + ".sc");
    if (!scFile) {
      Serial.println("Could not find SC " + String(filename));
      xScale = 1;
      yScale = 1;
    } else {
      xScale = read16(scFile);
      yScale = read16(scFile);
    }
    scFile.close();

    bmpFile = SD.open(name + "/" + String(filename) + ".bmp");

    if (!bmpFile) {
      Serial.println("File not found");
      return;
    }

    if (read16(bmpFile) != 0x4D42) {  // Check BMP signature
      Serial.println("Not a BMP file");
      bmpFile.close();
      return;
    }

    // Skip unnecessary BMP header details

    uint32_t fileSize = read32(bmpFile);


    read32(bmpFile); //skip bytes
    

    uint32_t imageOffset = read32(bmpFile);  // Start of image data


    uint32_t dibHeaderSize = read32(bmpFile);  // DIB header size

    // Now read the width and height of the image
    uint32_t bmpWidth = read32(bmpFile);
    int32_t bmpHeight = readS32(bmpFile);  // Read as signed 32-bit integer


    bool topDown = false;  // Flag to check if the image is top-down
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;  // Make height positive for processing
      topDown = true;          // Mark the BMP as top-down
    }

    if (read16(bmpFile) != 1) {  // Planes must be 1
      Serial.println("Invalid BMP file");
      bmpFile.close();
      return;
    }

    uint16_t depth = read16(bmpFile);  // Color depth


    if (depth != 24) {  // Only 24-bit BMP supported
      Serial.println("Only 24-bit BMP is supported");
      bmpFile.close();
      return;
    }

    if (read32(bmpFile) != 0) {  // No compression
      Serial.println("Unsupported BMP compression");
      bmpFile.close();
      return;
    }

    // Move to the start of the image data
    bmpFile.seek(imageOffset);

    uint8_t sdbuffer[3 * BUFFPIXEL];  // Buffer for 20 pixels (3 bytes per pixel)
    uint16_t color;
    uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;  // BMP rows are padded to 4-byte boundaries

    // Adjust x and y if image is larger than screen
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
      Serial.println("Image position out of screen bounds" + SCREEN_WIDTH);
      return;
    }

    uint32_t maxRow = min(bmpHeight, SCREEN_HEIGHT - y);
    uint32_t maxCol = min(bmpWidth, SCREEN_WIDTH - x);

    // Draw the image
    for (uint32_t row = 0; row < maxRow; row++) {
      int32_t rowPos = topDown ? row : bmpHeight - 1 - row;  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + rowPos * rowSize;
      bmpFile.seek(filePosition);  // Move to the correct row

      for (uint32_t col = 0; col < maxCol; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, maxCol - col);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);

            drawPixel(x + (col + i) * xScale, y + row * yScale, xScale, yScale, color);
          }
        }
      }
    }

    bmpFile.close();  // Close file when done
    unsigned long endTime = millis();
  }
  void drawBMPArea(const char *filename, int x, int y, int xAmt, int yAmt) {
    Serial.println("Started drawing bmp area");
    unsigned long startTime = millis();
    uint16_t xScale;

    uint16_t yScale;

    uint16_t xSize;
    uint16_t ySize;

    scFile = SD.open(name + "/" + String(filename) + ".sc");
    if (!scFile) {
      Serial.println("Could not find SC");
      xScale = 1;
      yScale = 1;
    } else {
      xScale = read16(scFile);
      yScale = read16(scFile);
      xSize = read16(scFile);
      ySize = read16(scFile);
    }
    scFile.close();
    Serial.println("Found sc file");
    bmpFile = SD.open(name + "/" + String(filename) + ".bmp");

    if (!bmpFile) {
      Serial.println("File not found");
      return;
    }

    if (read16(bmpFile) != 0x4D42) {  // Check BMP signature
      Serial.println("Not a BMP file");
      bmpFile.close();
      return;
    }


    uint32_t fileSize = read32(bmpFile);

    read32(bmpFile); //skip bytes
    
    uint32_t imageOffset = read32(bmpFile);  // Start of image data

    uint32_t dibHeaderSize = read32(bmpFile);  // DIB header size

    // Now read the width and height of the image
    uint32_t bmpWidth = read32(bmpFile);
    int32_t bmpHeight = readS32(bmpFile);  // Read as signed 32-bit integer


    bool topDown = false;  // Flag to check if the image is top-down
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;  // Make height positive for processing
      topDown = true;          // Mark the BMP as top-down
    }

    if (read16(bmpFile) != 1) {  // Planes must be 1
      Serial.println("Invalid BMP file");
      bmpFile.close();
      return;
    }

    uint16_t depth = read16(bmpFile);  // Color depth

    if (depth != 24) {  // Only 24-bit BMP supported
      Serial.println("Only 24-bit BMP is supported");
      bmpFile.close();
      return;
    }

    if (read32(bmpFile) != 0) {  // No compression
      Serial.println("Unsupported BMP compression");
      bmpFile.close();
      return;
    }

    // Move to the start of the image data
    bmpFile.seek(imageOffset);

    uint8_t sdbuffer[3 * BUFFPIXEL];  // Buffer for 20 pixels (3 bytes per pixel)
    uint16_t color;
    uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;  // BMP rows are padded to 4-byte boundaries

    // Adjust x and y if image is larger than screen
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
      Serial.println("Image position out of screen bounds"  + SCREEN_WIDTH);
      return;
    }

    uint32_t maxRow = min(bmpHeight, SCREEN_HEIGHT - y);
    uint32_t maxCol = min(bmpWidth, SCREEN_WIDTH - x);

    // Draw the image
    for (uint32_t row = 0; row < maxRow; row++) {
      int32_t rowPos = topDown ? row : bmpHeight - 1 - row;  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + rowPos * rowSize;
      bmpFile.seek(filePosition);  // Move to the correct row

      for (uint32_t col = 0; col < maxCol; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, maxCol - col);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);

            // Draw pixel on screen if within bounds
            for(int d = 0; d < xAmt; d++) {
              for(int s = 0; s < yAmt; s++) {
                drawPixel(x + (col + i) * xScale + (d * xSize), y + row * yScale + (s * ySize), xScale, yScale, color);
              }
            }
          }
        }
      }
    }

    bmpFile.close();  // Close file when done
    unsigned long endTime = millis();
  }
  // Function to get BMP image dimensions
  bool getBMPDimensions(const char *filename, uint32_t &width, uint32_t &height) {
    File scFile = SD.open(name + "/" + String(filename) + ".sc");
    File bmpFile = SD.open(name + "/" + String(filename) + ".bmp");
    if (!bmpFile) {
      Serial.println("File not found");
      return false;
    }

    if (read16(bmpFile) != 0x4D42) {  // Check BMP signature
      Serial.println("Not a BMP file");
      bmpFile.close();
      return false;
    }

    read32(bmpFile);  // Skip file size
    read32(bmpFile);
    
    uint32_t imageOffset = read32(bmpFile);  // Start of image data

    uint32_t dibHeaderSize = read32(bmpFile);  // DIB header size

    // Read the width and height of the image
    read32(scFile);
    

    width = read16(scFile);
    height = read16(scFile);

    bmpFile.close();  // Close the file
    scFile.close();
    return true;      // Success
  }
  void drawBorderBMP(const char *filename, int x, int y, uint8_t radius) {
    unsigned long startTime = millis();
    uint16_t xScale;
    uint16_t yScale;

    uint16_t xSize;
    uint16_t ySize;

    scFile = SD.open(name + "/" + String(filename) + ".sc");
    if (!scFile) {
      Serial.println("Could not find SC");
      xScale = 1;
      yScale = 1;
    } else {
      xScale = read16(scFile);
      yScale = read16(scFile);
      xSize = read16(scFile);
      ySize = read16(scFile);
    }

    scFile.close();
    uint8_t radX = max(radius / xScale, 1);
    uint8_t radY = max(radius / yScale, 1);

    bmpFile = SD.open(name + "/" + String(filename) + ".bmp");
    if (!bmpFile) {
      Serial.println("File not found");
      return;
    }

    if (read16(bmpFile) != 0x4D42) {  // Check BMP signature
      Serial.println("Not a BMP file");
      bmpFile.close();
      return;
    }



    uint32_t fileSize = read32(bmpFile);

    read32(bmpFile);                         // Reserved bytes (skip)
    uint32_t imageOffset = read32(bmpFile);  // Start of image data

    uint32_t dibHeaderSize = read32(bmpFile);  // DIB header size

    // Now read the width and height of the image
    uint32_t bmpWidth = read32(bmpFile);
    int32_t bmpHeight = readS32(bmpFile);  // Read as signed 32-bit integer
    

    bool topDown = false;  // Flag to check if the image is top-down
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;  // Make height positive for processing
      topDown = true;          // Mark the BMP as top-down
    }

    if (read16(bmpFile) != 1) {  // Planes must be 1
      Serial.println("Invalid BMP file");
      bmpFile.close();
      return;
    }

    uint16_t depth = read16(bmpFile);  // Color depth

    if (depth != 24) {  // Only 24-bit BMP supported
      Serial.println("Only 24-bit BMP is supported");
      bmpFile.close();
      return;
    }

    if (read32(bmpFile) != 0) {  // No compression
      Serial.println("Unsupported BMP compression");
      bmpFile.close();
      return;
    }
    

    // Move to the start of the image data
    bmpFile.seek(imageOffset);

    uint8_t sdbuffer[3 * BUFFPIXEL];  // Buffer for 20 pixels (3 bytes per pixel)
    uint16_t color;
    uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;  // BMP rows are padded to 4-byte boundaries
    
    if(bmpHeight == 1 && bmpWidth == 1) {//draw 1x1 bmp borders effeciently; as the entire image is one color, we can read that color and draw rectangles
      bmpFile.read(sdbuffer, 3);
      uint8_t b = sdbuffer[0];
      uint8_t g = sdbuffer[1];
      uint8_t r = sdbuffer[2];
      color = DIYables_TFT::colorRGB(r, g, b);
      display.fillRect(x, y, xScale, radius, color);
      display.fillRect(x, y + radius, radius, yScale, color);
      display.fillRect(x + xScale - radius, y + radius, radius, yScale, color);
      display.fillRect(x, y + yScale - radius, xScale, radius, color);
      return;
    }

    // Adjust x and y if image is larger than screen
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
      Serial.println("Image position out of screen bounds"  + SCREEN_WIDTH);
      return;
    }

    uint32_t maxRow = min(bmpHeight, SCREEN_HEIGHT - y);
    uint32_t maxCol = min(bmpWidth, SCREEN_WIDTH - x);

    // Draw the top row
    for (uint32_t row = 0; row < radY; row++) {
      int32_t rowPos = topDown ? row : bmpHeight - 1 - row;  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + rowPos * rowSize;
      bmpFile.seek(filePosition);  // Move to the correct row
      
      for (uint32_t col = 0; col < maxCol; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, maxCol - col);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);
            // Draw pixel on screen if within bounds
            //drawPixel(x + (col + i) * xScale, y + row * yScale, xScale, yScale, color);
            drawPixelInImage(x, y, col + i, row, xScale, yScale, color);
          }
        }
      }
    }
    //draw the sides
    for (uint32_t row = 0; row < maxRow - radY * 2; row++) {
      int32_t rowPos = topDown ? row + radY : bmpHeight - 1 - row - radY;  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + rowPos * rowSize;
      bmpFile.seek(filePosition);  // Move to the correct row
      
      for (uint32_t col = 0; col < radX; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, radX);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);
            // Draw pixel on screen if within bounds
            //drawPixel(x + (col + i) * xScale, y + (row) * yScale, xScale, yScale, color);
            drawPixelInImage(x, y, col + i, row + radY, xScale, yScale, color);
          }
        }
      }
      filePosition = imageOffset + rowPos * rowSize - 3 * radX;
      bmpFile.seek(filePosition);
      for (uint32_t col = 0; col < radX; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, radX);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);
            // Draw pixel on screen if within bounds
            //drawPixel(x + (col + i) * xScale, y + (row) * yScale, xScale, yScale, color);
            drawPixelInImage(x, y, col + i + bmpWidth - radX, row + radY, xScale, yScale, color);
          }
        }
      }
    }
    //draw the bottom row
    for (uint32_t row = 0; row < radY; row++) {
      int32_t rowPos = topDown ? (row + maxRow - radY): (row + maxRow - radY);  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + rowPos * rowSize;
      bmpFile.seek(filePosition);  // Move to the correct row
      
      for (uint32_t col = 0; col < maxCol; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, maxCol - col);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          uint8_t b = sdbuffer[i * 3];
          uint8_t g = sdbuffer[i * 3 + 1];
          uint8_t r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);
            // Draw pixel on screen if within bounds
            //drawPixel(x + (col + i) * xScale, y + row * yScale, xScale, yScale, color);
            drawPixelInImage(x, y, col + i, row + bmpHeight - radY, xScale, yScale, color);
          }
        }
      }
    }
    
    
    bmpFile.close();  // Close file when done
    unsigned long endTime = millis();
    //Serial.println("Finished drawing border BMP" + String(filename) + " in " + String(endTime - startTime) + "ms");
  }
};
#endif