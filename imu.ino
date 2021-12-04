#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3-5V DC
   Connect GROUND to common ground

   History
   =======
   2015/MAR/03  - First release (KTOWN)
*/

/* Set the delay between fresh samples */
uint16_t BNO055_SAMPLERATE_DELAY_MS = 100;

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

void setup(void)
{
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  
  File myFile = SD.open("debug.txt", FILE_WRITE);
  myFile.println("Orientation Sensor Test"); myFile.println("");

  /* Initialise the sensor */
  if (!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    myFile.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  myFile.close();

  delay(1000);
}

void loop(void)
{
  //could add VECTOR_ACCELEROMETER, VECTOR_MAGNETOMETER,VECTOR_GRAVITY...
  sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
  bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
  bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
  bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);

  File myFile = SD.open("data.csv", FILE_WRITE);
  myFile.println(millis());
  Serial.println(millis());
  
  printEvent(&orientationData, myFile);
  printEvent(&angVelocityData, myFile);
  printEvent(&linearAccelData, myFile);
  printEvent(&magnetometerData, myFile);
  printEvent(&accelerometerData, myFile);
  printEvent(&gravityData, myFile);

  myFile.close();

  int8_t boardTemp = bno.getTemp();
  //myFile.println();
  //myFile.print(F("temperature: "));
  //myFile.println(boardTemp);

  uint8_t system, gyro, accel, mag = 0;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  //myFile.println();
  //myFile.print("Calibration: Sys=");
  //myFile.print(system);
  //myFile.print(" Gyro=");
  //myFile.print(gyro);
  //myFile.print(" Accel=");
  //myFile.print(accel);
  // myFile.print(" Mag=");
  // myFile.println(mag);

//  myFile.println("--");
//  
//  myFile.println();
//  myFile.print(F("temperature: "));
//  myFile.println(boardTemp);
//  
//  myFile.println();
//  myFile.print("Calibration: Sys=");
//  myFile.print(system);
//  myFile.print(" Gyro=");
//  myFile.print(gyro);
//  myFile.print(" Accel=");
//  myFile.print(accel);
//  myFile.print(" Mag=");
//  myFile.println(mag);
//
//  myFile.println("--");
//
//  myFile.close();

  delay(BNO055_SAMPLERATE_DELAY_MS);
}

void printEvent(sensors_event_t* event, File myFile) {
  //File myFile = SD.open("data.csv", FILE_WRITE);
  double x = -1000000, y = -1000000 , z = -1000000; //dumb values, easy to spot problem
  if (event->type == SENSOR_TYPE_ACCELEROMETER) {
    myFile.print("Accl,");
    Serial.print("Accl,");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_ORIENTATION) {
    myFile.print("Orient,");
    Serial.print("Orient,");
    x = event->orientation.x;
    y = event->orientation.y;
    z = event->orientation.z;
  }
  else if (event->type == SENSOR_TYPE_MAGNETIC_FIELD) {
    myFile.print("Mag,");
    Serial.print("Mag,");
    x = event->magnetic.x;
    y = event->magnetic.y;
    z = event->magnetic.z;
  }
  else if (event->type == SENSOR_TYPE_GYROSCOPE) {
    myFile.print("Gyro,");
    Serial.print("Gyro,");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_ROTATION_VECTOR) {
    myFile.print("Rot,");
    Serial.print("Rot,");
    x = event->gyro.x;
    y = event->gyro.y;
    z = event->gyro.z;
  }
  else if (event->type == SENSOR_TYPE_LINEAR_ACCELERATION) {
    myFile.print("Linear,");
    Serial.print("Linear,");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else if (event->type == SENSOR_TYPE_GRAVITY) {
    myFile.print("Gravity,");
    Serial.print("Gravity,");
    x = event->acceleration.x;
    y = event->acceleration.y;
    z = event->acceleration.z;
  }
  else {
    myFile.print("Unk,");
    Serial.print("Unk,");
  }

  //myFile.print("\tx= ");
  Serial.print(x);
  Serial.print(",");
  //myFile.print(" |\ty= ");
  Serial.print(y);
  Serial.print(",");
  //myFile.print(" |\tz= ");
  Serial.print(z);
  Serial.println();
  
  //myFile.print("\tx= ");
  myFile.print(x);
  myFile.print(",");
  //myFile.print(" |\ty= ");
  myFile.print(y);
  myFile.print(",");
  //myFile.print(" |\tz= ");
  myFile.print(z);
  myFile.println();
}
