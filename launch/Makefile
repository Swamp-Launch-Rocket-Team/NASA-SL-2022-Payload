main:
	g++ main.cpp IMUlibrary_ADIS16470PCBZ.cpp CameraModule.cpp ArduCAM.cpp arducam_arch_raspberrypi.c \
	-I/usr/local/include \
	-L/usr/local/lib \
	-lwiringPi \
	-li2c \
	-lpthread \
	-o main.out



all: testIMU

testIMU: testIMU.cpp ../IMUlibrary_ADIS16470PCBZ.cpp ../IMUlibrary_ADIS16470PCBZ.h
	c++ -std=c++11 -g -o testIMU testIMU.cpp ../IMUlibrary_ADIS16470PCBZ.cpp -lwiringPi

clean:
	rm -f testIMU