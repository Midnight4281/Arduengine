#ifndef BOARD_H
#define BOARD_H

class Board {
  protected:
    int8_t _world[400];
    uint16_t _maxX;
    uint16_t _maxY;
  public:
    Board() {
    }
    void reset(int xSize, int ySize) {
      _maxX = xSize;
      _maxY = ySize;
      memset(_world, 0, sizeof(_world));
    }
    uint16_t xDrawSize;
    uint16_t yDrawSize;
    int16_t xDrawOffset;
    int16_t yDrawOffset;
    int16_t getYAtNumeric(uint32_t i) {
      return floor(i / _maxX);
    }
    int16_t getXAtNumeric(uint32_t i) {
      return i % _maxX;
    }
    int16_t moveNumericByDir(uint32_t i, uint16_t xd, uint16_t yd) {
      return getNumericAtPos(getXAtNumeric(i) + xd, getYAtNumeric(i) + yd);
    }
    int16_t getNumericAtPos(uint16_t x, uint16_t y) {
      return x + y * _maxX;
    }
    void setVal(uint32_t i, int8_t v) {
      _world[i] = v;
    }
    bool isOnBoard(int16_t x, uint16_t y) {
      return x >= 0 && x < _maxX && y >= 0 && y < _maxY;
    }
    bool movementIsOnBoard(int16_t x, int16_t y, int16_t xd, int16_t yd) {
      int16_t s = getNumericAtPos(x, y);
      int16_t d = getNumericAtPos(x + xd, y + yd);
      int _x = getXAtNumeric(d);
      int _y = getYAtNumeric(d);
      if(!isOnBoard(_x, _y) || _y != y + yd || _x != x + xd) {
        return false;
      }
      return true;
    }
    uint8_t getVal(uint32_t i) {
      return _world[i];
    }
    uint16_t getXSize() {
      return _maxX;
    }
    uint16_t getYSize() {
      return _maxY;
    }
};
#endif