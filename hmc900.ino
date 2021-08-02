/*
  Digital Pot Control

  This example controls an Analog Devices AD5206 digital potentiometer.
  The AD5206 has 6 potentiometer channels. Each channel's pins are labeled
  A - connect this to voltage
  W - this is the pot's wiper, which changes when you set it
  B - connect this to ground.

 The AD5206 is SPI-compatible,and to command it, you send two bytes,
 one with the channel number (0 - 5) and one with the resistance value for the
 channel (0 - 255).

 The circuit:
  * All A pins  of AD5206 connected to +5V
  * All B pins of AD5206 connected to ground
  * An LED and a 220-ohm resisor in series connected from each W pin to ground
  * CS - to digital pin 10  (SS pin)
  * SDI - to digital pin 11 (MOSI pin)
  * CLK - to digital pin 13 (SCK pin)

 created 10 Aug 2010
 by Tom Igoe

 Thanks to Heather Dewey-Hagborg for the original tutorial, 2005

NOT!!!

repurposed to try controlling HMC900

*/


// inslude the SPI library:
#include "HMC900.h"


/*
#define CALREADREG 0x09 // calibration measurement address
#define CALFREQREG 0x05 // calibration frequency address
#define COARSEREG 0x02 // coase setting address
#define FINEREG 0x03 // fine setting address
*/

const int slaveSelectPin = 10;

HMC900 hmc900(slaveSelectPin);




// set pin 10 as the slave select for the digital pot:

void setup() {
  // set the slaveSelectPin as an output:
  Serial.begin(115200);

  if (!hmc900.begin()){
    Serial.println("HMC900's initialisation failed");
  }
  hmc900.writeRegister(1,1);
 
  delay(1000);
  writeCoaseBandwidthCode(CBANDWHDTH20);
  //setCalibrationClockFrequency(20);
  
}

void loop() {
  //readRegister(OTPVALUESREG);
  //readRegister(SETTINGSREG);
}

//
void filterBandwidthSetting(uint32_t calFreqMhz){
  uint32_t cal_count = calibration(calFreqMhz);
  setBandwidthCodes(cal_count);
  enableBit(true, ENABLEREG, 0b1000);
}

//
uint32_t calibration(uint32_t calFreqMhz) {
  setCalibrationClockFrequency(calFreqMhz);
  enableBit(true, ENABLEREG, 0b10); //Enable RC Calibration circuit

  Serial.print("If you forgot to plug connecte and launch the signal generator for the calibration at "); 
  Serial.print(calFreqMhz); 
  Serial.println(" Mhz... Well, too late."); 
  enableBit(true, CALBISTRSTROBEREG, 1); //write anithing in "Calibration/RC-BIST Strobe" register starts a calibration cycle.
  delay(15); // time for the calibration > periode*2^18

  uint32_t calResult = 0b10000;
  Serial.println("cal: ");
  while((calResult & 0b10000) > 0){
    calResult = readRegister(CALSTATUSREG);
    calResult = (calResult >> 8);
    Serial.println(calResult, BIN);
    delay(1);
  }
  return readRegister(CALCOUNTREG);
}

//
uint32_t builtInSelfTest(uint32_t calFreqMhz){

  softReset();

  setCalibrationClockFrequency(calFreqMhz);

  enableBistMode(true);

  Serial.print("If you forgot to plug connecte and launch the signal generator for the calibration at "); 
  Serial.print(calFreqMhz); 
  Serial.println(" Mhz... Well, too late."); 
  enableBit(true, CALBISTRSTROBEREG, 1); //write anithing in "Calibration/RC-BIST Strobe" register starts a calibration cycle.
  delay(15); // time for the calibration > periode*2^18
  
  uint32_t bistrResult = 0b10000000000000000;
  Serial.println("bistrResult: ");
  while((bistrResult & 0b10000000000000000) > 0){
    bistrResult = readRegister(RCBISTOUTREG);
    bistrResult = (bistrResult >> 8);
    Serial.println(bistrResult, BIN);
    delay(1);
  }
  return bistrResult;
}

//
void writeCoaseBandwidthCode(uint32_t cbandwidthcode){
  
  uint32_t settingRegister = readRegister(SETTINGSREG);
  if (((settingRegister >> 3) &0b11111) == SETTINGSREG)
  {
    //Serial.println("reading SETTINGSREG succesfull"); 

    settingRegister = settingRegister>>8; //pour enlever l'entete
    settingRegister = settingRegister & 0b111111111111110000111111;
    settingRegister |= (cbandwidthcode << 6);
    writeRegister(SETTINGSREG, settingRegister);
    
    settingRegister = readRegister(SETTINGSREG);
    settingRegister = settingRegister>>8; //pour enlever l'entete
    if (((settingRegister >> 6) & 0b1111) == cbandwidthcode)  
    {
      Serial.println("Setting of the coarse bandwidth code successfull");       
    }else {
      Serial.println("Failed setting of the coarse bandwidth code");      
    }  
  }else {
    Serial.println("failed reading of SETTINGSREG"); 
  }
}

//
void writeFineBandwidthCode(uint32_t fbandwidthcode){
  
  uint32_t calibrationRegister = readRegister(CALREG);
  if (((calibrationRegister >> 3) &0b11111) == CALREG)
  {
    //Serial.println("reading CALREG succesfull"); 

    writeRegister(CALREG, fbandwidthcode);
    
    calibrationRegister = readRegister(CALREG);
    calibrationRegister = calibrationRegister>>8; //pour enlever l'entete
    if (calibrationRegister == fbandwidthcode)  
    {
      Serial.println("Setting of the fine bandwidth code successfull");       
    }else {
      Serial.println("Failed setting of the fine bandwidth code");      
    }  
  }else {
    Serial.println("failed reading of CALREG"); 
  }
}

//
void setBandwidthCodes(float cal_count){
  float ctune = cal_count/10370000;
  float fBW_norm_coarse = FREQWANTED * ctune;
  Serial.print(fBW_norm_coarse);
  float fBW_norm_coarse_typ = 0.0;
  char coarse_bandwidth_code = 0;
  if (fBW_norm_coarse >= 2.764 && fBW_norm_coarse <= 60.50){

    if (fBW_norm_coarse >= 2.764 && fBW_norm_coarse < 4.235){
      fBW_norm_coarse_typ = 3.5;    
      coarse_bandwidth_code = CBANDWHDTH3;
    } else if (fBW_norm_coarse > 3.948 && fBW_norm_coarse < 6.050){
        fBW_norm_coarse_typ = 5.0;   
        coarse_bandwidth_code = CBANDWHDTH5;
    } else if (fBW_norm_coarse > 5.527 && fBW_norm_coarse < 8.470){
        fBW_norm_coarse_typ = 7.0;   
        coarse_bandwidth_code = CBANDWHDTH7;
    } else if (fBW_norm_coarse > 7.896 && fBW_norm_coarse < 12.10){
        fBW_norm_coarse_typ = 10.0;   
        coarse_bandwidth_code = CBANDWHDTH10;
    } else if (fBW_norm_coarse > 11.055 && fBW_norm_coarse < 16.94){
        fBW_norm_coarse_typ = 14.0;   
        coarse_bandwidth_code = CBANDWHDTH14;
    } else if (fBW_norm_coarse > 15.792 && fBW_norm_coarse < 24.20){
        fBW_norm_coarse_typ = 20.0;   
        coarse_bandwidth_code = CBANDWHDTH20;
    } else if (fBW_norm_coarse > 22.109 && fBW_norm_coarse < 33.88){
        fBW_norm_coarse_typ = 28.0;   
        coarse_bandwidth_code = CBANDWHDTH28;
    } else if (fBW_norm_coarse > 27.637 && fBW_norm_coarse < 42.351){
        fBW_norm_coarse_typ = 35.0;   
        coarse_bandwidth_code = CBANDWHDTH35;
    } else if (fBW_norm_coarse > 39.480 && fBW_norm_coarse <= 60.50){
        fBW_norm_coarse_typ = 50.0;   
        coarse_bandwidth_code = CBANDWHDTH50;
    }
    Serial.print("coarse_bandwidth_code: ");
    Serial.println(coarse_bandwidth_code);
    writeCoaseBandwidthCode(coarse_bandwidth_code);
  } else {
      Serial.print("fBW_norm_coarse not within parameters: ");
      Serial.println(fBW_norm_coarse);
  }

  float fine_tune_ratio = fBW_norm_coarse / fBW_norm_coarse_typ;
  float fine_tune_ratio_typ = 0.0;
  char fine_bandwidth_code = 0;
  Serial.print("fine_tune_ratio: ");
  Serial.println(fine_tune_ratio);

  if (fine_tune_ratio >= 0.790 && fine_tune_ratio <= 1.210){
    if (fine_tune_ratio >= 0.790 && fine_tune_ratio < 0.818){
        fine_tune_ratio_typ = 0.803;    
        fine_bandwidth_code = FBANDWHDTH0803;
    } else if (fine_tune_ratio >= 0.818 && fine_tune_ratio < 0.846){
        fine_tune_ratio_typ = 0.832;   
        fine_bandwidth_code = FBANDWHDTH0832;
    } else if (fine_tune_ratio >= 0.846 && fine_tune_ratio < 0.878){
        fine_tune_ratio_typ = 0.862;   
        fine_bandwidth_code = FBANDWHDTH0862;
    } else if (fine_tune_ratio >= 0.878 && fine_tune_ratio < 0.909){
        fine_tune_ratio_typ = 0.893;   
        fine_bandwidth_code = FBANDWHDTH0893;
    } else if (fine_tune_ratio >= 0.909 && fine_tune_ratio < 0.943){
        fine_tune_ratio_typ = 0.926;   
        fine_bandwidth_code = FBANDWHDTH0926;
    } else if (fine_tune_ratio >= 0.943 && fine_tune_ratio < 0.976){
        fine_tune_ratio_typ = 0.959;   
        fine_bandwidth_code = FBANDWHDTH0959;
    } else if (fine_tune_ratio >= 0.976 && fine_tune_ratio < 1.012){
        fine_tune_ratio_typ = 0.994;   
        fine_bandwidth_code = FBANDWHDTH0994;
    } else if (fine_tune_ratio >= 1.012 && fine_tune_ratio < 1.048){
        fine_tune_ratio_typ = 1.030;   
        fine_bandwidth_code = FBANDWHDTH1030;
    } else if (fine_tune_ratio >= 1.048 && fine_tune_ratio < 1.087){
        fine_tune_ratio_typ = 1.068;   
        fine_bandwidth_code = FBANDWHDTH1068;
    } else if (fine_tune_ratio >= 1.087 && fine_tune_ratio < 1.128){
        fine_tune_ratio_typ = 1.107;   
        fine_bandwidth_code = FBANDWHDTH1107;
    } else if (fine_tune_ratio >= 1.128 && fine_tune_ratio < 1.169){
        fine_tune_ratio_typ = 1.148;   
        fine_bandwidth_code = FBANDWHDTH1148;
    } else if (fine_tune_ratio >= 1.169 && fine_tune_ratio <= 1.210){
        fine_tune_ratio_typ = 1.189;   
        fine_bandwidth_code = FBANDWHDTH1189;
    }
    Serial.print("fine_bandwidth_code: ");
    Serial.println(fine_bandwidth_code);
    writeFineBandwidthCode(fine_bandwidth_code);
  } else {
      Serial.print("fine_tune_ratio not within parameters: ");
      Serial.println(fine_tune_ratio);
  }

}

//
void enableFilterQ(bool en){
  uint32_t enableRegister = readRegister(ENABLEREG);
  if (((enableRegister >> 3) &0b11111) == ENABLEREG)
  {
    Serial.println("reading succesfull"); 

    enableRegister = enableRegister>>8; //pour enlever l'entete
 
      uint32_t qFilterBit = 0b1000; //le bit correspondant a "filter_Q_enable"

      if (((enableRegister) & qFilterBit)>0) // si Q filter est ENABLE
      {
        if (!en) // si on veut le desactiver
        {
          enableRegister = enableRegister ^ qFilterBit;
          writeRegister(ENABLEREG, enableRegister);
          enableRegister = readRegister(ENABLEREG);
          enableRegister = enableRegister >> 8;
          if (((enableRegister) & qFilterBit) == 0)
          {
            Serial.println("Q filter is now disabled"); 
          }else{
            Serial.println("Q filter disabling failed"); 
          }
        }
        else
        {
          Serial.println("Q filter is already enable"); 
        }
      }else //si Q filter est DISABLE
      {
        if (en)  // si on veut l'activer
        {
          enableRegister = enableRegister | qFilterBit;
          writeRegister(ENABLEREG, enableRegister);
          enableRegister = readRegister(ENABLEREG);
          enableRegister = enableRegister>>8;
          if (((enableRegister) & qFilterBit) > 0)
          {
            Serial.println("Q filter is now enable"); 
          }else{
            Serial.println("Q filter enabling failed"); 
          }
        }
        else{
          Serial.println("Q filter is already disable"); 
        }
        
        
      }    
  }else{
  Serial.print("reading failed: "); 
  Serial.println(enableRegister); 
  }
  
}

//
void enableDoubler(bool en){
  uint32_t enableRegister = readRegister(ENABLEREG);
  if (((enableRegister >> 3) &0b11111) == ENABLEREG)
  {
    //Serial.println("reading of ENABLEREG succesfull"); 

    enableRegister = enableRegister>>8; //pour enlever l'entete

      uint32_t doublerBit = 0b100000; //le bit correspondant a "doubler_enable"

      if (((enableRegister) & doublerBit)>0) // si Doubler est ENABLE
      {
        if (!en) // si on veut le desactiver
        {
          enableRegister = enableRegister ^ doublerBit;
          writeRegister(ENABLEREG, enableRegister);
          enableRegister = readRegister(ENABLEREG);
          enableRegister = enableRegister >> 8;
          if (((enableRegister) & doublerBit) == 0)
          {
            Serial.println("Doubler is now disabled"); 
            Serial.println(""); 
          }else{
            Serial.println("Doubler disabling failed"); 
            Serial.println(""); 
          }
        }
        else
        {
          Serial.println("Doubler is already enable"); 
          Serial.println(""); 
        }
      }else //si Doubler est DISABLE
      {
        if (en)  // si on veut l'activer
        {
          enableRegister = enableRegister | doublerBit;
          writeRegister(ENABLEREG, enableRegister);
          enableRegister = readRegister(ENABLEREG);
          enableRegister = enableRegister>>8;
          if (((enableRegister) & doublerBit) > 0)
          {
            Serial.println("Doubler is now enable"); 
            Serial.println(""); 
          }else{
            Serial.println("Doubler enabling failed"); 
            Serial.println(""); 
          }
        }
        else{
          Serial.println("Doubler is already disable"); 
          Serial.println(""); 
        }
        
        
      }    
  }else{
    Serial.print("reading of ENABLEREG failed: "); 
    Serial.println(enableRegister); 
  }
  
}

//
void enableBistMode(bool en){
  uint32_t rcBistRegister = readRegister(RCBISTENABLEREG);
  if (((rcBistRegister >> 3) &0b11111) == RCBISTENABLEREG)
  {
    //Serial.println("reading of RCBISTENABLEREG succesfull"); 

    rcBistRegister = rcBistRegister>>8; //pour enlever l'entete

      uint32_t bistBit = 0b1; //le bit correspondant a "enable_RCBIST_mode"

      if (((rcBistRegister) & bistBit)>0) // si RC-BIST est ENABLE
      {
        if (!en) // si on veut le desactiver
        {
          rcBistRegister = rcBistRegister ^ bistBit;
          writeRegister(RCBISTENABLEREG, rcBistRegister);
          rcBistRegister = readRegister(RCBISTENABLEREG);
          rcBistRegister = rcBistRegister >> 8;
          if (((rcBistRegister) & bistBit) == 0)
          {
            Serial.println("RC-BIST is now disabled"); 
            Serial.println(""); 
          }else{
            Serial.println("RC-BIST disabling failed"); 
            Serial.println(""); 
          }
        }
        else
        {
          Serial.println("RC-BIST is already enable"); 
          Serial.println(""); 
        }
      }else //si RC-BIST est DISABLE
      {
        if (en)  // si on veut l'activer
        {
          rcBistRegister = rcBistRegister | bistBit;
          writeRegister(RCBISTENABLEREG, rcBistRegister);
          rcBistRegister = readRegister(RCBISTENABLEREG);
          rcBistRegister = rcBistRegister>>8;
          if (((rcBistRegister) & bistBit) > 0)
          {
            Serial.println("RC-BIST is now enable"); 
            Serial.println(""); 
          }else{
            Serial.println("RC-BIST enabling failed"); 
            Serial.println(""); 
          }
        }
        else{
          Serial.println("RC-BIST is already disable"); 
          Serial.println(""); 
        }
        
        
      }    
  }else{
    Serial.print("reading of RCBISTENABLEREG failed: "); 
    Serial.println(rcBistRegister); 
  }
}

//
bool enableBit(bool toEnable, byte registerOfBit, uint32_t bitToEnable){
  /*
  toEnable:
  if yes toEnable=true
  if you wich to disable toEnable=false

  bitToEnable:
  if it is the third bit of the register  bitToEnable=0b100
  if fifth 0b10000
  */
  
  uint32_t registerBits = readRegister(registerOfBit);
  if (((registerBits >> 3) &0b11111) == registerOfBit)
  {
    //Serial.println("reading of register succesfull"); 

    registerBits = registerBits >> 8; //pour enlever l'entete

      if (((registerBits) & bitToEnable)>0) // si le bit est ENABLE
      {
        if (!toEnable) // si on veut le desactiver
        {
          registerBits = registerBits ^ bitToEnable;
          writeRegister(registerOfBit, registerBits);

          registerBits = readRegister(registerOfBit); //verifier le resultat de l'action
          registerBits = registerBits >> 8;
          if (((registerBits) & bitToEnable) == 0)
          {
            Serial.println("bit is now disabled"); 
            Serial.println(""); 
            return true;
          }else{
            Serial.println("bit disabling failed"); 
            Serial.println(""); 
            return false;
          }
        }
        else
        {
          Serial.println("bit is already enable"); 
          Serial.println(""); 
          return true;
        }
      }else //si le bit est DISABLE
      {
        if (toEnable)  // si on veut l'activer
        {
          registerBits = registerBits | bitToEnable;
          writeRegister(registerOfBit, registerBits);

          registerBits = readRegister(registerOfBit); //verifier le resultat de l'action
          registerBits = registerBits >> 8;
          if (((registerBits) & bitToEnable) > 0)
          {
            Serial.println("bit is now enable"); 
            Serial.println(""); 
            return true;
          }else{
            Serial.println("bit enabling failed"); 
            Serial.println(""); 
            return false;
          }
        }
        else{
          Serial.println("bit is already disable"); 
          Serial.println(""); 
          return true;
        }
        
        
      }    
  }else{
    Serial.print("reading of registerOfBit failed: "); 
    Serial.println(registerBits); 
  }
}

//
void softReset(){
  writeRegister(0, 0x20);
  writeRegister(0, 0x00);
}

//
void setCalibrationClockFrequency(uint32_t freqMhz){
  /*
  Entre frequency in Mhz between 20 and 100
  */
  uint32_t periode = 0;
  Serial.print("Setting calibration frequency at: "); 
  Serial.print(freqMhz); 
  Serial.println("Mhz"); 

  if (freqMhz >= 20 && freqMhz < 40)
  {
    periode = (uint32_t)((1.0/(freqMhz*2))*1000000); //periode en pico seconde
    enableDoubler(true);
    writeRegister(CLKPERIODREG, periode);
    /*Serial.print("Periode of: "); 
    Serial.println(periode); 
    Serial.println(periode, BIN); 
    Serial.println(""); */
  }
  else if(freqMhz >= 40 && freqMhz <= 100)  
  {
    periode = (uint32_t)((1.0/(freqMhz))*1000000); //periode en pico seconde
    enableDoubler(false);
    writeRegister(CLKPERIODREG, periode);
    /*Serial.print("Periode of: "); 
    Serial.println(periode); 
    Serial.println(periode, BIN); 
    Serial.println(""); */

  }else{
    Serial.print("Frequency not between 20 and 100Mhz: ");
    Serial.println(freqMhz);
  }

  uint32_t clockRegister = readRegister(CLKPERIODREG);
  
  if (((clockRegister >> 3) &0b11111) == CLKPERIODREG)
  {
    //Serial.println("reading of CLKPERIODREG succesfull"); 

    clockRegister = clockRegister>>8; //pour enlever l'entete
    if (clockRegister == periode)
    {
      Serial.println("Calibration frequency set successfully "); 
      Serial.println(""); 
    }else{
      Serial.print("Setting of calibration frequency failed : "); 
      Serial.println((uint32_t)(1.0/clockRegister)*1000000); 
      Serial.print("instead of : "); 
      Serial.println(freqMhz); 
      Serial.print("clockRegister : "); 
      Serial.println(clockRegister); 
      Serial.println(""); 
    }
  }else {
    Serial.println("failed reading of CLKPERIODREG"); 
  }


}

//
unsigned int writeRegister(byte reg, uint32_t data) {

  uint32_t response = 0;
  uint8_t temp;

  //byte dataToSend = reg;  

  uint32_t dataToSend = (data<<8); 
  dataToSend |= (reg << 3);
  dataToSend |= CHIPADDR;

  /*Serial.print("register to write: ");
  Serial.println(reg, HEX);

  Serial.print("data to write: ");
  Serial.println(dataToSend, BIN);
  Serial.println("");*/
  
  digitalWrite(slaveSelectPin, LOW); // take the chip select low to select the device
    
  temp = SPI.transfer(dataToSend >> 24);
  temp = SPI.transfer(dataToSend >> 16);
  temp = SPI.transfer(dataToSend >> 8);
  temp= SPI.transfer(dataToSend);
  //response = (response << 16) | temp;
  //Serial.println(response, BIN);

  
  digitalWrite(slaveSelectPin, HIGH); // take the chip select high to completes the first portion of the READ cycle
  
}

//
uint32_t readRegister(byte reg) {

  byte inByte = 0;           // incoming byte from the SPI

  uint32_t response = 0;
  uint16_t temp;

  //Serial.print("register to read: ");
  //Serial.println(reg, HEX);

  //byte dataToSend = reg;  

  uint32_t dataToSend = (reg << 8); 
  dataToSend |= CHIPADDR;  

  //Serial.print("dataToSend: ");
  //Serial.println(dataToSend, BIN);

  digitalWrite(slaveSelectPin, LOW); // take the chip select low to select the device
  //SPI.transfer(dataToSend);// send the device the register you want to read
  
  // Split Request into 8-bit sections and sending them one byte at a time
  temp = SPI.transfer(dataToSend >> 24);
  temp = SPI.transfer(dataToSend >> 16);
  temp = SPI.transfer(dataToSend >> 8);
  temp= SPI.transfer(dataToSend);
  //response = (response << 16) | temp;
  //Serial.println(response, BIN);

  digitalWrite(slaveSelectPin, HIGH); // take the chip select high to completes the first portion of the READ cycle
  //delay(50);
  digitalWrite(slaveSelectPin, LOW); // take the chip select low to begin th second portion of the READ cycle
  
  dataToSend = CHIPADDR;  

  temp = SPI.transfer(dataToSend >> 24);
  response = temp;
  temp = SPI.transfer(dataToSend >> 16);
  response = (response << 8) | temp;
  temp = SPI.transfer(dataToSend >> 8);
  response = (response << 8) | temp;
  temp= SPI.transfer(dataToSend);
  response = (response << 8) | temp;
  
  digitalWrite(slaveSelectPin, HIGH);

  

  //Serial.print("dataToSend while reading: ");
  //Serial.println(dataToSend, BIN);
  
  //Serial.print("RAW response: ");
  //Serial.println(response, BIN);
  
  response = (response<<1)+1; //pour corriger en attendant de savoir pourquoi le LSB(et pas les autre) se retrouve en MSB
  /*
  Serial.print("response: ");
  Serial.println(response, BIN);

  Serial.print("register read is: 0x");
  Serial.println((response>>3)&0b11111, HEX);
  Serial.print("data received is: ");
  Serial.println((response>>8), BIN);

  Serial.println("");
  Serial.println("");*/
  
  return (response) ; 

}
