
#include "FastLED.h"
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define LIGHT_PIN   7
#define CLOCK_PIN   2
#define CLK_PIN     13
#define LED_TYPE    NEOPIXEL
#define COLOR_ORDER GRB
#define NUM_LEDS    300
#define BRIGHTNESS 250
#define NUM_SEGMENTS 7
#define SUNRISE 0
#define DAY 1
#define SUNSET 2
#define NIGHT 3
#define OFF 4
#define SUNRISE_START 0
#define DAY_START 0
#define SUNSET_START 0
#define NIGHT_START 3
#define OFF_START 1
#define INTERVAL 20

//---NIGHT MODE VARIABLES---//

#define NUM_STARS 10
#define SHOOTING_STAR_ODDS 1500
#define STAR_MIN_BRIGHTNESS 50
#define STAR_MAX_BRIGHTNESS 250



//---GENERAL VARIABLES---//

CRGB leds[NUM_LEDS];


int current_mode = DAY;

int segments[NUM_SEGMENTS][2] = {
  {0, 0},
  {0, 0},
  {0, NUM_LEDS / 5},
  {NUM_LEDS / 5, (NUM_LEDS / 5 * 2)},
  {NUM_LEDS / 5 * 2, (NUM_LEDS / 5 * 3)},
  {NUM_LEDS / 5 * 3, (NUM_LEDS / 5 * 4)},
  {NUM_LEDS / 5 * 4, NUM_LEDS / 5 * 5}
};

CRGB night_palette[NUM_SEGMENTS] = {
  CRGB(64, 0, 64),
  CRGB(5, 0, 51),

  CRGB(0, 0, 255),
  CRGB(64, 0, 64),
  CRGB(5, 0, 51),
  CRGB(0, 0, 64),
  CRGB(36, 0, 64)
};

CRGB sunrise_palette[NUM_SEGMENTS] = {
  CRGB(255, 148, 54),
  CRGB(255, 61, 23),
  
  CRGB(255, 61, 23),
  CRGB(255, 148, 54),
  CRGB(252, 168, 0),
  CRGB(255, 143, 143),
  CRGB(255, 147, 31)
};

CRGB sunset_palette[NUM_SEGMENTS] = {
  CRGB(0, 2, 99),
  CRGB(133, 78, 20),

  CRGB(217, 0, 255),
  CRGB(255, 92, 64),
  CRGB(50, 0, 115),
  CRGB(255, 92, 64),
  CRGB(48, 43, 79)

};

CRGB day_palette[NUM_SEGMENTS] = {
  CRGB(255, 211, 77),
  CRGB(255, 211, 77),

  CRGB(255, 211, 77),
  CRGB(255, 211, 77),
  CRGB(255, 211, 77),
  CRGB(255, 211, 77),
  CRGB(255, 211, 77)
};

CRGB off_palette[NUM_SEGMENTS] = {
  CRGB(0, 0, 0),
  CRGB(0, 0, 0),
  CRGB(0, 0, 0),
  CRGB(0, 0, 0),
  CRGB(0, 0, 0),
  CRGB(0, 0, 0),
  CRGB(0, 0, 0)
};

CRGB current_palette[NUM_SEGMENTS];

CRGB color_start[NUM_SEGMENTS];
CRGB color_target[NUM_SEGMENTS];
CRGB color_current[NUM_SEGMENTS];

int k[NUM_SEGMENTS] = {0, 0, 0, 0, 0, 0, 0};


//-----NIGHT MODE VARIABLES----//

//Fading stars
//starBrightness
CHSV starValue = CHSV(255, 0, STAR_MIN_BRIGHTNESS);
//gettingBrighter
boolean fadeWhite = true;
//MAX_NUM_STARS
int starIndices[NUM_STARS];
int numVisibleStars;
int starDelay = 100;
int starFade = 1;

//Shooting stars
int shootingIndex;
int shootingDistance;
boolean shootingStarActive;

//---GENERAL FUNCTIONS----//

void fillSegment(int s[], CRGB c) {
  for (int i = s[0]; i < s[1]; ++i) {
    leds[i] = c;
  }
}

boolean equalColors(CRGB a, CRGB b) {
  return (a.r == b.r && a.g == b.g && a.b == b.b);
}

void setPalette(CRGB new_palette[]) {
  for (int i = 0; i < NUM_SEGMENTS; ++i) {
    current_palette[i] = new_palette[i];
  }
}

//---NIGHT MODE FUNCTIONS---//

void drawLightning() {
  CRGB white = CRGB(255, 255, 255);
  
}

void drawShootingStar() {
  if (!shootingStarActive) {
    if (random(SHOOTING_STAR_ODDS) == 1) {
      shootingStarActive = true;
      shootingIndex = random(200);
      shootingDistance = random(20, 60);
    }
  }
  if (shootingDistance > 0) {
    shootingDistance--;
    shootingIndex++;
    CRGB white = CRGB(100, 100, 100);
    leds[shootingIndex] = white;
    leds[shootingIndex + 1] = white;
    leds[shootingIndex + 2] = white;
    leds[shootingIndex + 3] = white;
  }
  else {
    shootingStarActive = false;
  }
}

void drawFadingStars() {
  if (starValue.val == STAR_MAX_BRIGHTNESS) {
    fadeWhite = false;
  }
  if (starValue.val <= STAR_MIN_BRIGHTNESS) {
    fadeWhite = true;
    if (starDelay > 0) {
      --starDelay;
      for (int i = 0; i < numVisibleStars; ++i) {
        leds[starIndices[i]] = CHSV(255, 0, STAR_MIN_BRIGHTNESS);
        leds[starIndices[i]] = CRGB(123, 232, 0);
        leds[starIndices[i]] = leds[starIndices[i] + 1];
      }
      return;
    }
    else {
      generateStars();
      starValue.val = STAR_MIN_BRIGHTNESS;
      starDelay = random(30, 500);
    }
  }
  if (fadeWhite) {
    int fade = starFade;
    if (starValue.val + starFade > STAR_MAX_BRIGHTNESS) {
      fade = STAR_MAX_BRIGHTNESS - starValue.val;
    }
    starValue = CHSV(255, 0, starValue.val + fade);
  }
  else {
    int fade = starFade;
    if (starValue.val - starFade <= STAR_MIN_BRIGHTNESS) {
      starValue = CHSV(255, 0, STAR_MIN_BRIGHTNESS);
    }
    else {
      starValue = CHSV(255, 0, starValue.val - fade);
    }
  }

  for (int i = 0; i < numVisibleStars; ++i) {
    leds[starIndices[i]] = starValue;
  }
}

boolean towardsWhite(CRGB c, CRGB p) {
  return (c.g >= p.g);
}

void generateStars() {
  numVisibleStars = random(NUM_STARS);
  starFade = random(1, 10);
  for (int i = 0; i < NUM_STARS; ++i) {
    starIndices[i] = random(NUM_LEDS);
  }
}


//---SETUP/LOOP---//

void testTime() {
  int m = getCurrentMinute();
  if (m > 12 && m <= 14) {
    fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
  }
  else {
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  }
  FastLED.show();

}


void setup() {
  delay(3000);
  while (!Serial); // for Leonardo/Micro/Zero
  Serial.begin(9600);

  //led setup
  FastLED.addLeds<LED_TYPE, LIGHT_PIN>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //set time
  setSyncProvider(RTC.get);
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");

  setInitialMode();
  setInitialColors();

}

void loop() {
  runCloud();
  //testTime();
  //displayPalette(sunrise_palette);
}


void runCloud() {
  delay(INTERVAL);
  setMode();
  moveColors();
  FastLED.show();
}

void displayPalette(CRGB palette[]) {
  for (int i = 0; i < NUM_SEGMENTS; ++i) {
    fillSegment(segments[i], palette[i]);
  }


  FastLED.show();
}

int getCurrentHour() {
  Serial.print(hour());
  Serial.println();
  return hour();
}

int getCurrentMinute() {
  Serial.print(minute());
  Serial.println();
  return minute();   
}

void setInitialColors() {
  for (int i = 0; i < NUM_SEGMENTS; ++i) {
    color_target[i] = current_palette[i];
    color_current[i] = current_palette[i];
    color_start[i] = current_palette[i];
  }
}

void setInitialMode() {
  int h = getCurrentHour();
  if (h >= OFF_START && h < SUNRISE_START) {
    current_mode = OFF;
    setPalette(off_palette);
  }
  else if (h >= SUNRISE_START && h < DAY_START) {
    current_mode = SUNRISE;
    setPalette(sunrise_palette);
  }
  else if (h >= DAY_START && h < SUNSET_START) {
    current_mode = DAY;
    setPalette(day_palette);
  }
  else if (h >= SUNSET_START && h < NIGHT_START) {
    current_mode = SUNSET;
    setPalette(sunset_palette);
  }
  else {
    current_mode = NIGHT;
    setPalette(night_palette);
  }
  setTarget();
}


//transitionMode
void setMode() {
  int h = getCurrentHour();
  if (h == OFF_START && current_mode != OFF) {
    current_mode = OFF;
    setPalette(off_palette);
    setTarget();
  }
  if (h == SUNRISE_START && current_mode != SUNRISE) {
    current_mode = SUNRISE;
    setPalette(sunrise_palette);
    setTarget();
  }
  if (h == DAY_START && current_mode != DAY) {
    current_mode = DAY;
    setPalette(day_palette);
    setTarget();
  }
  if (h == SUNSET_START && current_mode != SUNSET) {
    current_mode = SUNSET;
    setPalette(sunset_palette);
    setTarget();
  }
  if (h == NIGHT_START && current_mode != NIGHT) {
    current_mode = NIGHT;
    setPalette(night_palette);
    setTarget();
  }
}

void setTarget() {
  for (int i = 0; i < NUM_SEGMENTS; ++i) {
    color_target[i] = current_palette[i];
    k[i] = 0;
    color_start[i] = color_current[i];
  }
}

void moveColors() {
  for (int i = 0; i < NUM_SEGMENTS; ++i) {
    if (equalColors(color_current[i], color_target[i])) {
      color_start[i] = color_current[i];
      color_target[i] = current_palette[random(NUM_SEGMENTS)];
      k[i] = 0;
    }
    color_current[i] = blend(color_start[i], color_target[i], k[i]);
    fillSegment(segments[i], color_current[i]);
    ++k[i];
  }
  if (current_mode == NIGHT) {
    drawFadingStars();
    drawShootingStar();
    drawLightning();
  }

}
