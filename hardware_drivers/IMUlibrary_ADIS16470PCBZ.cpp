#include "IMUlibrary_ADIS16470PCBZ.h"

using namespace std;

#define RESET 3
#define CHIP_SELECT 4
#define DATA_READY 5
#define SYNC 6

#define CHANNEL 0
#define BAUDRATE 976000 // Max SCLK is 1 MHz when using burst reads
// #define WRITE 1
// #define READ 0

#define MSC_CTRL_ADDR 0x60
#define GLOB_CMD_ADDR 0x68

#define BURST_READ_DIN 0x6800
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

const float ACCEL_SCALER = 0.00122070312; // 40 / 2^15
const float GYRO_SCALER = 0.0610351562; // 2000 / 2^15

// static uint16_t SPI_RW_buffer_gen(uint8_t RW, uint8_t addr, uint8_t data)
// {
// 	unsigned short buffer;
// 	buffer = RW << 15;
// 	buffer |= addr << 8;
// 	buffer |= data;
// 	return buffer;
// }

uint16_t IMU_SPI_read(uint8_t addr)
{
	digitalWrite(CHIP_SELECT, 0);
	uint8_t buffer[4];
	// buffer[0]: R/W, A6-A0
	// buffer[1]: D7-D0, don't care on read
	buffer[0] = ~(1 << 7) & addr;
	wiringPiSPIDataRW(CHANNEL, buffer, 4);
	return buffer[2] << 7 | buffer[3];
	digitalWrite(CHIP_SELECT, 1);
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

imu_data_t IMU_read(){
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
	imu_data.accel.x = accel_format((int16_t)buffer[BURST_READ_X_ACCL_OUT]);
	imu_data.accel.y = accel_format((int16_t)buffer[BURST_READ_Y_ACCL_OUT]);
	imu_data.accel.z = accel_format((int16_t)buffer[BURST_READ_Z_ACCL_OUT]);
	imu_data.gyro.x = gyro_format((int16_t)buffer[BURST_READ_X_GYRO_OUT]);
	imu_data.gyro.y = gyro_format((int16_t)buffer[BURST_READ_Y_GYRO_OUT]);
	imu_data.gyro.z = gyro_format((int16_t)buffer[BURST_READ_Z_GYRO_OUT]);

	return imu_data;
}

static inline float accel_format(int16_t data)
{
	return data * ACCEL_SCALER;
}

static inline float gyro_format(int16_t data)
{
	return data * GYRO_SCALER;
}
