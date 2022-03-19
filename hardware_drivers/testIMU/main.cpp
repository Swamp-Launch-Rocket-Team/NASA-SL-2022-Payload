#include <iostream>
#include <chrono>
#include <ctime>


#

bool launchDetected() {
    return true;
}

bool landingDetected() {
    return true;
}

int main()
{
    //Variables
    bool launched = false;
    bool landed = false;
    //
    while (!launched) {
        if (launchDetected())
            launched = true;
    }
    

    while (!landed) {
        if (landingDetected())
            landed = true;
        
        system("/home/pi/attempt4/Arduino/ArduCAM/examples/RaspberryPi/ov5642_capture -c test.jpg 1920x1080");
    }

    //Put in transmission code
}