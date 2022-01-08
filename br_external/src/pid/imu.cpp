#include <iostream>
#include <iomanip>
extern "C"
{
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}
#include <unistd.h>
#include <cmath>
#include "imu.hpp"
#include "dmp.hpp"

#define ASSERT_CONNECTED() \
  do                       \
  {                        \
    if (!this->i2c_fd)     \
    {                      \
      return;              \
    }                      \
  } while (0)

uint8_t *dmpPacketBuffer;

bool IMU::isConnected()
{
  uint8_t WHO_AM_I_REG = readByte(0x75);
  return (WHO_AM_I_REG & 0x7e) == (0x34 << 1);
}

int IMU::init()
{

  uint8_t old;
  writeBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, true);
  usleep(30000);
  writeBit(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, false);

  // setMemoryBank(0, 0, 0)
  writeByte(MPU6050_RA_BANK_SEL, 0x0);

  // Setting slave 0 address to 0x7F
  writeByte(MPU6050_RA_I2C_SLV0_ADDR, 0x7F);
  // Disabling I2C Master mode...
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, false);
  // Setting slave 0 address to 0x68
  writeByte(MPU6050_RA_I2C_SLV0_ADDR, 0x68);
  // Resetting I2C Master control
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT, true);
  usleep(20000);

  // load DMP code into memory banks
  if (writeMemoryBlock(dmpMemory, MPU6050_DMP_CODE_SIZE, 0, 0, true))
    return 1;
  if (writeConfigSet(dmpConfig, MPU6050_DMP_CONFIG_SIZE))
    return 1;

  // setClockSource(MPU6050_CLOCK_PLL_ZGYRO);
  writeBits(MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, MPU6050_CLOCK_PLL_ZGYRO);

  // Setting DMP and FIFO_OFLOW interrupts enabled...
  writeByte(MPU6050_RA_INT_ENABLE, 1 << MPU6050_INTERRUPT_FIFO_OFLOW_BIT | 1 << MPU6050_INTERRUPT_DMP_INT_BIT);

  // Setting sample rate to 200Hz...
  writeByte(MPU6050_RA_SMPLRT_DIV, 4); // 1khz / (1 + 4) = 200 Hz

  // Setting external frame sync to TEMP_OUT_L[0]...
  // setExternalFrameSync(MPU6050_EXT_SYNC_TEMP_OUT_L);
  writeBits(MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, MPU6050_EXT_SYNC_TEMP_OUT_L);

  // Setting DLPF bandwidth to 42Hz...
  // setDLPFMode(MPU6050_DLPF_BW_42);
  writeBits(MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, MPU6050_DLPF_BW_42);

  // Setting gyro sensitivity to +/- 2000 deg/sec...
  // setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
  writeBits(MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, MPU6050_GYRO_FS_2000);
  writeBits(MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, MPU6050_ACCEL_FS_2);

  // write start address MSB into register
  writeByte(MPU6050_RA_DMP_CFG_1, 0x03);
  // write start address LSB into register
  writeByte(MPU6050_RA_DMP_CFG_2, 0x00);

  // Clearing OTP Bank flag...
  writeBit(MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, false);

  const uint8_t *dmpUpdate = &dmpUpdates[0];
  // writing final memory update 1/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  // writing final memory update 2/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  // Resetting FIFO
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);

  // Setting motion detection threshold to 2...
  writeByte(MPU6050_RA_MOT_THR, 0x02);

  // Setting zero-motion detection threshold to 156...
  writeByte(MPU6050_RA_ZRMOT_THR, 156);

  // Setting motion detection duration to 80...
  writeByte(MPU6050_RA_MOT_DUR, 80);

  // Setting zero-motion detection duration to 0...
  writeByte(MPU6050_RA_ZRMOT_DUR, 0);

  // Resetting FIFO
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
  // Enabling FIFO
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, true);
  // Enabling DMP
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, true);
  // Resetting DMP...
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, true);

  int fifoCount;
  uint8_t fifoBuffer[128];
  while ((fifoCount = getFIFOCount()) < 3)
  {
  };

  // writing final memory update 3/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  // writing final memory update 4/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  // writing final memory update 5/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  readBlock(MPU6050_RA_FIFO_R_W, fifoCount, fifoBuffer);

  // writing final memory update 6/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);
  dmpUpdate += dmpUpdate[2] + 3;

  while ((fifoCount = getFIFOCount()) < 3)
  {
  };

  readBlock(MPU6050_RA_FIFO_R_W, fifoCount, fifoBuffer);

  // writing final memory update 7/7 (function unknown)
  writeMemoryBlock(&dmpUpdate[3], dmpUpdate[2], dmpUpdate[0], dmpUpdate[1], 1);

  // Disabling DMP (you turn it on later)...
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, false);

  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);

  return 0;
}

void IMU::clearOffsets()
{
  uint8_t zeros[6] = {};
  writeBlock(MPU6050_RA_XA_OFFS_H, 6, (uint8_t *)zeros);
  writeBlock(MPU6050_RA_XG_OFFS_USRH, 6, (uint8_t *)zeros);
}

void IMU::setAccelOffset(int16_t x_off, int16_t y_off, int16_t z_off)
{
  uint8_t offsets[6] = {
      (uint8_t)(x_off >> 8),
      (uint8_t)(x_off & 0xFF),
      (uint8_t)(y_off >> 8),
      (uint8_t)(y_off & 0xFF),
      (uint8_t)(z_off >> 8),
      (uint8_t)(z_off & 0xFF),
  };
  writeBlock(MPU6050_RA_XA_OFFS_H, 6, (uint8_t *)offsets);
}

void IMU::setGyroOffset(int16_t x_off, int16_t y_off, int16_t z_off)
{
  uint8_t offsets[6] = {
      (uint8_t)(x_off >> 8),
      (uint8_t)(x_off & 0xFF),
      (uint8_t)(y_off >> 8),
      (uint8_t)(y_off & 0xFF),
      (uint8_t)(z_off >> 8),
      (uint8_t)(z_off & 0xFF),
  };
  writeBlock(MPU6050_RA_XG_OFFS_USRH, 6, (uint8_t *)offsets);
}

void IMU::enableDMP()
{
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, true);
}

uint8_t packet[dmpPacketSize];
float IMU::getNewAngle()
{

  int fifo_count = getFIFOCount();
  int int_status = readByte(MPU6050_RA_INT_STATUS);
  if (int_status & 0x10 || fifo_count == 1024)
  {
    writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
    fifo_count = 0;
  }

  do
  {
    int_status = readByte(MPU6050_RA_INT_STATUS);
  } while (!int_status & 0x02);

  while (fifo_count < dmpPacketSize)
  {
    fifo_count = getFIFOCount();
  }

  while (fifo_count > dmpPacketSize)
  {
    readBlock(MPU6050_RA_FIFO_R_W, dmpPacketSize, packet);
    fifo_count -= dmpPacketSize;
  }

  int16_t qw_i = ((int16_t)packet[0] << 8) | packet[1];
  int16_t qx_i = ((int16_t)packet[4] << 8) | packet[5];
  int16_t qy_i = ((int16_t)packet[8] << 8) | packet[9];
  int16_t qz_i = ((int16_t)packet[12] << 8) | packet[13];

  int16_t ax_i = ((int16_t)packet[28] << 8) | packet[29];
  int16_t ay_i = ((int16_t)packet[32] << 8) | packet[33];
  int16_t az_i = ((int16_t)packet[36] << 8) | packet[37];
  float ax = ax_i / 8192.0F;
  float ay = ay_i / 8192.0F;
  float az = az_i / 8192.0F;

  float qw = qw_i / 16384.0F;
  float qx = qx_i / 16384.0F;
  float qy = qy_i / 16384.0F;
  float qz = qz_i / 16384.0F;

  // float angle_0 =  atan2(2/mod*qx*qy - 2/mod*qw*qz, 2/mod*qw*qw + 2/mod*qx*qx - 1); // psi
  // float angle_1 = -asin(2/mod*qx*qz + 2/mod*qw*qy);                         // theta
  float angle_2 = atan2(2 * qy * qz - 2 * qw * qx, 2 * qw * qw + 2 * qz * qz - 1); // phi

  // float forward_acceleration = ay * cos(angle_2) + az * sin(angle_2);
  // std::cout << forward_acceleration << std::endl;

  return angle_2;
}

int IMU::writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, bool verify)
{
  writeByte(MPU6050_RA_BANK_SEL, bank);
  writeByte(MPU6050_RA_MEM_START_ADDR, address);

  uint8_t chunkSize;

  uint8_t *verifyBuffer = nullptr;
  if (verify)
    verifyBuffer = (uint8_t *)malloc(MPU6050_DMP_MEMORY_CHUNK_SIZE);

  for (int i = 0; i < dataSize;)
  {
    chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;
    if (i + chunkSize > dataSize)
      chunkSize = dataSize - i;
    if (chunkSize > 256 - address)
      chunkSize = 256 - address;
    writeBlock(MPU6050_RA_MEM_R_W, chunkSize, &data[i]);

    if (verify && verifyBuffer)
    {
      writeByte(MPU6050_RA_BANK_SEL, bank);
      writeByte(MPU6050_RA_MEM_START_ADDR, address);
      readBlock(MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);
      for (int k = 0; k < chunkSize; ++k)
      {
        if (data[i + k] != verifyBuffer[k])
        {
          std::cout << "ERROR" << std::endl;
          free(verifyBuffer);
          return 1;
        }
      }
    }

    i += chunkSize;
    address += chunkSize;

    if (address == 0)
    {
      ++bank;
    }
    writeByte(MPU6050_RA_BANK_SEL, bank);
    writeByte(MPU6050_RA_MEM_START_ADDR, address);
  }
  if (verify)
    free(verifyBuffer);
  return 0;
}

void IMU::calibrateGyro(int loops)
{
  float kP = .3;
  float kI = 90;
  float x = (100 - 20 - (loops - 1) * 5) * .01F;
  kP *= x;
  kI *= x;
  PID(0x43, kP, kI, loops);
}

void IMU::calibrateAccel(int loops)
{
  float kP = .3;
  float kI = 20;
  float x = (100 - 20 - (loops - 1) * 5) * .01F;
  kP *= x;
  kI *= x;
  PID(0x3B, kP, kI, loops);
}

void IMU::PID(int readAddress, float kP, float kI, int loops)
{
  uint8_t saveAddress = (readAddress == 0x3B) ? 0x06 : 0x13;

  int16_t data;
  float reading;
  int16_t bitZero[3];
  uint8_t shift = 2;
  float error, pTerm, iTerm[3];
  int16_t eSample;
  uint32_t eSum;
  uint16_t gravity = 8192;
  if (readAddress == 0x3B)
  {
    gravity = 16384;
  }
  std::fflush(stdout);
  for (int i = 0; i < 3; ++i)
  {
    uint16_t data = readWord(saveAddress + i * shift);
    reading = data;
    if (saveAddress != 0x13)
    {
      bitZero[i] = data & 1;
      iTerm[i] = reading * 8;
    }
    else
    {
      iTerm[i] = reading * 4;
    }
  }

  for (int L = 0; L < loops; ++L)
  {
    eSample = 0;
    for (int c = 0; c < 100; ++c)
    {
      eSum = 0;
      for (int i = 0; i < 3; ++i)
      {
        int16_t data = readWord(readAddress + i * 2);
        reading = data;
        if ((readAddress == 0x3B) && (i == 2))
        {
          reading -= gravity;
        }
        error = -reading;
        eSum += abs(reading);
        pTerm = kP * error;
        iTerm[i] += (error * 0.001F) * kI;
        if (saveAddress != 0x13)
        {
          data = round((pTerm + iTerm[i]) / 8);
          data = (data & 0xFFFE) | bitZero[i];
        }
        else
        {
          data = round((pTerm + iTerm[i]) / 4);
        }
        writeWord(saveAddress + (i * shift), data);
      }
      if (c == 99 && eSum > 1000)
      {
        c = 0;
      }
      float multiplyer = (readAddress == 0x3B) ? 0.05F : 1.F;
      if (eSum * multiplyer < 5)
      {
        eSample++;
      }
      if (eSum < 100 && c > 10 && eSample >= 10)
      {
        break;
      }
      usleep(1000);
    }
    kP *= .75F;
    kI *= .75F;
    for (int i = 0; i < 3; ++i)
    {
      if (saveAddress != 0x13)
      {
        data = round(iTerm[i] / 8);
        data = (data & 0xFFFE) | bitZero[i];
      }
      else
      {
        data = round(iTerm[i] / 4);
      }
      std::cout << "Setting offset of " << data << " for " << saveAddress + i * shift << std::endl;
      writeWord(saveAddress + i * shift, data);
    }
  }
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, true);
  writeBit(MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, true);
}

int IMU::writeConfigSet(const uint8_t *data, int size)
{
  int bank, offset, length, success;
  for (int i = 0; i < size;)
  {
    bank = data[i + 0];
    offset = data[i + 1];
    length = data[i + 2];
    i += 3;

    success = writeMemoryBlock(&data[i], length, bank, offset, true);
    i += length;

    if (success)
    {
      return success;
    }
  }
  return 0;
}

int IMU::getFIFOCount()
{
  uint8_t buf[2];
  readBlock(MPU6050_RA_FIFO_COUNTH, 2, buf);
  return ((int)buf[0] << 8) | buf[1];
}
