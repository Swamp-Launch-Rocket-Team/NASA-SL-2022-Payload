all: testIMU

testIMU: testIMU.cpp ../IMUlibrary_ADIS16470PCBZ.cpp ../IMUlibrary_ADIS16470PCBZ.h
	c++ -std=c++11 -Wall -g -o testIMU testIMU.cpp ../IMUlibrary_ADIS16470PCBZ.cpp -lwiringPi

clean:
	rm -f testIMU