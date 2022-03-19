/*-----------------------------------------

//Update History:
//2016/06/13 	V1.1	by Lee	add support for burst mode

--------------------------------------*/
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>

#include "CameraModule.h"
#define OV5642_CHIPID_HIGH 0x300a
#define OV5642_CHIPID_LOW 0x300b
#define OV5642_MAX_FIFO_SIZE		0x7FFFFF		//8MByte
#define BUF_SIZE 4096
#define CAM1_CS 22

#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
uint8_t buf[BUF_SIZE];
bool is_header = false;

ArduCAM myCAM(OV5642,CAM1_CS);
void setup(const char* res)
{
    uint8_t vid,pid;
    uint8_t temp;
    wiring_init();
    pinMode(CAM1_CS, OUTPUT);
    
    /*printf("Forcing CS low\n");
    digitalWrite(CAM1_CS, LOW);//Testing CS_LOW only
    printf("Forcing CS low done\n");
    while(true){}; //Testing CS_LOW only*/
    
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    //printf("temp=%x\n",temp);
  if(temp != 0x55) {
        printf("SPI interface error!\n");
        exit(EXIT_FAILURE);
	//myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    //temp = myCAM.read_reg(ARDUCHIP_TEST1);
    }  
    // Change MCU mode
    myCAM.write_reg(ARDUCHIP_MODE, 0x00); 
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    printf("Can't find OV5642 module!");
     else
     printf("OV5642 detected.\n");
    myCAM.set_format(JPEG);
    myCAM.InitCAM();

    if (strcmp(res, "320x240") == 0) myCAM.OV5642_set_JPEG_size(OV5642_320x240);
      else if (strcmp(res, "640x480") == 0) myCAM.OV5642_set_JPEG_size(OV5642_640x480);
      else if (strcmp(res, "1280x960") == 0) myCAM.OV5642_set_JPEG_size(OV5642_1280x960);
      else if (strcmp(res, "1600x1200") == 0) myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);
      else if (strcmp(res, "2048x1536") == 0) myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);
      else if (strcmp(res, "2592x1944") == 0) myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);
      else {
      printf("Unknown resolution %s\n", res);
      exit(EXIT_FAILURE);
      }
      printf("Changed resolution1 to %s\n", res);
      sleep(1); 
}
void domain(int argc, const char* name)
{
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);		//VSYNC is active HIGH   	  
    // Flush the FIFO
  myCAM.flush_fifo();    
  // Clear the capture done flag
  myCAM.clear_fifo_flag();
  // Start capture
  printf("Start capture\n");  
  myCAM.start_capture();
  while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK)){}
  printf("CAM Capture Done\n");
          
    // Open the new file
  std::thread t1(saveImg, name, myCAM);
  t1.join();
}

void saveImg(const char* name, ArduCAM saveCam)
{
  uint8_t temp = 0, temp_last = 0;
  
  FILE *fp1 = fopen(name, "w+");   
  if (!fp1) {
      printf("Error: could not open %s\n", name);
      exit(EXIT_FAILURE);
  }
    
  printf("Reading FIFO and saving IMG\n");    
  size_t length = saveCam.read_fifo_length();
  printf("The length is %d\r\n", length);
  if (length >= OV5642_MAX_FIFO_SIZE){
    printf("Over size.");
    exit(EXIT_FAILURE);
  }else if (length == 0 ){
    printf("Size is 0.");
    exit(EXIT_FAILURE);
  } 
  int32_t i=0;
  saveCam.CS_LOW();  //Set CS low       
  saveCam.set_fifo_burst();
  
  while ( length-- )
  {
    temp_last = temp;
    temp =  saveCam.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
        buf[i++] = temp;  //save the last  0XD9     
        //Write the remain bytes in the buffer
        saveCam.CS_HIGH();
        fwrite(buf, i, 1, fp1);    
        //Close the file
        fclose(fp1); 
        printf("IMG save OK !\n"); 
        is_header = false;
        i = 0;
    }  
    if (is_header == true)
    { 
        //Write image data to buffer if not full
        if (i < BUF_SIZE)
        buf[i++] = temp;
        else
        {
          //Write BUF_SIZE bytes image data to file
          saveCam.CS_HIGH();
          fwrite(buf, BUF_SIZE, 1, fp1);
          i = 0;
          buf[i++] = temp;
          saveCam.CS_LOW();
          saveCam.set_fifo_burst();
        }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  } 
}
