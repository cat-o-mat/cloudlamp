#include <Adafruit_NeoPixel.h>
#define LED_OUT 6         // LED Output
#define D_IN 7            // Mic Digital Input (sound sensor)
#define MC_NUMBER 50      // Number of WS2811 microcontrollers on LED strip
#define BUTTON 2          // Button to switch between modes
#define MIN_FLASH_ON 90   // Amount of time the LEDs light up (in ms)
#define MAX_FLASH_ON 150  // Amount of time the LEDs light up (in ms)
#define MIN_LED 3         // Min number of LEDs per flash
#define MAX_LED 5         // Max number of LEDs per flash
#define MIN_FLASH_OFF 50  // Min time between LED in a single flash (in ms)
#define MAX_FLASH_OFF 200 // Max time between LED in a single flash (in ms)
#define MIN_DELAY_BETWEEN_FLASH_CYCLES 1000 // Min time between flashes (in ms)
#define MAX_DELAY_BETWEEN_FLASH_CYCLES 2000 // Max time between flashes (in ms)
#define MIN_FLASH_LINE_SIZE                                                    \
  1 // Min length of flash line (number of WS2800 to be turned on)
#define MAX_FLASH_LINE_SIZE                                                    \
  10 // Min length of flash line (number of WS2800 to be turned on)
#define RAINBOW_WAIT 20 // Controlls the speed of the rainbow mode
#define ARRSIZE(x) (sizeof(x) / sizeof(x[0]))

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip =
    Adafruit_NeoPixel(MC_NUMBER, LED_OUT, NEO_KHZ800 + NEO_GRB);

uint32_t white = strip.Color(255, 255, 255);
static unsigned long int delay_time = 0;

void setup() {
  strip.begin();            // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();             // Turn OFF all pixels
  strip.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(D_IN, INPUT);   // Set up input pin for the mic
  pinMode(BUTTON, INPUT); // Set up input pin for the mic

  Serial.begin(9600); // Serial output with 9600 bps
}

int randomInRange(int min, int max) {
  int range = 1 + (max - min);
  int limit = range * (RAND_MAX / range);

  do {
    int r = rand();
    if (r <= limit)
      return min + (r % range);
  } while (true);
}

enum FlashOnSoundCycle {
  FLASH_ON,
  FLASH_OFF,
  WAIT
} __attribute__((packed));

/* FlashOnSound */
struct FlashOnSoundState {
  uint16_t remaining_delay;
  int num_of_leds_to_light_up;
  uint16_t i;
  enum FlashOnSoundCycle cycle_step;
};

void *flashOnSoundConstructor() {
  strip.clear();
  strip.show();
  struct FlashOnSoundState *state =
      (struct FlashOnSoundState *)calloc(1, sizeof(struct FlashOnSoundState));
  state->i = 0;
  state->num_of_leds_to_light_up = randomInRange(MIN_LED, MAX_LED);
  state->remaining_delay = 0;
  state->cycle_step = FLASH_ON;
  return state;
}

void flashOnSoundDestructor(void *state) { free(state); }

void flashOnSound(void *in_state) {
  // if (digitalRead(D_IN)) {

  struct FlashOnSoundState *state = ((struct FlashOnSoundState *)in_state);

  static unsigned long last_time = millis();
  unsigned long curr_time = millis();

  if (state->remaining_delay > 0) {
    unsigned long time_passed = max(curr_time - last_time, 0);
    if (state->remaining_delay <= time_passed) {
      state->remaining_delay = 0;
    } else {
      state->remaining_delay -= time_passed;
    }
  }

  if (state->cycle_step == FLASH_ON && state->remaining_delay == 0 &&
      (digitalRead(D_IN) || state->i > 0)) {
    // Get index of random LED on the strip
    int led_idx = randomInRange(0, MC_NUMBER - 1);
    // Light up random LED
    strip.setPixelColor(led_idx, white);
    strip.setPixelColor(led_idx + 1, white);
    strip.show();

    state->remaining_delay = randomInRange(MIN_FLASH_ON, MAX_FLASH_ON);
    state->cycle_step = FLASH_OFF;
  }

  if (state->cycle_step == FLASH_OFF && state->remaining_delay == 0) {
    // Keep LED on (almost) every second round
    int randomNum = rand();
    if ((randomNum % 2) == 0) {
      // All off after one led
      strip.clear();
      strip.show();
    }

    state->remaining_delay = randomInRange(MIN_FLASH_OFF, MAX_FLASH_OFF);
    state->i++;
    if (state->i < state->num_of_leds_to_light_up) {
      state->cycle_step = FLASH_ON;

    } else {
      state->cycle_step = WAIT;
    }
  }

  if (state->cycle_step == WAIT && state->remaining_delay == 0) {
    // All off after a flash
    strip.clear();
    strip.show();

    state->remaining_delay = randomInRange(MIN_DELAY_BETWEEN_FLASH_CYCLES,
                                           MAX_DELAY_BETWEEN_FLASH_CYCLES);
    // set up for next cycle
    state->cycle_step = FLASH_ON;
    state->i = 0;
    state->num_of_leds_to_light_up = randomInRange(MIN_LED, MAX_LED);
  }

  last_time = curr_time;
}

/* White */

void *allWhiteConstructor() {
  bool *state = (bool *)malloc(sizeof(bool));
  *state = true;
  return state;
}

void allWhiteDestructor(void *state) { free(state); }

void allWhite(void *in_state) {
  bool *state = ((bool *)in_state);
  if (*state) {
    for (int i = 0; i < MC_NUMBER; i++) {
      strip.setPixelColor(i, white);
      strip.show();
    }
    *state = false;
  }
}

/* Rainbow */

// Input a value 0 to 255 to get a color value.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

struct RainbowState {
  uint8_t remaining_delay;
  uint16_t j;
};

void *rainbowCycleConstructor() {
  return calloc(1, sizeof(struct RainbowState));
}

void rainbowCycleDestructor(void *state) { free(state); }

void rainbowCycle(void *in_state) {
  struct RainbowState *state = ((struct RainbowState *)in_state);

  static unsigned long last_time = millis();
  unsigned long curr_time = millis();

  if (state->remaining_delay > 0) {
    unsigned long time_passed = max(curr_time - last_time, 0);
    if (state->remaining_delay <= time_passed) {
      state->remaining_delay = 0;
    } else {
      state->remaining_delay -= time_passed;
    }
  }

  if (state->remaining_delay == 0) {
    state->j++;

    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(
          i, Wheel(((i * 256 / strip.numPixels()) + state->j) & 255));
    }
    strip.show();

    if (state->j == 256) {
      *state = {0};
    } else {
      state->remaining_delay = RAINBOW_WAIT;
    }
  }

  last_time = curr_time;
}

/* Controlls */

struct ProgramInfo {
  void *(*constructor)();
  void (*step)(void *state);
  void (*destructor)(void *state);
};

struct ProgramInfo PROGRAMMS[] = {{.constructor = &flashOnSoundConstructor,
                                   .step = &flashOnSound,
                                   .destructor = &flashOnSoundDestructor},
                                  {.constructor = &rainbowCycleConstructor,
                                   .step = &rainbowCycle,
                                   .destructor = &rainbowCycleDestructor},
                                  {.constructor = &allWhiteConstructor,
                                   .step = &allWhite,
                                   .destructor = &allWhiteDestructor}};
int mode_state = 0; // index of light mode (starts at 0 an max=NUM_MODES)
bool button_down = false;
void *state = NULL;

void loop() {
  bool button_pressed = false;

  if (digitalRead(BUTTON)) {
    button_down = true;
  } else if (button_down) {
    // butoon was released just now
    button_down = false;
    button_pressed = true;
  }

  if (button_pressed) {
    Serial.println(sizeof (enum FlashOnSoundCycle));
    if (state != NULL) {
      PROGRAMMS[mode_state % ARRSIZE(PROGRAMMS)].destructor(state);
      state = NULL;
    }
    mode_state++;
  }

  struct ProgramInfo prog = PROGRAMMS[mode_state % ARRSIZE(PROGRAMMS)];
  if (state == NULL) {
    state = prog.constructor();
  }

  prog.step(state);
}
