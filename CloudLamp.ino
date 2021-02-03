#include <Adafruit_NeoPixel.h>

#define LED_OUT 6 // LED Output
#define D_IN 7 // Mic Digital Input (sound sensor)
#define MC_NUMBER 50 // Number of WS2811 microcontrollers on LED strip 

#define MIN_LED_DURATION 90 // Amount of time the LEDs light up (in ms)
#define MAX_LED_DURATION 150 // Amount of time the LEDs light up (in ms)
#define MIN_LED 3 // Min number of LEDs per flash
#define MAX_LED 5 // Max number of LEDs per flash
#define MIN_LED_DELAY 50 // Min time between LED in a single flash (in ms)
#define MAX_LED_DELAY 200 // Max time between LED in a single flash (in ms)
#define MIN_FLASH_DELAY 1000 // Min time between flashes (in ms)
#define MAX_FLASH_DELAY 2000 // Max time between flashes (in ms)

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MC_NUMBER, LED_OUT, NEO_KHZ800 + NEO_GRB);

uint32_t white = strip.Color(255, 255, 255);

void setup () {

  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)
  
  pinMode (D_IN, INPUT); // Set up input pin for the mic
       
  Serial.begin (9600); // Serial output with 9600 bps
}

void loop () {
    if(digitalRead (D_IN)) {
      flash();
    }
}

void flash() {
  int leds = randInRange(MIN_LED, MAX_LED);
  int flash_delay = randInRange(MIN_FLASH_DELAY, MAX_FLASH_DELAY);
 
  for(int i=0; i < leds; i++) {
    // Get index of random LED on the strip
    int led_idx = randInRange(0, MC_NUMBER - 1);
    // Light up random LED
    strip.setPixelColor(led_idx, white);
    strip.setPixelColor(led_idx + 1, white);
    strip.show();

    int light_on = randInRange(MIN_LED_DURATION, MAX_LED_DURATION);
    delay(light_on);

    int randomNum = rand();
    if ((randomNum % 2) == 0){
      // All off after one led
      strip.clear();
      strip.show(); 
    }

    // Random delay between flashes
    int led_delay = randInRange(MIN_LED_DELAY, MAX_LED_DELAY);
    delay(led_delay);
  }

  // All off after a flash
  strip.clear();
  strip.show();

  delay(flash_delay);

  Serial.println();
}

int randInRange(int min, int max) {
  int range = 1 + (max - min);
  int limit = range * (RAND_MAX / range);

  do {
    int r = rand();
    if (r <= limit)
        return min + (r % range);
  } while (true);
}
