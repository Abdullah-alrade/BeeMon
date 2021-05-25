#ifndef FILE_H
#define FILE_H

#include "Arduino.h"
#include "FS.h"
#pragma once

// uniqueId
  extern String uniqueId;

// Sleep for x seconds
  extern uint64_t measurementInterval;

// recordingTime and samplingRate
  extern uint64_t recordingTime;
  extern uint64_t samplingRate;

// Network credentials
  extern String SSID;
  extern String Password;
  extern String content;
  
//*split a line
     String getValue (String data, char separator, int index);
     //read the config variables from the SD-card 
     void readFileconfig (fs::FS &fs, const char * path);
     void writeFile(fs::FS &fs, const char * path, const char * message);
     void appendFile(fs::FS &fs, const char * path, const char * message);
     void renameFile(fs::FS &fs, String path1, String path2);
     void readFile(fs::FS &fs, const char * path);
     void createDir(fs::FS &fs, const char * path);
     void defaultConfig(String pUniqueId, uint64_t pMeasurementInterval, uint64_t pRecordingTime, uint64_t pSamplingRate, String pSSID, String pPassword);
     
#endif