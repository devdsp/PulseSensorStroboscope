
#include <Adafruit_NeoPixel.h>

#include <Filters.h>
#include <Filters/Butterworth.hpp>
#include <AH/Timing/MillisMicrosTimer.hpp>

#define NUMLEDS 30.0
#define RATE (f_s*5)
#define F(frequency) (2*frequency/f_s)

const int16_t f_s = 1000;
const int16_t f_p = 100;

Timer<micros> s_timer = std::round(1e6 / f_s);
Timer<micros> p_timer = std::round(1e6 / f_p);

auto lpf1 = butter<1>( F( 0.15 ) );
auto lpf2 = butter<1>( F( 1.0  ) );

Adafruit_NeoPixel pixels(NUMLEDS, A4, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  
  // wait for the filters to settle a bit
  while(millis() < 2000) {
    if(s_timer) {
      double reading = (analogRead(A5)-512.0)/10.;
      lpf1(reading);
      lpf2(reading);
    }
  }
  pixels.begin();
}

double hf=0,lf=0;

void loop()
{
  if(s_timer) {
    double reading = (analogRead(A5)-512.0)/10.;
    double lowsignal = lpf1(reading);
    double highsignal = lpf2(reading)-lowsignal;
    lf = (lf*(RATE-1) + sq(lowsignal) ) / RATE;
    hf = (hf*(RATE-1) + sq(highsignal) ) / RATE;

    double ratio = hf/lf;
    
    if(p_timer) {
      for(uint8_t i = 0; i < NUMLEDS; i++ ) {
        float tri = constrain(asin(cos(-ratio*30+(float)i*PI*2.0/NUMLEDS))/1.5708,0,1);
        
        pixels.setPixelColor(
          i, 
          pixels.Color(
            constrain(ratio/10.0,0,1)*tri*127,
            constrain(1.0-ratio/10.0,0,1)*tri*127,
            ratio>10 && (millis()%1000 > 500) ? 255 : 0
          )
        );
      }
      pixels.show();

      Serial.println(ratio,6);
    }
  }
}
