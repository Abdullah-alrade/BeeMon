#ifndef FILE_H
#define FILE_H

#include "Arduino.h"
#include "FS.h"
#pragma once

/***********************************************************************************************/

/***********************************************************************************************/
//username
  extern String name;
// Sleep for x seconds
  extern uint64_t TIME_TO_SLEEP ;
//TIME_TO_recording and Sampling_Rate
  extern uint64_t TIME_TO_recording ;
  extern uint64_t Sampling_Rate ;
// Replace with your network credentials
  extern String SSID;
  extern String Passowrd ;
  extern String content;
  
/***************************************************************************************************/



//*split a line
     String getValue (String data, char separator, int index);
     //read the config variables from the SD-card 
     void readFileconfig (fs::FS &fs, const char * path);
     void writeFile(fs::FS &fs, const char * path, const char * message);
     void appendFile(fs::FS &fs, const char * path, const char * message);
     void renameFile(fs::FS &fs, String path1, String path2);
     void readFile(fs::FS &fs, const char * path) ;
     void default_config(String pUser_name,uint64_t pTIME_TO_SLEEP,uint64_t pTIME_TO_recording,
     uint64_t pSampling_rate , String pSSID, String pPassowrd  );
     


/************************************************************************************************************/

#endif