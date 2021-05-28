/*********
  BEEMON v0.3
  Abdualah (91alab1bif@hft-stuttgart.de),
  Medjen (91izme1bif@hft-stuttgart.de),
  Safak (91kasa1bif@hft-stuttgart.de),
  Henry (91elhe1bif@hft-stuttgart.de)
*********/

// DS1302 RTC Library
#include "TimeLib.h"
#include "DS1302RTC.h"

// Set pins: RST, IO, CLK
DS1302RTC RTC(5, 21, 0);

// Libraries for the SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Library for DHT sensor
#include "DHT.h"

//Library for audio
#include <Record.h>

// Libraries for WiFi and HTTP
#include <WiFi.h>
#include <HTTPClient.h>
#include <SDCard.h>

// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;

bool debug = true;
#define DHTPIN 22     // DHT Pin 22
#define DHTTYPE DHT22 // DHT 22  (AM2302)

// Create DHT sensor object
DHT dht(DHTPIN, DHTTYPE);

#define BUTTON_PIN_BITMASK 0x1000000000 // Button 1 (Pin 36), 2^36 in hex

// Define CS pin for the SD card module
#define SD_CS 13
#define SPI_MOSI 15
#define SPI_MISO 2
#define SPI_SCK 14
#define I2S_DSIN 25
#define I2S_BCLK 27
#define I2S_LRC 26
#define I2S_MCLK 0
#define I2S_DOUT 35

// Temperature and Humidity Sensor variables
float temperature;
float humidity;

String dataMessage; // Contains the temperature and humidity data

// Deep sleep weakup reasons
boolean print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused Key1 pressed");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Set time of the RTC time
int setRTCTime(String timestamp)
{
  time_t zeit = timestamp.toInt();

  Serial.println("Set time");
  RTC.writeEN(false);
  if (!RTC.writeEN())
  {
    Serial.println("DS1302 is write protected, write protection will be removed...");
    RTC.writeEN(true);
  }

  setTime(zeit);
  Serial.println(now());
  if (RTC.set(now()) == 0)
  {
    Serial.println("Success - Time has been set.");
    return 0;
  }
  else
  {
    Serial.println("Error - Time could not be set.");
    return 1;
  }
}

// Function to get temperature and humditiy
void getReadings()
{

  Serial.print("Timestamp: ");
  Serial.println(RTC.get());

  temperature = dht.readTemperature(); // Temperature in Celsius
  humidity = dht.readHumidity();       // Humditiy in percentage
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.print("Can not read from DHT22 sensor");
  }
  else
  {
    if (debug)
    {
      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Humditiy: ");
      Serial.println(humidity);
    }
  }
}

void wifiConnect(const char *ssid, const char *password)
{
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("Mac address: " + WiFi.macAddress());
}

void sendWAVHTTP(String fileName)
{
  if ((WiFi.status() == WL_CONNECTED))
  { //Check the current connection status
    Serial.println("Connected to the WiFi network");
    File file = SD.open(fileName, FILE_READ);
    if (!file)
    {
      Serial.println("FILE IS NOT AVAILABLE!");
      return;
    }

    fileName.remove(0, 8);

    Serial.println("===> Upload FILE to Server");

    HTTPClient client;
    client.begin("http://193.196.52.234/api/audio.php?id=" + uniqueId + "&name=" + fileName);
    client.addHeader("Content-Type", "audio/wav");
    int httpResponseCode = client.sendRequest("POST", &file, file.size());
    Serial.print("httpResponseCode: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200)
    {
      String response = client.getString();
      Serial.println("Upload: " + fileName);
      Serial.println(response);
      String oldFileName = "/audio/n" + fileName;
      String newFileName = "/audio/y" + fileName;
      renameFile(SD, oldFileName, newFileName);
    }
    else
    {
      Serial.println("Error");
    }
    file.close();
    client.end();
  }
  else
  {
    Serial.println("No wifi connection");
  }
}

void uploadWAV()
{
  File root = SD.open("/audio");
  if (root)
  {
    File file = root.openNextFile();
    while (file)
    {
      if (!file.isDirectory())
      {
        String fileName = file.name();
        if (fileName.indexOf("/n") > 0)
        { // check if file is already uploaded
          Serial.print("  FILE: ");
          Serial.println(fileName);
          sendWAVHTTP(fileName);
        }
      }
      file = root.openNextFile();
    }
  }
  else
  {
    Serial.println("Failed to open directory");
  }
}

void sendDataHTTP()
{
  if ((WiFi.status() == WL_CONNECTED))
  { // Check the current connection status
    Serial.println("Connected to the WiFi network");
    readFile(SD, "/sensor/data.csv");
    HTTPClient http;
    Serial.println(content);
    http.begin("http://193.196.52.234/api/sensor.php?id=" + uniqueId);
    int httpResponseCode = http.POST(content); // Make the request

    if (httpResponseCode == 200)
    { // Check for the returning code
      String response = http.getString();
      Serial.print("httpResponseCode: ");
      Serial.println(httpResponseCode);
      Serial.println(response);
      setRTCTime(response);

      String fileName = "/sensor/" + response + "." + "csv";
      renameFile(SD, "/sensor/data.csv", fileName);
    }
    else
    {
      Serial.println("Error on HTTP request");
    }
    http.end();
    content = "";
  }
  else
  {
    Serial.println("No wifi connection");
  }
}

// Write the sensor readings on the SD card in data.csv
void logSDCard(time_t timestamp)
{
  dataMessage = "\n" + String(temperature) + ";" + String(humidity) + ";" + timestamp;
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/sensor/data.csv", dataMessage.c_str());
}

// Initialisation of all components
void setup()
{

  // Initialise Real time clock (DS1302)
  RTC.haltRTC(false);
  if (RTC.haltRTC())
  {
    Serial.println("DS1302 is paused. With RTC.set the clock is restarted automatically.");
  }
  RTC.writeEN(false);
  if (!RTC.writeEN())
  {
    Serial.println("The DS1302 is write protected, this behavior is normal.");
    Serial.println();
  }

  defaultConfig("Bemoon", 10, 10, 44100, "beemon", "12345678"); // create default config

  Serial.begin(115200); // Start serial communication for debugging purposes
  delay(5000);          //Take some time to open up the Serial Monitor

  boolean button = print_wakeup_reason();

  // Initialize SD card
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  pinMode(SD_CS, OUTPUT);
  if (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin())
  {
    Serial.println("ERROR - SD card initialization failed!");
    return; // init failed
  }

  // setup microphone
  I2S_setup();

  // read settings from config file: UNIQUEID, SSID, PASSWORD, MEASUREMENTINTERVAL, RECORDINGTIME, SAMPLINGRATE
  readFileconfig(SD, "/config.txt");

  // Create new directories for sensor and audio data
  createDir(SD, "/sensor");
  createDir(SD, "/audio");

  // If the data.csv file doesn't exist create a file on the SD card
  File file1 = SD.open("/sensor/data.csv");
  if (!file1)
  {
    Serial.println("File data.csv in the folder sensor doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/sensor/data.csv", "temperature;humidity;time");
  }
  else
  {
    Serial.println("File already exists");
  }
  file1.close();

  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(measurementInterval * uS_TO_S_FACTOR);

  dht.begin(); // Start the DHT22 sensor
  delay(4000); // 4 seconds pre-reading time for the sensor (the sensor is sometimes a little slow)

  time_t timestamp = RTC.get(); // read current time from DS1302

  if (button)
  {                                              // check if KEY1 was pressed
    wifiConnect(SSID.c_str(), Password.c_str()); // connect to wifi with SSID and password from config file
    sendDataHTTP();                              // send temperature and humidity data to server
    uploadWAV();                                 // upload all wav files to the server
    WiFi.mode(WIFI_OFF);
  }
  else
  {
    getReadings();        // read temperature and humidity data
    logSDCard(timestamp); // save temperature and humidity data to the sd card with the current timestamp

    // Start audio recording
    String recFileName = "/audio/n" + String(timestamp) + ".wav";
    Serial.println(recFileName);
    record_start(recordingTime, recFileName);
  }

  // Configure the wake up source KEY1 (Pin 36)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

  // Start deep sleep
  Serial.println("DONE! Going to sleep now.");
  esp_deep_sleep_start();
}

void loop()
{
  // The ESP32 Audio Kit will be in deep sleep
  // it never reaches the loop()
}
