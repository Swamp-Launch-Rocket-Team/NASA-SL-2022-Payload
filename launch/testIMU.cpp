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

    while(1)
    {
        // uint16_t prodID = IMU_SPI_read(0x72);
        // printf("0x%X\n", prodID);
        imu_data_t data = IMU_read();

        cout << "\nACCEL" << endl;
        cout << "\tX: " << data.accel.x << endl;
        cout << "\tY: " << data.accel.y << endl;
        cout << "\tZ: " << data.accel.z << endl;
        cout << "GYRO" << endl;
        cout << "\tX: " << data.gyro.x << endl;
        cout << "\tY: " << data.gyro.y << endl;
        cout << "\tZ: " << data.gyro.z << endl;

        std::string s;
        std::getline(cin, s);
    }

    return 0;
}