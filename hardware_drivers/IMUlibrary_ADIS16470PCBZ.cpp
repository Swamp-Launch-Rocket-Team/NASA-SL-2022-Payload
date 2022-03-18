#include "IMUlibrary_ADIS16470PCBZ.h"

using namespace std;

#define RESET 3
#define CHIP_SELECT 4
#define DATA_READY 5
#define SYNC 6

#define CHANNEL 0
#define BAUDRATE 488000 // Max SCLK is 1 MHz when using burst reads

#define MSC_CTRL_ADDR 0x60
#define GLOB_CMD_ADDR 0x68

#define X_ACCL_OUT 0x12
#define Y_ACCL_OUT 0x16
#define Z_ACCL_OUT 0x1A
#define X_GYRO_OUT 0x06
#define Y_GYRO_OUT 0x0A
#define Z_GYRO_OUT 0x0E

#define BURST_READ_DIN 0x68
#define BURST_READ_DIAG_STAT 1
#define BURST_READ_X_GYRO_OUT 2
#define BURST_READ_Y_GYRO_OUT 3
#define BURST_READ_Z_GYRO_OUT 4
#define BURST_READ_X_ACCL_OUT 5
#define BURST_READ_Y_ACCL_OUT 6
#define BURST_READ_Z_ACCL_OUT 7
#define BURST_READ_TEMP_OUT 8
#define BURST_READ_DATA_CNTR 9
#define BURST_READ_CHECKSUM 10

const double ACCEL_SCALER = 0.01197094726; // 40 * 9.8066 / 2^15
const double GYRO_SCALER = 0.00106526443; // 2000 * pi / 180 / 2^15

uint16_t IMU_SPI_read(uint8_t addr)
{
	uint8_t buffer[4];
	// buffer[0]: R/W, A6-A0
	// buffer[1]: D7-D0, don't care on read
	buffer[0] = ~(1 << 7) & addr;
	memset(buffer+1, 0, sizeof(uint8_t) * 3);
	digitalWrite(CHIP_SELECT, 0);
	wiringPiSPIDataRW(CHANNEL, buffer, 2);
	digitalWrite(CHIP_SELECT, 1);
	timespec req, rem;
	req.tv_nsec = 16000;
	req.tv_sec = 0;
	while(nanosleep(&req, &rem) == -1)
		req = rem;
	digitalWrite(CHIP_SELECT, 0);
	wiringPiSPIDataRW(CHANNEL, buffer+2, 2);
	digitalWrite(CHIP_SELECT, 1);
	return buffer[2] << 8 | buffer[3];
}

static void IMU_SPI_write(uint8_t addr, uint8_t data)
{
	digitalWrite(CHIP_SELECT, 0);
	uint8_t buffer[2];
	// buffer[0]: R/W, A6-A0
	// buffer[1]: D7-D0
	buffer[0] = 1 << 7 | addr;
	buffer[1] = data;
	wiringPiSPIDataRW(CHANNEL, buffer, 2);
	digitalWrite(CHIP_SELECT, 1);
}

void IMU_HW_reset()
{
	cout << "Resetting" << endl;
	digitalWrite(RESET, 0);
	timespec  req, rem;
	req.tv_nsec = 2e8;
	req.tv_sec = 0;
	while(nanosleep(&req, &rem) == -1)
		req = rem;
	digitalWrite(RESET, 1);
}

void IMU_setup()
{
	int fd, result;
	unsigned short buffer;

	// Configure pins
	pinMode(RESET, OUTPUT);
	pinMode(CHIP_SELECT, OUTPUT);
	pinMode(DATA_READY, INPUT);
	pinMode(SYNC, OUTPUT);

	IMU_HW_reset();

	cout << "Initializing SPI" << endl;

	// Configure the interface.
	// CHANNEL insicates chip select,
	// 500000 indicates bus speed.
	fd = wiringPiSPISetupMode(CHANNEL, BAUDRATE, 3);

	cout << "Init result: " << fd << endl;

	/*Enabled linear g compensation, enabled point of percussion alignment,
	Sync(CLK) set to default(internal clock), SYNC set to rising edge, DR polarity
	is set to active high*/

	// buffer = SPI_RW_buffer_gen(WRITE, MSC_CTRL_ADDR, 0xC3); //0xE0C3
	// wiringPiSPIDataRW(CHANNEL, (unsigned char*)&buffer, 2);

	IMU_SPI_write(MSC_CTRL_ADDR, 0xC3);

	//Initial Bias Correction
	IMU_calibrate();
}

void IMU_calibrate()
{
	// unsigned short buffer;
	// buffer = SPI_RW_buffer_gen(WRITE, GLOB_CMD_ADDR, 0x01); //Sets the bias correction value to true
	// wiringPiSPIDataRW(CHANNEL, (unsigned char*)&buffer, 2);

	// Trigger bias correction using Continuous Bias Estimation (NULL_CNFG register)
	IMU_SPI_write(GLOB_CMD_ADDR, 0x01);
}

imu_data_t IMU_read()
{
	imu_data_t imu_data;

	uint8_t buffer[14];
	buffer[0] = (~(1 << 7) & X_ACCL_OUT);
	buffer[1] = 0;
	buffer[2] = (~(1 << 7) & Y_ACCL_OUT);
	buffer[3] = 0;
	buffer[4] = (~(1 << 7) & Z_ACCL_OUT);
	buffer[6] = 0;
	buffer[6] = (~(1 << 7) & X_GYRO_OUT);
	buffer[7] = 0;
	buffer[8] = (~(1 << 7) & Y_GYRO_OUT);
	buffer[9] = 0;
	buffer[10] = (~(1 << 7) & Z_GYRO_OUT);
	buffer[11] = 0;
	buffer[12] = 0;
	buffer[13] = 0;

	for(int transferNo = 0; transferNo < 7; transferNo++)
	{
		digitalWrite(CHIP_SELECT, 0);
		wiringPiSPIDataRW(CHANNEL, buffer + 2 * transferNo, 2);
		digitalWrite(CHIP_SELECT, 1);
		if(transferNo != 6)
		{
			timespec req, rem;
			req.tv_nsec = 16000;
			req.tv_sec = 0;
			while(nanosleep(&req, &rem) == -1)
				req = rem;
		}
	}

	for(int i = 0; i < 14; i += 2)
	{
		std::cout << (int16_t)(buffer[i] << 8 | buffer[i + 1]) << std::endl;
	}

	// Data is signed and converted to a float-point value in real-world units
	// Acceleration is in m/s^2
	// Angular velocity is in rad/s
	imu_data.accel.x = (int16_t)(buffer[2] << 8 | buffer[3]) * ACCEL_SCALER;
	imu_data.accel.y = (int16_t)(buffer[4] << 8 | buffer[5]) * ACCEL_SCALER;
	imu_data.accel.z = (int16_t)(buffer[6] << 8 | buffer[7]) * ACCEL_SCALER;
	imu_data.gyro.x = (int16_t)(buffer[8] << 8 | buffer[9]) * GYRO_SCALER;
	imu_data.gyro.y = (int16_t)(buffer[10] << 8 | buffer[11]) * GYRO_SCALER;
	imu_data.gyro.z = (int16_t)(buffer[12] << 8 | buffer[13]) * GYRO_SCALER;

	return imu_data;
}

// Unfinished, do not use
imu_data_t IMU_burst()
{
	imu_data_t imu_data;

	// Define burst read buffer
	uint16_t buffer[11];
	buffer[0] = BURST_READ_DIN;
	memset(buffer+1, 0, sizeof(uint16_t)*10);

	// Poll for data ready (busy wait for now)
	// while(digitalRead(DATA_READY) == 0)
	// {
	// 	timespec req, rem;
	// 	req.tv_nsec = 100;
	// 	req.tv_sec = 0;
	// 	nanosleep(&req, &rem);
	// }

	// Burst Read
	digitalWrite(CHIP_SELECT, 0);
	wiringPiSPIDataRW(CHANNEL, (uint8_t*)buffer, 22);
	digitalWrite(CHIP_SELECT, 1);

	// TODO: Check DIAG_STAT and Checksum for error handling
	uint16_t DIAG_STAT = buffer[BURST_READ_DIAG_STAT];
	uint16_t TEMP_OUT = buffer[BURST_READ_TEMP_OUT];
	uint16_t DATA_CNTR = buffer[BURST_READ_DATA_CNTR];

	cout << "DIAG_STAT:\t" << buffer[BURST_READ_DIAG_STAT] << endl;
	cout << "X_GYRO_OUT:\t" << buffer[BURST_READ_X_GYRO_OUT] << endl;
	cout << "Y_GYRO_OUT:\t" << buffer[BURST_READ_Y_GYRO_OUT] << endl;
	cout << "Z_GYRO_OUT:\t" << buffer[BURST_READ_Z_GYRO_OUT] << endl;
	cout << "X_ACCL_OUT:\t" << buffer[BURST_READ_X_ACCL_OUT] << endl;
	cout << "Y_ACCL_OUT:\t" << buffer[BURST_READ_Y_ACCL_OUT] << endl;
	cout << "Z_ACCL_OUT:\t" << buffer[BURST_READ_Z_ACCL_OUT] << endl;
	cout << "TEMP_OUT:\t" << buffer[BURST_READ_TEMP_OUT] << endl;
	cout << "DATA_CNTR:\t" << buffer[BURST_READ_DATA_CNTR] << endl;
	cout << "Checksum:\t" << buffer[BURST_READ_CHECKSUM] << endl;

	// Data is signed and converted to a float-point value in real-world units
	imu_data.accel.x = (int16_t)buffer[BURST_READ_X_ACCL_OUT] * ACCEL_SCALER;
	imu_data.accel.y = (int16_t)buffer[BURST_READ_Y_ACCL_OUT] * ACCEL_SCALER;
	imu_data.accel.z = (int16_t)buffer[BURST_READ_Z_ACCL_OUT] * ACCEL_SCALER;
	imu_data.gyro.x = (int16_t)buffer[BURST_READ_X_GYRO_OUT] * GYRO_SCALER;
	imu_data.gyro.y = (int16_t)buffer[BURST_READ_Y_GYRO_OUT] * GYRO_SCALER;
	imu_data.gyro.z = (int16_t)buffer[BURST_READ_Z_GYRO_OUT] * GYRO_SCALER;

	return imu_data;
}
