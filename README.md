CREDITS
All credits for engine belong to Midnight4281 on GitHub, as well as the included Minesweeper and Tic-Tac-Toe

REQUIRED HARDWARE
  -Arduino Uno R4 Wifi
  -PCF8574 I2C communicator 
    https://www.amazon.com/Comimark-PCF8574-PCF8574T-Expander-Raspberry/dp/B07X3KWQZ7
  -DIYables TFT Shield 3.5 inch non-touch
    https://diyables.io/products/3.5-inch-color-tft-lcd-display-screen-module-320x480-non-touch-for-arduino-uno-and-mega-ili9486-8-bit-parallel-interface
  -Assorted wires

HARDWARE SETUP
  Arduino Uno R4 -> DIYables screen
    RESET -> LCD_RST
      DO NOT USE A4 FOR LCD_RST: WILL CAUSE THE SCREEN TO BE CONSTANTLY RESTARTED
    A3 -> LCD_CS
    A2 -> LCD_RS
    A1 -> LCD_WR
    A0 -> LCD_RD
    GND -> GND
    5V -> 5V
    3.3V -> 3V3
    2 -> LCD_02
    3 -> LCD_03
    4 -> LCD_04
    5 -> LCD_05
    6 -> LCD_06
    7 -> LCD_07
    8 -> LCD_D0
    9 -> LCD_D1
    10 -> SD_SS
    11 -> SD_DI
    12 -> SD_DO
    13 -> SD_SCK
  Arduino Uno R4 -> PCF8574
    3.3 -> VCC
    GND -> GND
    SDA -> SDA
    SCL -> SCL
  PCF8574
    P0-P7 should be wired to your buttons
    Then, bring the buttons back to GND
    (e.g. P0 -> Pushbutton -> GND)

IMPORTANT HARDWARE NOTES
  Screen is 320x480, and in portrait. (0,0) refers to the top left, and (320, 480) refers to the bottom right

STARTING THE ENGINE
  Using the Arduino IDE (or your preferred IDE with Arduino compatibility), create a project. Then, copy all files within the "code" folder into the project.
  After uploading the all files to the Arduino, the game should run. To change the game run, change line 30 of Arduengine.ino to be your game class

  If your buttons do not align with the default order, you can change the linked button in the ButtonManager class.
  P7 on the PCF8574 accounts for keyCodes[0], P6 for keyCodes[1], and so on
  The character for the keycode can also be changed, but it cannot be more than one character
  
CREATING YOUR OWN GAME
  Again in your IDE, create a c++ file that extends AbstractGame.
  I will not being going over c++ syntax, see https://www.w3schools.com/cpp/cpp_inheritance.asp
  Or look through TestGame.h and TestGame.cpp (these are actually tic-tac-toe, I'm just too lazy to rename them)
  
  Important note: the screen used does not work well for full-screen refreshes, so any drawing should be on a need-to-draw basis instead of a full refresh every frame

  Useful inherited functions from AbstractGame
    *REQUIRED quitGame(): currently unused, but will be used as a place to save game data (e.g. highscores) when exiting
    *REQUIRED startGame(): used when game is initialized, use for setup
    *REQUIRED frameTick(): runs every frame (default 20fps), used for main game loop
    *REQUIRED constructor: ensure you have a constructor for the game. You can change the name of the game in here with name = "newName";
    changeSeed(): reseeds the rand() function to a random seed
    drawBMP(filename, x, y): draws a BMP from the SD card at the x and y position
    drawBMPArea(filename, x, y, xAmt, yAmt): draws a grid of identical images at (x, y) with xAmt and yAmt being how many rows and columns
      Faster than using drawBMP repeatedly
    drawBorderBMP(filename, x, y, radius): draws the edge of a BMP at (x, y). The edge is as thick as the radius parameter in screen-space

  Useful inherited variables
    buttons: used for player input, see the ButtonManager class for all functions
    display: used for drawing, can be used for directly drawing pixels

  Useful non-inherited classes
    Board: stores up to 400 unsigned 8-bit integers, and has 2d coordinate logic inbuilt, x and y coords start at 0
      reset(xSize, ySize): resets the entire board to 0, and sets the xSize and ySize for coordinate logic
      getYAtNumeric(i): returns the y-coordinate of a numeric pos (e.g. on a 10x10 board, 15 will return 1)
      getXAtNumeric(i): returns the x-coordinate of a numeric pos (e.g. on a 10x10 board, 15 will return 5)
      getNumericAtPos(x, y): opposite of previous functions; takes an x and y and turns it into a numerical position
      setVal(i, v): sets the numeric pos i to the value of v on the board
      isOnBoard(x, y): returns true if the position is in bounds; returns false if oob
      movementIsOnBoard(x, y, xd, yd): moves from (x, y) in the direction (xd, yd) as a vector. Returns true if this motion stays on the board
      getVal(i): returns the position of i on the board
      getXSize(): returns the x size
      getYSize(): returns the y size

  ADDING IMAGES
    Open an IDE that is compatible with Java (IntelliJ by JetBrains recommended), and create a project with the file "Main" from this repository's BMPT Converter file.
    Run the program, and enter the number of colorSteps (3 is highly recommended with this screen), and enter the filepath of a png image, or a folder containing nothing but pngs
    Enter the image scale for each of the images.
    Copy the .bmps and the .scs to a folder on an SD card, and insert into the slot on the screen.
    Note that the Arduino's rendering code is programmed to ignore rgb(1,1,1) and treat it as transparent. Transparent pixels are automatically converted from PNGs when the Java BMPT Converter runs; do not attempt to do it yourself.

  IMPORTANT NOTES
    When drawing a BMP from any of the functions, make sure the .bmp and the .sc are stored in the folder matching your game's name in the SD card.
    The .sc file is used for scaling your images, and having a small image scaled up WILL load faster than a large image.
    When reading files from the SD card, any filename longer than 8 characters will always fail to load (quirk of arduinos)

  EASY OPTIMIZATION TACTICS
    Ensure that you do not redraw the entire screen per frame; will lead to extremely slow performance, and can lead to missed player inputs
    Use small images that are scaled up when applicable, as reading the image data from the SD card is the slowest part of the process
    
    
