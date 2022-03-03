#ifndef IMU_H
#define IMU_H

#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <cstring>

struct imu_data_t
{
	struct gyro_t
	{
		float x, y, z;
	};
	struct accel_t
	{
		float x, y, z;
	};

	gyro_t gyro;
	accel_t accel;
};

// static uint16_t SPI_RW_buffer_gen(uint8_t RW, uint8_t addr, uint8_t data);

void IMU_HW_reset();

void IMU_setup();

void IMU_calibrate();

imu_data_t IMU_read();

uint16_t IMU_SPI_read(uint8_t addr);

static void IMU_SPI_write(uint8_t addr, uint8_t data);

static inline float accel_format(int16_t data);

static inline float gyro_format(int16_t data);

#endif
