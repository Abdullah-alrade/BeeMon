#include "Arduino.h"
#include "SD_FILE.h"


/***********************************************************************************************/
//username
   String name;
// Sleep for x seconds
   uint64_t TIME_TO_SLEEP ;
//TIME_TO_recording and Sampling_Rate
   uint64_t TIME_TO_recording ;
   uint64_t Sampling_Rate ;
// Replace with your network credentials
   String SSID;
   String Passowrd ;
   String content;
  
/***************************************************************************************************/
//set a defualt_configurations
void default_config(String pUser_name,uint64_t pTIME_TO_SLEEP,uint64_t pTIME_TO_recording,
uint64_t pSampling_rate , String pSSID, String pPassowrd  ){
  name=pUser_name;
   // Sleep for 10 seconds
  TIME_TO_SLEEP =pTIME_TO_SLEEP;
   //Time TIME_TO_recording and Sampling_Rate
  TIME_TO_recording =pTIME_TO_recording;
  Sampling_Rate =pSampling_rate ;
   // Replace with your network credentials 
  SSID =pSSID;
  Passowrd =pPassowrd;
}
/*****************************************getValue**********************************************************/
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
  }

  /***************************************************************************************************/
  void readFileconfig(fs::FS &fs, const char * path ) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read variable from  config_file: \n");
  String line="";
  
  while (file.available()) {
       char c =  file.read();
        line +=c;
  }
 
   name = (getValue(line,';',0));
   SSID = getValue(line,';',1).c_str();
   Passowrd = getValue(line,';',2).c_str();
   TIME_TO_SLEEP =getValue(line,';',4).toInt();
   TIME_TO_recording = getValue(line,';',5).toInt();
   Sampling_Rate = getValue(line,';',6).toInt();
   file.close();
  
}
/***************************************************************************************************/
// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}
/***************************************************************************************************/
// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
/***************************************************************************************************/
// rename file on SD card
void renameFile(fs::FS &fs, String path1, String path2) {
  Serial.println("Renaming file " + path1 + " to " + path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}
/***************************************************************************************************/
// Read file sensor.txt from the SD card
void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
}
/***************************************************************************************************/
