#include <iostream>
#include <wiringPi.h>
#include "../IMUlibrary_ADIS16470PCBZ.h"

using namespace std;

int main()
{
    // Must be called at the start of program
    wiringPiSetup();

    IMU_setup();

    timespec req, rem;
	req.tv_nsec = 0;
	req.tv_sec = 3;
	while(nanosleep(&req, &rem) == -1)
		req = rem;

    // imu_data_t data = IMU_read();

    // cout << "\nACCEL" << endl;
    // cout << "\tX: " << data.accel.x << endl;
    // cout << "\tY: " << data.accel.y << endl;
    // cout << "\tZ: " << data.accel.z << endl;
    // cout << "GYRO" << endl;
    // cout << "\tX: " << data.gyro.x << endl;
    // cout << "\tY: " << data.gyro.y << endl;
    // cout << "\tZ: " << data.gyro.z << endl;

    uint16_t z_accel = IMU_SPI_read(0x1A);

    cout << z_accel << endl;

    return 0;
}