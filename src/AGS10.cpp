//  AGS10.cpp

#include "AGS10.h"

uint8_t AGS10::Calc_CRC8(uint8_t *dat, uint8_t Num)
{
  uint8_t i,byte1,crc=0xFF;
  for(byte1=0; byte1<Num; byte1++)
  {
    crc^=(dat[byte1]);
    for(i=0;i<8;i++)
    {
      if(crc & 0x80) crc=(crc<<1)^0x31;
      else crc=(crc<<1);
    }
  }
  return crc;
}

AGS10::AGS10()
{
  _tvoc = 0;
  _version = 0;
  _resist = 0;
  _wire = &Wire;
  _lastTVOCReadTime = 0;
  _lastResistReadTime = 0;
  _i2cAddr = 0x1A; // default
}

void AGS10::begin(TwoWire *wire, uint8_t i2cAddr)
{
  if (wire != nullptr) {
    _wire = wire;
  }
  _i2cAddr = i2cAddr;
}

void AGS10::calibrateFact()
{
  uint8_t data[4] = {CALIB_CMD1, CALIB_CMD2, CALIB_FACT, CALIB_FACT};
  // Datasheet confirms CRC is calculated over all 4 data bytes.
  _crc = Calc_CRC8((uint8_t*) data, 4);
  
  _wire->beginTransmission(_i2cAddr);
  _wire->write(CALIB_REG);
  _wire->write(CALIB_CMD1);
  _wire->write(CALIB_CMD2);
  _wire->write(CALIB_FACT);
  _wire->write(CALIB_FACT);
  _wire->write(_crc);
  _wire->endTransmission();
}
  
void AGS10::calibrateCust(uint8_t CALIB_RES1, uint8_t CALIB_RES2)
{
    byte data[4] = {CALIB_CMD1, CALIB_CMD2, CALIB_RES1, CALIB_RES2};
    _crc = Calc_CRC8(data, 4);
    _wire->beginTransmission(_i2cAddr);
    _wire->write(CALIB_REG);
    _wire->write(CALIB_CMD1);
    _wire->write(CALIB_CMD2);
    _wire->write(CALIB_RES1);
    _wire->write(CALIB_RES2);
    _wire->write(_crc);
    _wire->endTransmission();
}

void AGS10::setAddress(uint8_t newAddr)
{
  if (newAddr == _i2cAddr) {
    return; // Prevent unnecessary I2C traffic and flash/EEPROM wear on the sensor
  }

  uint8_t newAddrInv = (uint8_t) ~newAddr;
  uint8_t data[4] = {newAddr, newAddrInv, newAddr, newAddrInv}; 
  _crc = Calc_CRC8(data, 4);
  _wire->beginTransmission(_i2cAddr);
  _wire->write(SLAVE_REG);
  _wire->write(newAddr);
  _wire->write(newAddrInv);
  _wire->write(newAddr);
  _wire->write(newAddrInv);
  _wire->write(_crc);
  _wire->endTransmission();
  
  // Update internal address variable so subsequent calls work
  _i2cAddr = newAddr;
}

int AGS10::readVersion()
{
  _wire->beginTransmission(_i2cAddr);
  _wire->write(VERS_REG);
  _wire->endTransmission();
  _wire->requestFrom(_i2cAddr, 5);

  if (_wire->available() == 5)  {
    byte reserved1 = _wire->read();
    byte reserved2 = _wire->read();
    byte reserved3 = _wire->read();
    byte version = _wire->read();
    byte crcByte = _wire->read();
    
    // Check CRC over the 4 data bytes
    byte data[4] = {reserved1, reserved2, reserved3, version};
    if (Calc_CRC8(data, 4) == crcByte) {
      _version = version;
    }

    return _version;
  }
  return 0;
}

uint32_t AGS10::readResist()
{
  if (millis() - _lastResistReadTime < 1500 && _lastResistReadTime != 0) {
    return _resist;
  }

  _wire->beginTransmission(_i2cAddr);
  _wire->write(RESIST_REG);
  _wire->endTransmission();

  _wire->requestFrom(_i2cAddr, 5);

  if (_wire->available() == 5)  {
    byte resistByteA = _wire->read();
    byte resistByteB = _wire->read();
    byte resistByteC = _wire->read();
    byte resistByteD = _wire->read();
    byte crcByte = _wire->read();

    // Check CRC
    byte data[4] = {resistByteA, resistByteB, resistByteC, resistByteD};
    if (Calc_CRC8(data, 4) == crcByte) {
      _resist = ((uint32_t)resistByteA << 24) | ((uint32_t)resistByteB << 16) | ((uint32_t)resistByteC << 8) | resistByteD;
    }
  }
  _lastResistReadTime = millis();
  return _resist;
}

uint32_t AGS10::readTVOC()
{
  if (millis() - _lastTVOCReadTime < 1500 && _lastTVOCReadTime != 0) {
    return _tvoc;
  }

  _wire->beginTransmission(_i2cAddr);
  _wire->write(TVOC_REG);
  _wire->endTransmission();

  _wire->requestFrom(_i2cAddr, 5);

  if (_wire->available() == 5)  {
    byte statusByte = _wire->read();
    byte tvocByteA = _wire->read();
    byte tvocByteB = _wire->read();
    byte tvocByteC = _wire->read();
    byte crcByte = _wire->read();

    // Check CRC
    byte data[4] = {statusByte, tvocByteA, tvocByteB, tvocByteC};
    if (Calc_CRC8(data, 4) == crcByte) {
      // Bit 0 of statusByte indicates readiness (0 = ready, 1 = busy)
      if ((statusByte & 0x01) == 0) {
        _tvoc = ((uint32_t)tvocByteA << 16) | ((uint32_t)tvocByteB << 8) | tvocByteC;
      }
    }
  }
  _lastTVOCReadTime = millis();
  return _tvoc;
}

bool AGS10::isReady()
{
  if (millis() - _lastTVOCReadTime < 1500 && _lastTVOCReadTime != 0) {
    // If we've read recently, we shouldn't spam the I2C bus.
    // Return false to make the user wait, or if they call readTVOC() it will return cached value anyway.
    return false;
  }

  _wire->beginTransmission(_i2cAddr);
  _wire->write(TVOC_REG);
  _wire->endTransmission();

  _wire->requestFrom(_i2cAddr, 5);

  if (_wire->available() == 5)  {
    byte statusByte = _wire->read();
    byte tvocByteA = _wire->read();
    byte tvocByteB = _wire->read();
    byte tvocByteC = _wire->read();
    byte crcByte = _wire->read();
    
    byte data[4] = {statusByte, tvocByteA, tvocByteB, tvocByteC};
    if (Calc_CRC8(data, 4) == crcByte) {
      if ((statusByte & 0x01) == 0) {
         _tvoc = ((uint32_t)tvocByteA << 16) | ((uint32_t)tvocByteB << 8) | tvocByteC;
         _lastTVOCReadTime = millis();
         return true;
      }
    }
  }
  _lastTVOCReadTime = millis();
  return false;
}
