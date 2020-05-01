#define LED_TYPE    DOTSTAR
#define COLOR_ORDER BGR
#define NUM_LEDS  167
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120


uint8_t BGiParams[] = {0, 0, 0, 0, NUM_LEDS}; // R, G, B, LED_start, len
uint8_t HSVParams[] = {0, 0, 0, 0, NUM_LEDS}; // H, S, V, LED_start, len
uint8_t BOWParams[] = {0, NUM_LEDS, 5, 0};  // Start, len, hue Delta
uint8_t FIRParams[] = {0, 0, 0, 0, NUM_LEDS}; // Hue, Sparking, Cooling, LED_start, len
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))



class Pattern{  
  int npattern;
  public:
    void setVal(int);
    int nextPattern(){return npattern+1;}
    
};

void Pattern::setVal(int num){
  npattern= num;
}


class Rainbow{
  public:
    void callPattern(uint8_t nGhue);
};

void Rainbow::callPattern(uint8_t num){
  fill_rainbow( leds, NUM_LEDS, num, 7); //num is gHue
}

/*
class rainbowWithGlitter : public Rainbow{
  public:
    //rainbow();
    void addGlitter();
};
*/
class addGlitter{
  public:
    void callPattern(uint8_t nGhue);
    
};

void addGlitter::callPattern(uint8_t num){
  if( random8() < 80) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

class Confetti{
  public:
    void drawPattern(uint8_t gHue);
  
};

void Confetti::drawPattern(uint8_t hue){
  int pos = random16(NUM_LEDS);
  fadeToBlackBy( leds, NUM_LEDS, 10);
  leds[pos] += CHSV( hue + random8(64), 200, 255);
}

class Sinelon{
  public:
    void callPattern(uint8_t gHue);
};

void Sinelon::callPattern(uint8_t num){
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( num, 255, 192);
}

class Bpm{
  public:
    void callPattern(uint8_t gHue);
};

void Bpm::callPattern(uint8_t num){
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
    for( int i = 0; i < NUM_LEDS; i++) { //9948
      leds[i] = ColorFromPalette(palette, num+(i*2), beat-num+(i*10));
    }
  
}

class Juggle{
  public:
    void callPattern(uint8_t gHue);
};

void Juggle::callPattern(uint8_t num){
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
