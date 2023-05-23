/*****************************************************************
  File:           BM25S3321-1.cpp
  Author:         BESTMODULES
  Description:    None
  History：       None
  V1.0.1   -- initial version; 2023-04-04; Arduino IDE : v1.8.19
******************************************************************/
#include "BM25S3321-1.h"

/**********************************************************
Description: Constructor
Parameters: statusPin: Status pin connection with Arduino or BMduino
            *theSerial: Serial object, if your board has multiple UART interfaces
Return: None
Others: None
**********************************************************/
BM25S3321_1::BM25S3321_1(uint8_t statusPin, HardwareSerial *theSerial)
{
  _softSerial = NULL;
  _statusPin = statusPin;
  _hardSerial = theSerial;
}

/**********************************************************
Description:  Constructor
Parameters: statusPin: Status pin connection with Arduino or BMduino
            rxPin : Receiver pin of the UART
            txPin : Send signal pin of UART
Return: None
Others: None
**********************************************************/
BM25S3321_1::BM25S3321_1(uint8_t statusPin, uint8_t rxPin, uint8_t txPin)
{
  _hardSerial = NULL;
  _statusPin = statusPin;
  _rxPin = rxPin;
  _txPin = txPin;
  _softSerial = new SoftwareSerial(_rxPin, _txPin);
}

/**********************************************************
Description: Module initial
Parameters: None
Return: None
Others: None
**********************************************************/
void BM25S3321_1::begin()
{
  if (_softSerial != NULL)
  {
    _softSerial->begin(BAUDRATE); // baud rate:9600
  }
  if (_hardSerial != NULL)
  {
    _hardSerial->begin(BAUDRATE); // baud rate:9600
  }
  pinMode(_statusPin, INPUT);
}

/**********************************************************
Description: Preheat Module(about 30 second)
Parameters: None
Return: None
Others: None
**********************************************************/
void BM25S3321_1::preheatCountdown()
{
  for (uint8_t i = 60; i > 0; i--)
  {
    delay(1030);
  }
}

/**********************************************************
Description: Read CO2 concentration
Parameters: None
Return: CO2 concentration value, unit:ppm
Others: None
**********************************************************/
uint16_t BM25S3321_1::readCO2Value()
{
  uint16_t tmp = 0;
  uint8_t sendBuf[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  uint8_t recBuf[9] = {0};
  writeBytes(sendBuf);
  delay(20);
  if ((readBytes(recBuf) == 0) && (recBuf[1] == 0x86))
  {
    tmp = recBuf[2];
    tmp = tmp << 8;
    tmp += recBuf[3]; // CO2
  }
  return tmp;
}

/**********************************************************
Description: Calibrate zero point
Parameters: None
Return: 0: Check ok
        1: Check error
        2: Timeout error
Others: None
**********************************************************/
uint8_t BM25S3321_1::calibrateZeroPoint()
{
  uint8_t sendBuf[9] = {0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
  writeBytes(sendBuf);
  delay(20);
  return checkResponse(0x87);
}

/**********************************************************
Description: Calibration carbon dioxide concentration
Parameters: concentration value(400ppm-1500ppm)
Return: 0: Check ok
        1: Check error
        2: Timeout error
        3: Parameter error
Others: None
**********************************************************/
uint8_t BM25S3321_1::calibrateCO2Value(uint16_t value)
{
  uint8_t sendBuf[9] = {0xFF, 0x01, 0xAD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if ((value < 400) || (value > 1500))
  {
    return 3;
  }
  sendBuf[3] = value >> 8;
  sendBuf[4] = value & 0xff;
  for (uint8_t i = 1; i < 8; i++)
  {
    sendBuf[8] += sendBuf[i];
  }
  sendBuf[8] = ~sendBuf[8] + 1;
  writeBytes(sendBuf);
  delay(20);
  return checkResponse(0xAD);
}

/**********************************************************
Description: Set measurement range
Parameters: Maximum value of measurement range, the value = (400, 5000]
Return: 0: Check ok
        1: Check error
        2: Timeout error
        3: Parameter error
Others: For example, when value = 5000(ppm), the measurement range is 400 ~ 5000(ppm)
**********************************************************/
uint8_t BM25S3321_1::setRangeMax(uint16_t value)
{
  uint8_t errFlag = 1;
  uint8_t sendBuf[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  if ((value <= 400) || (value > 5000))
  {
    return 3;
  }
  sendBuf[6] = value >> 8;
  sendBuf[7] = value & 0xff;
  for (uint8_t i = 1; i < 8; i++)
  {
    sendBuf[8] += sendBuf[i];
  }
  sendBuf[8] = ~sendBuf[8] + 1;
  writeBytes(sendBuf);
  delay(40);
  return checkResponse(0x99);
}

/**********************************************************
Description: Set up automatic calibration
Parameters: modeCode = 0xA0: Enable automatic calibration function
            modeCode = 0x00: Disable automatic calibration function
Return: 0: Check ok
        1: Check error
        2: Timeout error
        3: Parameter error
Others: None
**********************************************************/
uint8_t BM25S3321_1::setAutoCalibration(uint8_t modeCode)
{
  uint8_t sendBuf[9] = {0xFF, 0x01, 0x79, modeCode, 0x00, 0x00, 0x00, 0x00, 0x00};
  if ((modeCode != 0xA0) || (modeCode != 0x00))
  {
    return 3;
  }
  for (uint8_t i = 1; i < 8; i++)
  {
    sendBuf[8] += sendBuf[i];
  }
  sendBuf[8] = ~sendBuf[8] + 1;
  writeBytes(sendBuf);
  delay(40);
  return checkResponse(0x79);
}

/**********************************************************
Description: Set automatic calibration cycle
Parameters: Automatic calibration cycle, unit: day
Return: 0: Check ok
        1: Check error
        2: Timeout error
        3: Parameter error/Setup failed
Others: None
**********************************************************/
uint8_t BM25S3321_1::setAutoCalibrationCycle(uint8_t day)
{
  uint8_t errFlag;
  uint8_t sendBuf[9] = {0xFF, 0x01, 0xAF, 0x01, day, 0x00, 0x00, 0x00, 0x00};
  uint8_t recBuf[9] = {0};
  for (uint8_t i = 1; i < 8; i++)
  {
    sendBuf[8] += sendBuf[i];
  }
  sendBuf[8] = ~sendBuf[8] + 1;
  writeBytes(sendBuf);
  delay(40);
  errFlag = readBytes(recBuf);
  if ((errFlag == 0) && (recBuf[1] == 0xAF))
  {
    if (recBuf[3] == day)
    {
      return 0;
    }
    else
    {
      return 3;
    }
  }
  else
  {
    return errFlag;
  }
}

/**********************************************************
Description: Calibrate span point
Parameters: concentration value
Return: 0: Check ok
        1: Check error
        2: Timeout error
Others: Calibration of span points is required in specific environments
        and is not recommended for users
**********************************************************/
uint8_t BM25S3321_1::calibrateSpanPoint(uint16_t value)
{
  uint8_t sendBuf[9] = {0xFF, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendBuf[3] = value >> 8;
  sendBuf[4] = value & 0xff;
  for (uint8_t i = 1; i < 8; i++)
  {
    sendBuf[8] += sendBuf[i];
  }
  sendBuf[8] = ~sendBuf[8] + 1;
  writeBytes(sendBuf);
  delay(20);
  return checkResponse(0x88);
}

/**********************************************************
Description: Write data through UART
Parameters: wBuf: The array for storing data to be sent(9 bytes)
            wLen:Length of data sent
Return: None
Others: None
**********************************************************/
void BM25S3321_1::writeBytes(uint8_t wbuf[], uint8_t wLen)
{
  /*Select hardSerial or softSerial according to the setting*/
  if (_softSerial != NULL)
  {
    while (_softSerial->available() > 0)
    {
      _softSerial->read();
    }
    _softSerial->write(wbuf, wLen);
    _softSerial->flush(); // Wait for the end of serial port data transmission
  }
  else
  {
    while (_hardSerial->available() > 0)
    {
      _hardSerial->read();
    }
    _hardSerial->write(wbuf, wLen);
    _hardSerial->flush(); // Wait for the end of serial port data transmission
  }
}

/**********************************************************
Description: Read data through UART
Parameters: rBuf: The array for storing Data to be sent
            rlen: Length of data to be read
            timeout: Receive timeout(unit: ms)
Return: 0: Verification succeeded
        1: Verification failed
        2: Timeout error
Others: None
**********************************************************/
uint8_t BM25S3321_1::readBytes(uint8_t rBuf[], uint8_t rLen, uint16_t timeout)
{
  uint16_t delayCnt = 0;
  uint8_t i = 0, checkSum = 0;

  /* Select SoftwareSerial Interface */
  if (_softSerial != NULL)
  {
    for (i = 0; i < rLen; i++)
    {
      delayCnt = 0;
      while (_softSerial->available() == 0)
      {
        if (delayCnt > timeout)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1); // delay 1ms
        delayCnt++;
      }
      rBuf[i] = _softSerial->read();
    }
  }
  /* Select HardwareSerial Interface */
  if (_hardSerial != NULL)
  {
    for (i = 0; i < rLen; i++)
    {
      delayCnt = 0;
      while (_hardSerial->available() == 0)
      {
        if (delayCnt > timeout)
        {
          return TIMEOUT_ERROR; // Timeout error
        }
        delay(1);
        delayCnt++;
      }
      rBuf[i] = _hardSerial->read();
    }
  }
  /* check Sum */
  for (i = 1; i < (rLen - 1); i++)
  {
    checkSum += rBuf[i];
  }
  checkSum = ~checkSum + 1;
  if (checkSum == rBuf[rLen - 1])
  {
    return CHECK_OK; // Check correct
  }
  else
  {
    return CHECK_ERROR; // Check error
  }
}

/**********************************************************
Description: Check if the module responds correctly
Parameters: cmd: Instruction code
Return: 0: Response ok
        1: Response error
        2: Timeout error
Others: None
**********************************************************/
uint8_t BM25S3321_1::checkResponse(uint8_t cmd)
{
  uint8_t recBuf[9] = {0}, errFlag;
  errFlag = readBytes(recBuf);
  if ((errFlag == 0) && (recBuf[1] == cmd))
  {
    return 0;
  }
  else
  {
    return errFlag;
  }
}