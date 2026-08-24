#include <array>
#include <Arduino.h>
#include <algorithm> // Required for std::copy
#include <iterator> 

#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H
class ButtonManager {
  protected:
    String keyCodes[8] = {"L", "U", "D", "R", "-", "+", "B", "A"};
  public:
    int buttonHeldTimes[8];
    int buttonLastHeldTimes[8];
    String n;
    int searchKeys(String tar) {
      int arraySize = 8;
      for (int i = 0; i < arraySize; i++) {
        // strcmp returns 0 if the strings match perfectly
        if (keyCodes[i] == tar) {
          return i; // Found the string, return its index position
        }
      }
      return -1; // Return -1 if the string is not in the array
    }
    int getButtonPressTime(String keyCode) {//returns how many frames a button has been held, searches with KeyCode
      int i = searchKeys(keyCode);
      int a = buttonHeldTimes[i];
      return a;
    }
    int getButtonPressTime(int i) {//returns how many frames a button has been held, searches with KeyCode
      int a = buttonHeldTimes[i];
      return a;
    }
    bool getButtonPressed(String keyCode) {//returns if a key is pressed
      return getButtonPressTime(keyCode) > 0;
    }
    bool getButtonPressed(int i) {//returns if a key is pressed
      return buttonHeldTimes[i] > 0;
    }
    bool getButtonWentDown(String keyCode) {
      int i = searchKeys(keyCode);
      return getButtonWentDown(i);
    }
    bool getButtonWentDown(int i) {
      int p = buttonLastHeldTimes[i];
      int f = buttonHeldTimes[i];
      return (p == 0 && f > 0);
    }
    bool getButtonWentUp(String keyCode) {
      int i = searchKeys(keyCode);
      return getButtonWentUp(i);
    }
    bool getButtonWentUp(int i) {
      int p = buttonLastHeldTimes[i];
      int f = buttonHeldTimes[i];
      return (p > 0 && f == 0);
    }
    void passInputs(const bool (&arr)[8]) { //input a list of booleans of current player inputs
      std::copy(std::begin(buttonHeldTimes), std::end(buttonHeldTimes), std::begin(buttonLastHeldTimes)); // copies held data into last frame data
      int arraySize = 8;
      for (int i = 0; i < arraySize; i++) {
        // check for each one being true; if so, increase; else, reset to zero
        if (arr[i]) {
          buttonHeldTimes[i]++;
        } else {
          buttonHeldTimes[i] = 0;
        }
      }
      
    }
};
#endif