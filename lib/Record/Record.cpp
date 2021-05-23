#include "Arduino.h"
#include "Record.h"

/***********************************************************************************************/
const int headerSize = 44;
// int const waveDataSize = record_time * 88000;
const int numCommunicationData = 8000;
int const numPartWavData = numCommunicationData/4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;


/**************************************************************************************************/
int I2S_Read(char *data, int numData){
  return i2s_read_bytes(I2S_NUM_0, (char *)data, numData, portMAX_DELAY);
}

void I2S_Write(char *data, int numData){
  i2s_write_bytes(I2S_NUM_0, (const char *)data, numData, portMAX_DELAY);
}


/************************************************************************************************/

void I2S_setup(){
Serial.printf("Connect to AC101 codec... ");
  while (not ac.begin(IIC_DATA, IIC_CLK))
  {
    Serial.printf("Failed!\n");
    delay(1000);
  }
  
   ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
   ac.SetI2sSampleRate(AC101::SAMPLE_RATE_44100);
   ac.SetI2sClock(AC101::BCLK_DIV_16, false, AC101::LRCK_DIV_32, false);
   ac.SetI2sMode(AC101::MODE_SLAVE);
   ac.SetI2sWordSize(AC101::WORD_SIZE_16_BITS);
   ac.SetI2sFormat(AC101::DATA_FORMAT_I2S);

}

/***********************************************************************************************/
//i2S_Init
void I2S_Init(){
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 44100,
      .bits_per_sample = i2s_bits_per_sample_t(32),
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = 0,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false
    };
  
    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = 27;
    pin_config.ws_io_num = 26;
    pin_config.data_out_num = -1;
    pin_config.data_in_num = 35;

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_clk(I2S_NUM_0, 44100, i2s_bits_per_sample_t(32), I2S_CHANNEL_STEREO);
}

/****************************************************************************************CreateWavHeader*/

void CreateWavHeader(byte* header, int waveDataSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSizeMinus8 = waveDataSize + 44 - 8;
  header[4] = (byte)(fileSizeMinus8 & 0xFF);
  header[5] = (byte)((fileSizeMinus8 >> 8) & 0xFF);
  header[6] = (byte)((fileSizeMinus8 >> 16) & 0xFF);
  header[7] = (byte)((fileSizeMinus8 >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;  // linear PCM
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;  // linear PCM
  header[21] = 0x00;
  header[22] = 0x01;  // monoral
  header[23] = 0x00;
  header[24] = 0x44;  // sampling rate 44100
  header[25] = 0xAC;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x88;  // Byte/sec = 44100x2x1 = 88200
  header[29] = 0x58;
  header[30] = 0x01;
  header[31] = 0x00;
  header[32] = 0x02;  // 16bit monoral
  header[33] = 0x00;
  header[34] = 0x10;  // 16bit
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(waveDataSize & 0xFF);
  header[41] = (byte)((waveDataSize >> 8) & 0xFF);
  header[42] = (byte)((waveDataSize >> 16) & 0xFF);
  header[43] = (byte)((waveDataSize >> 24) & 0xFF);
}

/********************************************************************************************************Start*/
void record_start(int record_time, String recFileName){
   int const waveDataSize = record_time * 88000;
 Serial.println("start");
  CreateWavHeader(header, waveDataSize);
  SD.remove(recFileName);
  file = SD.open(recFileName, FILE_WRITE);
  if (!file) return;
  file.write(header, headerSize);
  pinMode(35, INPUT);
  pinMode(27, OUTPUT);
  pinMode(26, OUTPUT);
  I2S_Init();
  
  for (int j = 0; j < waveDataSize/numPartWavData; ++j) {
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData/8; ++i) {
      partWavData[2*i] = communicationData[8*i + 2];
      partWavData[2*i + 1] = communicationData[8*i + 3];
    }
    file.write((const byte*)partWavData, numPartWavData);
  }
  file.close();
  Serial.println("finish");

}

/***********************************************************************************************/

