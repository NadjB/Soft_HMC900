/*
	Library to control the HMC900.
	Created by Nadjirou Ba 24 June 2021
	Released into the public domain.
*/
#ifndef HMC900_h
#define HMC900_h

#include "Arduino.h"
#include <SPI.h>

//debug tools
//#define DEBUG true
#ifdef DEBUG
  #define DEBUG_PRINT(x)     Serial.print (x)
  #define DEBUG_PRINT2(x,y)  Serial.print (x,y)
  #define DEBUG_PRINTLN(x)   Serial.println (x)
  #define DEBUG_PRINTLN2(x,y)  Serial.println (x,y)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT2(x,y)
  #define DEBUG_PRINTLN(x) 
  #define DEBUG_PRINTLN2(x,y)
#endif


#define CHIPADDR 5 // HMC900 address

#define USESETTING 0x01 // use settint address

#define FREQWANTED 20 // user defined wanted cuttoff frequency (Mhz)
#define CALFREQ 0b100111000100000 // 50Mhz ( 110000110101000 pour 40Mhz)

//Register addresses
#define READREG 0x00 //

#define ENABLEREG 0x01 // Enable register
#define ENABLEREGDEF 0b0000001100 // default state

#define SETTINGSREG 0x02 // Settings
#define SETTINGSREGDEF 0b00000001001 //

#define CALREG 0x03 // Calibration
#define CALREGDEF 0b0000 //

#define CALBISTRSTROBEREG 0x04 // Calibration/RC-BIST Strobe

#define CLKPERIODREG 0x05 // Clock Period
#define CLKPERIODREGDEF 0x0000 //

#define MEASUREADJUSTREG 0x06 // Measure Adjust
#define MEASUREADJUSTREGDEF 0x000 //

#define CALSTATUSREG 0x08 // Calibration Status (read only)

#define CALCOUNTREG 0x09 // Calibration Count (read-only)

#define OTPVALUESREG 0x0A // OTP Values (read-only)

#define OTPWRITENABLEREG 0x0B // OTP Write Enable
#define OTPWRITENABLEREGDEF 0b0 

#define OTPWRITEREG 0x0B // OTP Write
#define OTPWRITEREGDEF 0b0

#define OTPWRITEPULSEREG 0x0D // OTP Write Pulse
#define OTPWRITEPULSEREGDEF 0b0

#define RCBISTENABLEREG 0x0E // RC-BIST Enable
#define RCBISTENABLEREGDEF 0b0 

#define RCBISTOUTREG 0x0F // RC-BIST Out
#define RCBISTOUTREGDEF 0x00000 


//define bits codes for commands
#define CBANDWHDTH3 0b0000  // 2.764  < 3.5  < 4.235
#define CBANDWHDTH5 0b0001  // 3.948  < 5.0  < 6.050
#define CBANDWHDTH7 0b0010  // 5.527  < 7.0  < 8.470
#define CBANDWHDTH10 0b0011 // 7.896  < 10.0 < 12.10
#define CBANDWHDTH14 0b0100 // 11.055 < 14.0 < 16.94
#define CBANDWHDTH20 0b0101 // 15.792 < 20.0 < 24.20
#define CBANDWHDTH28 0b0110 // 22.109 < 28.0 < 33.88
#define CBANDWHDTH35 0b0111 // 27.637 < 35.0 < 42.351
#define CBANDWHDTH50 0b1000 // 39.480 < 50.0 < 60.50

#define FBANDWHDTH0803 0b0000  // 0.790  < 0.803  < 0.818
#define FBANDWHDTH0832 0b0001  // 0.818  < 0.832  < 0.846
#define FBANDWHDTH0862 0b0010  // 0.846  < 0.862  < 0.878
#define FBANDWHDTH0893 0b0011  // 0.878  < 0.893 < 0.909
#define FBANDWHDTH0926 0b0100  // 0.909 < 0.926 < 0.943
#define FBANDWHDTH0959 0b0101  // 0.943 < 0.959 < 0.976
#define FBANDWHDTH0994 0b0110  // 0.976 < 0.994 < 1.012
#define FBANDWHDTH1030 0b0111  // 1.012 < 1.030 < 1.048
#define FBANDWHDTH1068 0b1000  // 1.048 < 1.068 < 1.087
#define FBANDWHDTH1107 0b1000  // 1.087  < 1.107  < 1.128
#define FBANDWHDTH1148 0b1001  // 1.128  < 1.148  < 1.169
#define FBANDWHDTH1189 0b1010  // 1.169  < 1.189  < 1.210



class HMC900
{
	public:
		HMC900(int slaveSelectPin);
		bool begin();
		uint32_t readRegister(byte reg);
    void writeRegister(byte reg, uint32_t data);
    void softReset();
    bool enableBit(bool toEnable, byte registerOfBit, uint32_t bitToEnable);
    void enableDoubler(bool en);
    void setCalibrationClockFrequency(uint32_t freqMhz);
    void enableBistMode(bool en);
    void enableFilterQ(bool en);
    uint32_t calibration(uint32_t calFreqMhz);
    uint32_t builtInSelfTest(uint32_t calFreqMhz);
    void writeCoaseBandwidthCode(uint32_t cbandwidthcode);
    uint32_t readCoaseBandwidthCode();
    void writeFineBandwidthCode(uint32_t fbandwidthcode);
    uint32_t readFineBandwidthCode();
    void filterBandwidthSetting(uint32_t calFreqMhz);
    void setBandwidthCodes(float cal_count);
		
	private:
		int _slaveSelectPin;
};

#endif