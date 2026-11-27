#include <Arduino.h>
#include "ButtonManager.h"
#include <EEPROM.h>
#include <DIYables_TFT_Shield.h>
#include <SD.h>
#include <ArduinoJson.h>

#define BUFFPIXEL 20  // Buffer size remains the same
#define SD_CS 10

#ifndef ABSTRACT_GAME_H
#define ABSTRACT_GAME_H
class AbstractGame {
  protected:
    uint8_t frameRate;
    const int seed_addr = 0;
    unsigned long seed;
    bool* _isDeletedFlag;

    File bmpFile; //initialize the BMP file for reading & pipelining
    uint32_t fileSize, imageOffset, dibHeaderSize, bmpWidth, bmpHeight; //all the settings for reading the BMP, stored & initialized here for same-image pipelining
    bool topDown; //used by BMP drawing algorithm
    uint16_t planes, depth; // used during BMP verification; otherwise useless
    uint8_t sdbuffer[3 * BUFFPIXEL];  // Buffer for 20 pixels (3 bytes per pixel)
    uint16_t color; //allocating the bytes for the color used during drawing here for improved performance

    uint8_t r, g, b; //r,g,b values for color; allocated here to save power at runtime

    File scFile;
    File saveFile;
    uint16_t SCREEN_WIDTH;
    uint16_t SCREEN_HEIGHT;
  public:
    AbstractGame(bool* flag) {
      _isDeletedFlag = flag;
      *_isDeletedFlag = false;
      name = getName();
    }
    ~AbstractGame() {
        if (_isDeletedFlag != nullptr) {
            *_isDeletedFlag = true; // Alert the system it's gone
        }
    }
    ButtonManager* buttons; //reference this in children to use

    String name;

    DIYables_TFT_ILI9486_Shield display;

    virtual void quitGame() = 0;//run by code before ending game instance, use for storing highscores/settings
    void exitGame() {//run by code when game ends, DO NOT EDIT
      quitGame();
      display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
      delete this;
      return;
    }
    //must be overwritten by children, runs on initialization of the game after the constructor
    virtual void startGame() = 0;
    //make sure you override this; otherwise it the game will not find assets properly. All assets should be stored under [gameName/assetName] on the SD card
    //note that when reading from the SD card, the arduino will fail to find any files with any folder or file longer than 8 characters, or an extension longer than 3 characters
    virtual String getName() {return "Default";}
    //used to display the name; can be anything
    static String getDisplayName() {};
    //DO NOT EDIT; USED TO STARTUP GAME SYSTEMS
    void gameSetup(ButtonManager *b, const DIYables_TFT_ILI9486_Shield& d) {
      name = getName();
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
    //Function to psuedo-randomly reseed the seed based off a value stored in EEPROM
    void changeSeed() {
      seed = EEPROM.read(seed_addr);
      EEPROM.write(seed_addr, seed+1);
      randomSeed(seed);
    }
    //Function to set the random seed to a known number
    void setSeed(unsigned long val) {
      seed = val;
      randomSeed(seed);
    }
    //generates a random integer between min and max (min inclusive, max exclusive) using the seed (i.e. randInt(5,10) will return 5, 6, 7, 8, or 9 randomly)
    int16_t randInt(int16_t min, int16_t max) {
      return min + (rand() % (max - min));
    }
    //generates a random integer between 0 and max (max inclusive) using the seed (i.e randInt(2) will return 0, 1, or 2 randomly)
    int16_t randInt(int16_t max) {
      return randInt(0, max + 1);
    }
    void clearScreen() {
      display.fillScreen(DIYables_TFT::colorRGB(0, 0, 0));
    }
    float randomFloatRange(float minVal, float maxVal) {
      return minVal + (random(0, 1000000) / 1000000.0) * (maxVal - minVal);
    }
    virtual void frameTick() = 0; //override this in children for the central refresh logic (this runs once per frame)
    uint8_t getFrameRate() {// Used to get the active framerate of the game
      if(frameRate != 0) {
        return frameRate;
      } else {
        return 20;
      }
    }
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
    display.fillRect(x, y, xs, ys, color);
  }
  void drawPixelInImage(int16_t x, int16_t y, uint8_t xd, uint8_t yd, uint8_t xs, uint8_t ys, uint16_t color) {
    drawPixel(x + (xd * xs), y + (yd * ys), xs, ys, color);
  }
  /*
  Used to read information from the save file at any position.
  If you try to read to a non-existent value, it will return 0
  Only supports unsigned 16-bit integers*/
  uint16_t readSaveFile(uint16_t pos, bool openFile = true, bool closeFile = true, String customName = "") {
    if(openFile) {
      saveFile = SD.open(name + "/" + (customName == "" ? name : customName) + ".sv");
    }
    if(!saveFile) {
      return 0;
    }
    unsigned long fileLength = saveFile.size();
    if(fileLength <= pos * 2) {
      if(closeFile) {
        saveFile.close();
      }
      return 0;
    }
    saveFile.seek(pos * 2);
    uint16_t toReturn = makeWord(saveFile.read(), saveFile.read());
    if(closeFile) {
      saveFile.close();
    }
    return(toReturn);
  }
  //helper used to edit/replace existing data in the save file; not recommended to use manually
  void editSaveFile(uint16_t pos, uint16_t val, String n, bool openFile = true, bool closeFile = true) {
    saveFile = SD.open(n, O_RDWR);
    if(!saveFile) {
      return;
    }
    saveFile.seek(pos * 2);
    saveFile.write(highByte(val));
    saveFile.write(lowByte(val));
    saveFile.close();
  }
  //helper used to append data to the end of the save file; not recommended to use manually
  void appendSaveFile(uint16_t val, String n, bool openFile = true, bool closeFile = true) {
    if(openFile) {
      saveFile = SD.open(n, FILE_WRITE);
    }
    if(!saveFile) {
      return;
    }
    saveFile.write(highByte(val));
    saveFile.write(lowByte(val));
    if(closeFile) {
      saveFile.close();
    }
  }
  /*
  Function to write data to the save file at any position.
  If you try to write to a position past the end of the current save, it will add to the end of the file, filling in zeros to fix the empty space.
  Note that currently this only supports unsigned 16-bit integers. (0-65,535)*/
  void writeToSave(uint16_t pos, uint16_t val, String customName = "") {
    saveFile = SD.open(name + "/" + (customName == "" ? name : customName) + ".sv", FILE_WRITE);
    unsigned long fileLength = saveFile.size() / 2;
    saveFile.close();
    if(fileLength < pos) {
      for(int i = 0; i < pos - fileLength; i++) {
        appendSaveFile(0, (customName == "" ? name : customName), i == 0, i == pos - fileLength - 1);
      }
    }
    editSaveFile(pos, val, (customName == "" ? name : customName));
  }
  //WIP
  JsonDocument readTextFromJson(const char *filename) {
    File configFile = SD.open(name + "/jsonStorage/" + filename + ".json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    return doc;
  }
  /*Function to draw a section of a BMP image; used by most of the drawing processes in this engine
    Filename: name of the file, automatically sorts for correct folder
    x: x position (pixels)
    y: y position (pixels)
    sx: start x; the first X position on the BMP to draw from (i.e. 0 draws starting at the first column, 9 will draw starting at the 10th)
    sy: start y; same idea as sx, but for vertical
    tx: target x; the last X position to draw; uses index-based systems, but automatically accounts for overflow
    ty: target y; same idea as tx, but for verticals
    openFile: do you have to open & initialize a different image than the last drawing process?
    closeFile: should you close the file at the end of the process to avoid wasting memory?
  */
  void drawBMPSector(const char *filename, int x, int y, uint16_t sx, uint16_t sy, uint16_t tx, uint16_t ty, bool openFile = true, bool closeFile = true) {
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
    if(openFile) {
      verifyBMP(filename, true, false);
    } else {
      bmpFile.seek(0);
      read16(bmpFile);
    }

    // Move to the start of the image data
    bmpFile.seek(imageOffset);

    uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;  // BMP rows are padded to 4-byte boundaries

    // Adjust x and y if image is larger than screen
    if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
      Serial.println("Image position out of screen bounds" + SCREEN_WIDTH);
      return;
    }

    uint32_t maxRow = min(ty - sy, SCREEN_HEIGHT - y);
    uint32_t maxCol = min(tx - sx, SCREEN_WIDTH - x);

    // Draw the image
    for (uint32_t row = 0; row < maxRow; row++) {
      int32_t rowPos = topDown ? row + sy : bmpHeight - 1 - row - sy;  // Adjust for top-down BMPs

      uint32_t filePosition = imageOffset + (rowPos) * rowSize + sx * 3;
      bmpFile.seek(filePosition);  // Move to the correct row

      for (uint32_t col = 0; col < maxCol; col += BUFFPIXEL) {
        uint32_t pixelsToRead = min(BUFFPIXEL, maxCol - col);  // Avoid reading beyond row width
        bmpFile.read(sdbuffer, 3 * pixelsToRead);              // Read multiple pixels at once

        for (uint32_t i = 0; i < pixelsToRead; i++) {
          b = sdbuffer[i * 3];
          g = sdbuffer[i * 3 + 1];
          r = sdbuffer[i * 3 + 2];
          if(r != 1 && g != 1 && b != 1) {
            color = DIYables_TFT::colorRGB(r, g, b);
            drawPixel(x + (col + i + sx) * xScale, y + (row + sy) * yScale, xScale, yScale, color);
          }
        }
      }
    }
    if(closeFile) {
      bmpFile.close();  // Close file when done
    }
    unsigned long endTime = millis();
  }

  // Function to draw BMP from SD card
  void drawBMP(const char *filename, int x, int y) {
    if (verifyBMP(filename, true, false)) {
      drawBMPSector(filename, x, y, 0, 0, bmpWidth, bmpHeight, false, true);
    }
  }
  /*Function to draw a grid of the same BMP images; uses slightly different algorithm than drawBMPsector to draw the same image at multiple points simultaneously
    Filename: name of the file, automatically sorts for correct folder
    x: x position (pixels)
    y: y position (pixels)
    xAmt: number of columns to draw
    yAmt: number of rows to draw
    openFile: do you have to open & initialize a different image than the last drawing process?
    closeFile: should you close the file at the end of the process to avoid wasting memory?
  */
  void drawBMPArea(const char *filename, int x, int y, int xAmt, int yAmt, bool openFile = true, bool closeFile = true) {
    uint16_t xScale, yScale, xSize, ySize;
    scFile = SD.open(name + "/" + String(filename) + ".sc");
    if (!scFile) {
      Serial.println("Could not find SC " + String(filename));
      xScale = 1;
      yScale = 1;
    } else {
      xScale = read16(scFile);
      yScale = read16(scFile);
      xSize = read16(scFile);
      ySize = read16(scFile);
    }
    scFile.close();
    if(openFile) {
      verifyBMP(filename, true, false);
    } else {
      bmpFile.seek(0);
      read16(bmpFile);
    }

    // Move to the start of the image data
    bmpFile.seek(imageOffset);

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
          b = sdbuffer[i * 3];
          g = sdbuffer[i * 3 + 1];
          r = sdbuffer[i * 3 + 2];
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
    if(closeFile) {
      bmpFile.close();  // Close file when done
    }
  }
  // Function to get BMP image dimensions and settings; stores them in all of the global variables declared earlier
  bool verifyBMP(const char *filename, bool openFile = true, bool closeFile = true) {
    if(openFile) {
      bmpFile = SD.open(name + "/" + String(filename) + ".bmp");
      if (!bmpFile) {
        Serial.println("File not found");
        return false;
      }
      if (read16(bmpFile) != 0x4D42) {  // Check BMP signature
        Serial.println("Not a BMP file");
        bmpFile.close();
        return false;
      }
    } else {
      bmpFile.seek(0);
      read16(bmpFile);
    }

    // Skip unnecessary BMP header details
    fileSize = read32(bmpFile);
    read32(bmpFile); //skip bytes
    imageOffset = read32(bmpFile);  // Start of image data

    dibHeaderSize = read32(bmpFile);  // DIB header size

    // Now read the width and height of the image
    bmpWidth = read32(bmpFile);
    bmpHeight = readS32(bmpFile);  // Read as signed 32-bit integer

    topDown = false;  // Flag to check if the image is top-down
    if (bmpHeight < 0) {
      bmpHeight = -bmpHeight;  // Make height positive for processing
      topDown = true;          // Mark the BMP as top-down
    }
    planes = read16(bmpFile);
    //Serial.println("Planes: " + String(planes));
    if (planes != 1) {  // Planes must be 1
      Serial.println("Invalid BMP file");
      bmpFile.close();
      return false;
    }
    depth = read16(bmpFile);  // Color depth
    if (depth != 24) {  // Only 24-bit BMP supported
      Serial.println("Only 24-bit BMP is supported");
      bmpFile.close();
      return false;
    }
    if (read32(bmpFile) != 0) {  // No compression
      Serial.println("Unsupported BMP compression");
      bmpFile.close();
      return false;
    }
    if(closeFile) {
      bmpFile.close();  // Close the file
    }
    return true;
  }
  bool getSCDimensions(const char *filename, uint16_t &xScale, uint16_t &yScale) {
    scFile = SD.open(name + "/" + String(filename) + ".sc");
    if (!scFile) {
      Serial.println("Could not find SC");
      xScale = 1;
      yScale = 1;
    } else {
      xScale = read16(scFile);
      yScale = read16(scFile);
    }
    scFile.close(); // Close the file
    return true;      // Success
  }
  /*Filename: name of file, automatically sorts for the correct folder
  x: x position
  y: y position
  radius: how many pixels in to search
  borderOutsideBorderColorConstant: draws the entire border on the first color read*/
  void drawBorderBMP(const char *filename, int x, int y, uint8_t radius, bool knownOutsideBorderColorConstant = false) {
    unsigned long startTime = millis();
    uint16_t xScale, yScale;
    uint16_t xSize, ySize;

    getSCDimensions(filename, xScale, yScale);

    uint8_t radX = max(radius / xScale, 1);
    uint8_t radY = max(radius / yScale, 1);
    if(verifyBMP(filename, true, false)) {
      if(bmpHeight == 1 && bmpWidth == 1 || knownOutsideBorderColorConstant) {//draw 1x1 bmp borders effeciently; as the entire image is one color, we can read that color and draw rectangles
        bmpFile.seek(imageOffset);

        uint8_t sdbuffer[3 * BUFFPIXEL];  // Buffer for 20 pixels (3 bytes per pixel)
        uint16_t color;
        uint32_t rowSize = (bmpWidth * 3 + 3) & ~3;  // BMP rows are padded to 4-byte boundaries
        bmpFile.read(sdbuffer, 3);
        b = sdbuffer[0];
        g = sdbuffer[1];
        r = sdbuffer[2];
        color = DIYables_TFT::colorRGB(r, g, b);
        display.fillRect(x, y, xScale * bmpWidth, radius, color);
        display.fillRect(x, y + radius, radius, yScale * bmpHeight - radius * 2, color);
        display.fillRect(x + xScale * bmpWidth - radius, y + radius, radius, yScale * bmpHeight - radius * 2, color);
        display.fillRect(x, y + yScale * bmpHeight - radius, xScale * bmpWidth, radius, color);
        return;
      } else {//standard, detailed border drawing
        // Adjust x and y if image is larger than screen
        if (x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
          Serial.println("Image position out of screen bounds"  + SCREEN_WIDTH);
          return;
        }

        uint32_t maxRow = min(bmpHeight, SCREEN_HEIGHT - y);
        uint32_t maxCol = min(bmpWidth, SCREEN_WIDTH - x);
        Serial.println("Width: " + String(bmpWidth));
        Serial.println("Height: " + String(bmpHeight));
        Serial.println("RadX: " + String(radX));
        Serial.println("RadY: " + String(radY));
        drawBMPSector(filename, x, y, 0, 0, bmpWidth, radY, false, false);
        drawBMPSector(filename, x, y, 0, radY, radX, bmpHeight - radY, false, false);
        drawBMPSector(filename, x, y, bmpWidth - radX, radY, bmpWidth, bmpHeight - radY, false, false);
        drawBMPSector(filename, x, y, 0, bmpHeight - radY, bmpWidth, bmpHeight, false, false);
        
        bmpFile.close();  // Close file when done
      }
    }
  }
};
#endif