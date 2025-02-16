#include <driver/adc.h>
#include <PhoneDTMF.h>
#include <Tone32.h>

#define BUZZER_PIN 16
#define BUZZER_CHANNEL 0

PhoneDTMF dtmf = PhoneDTMF(300, 1.0f);

//String CALLSIGN = String('-.- -.-. .---- .-. .--. .--');
const char CALLSIGN[] = "-.- -.-. .---- .-. .--. .--";

void setup()
{
    Serial.begin(9600);
    adc1_config_width(ADC_WIDTH_BIT_12); // set 12 bit (0-4096)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0); // do not use attenuation
    uint32_t freq = dtmf.begin((uint8_t)ADC1_CHANNEL_6_GPIO_NUM, 12000); // Use ADC 1, Channel 0 (GPIO36 on Wroom32)
    Serial.print("Begin RETURN: "); Serial.println(freq);
}

void loop() {
  // put your main code here, to run repeatedly:
    float     afMag[8];
    uint8_t   tones = dtmf.detect(afMag, -1.0f);
    char      button = dtmf.tone2char(tones);
    if(button > 0) {
      Serial.print(button); Serial.println(" pressed");
    }
    if(button == '1'){
    /**  
      //Output tone and morse code
      tone(BUZZER_PIN, NOTE_F2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_AS2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_AS2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_AS2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_A1, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_G1, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_F2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_E2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
      tone(BUZZER_PIN, NOTE_D2, 500, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(100);
**/

    identify();

    }
    //Serial.println("Listening...");
    //delay(500);

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
