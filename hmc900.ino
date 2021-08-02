/*


  * SDI - to digital pin 11 (MOSI pin)
  * SDO - to digital pin 12 (MISO pin)
  * CLK - to digital pin 13 (SCK pin)


*/


// inslude the SPI library:
#include <HMC900.h>


const int slaveSelectPin = 10;

HMC900 hmc900<true>(slaveSelectPin);




// set pin 10 as the slave select for the digital pot:

void setup() {
  // set the slaveSelectPin as an output:

  if (!hmc900.begin()){
    Serial.println("HMC900's initialisation failed");
  }
 
  delay(1000);
  hmc900.writeCoaseBandwidthCode(CBANDWHDTH5);
  Serial.println(hmc900.readCoaseBandwidthCode(),BIN); //Aswer should be "1"
  hmc900.writeCoaseBandwidthCode(CBANDWHDTH20);
  Serial.println(hmc900.readCoaseBandwidthCode(),BIN); //Aswer should be "101"
  //setCalibrationClockFrequency(20);
  
}

void loop() {
  delay(1000);
}


