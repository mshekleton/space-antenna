

#include <driver/adc.h>
#include <PhoneDTMF.h>
#include <Tone32.h>
#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"

#define BUZZER_PIN 16
#define BUZZER_CHANNEL 0

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23    // SD Card
#define SPI_MISO      19
#define SPI_SCK       18
 
#define I2S_DOUT      25
#define I2S_BCLK      27    // I2S
#define I2S_LRC       26

PhoneDTMF dtmf = PhoneDTMF(300, 1.0f);
Audio audio;
int PTT = 4;

const char CALLSIGN[] = "-.- -.-. .---- .-. .--. .--";

void setup()
{
  Serial.begin(115200);
  pinMode (PTT, OUTPUT);
  
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);
  uint32_t freq = dtmf.begin((uint8_t)ADC1_CHANNEL_6_GPIO_NUM, 12000);
  pinMode(SD_CS, OUTPUT);      
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  if(!SD.begin(SD_CS))
  {
    Serial.println("Error talking to SD card!");
    //while(true);  // end program
  }
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15); // 0...21
  audio.connecttoFS(SD,"/foxbox.mp3");
  Serial.println("Init complete.");
}

void loop() {
  if (0){} /**
  if (audio.isRunning()) {
    digitalWrite(PTT, HIGH); //PTT on
    audio.loop();
    Serial.println("Loop running...");
  }**/
  else {
    digitalWrite(PTT, LOW); //PTT off
    Serial.println("sampling...");
    float     afMag[8];
    uint8_t   tones = dtmf.detect(afMag, -1.0f);
    char      button = dtmf.tone2char(tones);
  
    if(button > 0) {
      Serial.print(button); Serial.println(" pressed");
    }
    if(button == '1'){
      Serial.println("Playing...");
      delay(500);
      //audio.loop();
      //identify();
    }
  }
}

void identify(){
  for(int i =0; i < strlen(CALLSIGN); i++ ) {
    char c = CALLSIGN[i];
    Serial.print(c);
    switch (CALLSIGN[i]) {
      case '-':
        tone(BUZZER_PIN, NOTE_D8, 150, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        delay(150);
        break;
      case '.':
        tone(BUZZER_PIN, NOTE_D8, 50, BUZZER_CHANNEL);
        noTone(BUZZER_PIN, BUZZER_CHANNEL);
        delay(150);
        break;
      case ' ':
        delay(200);
        break;
      default:
        break;
    }
  }
}
