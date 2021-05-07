/*********
  BEEMON v0.1
  Abdualah Al Rade (91alab1bif@hft-stuttgart.de),
  Medjen Izairi (91izme1bif@hft-stuttgart.de),
  Safak Karman (91kasa@hft-stuttgart.de),
  Henry Bahnasawy (91elhe1bif@hft-stuttgart.de)
*********/



// Libraries for the SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Library for DHT sensor
#include "DHT.h"

// Libraries for WiFi and HTTP
#include <WiFi.h>
#include <HTTPClient.h>

//User_info
 static String name;
// Define deep sleep options
uint64_t uS_TO_S_FACTOR = 1000000;  // Conversion factor for micro seconds to seconds
// Sleep for 10 seconds
static uint64_t TIME_TO_SLEEP = 10;
//Time TIME_TO_recording and Sampling_Rate
static uint64_t TIME_TO_recording ;
static uint64_t Sampling_Rate ;


// Replace with your network credentials
static String config_SSID ="bemmon";
static  String config_Passowrd ="bemoon";


bool debug = true;

#define DHTPIN 0               // DHT Pin 0
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
String content;

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

//_______________________________________________________________getValue
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
//_______________________________________________________________________readFileconfig
void readFileconfig(fs::FS &fs, const char * path) {
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
  file.close();
   name = (getValue(line,';',0));
   config_SSID = getValue(line,';',1).c_str();
   config_Passowrd = getValue(line,';',2).c_str();
   TIME_TO_SLEEP =getValue(line,';',4).toInt();
   TIME_TO_recording = getValue(line,';',5).toInt();
   Sampling_Rate = getValue(line,';',6).toInt();
   
   


}
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
// rename file on SD card
void renameFile(fs::FS &fs, String path1, String path2) {
  Serial.println("Renaming file " + path1 + " to " + path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}
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
//_____________________________________________________________________________setup
void setup() {
  // Start serial communication for debugging purposes
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
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
   readFileconfig(SD, "/config.txt");

  // If the sensor.txt file doesn't exist
  // Create a file on the SD card
  File file = SD.open("/sensor.txt");
  if (!file) {
    Serial.println("File sensor.txt doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/sensor.txt", "");
  } else {
    Serial.println("File already exists");
  }
  file.close();
  // Enable Timer wake_up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Start the DHT22 sensor
  dht.begin();
  delay(4000); // 4 seconds pre-reading time for the sensor (the sensor is sometimes a little slow)

  if (button) { // check if KEY1 was pressed
    wifiConnect(config_SSID.c_str() ,config_Passowrd.c_str());
    sendDataHTTP();
  } else {
    getReadings();
    logSDCard();
  }
  // Configure the wake up source KEY1 (Pin 36)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

  // Start deep sleep
  Serial.println("DONE! Going to sleep now.");
    Serial.println("________________________________________");

  
  esp_deep_sleep_start();
}

// Function to get temperature
void getReadings() {
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
    readFile(SD, "/sensor.txt");
    HTTPClient http;
    Serial.println(content);
    http.begin("http://193.196.52.234/sensor2.php");
    content += "-" + name;
    int httpCode = http.POST(content);                                        //Make the request

    if (httpCode > 0) { //Check for the returning code

      String payload = http.getString();
      Serial.println(httpCode);
      Serial.println(payload);
      String fileName = "/s" + payload + "." + "txt";
      renameFile(SD, "/sensor.txt", fileName);
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
// Write the sensor readings on the SD card in sensor.txt
void logSDCard() {
  dataMessage = String(temperature) + "," + String(humidity) + ";";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/sensor.txt", dataMessage.c_str());
}

void loop() {
  // The ESP32 Audio Kit will be in deep sleep
  // it never reaches the loop()
 
}
