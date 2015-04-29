#include <Arduino.h>
#include <avr/pgmspace.h>

#include <Adafruit_NeoPixel.h>

#define DEBUG
#define MIC_PIN 0 // analog pin to which the microphone is connected
#define LED_PIN 6 // digital pin to which the LED string is connected
#define NLED 60 // number of LEDs in string

#define N 256  // Number of samples to take before computing period of signal.

#define N_COLORS 60

/* These values worked well for the range of a trombone using an Arduino Uno.
 * Actual values will depend on the hardware, sample size, and source of the
 * signal.
 */
#define MIN_PERIOD 15
#define MAX_PERIOD 125

// Peak detection states
#define INITIAL 0
#define POS_SLOPE 1
#define NEG_SLOPE 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NLED, LED_PIN, NEO_GRB + NEO_KHZ800);
int n = 0;
int sum = 0;
int thresh = 0;
byte pd_state = 0;
int samples[N];
int sum_old, period, wavelength, r, g, b;

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("setup");
#endif
  strip.begin();
  strip.show();  // Initialize the strip with all lights turned off.
}



/*******************************************************************************
 *                             M A I N   L O O P 
 ******************************************************************************/


void loop() {
  samples[n] = analogRead(MIC_PIN) - 62;   // Neutral mic reading should be 64,
                                           // but seems to be ~62.
  if (++n >= N) {
    n = 0;
    
    period = get_period(samples, N);
    period_to_rgb(period, &r, &g, &b);
#ifdef DEBUG
    Serial.print("Period: ");
    Serial.println(period);
    Serial.print("RGB: ");
    Serial.print(r);
    Serial.print(" ");
    Serial.print(g);
    Serial.print(" ");
    Serial.print(b);
    Serial.println();
#endif
    for (int i=0; i < NLED; i++) {
      strip.setPixelColor(i, r, g, b);
    }
    strip.show();
  }
}

// Autocorrelation on peak amplitudes.
// www.instructables.com/id/Reliable-Frequency-Detection-Using-DSP-Techniques/
int get_period(int samples[], int len) {
  pd_state = INITIAL;
  for(int i=0; i < len; i++)
  {
    sum_old = sum;
    sum = 0;
    for(int k=0; k < len-i; k++) sum += (samples[k])*(samples[k+i])/128;
    
    // Peak detection state machine
    if (pd_state == NEG_SLOPE && (sum-sum_old) <= 0) {
      return i;
    }
    if (pd_state == POS_SLOPE && (sum > thresh) && (sum-sum_old) > 0) {
      pd_state = NEG_SLOPE;
    }
    if (pd_state == INITIAL) {
      thresh = sum * 0.5;
      pd_state = POS_SLOPE;
    }
  }
  return -1;
}

/* RGB color representations for a range of wavelengths in the visual spectrum.
 * Computed with much help from http://www.fourmilab.ch/documents/specrend
 * and fudged a bit from there to get a prettier range of colors.
 */
const static int colors[N_COLORS][3] = {
    { 82,  0,255}, { 74,  0,255}, { 70,  0,255}, { 67,  0,255}, { 64,  0,255},
    { 59,  0,255}, { 53,  0,255}, { 45,  0,255}, { 33,  0,255}, { 18,  0,255},
    {  0,  1,255}, {  0, 31,255}, {  0, 70,255}, {  0,118,255}, {  0,176,255},
    {  0,239,255}, {  0,255,211}, {  0,255,174}, {  0,255,150}, {  0,255,132},
    {  0,255,118}, {  0,255,106}, {  0,255, 96}, {  0,255, 85}, {  0,255, 72},
    {  0,255, 57}, {  0,255, 37}, {  0,255,  7}, { 34,255,  0}, { 87,255,  0},
    {154,255,  0}, {243,255,  0}, {255,177,  0}, {255,120,  0}, {255, 81,  0},
    {255, 52,  0}, {255, 31,  0}, {255, 16,  0}, {255,  5,  0}, {255,  0,  3},
    {255,  0,  9}, {255,  0, 13}, {255,  0, 16}, {255,  0, 19}, {255,  0, 23},
    {255,  0, 26}, {255,  0, 29}, {255,  0, 36}, {255,  0, 45}, {255,  0, 62}
};

void period_to_rgb(int p, int *re, int *gr, int *bl) {
  if (p == -1) {
    *re = 0;
    *gr = 0;
    *bl = 0;
    return;
  }
  int k = N_COLORS * (period - MIN_PERIOD) / (MAX_PERIOD - MIN_PERIOD);
#ifdef DEBUG
  Serial.print("Color bucket: ");
  Serial.println(k);
#endif
  *re = colors[k][0];
  *gr = colors[k][1];
  *bl = colors[k][2];
}
