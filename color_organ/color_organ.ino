#include "Adafruit_NeoPixel.h"
#include "specrend.h"

#define MIC_PIN 0 // analog pin to which the microphone is connected
#define LED_PIN 6 // digital pin to which the LED string is connected
#define NLED 60 // number of LEDs in string
// Peak detection states
#define INITIAL 0
#define POS_SLOPE 1
#define NEG_SLOPE 2
#define PEAK_FOUND 3
#define N 256

#define MIN_HZ 100
#define MAX_HZ 1300
#define MIN_TEMP 1000
#define MAX_TEMP 10000

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NLED, LED_PIN, NEO_GRB + NEO_KHZ800);



/*******************************************************************************
 *                             M A I N   L O O P 
 ******************************************************************************/

const float sample_freq = 18043;  // Found empirically.

int n = 0;
long t0 = millis();
long sum = 0;
long sum_old;
int thresh = 0;
float freq;
byte pd_state = 0;
int samples[N];
long period;
double temp;
double *r, *g, *b;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  strip.begin();
  for (int i = 0; i < NLED; i++) {
    strip.setPixelColor(i, 0, 255, 0);
  }
  strip.show();  // Initialize the strip with all lights turned off.
}

void loop() {
  samples[n] = analogRead(MIC_PIN) - 62; // Neutral mic reading should be 64, but seems to be ~62.
  if (n >= N) {
//    Serial.print("Period for 256 samples: ");
//    Serial.print(millis()-t0);
//    Serial.println();
//    t0 = millis();
    n = 0;
    
    freq = frequency(samples, N);
    temp = freq_to_temp(freq);
   // spectrum_to_rgb(temp, r, g, b);
    Serial.print(freq);
    Serial.print(", ");
    Serial.print(temp);
    Serial.print(": ");
    Serial.print((int)(*r*255.));
    Serial.print(", ");
    Serial.print((int)(*g*255.));
    Serial.print(", ");
    Serial.print((int)(*b*255.));
    Serial.println();
  }
  n++;
}

float frequency(int samples[], int len) {
  // Autocorrelation on peak amplitudes.
  // www.instructables.com/id/Reliable-Frequency-Detection-Using-DSP-Techniques/
  pd_state = INITIAL;
  for(int i=0; i < len; i++)
  {
    sum_old = sum;
    sum = 0;
    for(int k=0; k < len-i; k++) sum += (samples[k])*(samples[k+i])/128;
    
    // Peak Detect State Machine
    if (pd_state == NEG_SLOPE && (sum-sum_old) <= 0) {
      period = i;
      pd_state = PEAK_FOUND;
    }
    if (pd_state == POS_SLOPE && (sum > thresh) && (sum-sum_old) > 0) {
      pd_state = NEG_SLOPE;
    }
    if (pd_state == INITIAL) {
      thresh = sum * 0.5;
      pd_state = POS_SLOPE;
    }
  }
  // Frequency identified in Hz
  return sample_freq/period;
}

/*  Linear conversion of a frequency to a temperature, where:
    freq_to_temp(MIN_HZ) = MIN_TEMP
    freq_to_temp(MAX_HZ) = MAX_TEMP
*/
double freq_to_temp(float f) {
  float fRatio = (f - MIN_HZ) / (MAX_HZ - MIN_HZ);
  return MIN_TEMP + (fRatio*(MAX_TEMP - MIN_TEMP));
}


