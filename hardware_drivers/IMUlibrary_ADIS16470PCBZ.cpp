#include <iostream>
#include <errno.h>
#include <wiringPiSPI.h>
#include <unistd.h>

using namespace std;

#define CHANNEL 0
#define BAUDRATE 1900000
#define WRITE 1
#define READ 0

void IMU_calibrate();
unsigned short IMU_read_XXX();
unsigned short SPI_RW_buffer_gen(unsigned char RW, unsigned char addr, unsigned char data);
void IMU_setup();

struct accel_t
{
	float x, y, z;
};

struct gyro_t
{
	float x, y, z;
};

struct imu_data_t
{
	gyro_t gyro;
	accel_t accel;
};

void IMU_setup()
{
int fd, result;
   unsigned short buffer;

   cout << "Initializing" << endl ;

   // Configure the interface.
   // CHANNEL insicates chip select,
   // 500000 indicates bus speed.
   fd = wiringPiSPISetup(CHANNEL, BAUDRATE);

   cout << "Init result: " << fd << endl;

	/*Enabled linear g compensation, enabled point of percussion alignment,
	Sync(CLK) set to default(internal clock), SYNC set to rising edge, DR polarity 
	is set to active high*/
	
	buffer= SPI_RW_buffer_gen(WRITE,0x60,0xC3); //0xE0C3 
	wiringPiSPIDataRW(CHANNEL, buffer, 2);
	
	//Initial Bias Correction
	IMU_calibrate();
	
}


unsigned short SPI_RW_buffer_gen(unsigned char RW, unsigned char addr, unsigned char data)
{

	unsigned short buffer;
	buffer = RW << 15;
	buffer |= addr << 8;
	buffer |= data;
	return buffer;
}

void IMU_calibrate()
{
	unsigned short buffer;
	buffer = SPI_RW_buffer_gen(WRITE, 0x68, 0x01); //Sets the bias correction value to true
	wiringPiSPIDataRW(CHANNEL, buffer, 2);
}
unsigned short IMU_read_XXX(){
	
}
