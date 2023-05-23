/*****************************************************************
  File:             BM25S3321-1.h
  Author:           BESTMODULES
  Description:      Define classes and required variables
  History：         None
  V1.0.1   -- initial version; 2023-04-04; Arduino IDE : v1.8.19
******************************************************************/
#ifndef _BM25S3321_1_H_
#define _BM25S3321_1_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

#define BAUDRATE 9600
#define CHECK_OK 0
#define CHECK_ERROR 1
#define TIMEOUT_ERROR 2

class BM25S3321_1
{
public:
  BM25S3321_1(uint8_t statusPin, HardwareSerial *theSerial = &Serial);
  BM25S3321_1(uint8_t statusPin, uint8_t rxPin, uint8_t txPin);
  void begin();
  void preheatCountdown();
  uint16_t readCO2Value();
  uint8_t calibrateZeroPoint();
  uint8_t calibrateCO2Value(uint16_t value);
  uint8_t setRangeMax(uint16_t value);
  uint8_t setAutoCalibration(uint8_t modeCode);
  uint8_t setAutoCalibrationCycle(uint8_t day);

private:
  uint8_t _statusPin, _rxPin, _txPin;
  uint8_t calibrateSpanPoint(uint16_t value);
  void writeBytes(uint8_t wBuf[], uint8_t wLen = 9);
  uint8_t readBytes(uint8_t rBuf[], uint8_t rLen = 9, uint16_t timeout = 10);
  uint8_t checkResponse(uint8_t cmd);
  HardwareSerial *_hardSerial = NULL;
  SoftwareSerial *_softSerial = NULL;
};

#endif
