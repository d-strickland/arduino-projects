#include <Adafruit_NeoPixel.h>
#include <specrend.h>

#define MIC_PIN 0 // analog pin to which the microphone is connected
#define LED_PIN 6 // digital pin to which the LED string is connected
#define NLED 60 // number of LEDs in string
// Peak detection states
#define INITIAL 0
#define POS_SLOPE 1
#define NEG_SLOPE 2
#define PEAK_FOUND 3
#define N 256  // Number of samples to take before computing frequency

#define VS 340.29         // Speed of sound (m/s)
#define MIN_F 100         // Bottom of instrument range (Hz)
#define MAX_F 1200        // Top of instrument range
#define MIN_WS VS/MAX_F   // Shortest wavelength instrument can produce (m)
#define MAX_WS VS/MIN_F   // Longest wavelength instrument can produce
#define MIN_WL 400        // Shortest visual wavelength (nanometers)
#define MAX_WL 730        // Longest visual wavelength

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
int sum = 0;
int sum_old;
int thresh = 0;
float freq;
byte pd_state = 0;
char samples[N];
int period;
int wavelength;
float r, g, b;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");
  strip.begin();
  strip.show();  // Initialize the strip with all lights turned off.
}

void loop() {
  samples[n] = analogRead(MIC_PIN) - 62; // Neutral mic reading should be 64, but seems to be ~62.
  if (++n >= N) {
//    Serial.print("Period for 256 samples: ");
//    Serial.print(millis()-t0);
//    Serial.println();
//    t0 = millis();
    n = 0;
    
    freq = frequency(samples, N);
    wavelength = fs_to_wl(freq);
    wavelength_to_rgb(wavelength, &r, &g, &b);
    Serial.print(freq);
    Serial.print(" Hz, ");
    Serial.print(wavelength);
    Serial.print(" nm: ");
    Serial.print((int)(r*255));
    Serial.print(", ");
    Serial.print((int)(g*255));
    Serial.print(", ");
    Serial.print((int)(b*255));
    Serial.println();
    for (int i=0; i < NLED; i++) {
      strip.setPixelColor(i, (int)(r*255), (int)(g*255), (int)(b*255));
    }
    strip.show();
  }
}

float frequency(char samples[], int len) {
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

/*  Map a sound frequency in Hz to a light wavelength in nm, where
    hz_to_nm(MIN_F) = MAX_WL
    hz_to_nm(MAX_F) = MIN_WL
*/
int fs_to_wl(float f) {
  float lambda = VS / ((float)f);  // Wavelength of fundamental tone
  return (int) (MIN_WL + ((lambda - MIN_WS) * (MAX_WL - MIN_WL) / (MAX_WS - MIN_WS)));
}


