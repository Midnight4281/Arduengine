#include "DebugGame.h"
void DebugGame::reset() {
  clearScreen();
  drawBMPSector("noise", 0, 0, 0, 50, 42, 100);
}