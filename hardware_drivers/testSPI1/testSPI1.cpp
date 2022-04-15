extern "C"{
	#include <i2c/smbus.h>
	#include <linux/i2c.h>
}
#include <linux/i2c-dev.h>
#include <fcntl.h> //adding this and ioctl
#include <sys/ioctl.h>
#include <wiringPiSPI.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <bcm2835.h>
#include <cstdio>
//#include "arducam_arch_raspberrypi.h"

#define	SPI_ARDUCAM_SPEED	1000000
#define	SPI_ARDUCAM		      0

static int FD;

bool wiring_init(void)
{
	printf("1");
	//wiringPiSetup(); //need this
	printf("2");
	//int spi = wiringPiSPISetup(SPI_ARDUCAM, SPI_ARDUCAM_SPEED);
	int spi = bcm2835_aux_spi_begin();
	printf("3");
	uint16_t clkdiv = bcm2835_aux_spi_CalcClockDivider(SPI_ARDUCAM_SPEED);
	printf("4");
	bcm2835_aux_spi_setClockDivider(clkdiv);
	printf("5");
	return spi != -1;
}
bool arducam_i2c_init(uint8_t sensor_addr)
{
	FD = wiringPiI2CSetup(sensor_addr);
	return FD != -1;
}
void arducam_delay_ms(uint32_t delay)
{
	usleep(1000*delay);	
}

void arducam_spi_write(uint8_t address, uint8_t value)
{
	char spiData [2] ;
	spiData [0] = address ;
	spiData [1] = value ; 
	//wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 2) ;
	bcm2835_aux_spi_writenb(&spiData[0], 2);
}

uint8_t arducam_spi_read(uint8_t address)
{
	uint8_t spiData[2];
	spiData[0] = address ;
	spiData[1] = 0x00 ;
  	//wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 2) ;
	bcm2835_aux_spi_transfern((char*)&spiData[0], 2);
  	return spiData[1];
}

void arducam_spi_transfers(uint8_t *buf, uint32_t size)
{
	//wiringPiSPIDataRW (SPI_ARDUCAM, buf, size) ;
	bcm2835_aux_spi_transfern((char*)buf, size);
}

uint8_t arducam_spi_transfer(uint8_t data)
{
	uint8_t spiData [1] ;
	spiData [0] = data ;
	//wiringPiSPIDataRW (SPI_ARDUCAM, spiData, 1) ;
	bcm2835_aux_spi_transfern((char*)&spiData[0], 1);
	return spiData [0];
}
int main(void)
{
	printf("a");
	wiring_init();
	printf("b");
	while(1)
	{
		printf("WHY");
		arducam_spi_write(0, 0x69);
		printf("send help");
	}
}
