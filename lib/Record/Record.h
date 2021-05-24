#ifndef Record_H
#define Record_H

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "AC101.h"
#include "driver/i2s.h"
#include "esp_system.h"
#include <SD_FILE.h>
#pragma once
/*************************************************************************************************/
//// I2C GPIOs 
#define IIC_CLK       32
#define IIC_DATA      33

#define IIS_SCLK                    27
#define IIS_LCLK                    26
#define IIS_DSIN                    25

#define GPIO_PA_EN                  GPIO_NUM_21
/***********************************************************************************************/
//set user_config
static AC101 ac;

extern const int headerSize;  
extern const int waveDataSize;
extern const int numCommunicationData ;
extern const int numPartWavData ;
extern byte header[];
extern char communicationData[];
extern char partWavData[];

/***********************************************************************************************/
void I2S_setup();
int I2S_Read(char *data, int numData);
void I2S_Write(char *data, int numData);
void  I2S_Init();
void record_start(int record_time, String recFileName);

/***************************************************************************************************/
#endif
