
#include "Arduino.h"
#include "HMC900.h"


HMC900::HMC900(int slaveSelectPin)
    :_slaveSelectPin{slaveSelectPin}
{
  pinMode(_slaveSelectPin, OUTPUT);
}

bool HMC900::begin(){
  Serial.begin(115200);
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.endTransaction();

  delay(500);
  DEBUG_PRINTLN("Start of the Begin sequence");
  
  
  DEBUG_PRINT("_slaveSelectPin: ");
  DEBUG_PRINTLN(_slaveSelectPin);
    
  uint32_t settingRegister = readRegister(SETTINGSREG); // check if the communication works
  DEBUG_PRINT("return of the 'begin' communication test: ");
  DEBUG_PRINTLN2(settingRegister,BIN);
  
  if (((settingRegister >> 3) &0b11111) == SETTINGSREG){
	  return true;
  }else{
	  return false;
  }
}

uint32_t HMC900::readRegister(byte reg) {

  byte inByte = 0;           // incoming byte from the SPI

  uint32_t response = 0;
  uint16_t temp;

  DEBUG_PRINT("Reading register: 0x");
  DEBUG_PRINTLN2(reg,HEX);

  uint32_t dataToSend = (reg << 8); 
  dataToSend |= CHIPADDR;  

  //DEBUG_PRINT("dataToSend: ");
  //DEBUG_PRINTLN2(dataToSend,BIN);

  digitalWrite(_slaveSelectPin, LOW); // take the chip select low to select the device
  
  // Split Request into 8-bit sections and sending them one byte at a time
  temp = SPI.transfer(dataToSend >> 24);
  temp = SPI.transfer(dataToSend >> 16);
  temp = SPI.transfer(dataToSend >> 8);
  temp= SPI.transfer(dataToSend);

  digitalWrite(_slaveSelectPin, HIGH); // take the chip select high to completes the first portion of the READ cycle
  
  digitalWrite(_slaveSelectPin, LOW); // take the chip select low to begin th second portion of the READ cycle
  
  dataToSend = CHIPADDR;  

  temp = SPI.transfer(dataToSend >> 24);
  response = temp;
  temp = SPI.transfer(dataToSend >> 16);
  response = (response << 8) | temp;
  temp = SPI.transfer(dataToSend >> 8);
  response = (response << 8) | temp;
  temp= SPI.transfer(dataToSend);
  response = (response << 8) | temp;
  
  digitalWrite(_slaveSelectPin, HIGH);

  
  DEBUG_PRINT("RAW response: ");
  DEBUG_PRINTLN2(response,BIN);
  
  if ((response &0b11) == 0b10)
  {
    response = (response<<1)+1;
  } // To correct the switch of the LSB to the MSB position (see "RAW response": begin with "10" instead of "101" and always end with 1"
  
  DEBUG_PRINT("response: ");
  DEBUG_PRINTLN2(response,BIN);
  DEBUG_PRINT("register read is: 0x");
  DEBUG_PRINTLN2(((response>>3)&0b11111),HEX);
  DEBUG_PRINT("data received is: ");
  DEBUG_PRINTLN2((response>>8),BIN);
  DEBUG_PRINTLN("");
  
  return (response) ; 

}


void HMC900::writeRegister(byte reg, uint32_t data) {

  uint32_t response = 0;
  uint8_t temp;

  uint32_t dataToSend = (data<<8); 
  dataToSend |= (reg << 3);
  dataToSend |= CHIPADDR;

  DEBUG_PRINT("register to write: 0x");
  DEBUG_PRINTLN2(reg,HEX);

  DEBUG_PRINT("data writen: ");
  DEBUG_PRINTLN2(dataToSend,BIN);
  
  digitalWrite(_slaveSelectPin, LOW); // take the chip select low to select the device
    
  temp = SPI.transfer(dataToSend >> 24);
  temp = SPI.transfer(dataToSend >> 16);
  temp = SPI.transfer(dataToSend >> 8);
  temp = SPI.transfer(dataToSend);
  //response = (response << 16) | temp;
  //Serial.println(response, BIN);

  
  digitalWrite(_slaveSelectPin, HIGH); // take the chip select high to completes the first portion of the READ cycle
  
}


void HMC900::softReset(){
  writeRegister(0, 0x20);
  writeRegister(0, 0x00);
  delay(1);
}


bool HMC900::enableBit(bool toEnable, byte registerOfBit, uint32_t bitToEnable){
  /*
  toEnable:
  if yes toEnable=true
  if you wich to disable toEnable=false

  bitToEnable:
  if it is the third bit of the register  bitToEnable=0b100
  if fifth 0b10000
  */

  DEBUG_PRINT("In register '0x");
  DEBUG_PRINTLN2(registerOfBit,HEX);
  DEBUG_PRINT("Enabling of bit 0b");
  DEBUG_PRINTLN2(bitToEnable,BIN);
  
  uint32_t registerBits = readRegister(registerOfBit);
  if (((registerBits >> 3) &0b11111) == registerOfBit)
  {
    //DEBUG_PRINTLN("reading of register succesfull"); 

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
            DEBUG_PRINTLN("bit is now disabled"); 
            return true;
          }else{
            DEBUG_PRINTLN("bit disabling failed"); 
            return false;
          }
        }
        else
        {
          DEBUG_PRINTLN("bit is already enable"); 
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
            DEBUG_PRINTLN("bit is now enable"); 
            return true;
          }else{
            DEBUG_PRINTLN("bit enabling failed"); 
            return false;
          }
        }
        else{
          DEBUG_PRINTLN("bit is already disable"); 
          return true;
        }
        
        
      }    
  }else{
    DEBUG_PRINT("reading of register failed: "); 
    DEBUG_PRINTLN(registerBits); 
    return false;
  }
}


void HMC900::enableDoubler(bool en){
  //"0b100000" le bit correspondant a "doubler_enable"  
  if(enableBit(en, SETTINGSREG, 0b100000)){ 
    DEBUG_PRINTLN("Setting of Doubler successfull");
  }else{
    Serial.println("Setting of Doubler failed");    
  }
}


void HMC900::setCalibrationClockFrequency(uint32_t freqMhz){
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
    DEBUG_PRINT("Periode of: "); 
    DEBUG_PRINTLN(periode); 
    DEBUG_PRINTLN2(periode,BIN); 
  }
  else if(freqMhz >= 40 && freqMhz <= 100)  
  {
    periode = (uint32_t)((1.0/(freqMhz))*1000000); //periode en pico seconde
    enableDoubler(false);
    writeRegister(CLKPERIODREG, periode);
    DEBUG_PRINT("Periode of: "); 
    DEBUG_PRINTLN(periode); 
    DEBUG_PRINTLN2(periode,BIN); 

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


void HMC900::enableBistMode(bool en){
  //"0b1" le bit correspondant a "enable_RCBIST_mode"
  if(enableBit(en, RCBISTENABLEREG, 0b1)){ 
    DEBUG_PRINTLN("Setting of RC-BISTR successfull");
  }else{
    Serial.println("Failed to set RC-BISTR");
  }
}


void HMC900::enableGain10dB(bool en){
  //"0b10000" le bit correspondant a "gain_10dB"
  if(enableBit(en, SETTINGSREG, 0b10000)){ 
    DEBUG_PRINTLN("Setting of gain_10dB successfull");
  }else{
    Serial.println("Failed to set gain_10dB");
  }
}


void HMC900::enableFineBandwidthCodes(bool en){
  //"0b10000" le bit correspondant a "force_cal_code"
  if(enableBit(en, ENABLEREG, 0b10000)){ 
    DEBUG_PRINTLN("Setting of gain_10dB successfull");
  }else{
    Serial.println("Failed to set gain_10dB");
  }
}


void HMC900::enableFilterQ(bool en){
  //"0b1000" le bit correspondant a "enable_RCBIST_mode"
  if(enableBit(en, RCBISTENABLEREG, 0b1)){ 
    DEBUG_PRINTLN("Setting of RC-BISTR successfull");
  }else{
    Serial.println("Failed to set RC-BISTR");    
  }


  uint32_t enableRegister = readRegister(ENABLEREG);
  if (((enableRegister >> 3) &0b11111) == ENABLEREG)
  {
    DEBUG_PRINTLN("reading of ENABLEREG succesfull"); 

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
            DEBUG_PRINTLN("Q filter is now disabled"); 
          }else{
            Serial.println("Q filter disabling failed"); 
          }
        }
        else
        {
          DEBUG_PRINTLN("Q filter is already enable"); 
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
            DEBUG_PRINTLN("Q filter is now enable"); 
          }else{
            Serial.println("Q filter enabling failed"); 
          }
        }
        else{
          DEBUG_PRINTLN("Q filter is already disable"); 
        }
        
        
      }    
  }else{
  Serial.print("reading of ENABLEREG failed: "); 
  Serial.println(enableRegister); 
  }
  
}


uint32_t HMC900::calibration(uint32_t calFreqMhz) {
  setCalibrationClockFrequency(calFreqMhz);
  enableBit(true, ENABLEREG, 0b10); //Enable RC Calibration circuit

  Serial.print("If you forgot to plug, connecte, and launch the signal generator for the calibration at "); 
  Serial.print(calFreqMhz); 
  Serial.println(" Mhz... Well, too late."); 
  enableBit(true, CALBISTRSTROBEREG, 1); //write anithing in "Calibration/RC-BIST Strobe" register starts a calibration cycle.
  delay(15); // time for the calibration > periode*2^18

  uint32_t calResult = 0b10000; // Answer when the calibration is still ongoing
  Serial.println("cal: ");
  while((calResult & 0b10000) > 0){ 
    calResult = readRegister(CALSTATUSREG);
    calResult = (calResult >> 8);
    DEBUG_PRINTLN2(calResult, BIN);
    delay(1);
  }
  Serial.println(calResult, BIN);
  return readRegister(CALCOUNTREG);
}

uint32_t HMC900::builtInSelfTest(uint32_t calFreqMhz){

  softReset();

  setCalibrationClockFrequency(calFreqMhz);

  enableBistMode(true);

  Serial.print("If you forgot to plug connecte and launch the signal generator for the calibration at "); 
  Serial.print(calFreqMhz); 
  Serial.println(" Mhz... Well, too late."); 
  enableBit(true, CALBISTRSTROBEREG, 1); //write anithing in "Calibration/RC-BIST Strobe" register starts a calibration cycle.
  delay(15); // time for the calibration > periode*2^18
  
  uint32_t bistrResult = 0b10000000000000000; // Answer when the RC-BIST is still ongoing
  Serial.println("bistrResult: ");
  while((bistrResult & 0b10000000000000000) > 0){
    bistrResult = readRegister(RCBISTOUTREG);
    bistrResult = (bistrResult >> 8);
    DEBUG_PRINTLN2(bistrResult, BIN);
    delay(1);
  }
  Serial.println(bistrResult, BIN);
  return bistrResult;
}


void HMC900::writeCoaseBandwidthCode(uint32_t cbandwidthcode){
  
  uint32_t settingRegister = readRegister(SETTINGSREG);
  if (((settingRegister >> 3) &0b11111) == SETTINGSREG)
  {
    DEBUG_PRINTLN("reading SETTINGSREG succesfull"); 

    settingRegister = settingRegister>>8; //pour enlever l'entete
    settingRegister = settingRegister & 0b111111111111110000111111;
    settingRegister |= (cbandwidthcode << 6);
    writeRegister(SETTINGSREG, settingRegister);
    
    settingRegister = readRegister(SETTINGSREG);
    settingRegister = settingRegister>>8; //pour enlever l'entete
    if (((settingRegister >> 6) & 0b1111) == cbandwidthcode)  
    {
      DEBUG_PRINTLN("Setting of the coarse bandwidth code successfull");       
    }else {
      Serial.print("Failed setting of the coarse bandwidth code: "); 
      Serial.print(((settingRegister >> 6) & 0b1111), BIN); 
      Serial.print(" vs "); 
      Serial.println(cbandwidthcode, BIN); 

    }  
  }else {
    Serial.print("failed reading of SETTINGSREG: "); 
    Serial.println(settingRegister, BIN); 
  }
}


uint32_t HMC900::readCoaseBandwidthCode(){
  
  uint32_t settingRegister = readRegister(SETTINGSREG);
  if (((settingRegister >> 3) &0b11111) == SETTINGSREG)
  {
    DEBUG_PRINTLN("reading SETTINGSREG succesfull"); 

    settingRegister = settingRegister>>8; //pour enlever l'entete
    return((settingRegister >> 6) & 0b1111); 

  }else {
    Serial.print("failed reading of SETTINGSREG: "); 
    Serial.println(settingRegister, BIN); 
  }
}


void HMC900::writeFineBandwidthCode(uint32_t fbandwidthcode){

  if(fbandwidthcode >= 0 && fbandwidthcode <= 11){
       uint32_t calibrationRegister = readRegister(CALREG);
     if (((calibrationRegister >> 3) &0b11111) == CALREG)
     {
       DEBUG_PRINTLN("reading CALREG succesfull"); 

       writeRegister(CALREG, fbandwidthcode);
    
       calibrationRegister = readRegister(CALREG);
       calibrationRegister = calibrationRegister>>8; //pour enlever l'entete
       if (calibrationRegister == fbandwidthcode)  
       {
         DEBUG_PRINTLN("Setting of the fine bandwidth code successfull");       
       }else {
         Serial.print("Failed setting of the fine bandwidth code: "); 
         Serial.print(calibrationRegister, BIN); 
         Serial.print(" vs "); 
         Serial.println(fbandwidthcode, BIN);      
       }  
     }else {
       Serial.print("failed reading of CALREG: "); 
       Serial.println(calibrationRegister, BIN); 
     }
  }else{
     Serial.println("!!! fbandwidthcode must be betwin 0 and 11"); 
  }
  

}


uint32_t HMC900::readFineBandwidthCode(){
  
  uint32_t calibrationRegister = readRegister(CALREG);
  if (((calibrationRegister >> 3) &0b11111) == CALREG)
  {
    DEBUG_PRINTLN("reading CALREG succesfull"); 
    
    calibrationRegister = readRegister(CALREG);
    calibrationRegister = calibrationRegister>>8; //pour enlever l'entete

    return(calibrationRegister); // Be carefull that you have to enable the fine bandwidth setting override bits (register 01 bit 4, force_cal_code, must be set).
      
  }else {
    Serial.print("failed reading of CALREG: "); 
    Serial.println(calibrationRegister, BIN); 
  }
}


void HMC900::filterBandwidthSetting(uint32_t calFreqMhz){
  uint32_t cal_count = calibration(calFreqMhz);
  setBandwidthCodes(cal_count);
  enableBit(true, ENABLEREG, 0b1000);
}


void HMC900::setBandwidthCodes(float cal_count){
  float ctune = cal_count/10370000;
  float fBW_norm_coarse = FREQWANTED * ctune;
  DEBUG_PRINTLN("Setting of bandwidth codes");
  DEBUG_PRINT("Normalize the desired frequency: ");  
  DEBUG_PRINTLN(fBW_norm_coarse);
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
    DEBUG_PRINT("coarse_bandwidth_code: ");
    DEBUG_PRINTLN(coarse_bandwidth_code);
    writeCoaseBandwidthCode(coarse_bandwidth_code);
  } else {
      Serial.print("fBW_norm_coarse not within parameters: ");
      Serial.println(fBW_norm_coarse);
  }

  float fine_tune_ratio = fBW_norm_coarse / fBW_norm_coarse_typ;
  float fine_tune_ratio_typ = 0.0;
  char fine_bandwidth_code = 0;
  DEBUG_PRINT("fine_tune_ratio: ");
  DEBUG_PRINTLN(fine_tune_ratio);

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
    DEBUG_PRINT("fine_bandwidth_code: ");
    DEBUG_PRINTLN(fine_bandwidth_code);
    writeFineBandwidthCode(fine_bandwidth_code);
  } else {
      Serial.print("fine_tune_ratio not within parameters: ");
      Serial.println(fine_tune_ratio);
  }

}
