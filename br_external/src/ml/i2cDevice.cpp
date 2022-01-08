#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <unistd.h>
extern "C"
{
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>
}

#include "i2cDevice.hpp"

i2cDevice::i2cDevice(uint8_t devAddr)
{
  const char *filename = "/dev/i2c-1";
  this->i2c_fd = open(filename, O_RDWR); // Open the i2c file descriptor in read/write mode
  if (this->i2c_fd < 0)
  {
    std::cerr << "Can't open I2C BUS" << std::endl; // If there's an error opening this, then display it.
  }
  if (ioctl(this->i2c_fd, I2C_SLAVE, devAddr) < 0)
  {                                                                             // Using ioctl set the i2c device to talk to address in the "addr" variable.
    std::cerr << "Can't set the I2C address for the slave device" << std::endl; // Display error setting the address for the slave.
  }
}

void i2cDevice::writeBit(uint8_t reg, uint8_t bit, uint8_t data)
{
  uint8_t old = i2c_smbus_read_byte_data(this->i2c_fd, reg);
  // std::cout << std::hex << (int)old << ' ';
  old = (old & ~(1 << bit)) | (data << bit);
  // std::cout << (int)old << std::endl;
  // old |= data & (1<<bit); // combine data with existing bit
  i2c_smbus_write_byte_data(this->i2c_fd, reg, old);
}

void i2cDevice::writeBits(uint8_t reg, uint8_t start, uint8_t length, uint8_t data)
{
  uint8_t old = i2c_smbus_read_byte_data(this->i2c_fd, reg);

  uint8_t mask = ((1 << length) - 1) << (start - length + 1);
  data <<= (start - length + 1); // shift data into correct position
  old &= ~(mask);                // zero all important bits in existing byte
  old |= data;                   // combine data with existing byte

  i2c_smbus_write_byte_data(this->i2c_fd, reg, old);
}

void i2cDevice::writeByte(uint8_t reg, uint8_t data)
{
  i2c_smbus_write_byte_data(this->i2c_fd, reg, data);
}

void i2cDevice::writeWord(uint8_t reg, uint16_t data)
{
  uint8_t buf[2];
  buf[0] = data >> 8;
  buf[1] = data & 0xFF;
  writeBlock(reg, 2, buf);
}

uint8_t i2cDevice::readByte(uint8_t reg)
{
  return i2c_smbus_read_byte_data(this->i2c_fd, reg);
}

uint16_t i2cDevice::readWord(uint8_t reg)
{
  uint8_t buf[2];
  readBlock(reg, 2, buf);
  // int val = i2c_smbus_read_word_data(this->i2c_fd, reg);
  return ((uint16_t)buf[0] << 8) | buf[1];
}

void i2cDevice::readBlock(uint8_t regStart, uint8_t length, uint8_t *buf)
{
  int res;
  while (length > 0)
  {
    int to_read = length > 32 ? 32 : length;
    res = i2c_smbus_read_i2c_block_data(this->i2c_fd, regStart, to_read, buf);
    buf += to_read;
    length -= to_read;
    if (res < 0)
    {
      std::cout << "Error during readBlock of " << (int)length << " bytes from: " << (int)regStart << std::endl;
      return;
    }
  }
}

void i2cDevice::writeBlock(uint8_t regStart, uint8_t length, const uint8_t *buf)
{
  int res;
  while (length > 0)
  {
    int to_write = length > 32 ? 32 : length;
    res = i2c_smbus_write_i2c_block_data(this->i2c_fd, regStart, to_write, buf);
    buf += to_write;
    length -= to_write;
    if (res < 0)
    {
      std::cout << "Error during writeBlock of " << (int)length << " bytes from: " << (int)regStart << std::endl;
      return;
    }
  }
}

i2cDevice::~i2cDevice()
{
  if (this->i2c_fd)
  {
    close(this->i2c_fd);
  }
}
