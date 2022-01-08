#pragma once

#include <cstdint>

class i2cDevice
{
protected:
  int i2c_fd;

  i2cDevice(uint8_t devAddr);
  ~i2cDevice();
  void writeBit(uint8_t reg, uint8_t bit, uint8_t data);
  void writeBits(uint8_t reg, uint8_t start, uint8_t length, uint8_t data);
  void writeByte(uint8_t reg, uint8_t data);
  void writeWord(uint8_t reg, uint16_t data);
  void writeBlock(uint8_t regStart, uint8_t length, const uint8_t *buf);
  void readBlock(uint8_t regStart, uint8_t length, uint8_t *buf);
  uint16_t readWord(uint8_t reg);
  uint8_t readByte(uint8_t reg);
};
