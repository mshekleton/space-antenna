#include <driver/adc.h>
#include <PhoneDTMF.h>
PhoneDTMF dtmf = PhoneDTMF(300, 1.0f);

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
    if(button == "1"){
      //Output tone and morse code
      
    }
    //Serial.println("Listening...");
    //delay(500);

}
