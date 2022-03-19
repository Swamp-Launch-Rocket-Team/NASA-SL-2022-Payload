#include <iostream>
#include <chrono>
#include <ctime> 
#include <fstream>

#include "IMUlibrary_ADIS16470PCBZ.h"
#include "CameraModule.h"

bool launchDetected() {
    imu_data_t data = IMU_read();

    if (data.accel.x > 20)
        return true;

    if (data.accel.y > 20)
        return true;

    if (data.accel.z > 20)
        return true;

    return false;
}

bool landingDetected() {
    imu_data_t data = IMU_read();

    if (data.accel.x < 1)
        return true;

    if (data.accel.y < 1)
        return true;

    if (data.accel.z < 1)
        return true;

    return false;
}

void writeIMUToFile()
{
    imu_data_t data = IMU_read();
    
    std::ofstream file("imu_data.csv", std::ios::app);

    std::string accel = "accel, " + std::to_string(data.accel.x) + ", " + std::to_string(data.accel.y) + ", " + std::to_string(data.accel.z); 
    std::string gyro = "gyro, " + std::to_string(data.gyro.x) + ", " + std::to_string(data.gyro.y) + ", " + std::to_string(data.gyro.z); 

    file << accel.c_str();
    file << "\n";
    file << gyro.c_str();
    file << "\n";

    file.close();
}

int main()
{
    //Variables
    bool launched = false;
    bool landed = false;
    int numImages = 1;
    
    //
    while (!launched) {
        if (launchDetected())
        {
            setup("1600x1200");
            launched = true;
        }
        
        writeIMUToFile();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    while (!landed) {
        if (landingDetected())
            landed = true;

        writeIMUToFile();
        
        std::string test = std::to_string(numImages) + ".jpg";
        domain(0, test.c_str());
        //std::string command = " -c " + std::to_string(numImages) + ".jpg 1600x1200";
        //system(command.c_str());
        numImages++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    //Put in transmission code
}