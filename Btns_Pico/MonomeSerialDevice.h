#ifndef MONOMESERIAL_H
#define MONOMESERIAL_H

#include <Arduino.h>

class MonomeGridEvent {
public:
  uint8_t x;
  uint8_t y;
  uint8_t pressed;
};

class MonomeArcEvent {
public:
  uint8_t index;
  int8_t delta;
};

class MonomeEventQueue {
public:
  //void clearQueue();

  bool gridEventAvailable();
  MonomeGridEvent readGridEvent();
  MonomeGridEvent sendGridKey();

  bool arcEventAvailable();
  MonomeArcEvent readArcEvent();
  MonomeArcEvent sendArcDelta();
  MonomeArcEvent sendArcKey();

  void addGridEvent(uint8_t x, uint8_t y, uint8_t pressed);
  void sendGridKey(uint8_t x, uint8_t y, uint8_t pressed);
  void addArcEvent(uint8_t index, int8_t delta);
  void sendArcDelta(uint8_t index, int8_t delta);
  void sendArcKey(uint8_t index, uint8_t pressed);
  void sendTiltEvent(uint8_t n, uint8_t xh, uint8_t xl, uint8_t yh, uint8_t yl, uint8_t zh, uint8_t zl);

protected:

private:
  static const int MAXEVENTCOUNT = 50;

  MonomeGridEvent emptyGridEvent;
  MonomeGridEvent gridEvents[MAXEVENTCOUNT];
  int gridEventCount = 0;
  int gridFirstEvent = 0;

  MonomeArcEvent emptyArcEvent;
  MonomeArcEvent arcEvents[MAXEVENTCOUNT];
  int arcEventCount = 0;
  int arcFirstEvent = 0;
};

class MonomeSerialDevice : public MonomeEventQueue {
public:
  MonomeSerialDevice();
  void initialize();
  void setupAsGrid(uint8_t _rows, uint8_t _columns);
  void setupAsArc(uint8_t _encoders);
  void getDeviceInfo();
  void poll();
  void refresh();
  void sendSysSize();
  void sendSysRotation();
  void setGridLed(uint8_t x, uint8_t y, uint8_t level);
  void clearGridLed(uint8_t x, uint8_t y);
  void setArcLed(uint8_t enc, uint8_t led, uint8_t level);
  void setAllLEDs(int value);
  void clearArcLed(uint8_t enc, uint8_t led);
  void clearAllLeds();
  void clearArcRing(uint8_t ring);
  void refreshGrid();
  void refreshArc();

  void setTiltActive(uint8_t sensor, bool active);
  void sendTiltEvent(uint8_t sensor, int16_t x, int16_t y, int16_t z);


  bool active;
  bool isMonome;
  bool isGrid;
  uint8_t rows;
  uint8_t columns;
  uint8_t encoders;
  uint8_t gridX;
  uint8_t gridY;

  static const int variMonoThresh = 0;
  static const int MAXLEDCOUNT = 256;
  uint8_t leds[MAXLEDCOUNT];
  String deviceID;

private:
  bool arcDirty = false;
  bool gridDirty = false;
  uint8_t gridRotation;  // 0, 1, 2, or 3 for 0, 90, 180, 270 degrees
 bool tiltActive[4] = {false, false, false, false};
    int16_t lastTiltX[4] = {0};
    int16_t lastTiltY[4] = {0};
    int16_t lastTiltZ[4] = {0};

  //        MonomeSerialDevice();
  void processSerial();
};

#endif