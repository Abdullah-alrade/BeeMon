#include "Arduino.h"
#include "SDCard.h"

// uniqueID
String uniqueId;

// Sleep for x seconds
uint64_t measurementInterval;

// recordingTime and samplingRate
uint64_t recordingTime;
uint64_t samplingRate;

// Replace with your network credentials
String SSID;
String Password;
String content;

//set a defualt_configurations
void defaultConfig(String pUniqueId, uint64_t pMeasurementInterval, uint64_t pRecordingTime, uint64_t pSamplingRate, String pSSID, String pPassword)
{
  uniqueId = pUniqueId;
  measurementInterval = pMeasurementInterval;
  recordingTime = pRecordingTime;
  samplingRate = pSamplingRate;
  SSID = pSSID;
  Password = pPassword;
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void readFileconfig(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read variable from  config_file: \n");
  String line = "";

  while (file.available())
  {
    char c = file.read();
    line += c;
  }

  String temp_str = (getValue(line, '\n', 0));
  uniqueId = (getValue(temp_str, '=', 1));
  temp_str = getValue(line, '\n', 1);
  SSID = getValue(temp_str, '=', 1).c_str();
  temp_str = getValue(line, '\n', 2);
  Password = getValue(temp_str, '=', 1).c_str();
  temp_str = getValue(line, '\n', 3);
  measurementInterval = getValue(temp_str, '=', 1).toInt();
  temp_str = getValue(line, '\n', 4);
  recordingTime = getValue(temp_str, '=', 1).toInt();
  temp_str = getValue(line, '\n', 5);
  samplingRate = getValue(temp_str, '=', 1).toInt();
  file.close();
}

// Write to the SD card
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

// Create new directory
void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

// Rename file on SD card
void renameFile(fs::FS &fs, String path1, String path2)
{
  Serial.println("Renaming file " + path1 + " to " + path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("File renamed");
  }
  else
  {
    Serial.println("Rename failed");
  }
}

// Read file sensor.txt from the SD card
void readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available())
  {
    content += (char)file.read();
  }
  file.close();
}