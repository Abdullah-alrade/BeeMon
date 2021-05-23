/*********
  BEEMON v0.1
  Abdualah Al Rade (91alab1bif@hft-stuttgart.de),
  Medjen Izairi (91izme1bif@hft-stuttgart.de),
  Safak Karman (91kasa@hft-stuttgart.de),
  Henry Bahnasawy (91elhe1bif@hft-stuttgart.de)
*********/
//*********************************************************
//    --------RTC NEW--------------
#include "TimeLib.h"
#include "DS1302RTC.h"

// Set pins: CE / RST (je nach Modul), IO, CLK
DS1302RTC RTC(5, 21, 0);
//***********************************************

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
#include <SD_file.h>


// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;
  
bool debug = true;
#define DHTPIN 22               // DHT Pin 0
#define DHTTYPE DHT22         // DHT 22  (AM2302)

// Create object
DHT dht(DHTPIN, DHTTYPE);

#define BUTTON_PIN_BITMASK 0x1000000000 // Button 1 (Pin 36), 2^36 in hex

// Define CS pin for the SD card module
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO       2
#define SPI_SCK       14
#define I2S_DSIN      25
#define I2S_BCLK      27
#define I2S_LRC       26
#define I2S_MCLK       0
#define I2S_DOUT      35

String dataMessage;

// Temperature and Humidity Sensor variables
float temperature;
float humidity;

boolean print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused Key1 pressed"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    return true;
  } else {
    return false;
  }
}


//_____________________________________________________________________________setup
void setup() {
  //************************************************
  //    --------RTC NEW--------------
 RTC.haltRTC(false);
  if (RTC.haltRTC()) {
    Serial.println("DS1302 ist angehalten. Mit RTC.set wird die Uhr automaisch wieder gestartet.");
  }
 RTC.writeEN(false);
  if (!RTC.writeEN()) {
    Serial.println("Der DS1302 / VMA301 ist Schreibgeschuetzt, dies ist normal.");
    Serial.println();
  }

  //************************************************
  default_config("Bemoon", 10,10,4410,"beemon","12345678");
  // content;
  // Start serial communication for debugging purposes
  Serial.begin(115200);
  delay(5000); //Take some time to open up the Serial Monitor

  boolean button = print_wakeup_reason();
  // Initialize SD card
 SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  pinMode(SD_CS, OUTPUT);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  //mic _setup to use
   I2S_setup();
   //read  varaible from config file // user,SSID,password,Time to sleep,-record,sampling_rate
  readFileconfig(SD, "/config.txt");

  // If the sensor.csv file doesn't exist
  // Create a file on the SD card
  File file1 = SD.open("/sensor.csv");
  if (!file1) {
    Serial.println("File sensor.csv doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/sensor.csv",  "temperature;humidity;time");
  } else {
    Serial.println("File already exists");
  }
  file1.close();
  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Start the DHT22 sensor
  dht.begin();
  delay(4000); // 4 seconds pre-reading time for the sensor (the sensor is sometimes a little slow)

  if (button) { // check if KEY1 was pressed
    wifiConnect(SSID.c_str() ,Passowrd.c_str());
    sendDataHTTP();
  } else {
    getReadings();
    logSDCard();
  }
  // Configure the wake up source KEY1 (Pin 36)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);
    Serial.println("audio starten");

    //Record Start
   record_start(TIME_TO_recording);

  // Start deep sleep
  Serial.println("DONE! Going to sleep now.");
    Serial.println("________________________________________");
 // uhrzeit_setzen("1621690260");
  esp_deep_sleep_start();
}
//*****************************************
//    --------RTC NEW--------------
int uhrzeit_setzen(String timestamp) {
  time_t zeit = timestamp.toInt();
  
  Serial.println("Uhrzeit setzen");
  RTC.writeEN(false);
  if ( ! RTC.writeEN() ) {
    Serial.println("DS1302 ist Schreibgeschuetzt, Schreibschutz wird aufgehoben...");
    RTC.writeEN(true);
  }
 // Serial.println("da dem " + Jahr);
  setTime(zeit);
  Serial.println(now());
  if (RTC.set(now()) == 0) {
    Serial.println("Erfolg - Uhrzeit wurde gesetzt.");
    return 0;
  }
  else
  {
    Serial.println("Fehlschlag - Uhrzeit konnte nicht gesetzt werden.");
    Serial.println("Eventuell wurden VCC und Ground nicht korrekt angegeben.");
    return 1;
  }
  
}
//**************************************************

// Function to get temperature
void getReadings() {
  ///************************************
  //    --------RTC NEW--------------
  Serial.println(RTC.get());
//*****************************************************
  temperature = dht.readTemperature(); // Temperature in Celsius
  humidity = dht.readHumidity(); // Humditiy in percentage
  if ( isnan(temperature) || isnan(humidity)) {
    Serial.print("Can not read from DHT22 sensor");
  } else {
    if (debug) {
      Serial.print("Temperature: ");
      Serial.println(temperature);
      Serial.print("Humditiy: ");
      Serial.println(humidity);
    }
  }
}
//_____________________________________________________________________________wifiConnect
void wifiConnect( const char* ssid ,const char* password ) {
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("Mac address: " + WiFi.macAddress());
}
//_____________________________________________________________________________sendDataHTTP
void sendDataHTTP() {
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
    Serial.println("Connected to the WiFi network");
    readFile(SD, "/sensor.csv");
    HTTPClient http;
    Serial.println(content);
    http.begin("http://193.196.52.234/api/sensor.php?id=" + name);
    int httpCode = http.POST(content);                                        //Make the request

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
      //uhrzeit_setzen (payload);
      //String dateTime = payload.remove(0, 1);
      String fileName = "/" + payload + "." + "csv";
      renameFile(SD, "/sensor.csv", fileName);
    }
    else {
      Serial.println("Error on HTTP request");
    }
    http.end();
    //  connected = false;
    WiFi.mode(WIFI_OFF);
    //    ledState = false;
    content = "";
  } else {
    Serial.println("No wifi connection");
  }
}
//_____________________________________________________________________________logSDCard
// Write the sensor readings on the SD card in sensor.csv
void logSDCard() {
  dataMessage = "\n" + String(temperature) + ";" + String(humidity) + ";" + RTC.get();
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/sensor.csv", dataMessage.c_str());
}

void loop() {
  // The ESP32 Audio Kit will be in deep sleep
  // it never reaches the loop()
 
}
