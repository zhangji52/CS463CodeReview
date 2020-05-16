#define LED_TYPE    DOTSTAR
#define COLOR_ORDER BGR
#define NUM_LEDS    169
#include <FastLED.h>

#define PRESSURE_CTRL     2
#define POSITION_CTRL     0
#define IR_CTRL           0
#define IR_ACT_Z_DIST     20
#define IR_ACT_Z_ERR_DIST 0

CRGB leds[NUM_LEDS];        // Place for final LED buffer at end of each cycle
CRGB LED_BUFFER[NUM_LEDS];      // volatile LED buffer for pushing/poping
CRGB tickOverlay[NUM_LEDS]; // Create tick overlay
CRGB drumKitOverlay[NUM_LEDS];  // Drumkit sprite buffer
CRGB ARGBOverlay[NUM_LEDS];  // ARGB sprite buffer
CRGB funfettiOverlay[NUM_LEDS];  // wiper buffer
CRGB bellOverlay[NUM_LEDS];  // Drumkit sprite buffer.0
CRGB ledbuffer[NUM_LEDS];  // buffer for per show operations
bool ledMask[NUM_LEDS];  // buffer for masked to BLACK LEDs
CRGB mask_color = CRGB(0,0,0);

// colored dots that pulse for drums
CRGBPalette16 drumPalette = RainbowColors_p; // Choose a color palette
// colored dots that glow for bells
CRGBPalette16 bellPalette = ForestColors_p; // Choose a color palette
// Support vars for Fire effect
CRGBPalette16 gPal; // https://github.com/FastLED/FastLED/wiki/Gradient-color-palettes

#define BRIGHTNESS          200
#define FRAMES_PER_SECOND  120
//#define FRAMES_PER_SECOND  240
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
int myBrightness=0;

// List of patterns to cycle through.  Each is defined as a separate function below.
// The order in which you declare things is the order of layering from bottom to top
// first in is written first to the strip
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { BGi, HSv, rainbow, ticks, drumKit, ARGB, fairyFire, funfetti, bell, mouth, addGlitter };

bool patternEnArray[ARRAY_SIZE( gPatterns)] = {0}; // Create an enable array of same size as pattern array to enable/dissable pattern layers

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

// LED Function Parameter Arrays (see LEDPatterns tab for functions and param descriptions)
uint8_t BGiParams[] = {0, 0, 0, 0, NUM_LEDS}; // R, G, B, LED_start, len
uint8_t HSVParams[] = {0, 0, 0, 0, NUM_LEDS}; // H, S, V, LED_start, len
uint8_t BOWParams[] = {0, NUM_LEDS, 5, 0};  // Start, len, hue Delta
uint8_t FIRParams[] = {0, 0, 0, 0, NUM_LEDS}; // Hue, Sparking, Cooling, LED_start, len
uint8_t DKTParams[] = {0}; // on/off
uint8_t ARGBParams[] = {1, 1, 1, 2, 2, 2, 3, 3, 3}; // indexR, wideR, brightR, indexG, wideG, brightG,indexB, wideB, brightB
uint8_t TCKParams[] = {4, 0, 0, 0, 127, 20}; // density, direction, hue, saturation, volume, tickSteps
uint8_t CFIParams[] = {0, 0, 0, 2, 1, 169}; // on/off h, s, v, start, len
uint8_t MOUParams[] = {0, 0, 0, 2, 1, 169}; // on/off h, s, v, start, len

uint8_t RGBHSV[] = {0, 96, 160}; //hue translation for RGB
uint8_t glitterForce = 0; // intensity of Glitter
uint8_t glitterPos = -1; // Place glitter randomly lights up to report back to MaxMSP

// left-of-center Positions of ZX sensors relative to DotStar strip LED positions for visualization
uint8_t ZXSensorPos[] = {10, 31, 52, 73, 94, 115, 136, 157};

class bgi{
  public:
    void callPattern();
};

void bgi::callPattern(){
   fill_solid( &(leds[BGiParams[3]]), BGiParams[4], CRGB( BGiParams[0], BGiParams[1], BGiParams[2]) );
}

class hsv{
  public:
      void callPattern();
};

void hsv::callPattern(){
  fill_solid( &(leds[HSVParams[3]]), HSVParams[4], CHSV( HSVParams[0], HSVParams[1], HSVParams[2]) );
}

class RainBow{
  public:
    void callPattern();  
};

void RainBow::callPattern(){
  fill_rainbow( &(leds[BOWParams[1]]), BOWParams[2], gHue, BOWParams[0]);
  fadeLightBy( leds, NUM_LEDS, BOWParams[3]);
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

class Ticks{
  public:
    void callPattern();
};

void Ticks::callPattern(){
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  uint8_t numTicks = TCKParams[5]; // pull num ticks 

  for(int i=0; i < NUM_LEDS; i++){
    LED_BUFFER[i] += tickOverlay[i];
  }

// Fade out and shift over in direction till you hit original position of next led over
  EVERY_N_MILLISECONDS(1){    
      if(numTicks>0){
        tickOverlay[NUM_LEDS-1] = CRGB( 0, 0, 0); // set top led to black
        
        for(int i=0; i < NUM_LEDS-1; i++){
          tickOverlay[i] = tickOverlay[i+1];
        }
        
        TCKParams[5] = numTicks-1; // decrease tick count by 1
      }
      
      fadeLightBy( tickOverlay, NUM_LEDS, 40);
  }
  
  // Push LED buffer to LED array for writing
  memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
}

class DrumKit{
  public:
    void callPattern();
};

void DrumKit::callPattern(){
  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  // Step 1:
  // First, create overlay of dim colored dots at the ZX sensor positions
  
  uint8_t len = 4; // Length 
  
  // fill overlay cells with dim color blobs at each ZX position
  for( byte i = 0; i < 8; i++){ 
    uint8_t index = ZXSensorPos[i];
                                                      // There are 169 LEDs, but 255 steps in the gradient
    int colorindex = map(index, 0, NUM_LEDS, 0, 240); // map total number of LEDs to full gradient value depending on ZX position

    // ADD minimum colors/brightness targets to buffer
    // Have color volume of 10 no matter what so I can see where to hit
      LED_BUFFER[index] += ColorFromPalette(drumPalette, colorindex, 10);
      LED_BUFFER[index+1] += ColorFromPalette(drumPalette, colorindex+1, 10);
    
    // If drumKitSprites at the current ZX sensor have color in them still, add to to LED_BUFFER
    if(drumKitOverlay[index])
    {
      LED_BUFFER[index] += drumKitOverlay[index];
      // Set colors of pixels from outside toward center, increasing brightness in 32 steps    
      for(int j = 0; j < len; j++)
      {
        byte mypoint = (len-j);
        
          if((index-mypoint)>=0)  // ensure we're not writing to index off the strip
          {
            // Map to main leds array immediately
            LED_BUFFER[index-mypoint] = drumKitOverlay[index-mypoint];
          }
          if((index+mypoint)<(NUM_LEDS-1)) // ensure we're not writing to index off the strip
          {
            LED_BUFFER[index+mypoint] = drumKitOverlay[index+mypoint];
          }

        }
    }

    // Fade all Kit Sprites down a little
    // last number is fade speed
    EVERY_N_MILLISECONDS(9){
      fadeToBlackBy( drumKitOverlay, NUM_LEDS, 20);
    }
    // Push LED buffer to LED array for writing
    memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
  
  }
}

class ArGB{
  public:
    void callPattern();
};

void ArGB::callPattern(){
  // We're realy using HSV, not RGB, so Hue values will be
  // 0 = RED, 96 = GREEN, 160 = BLUE
  
  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
    for(int i = 0; i < 3; i++)
      {
        int RC = ARGBParams[0+(i*3)];  // RmyIndex point start
        int RW = ARGBParams[1+(i*3)];  // R wideness
        int RB = ARGBParams[2+(i*3)];  // R brightness
        
  // draw pixels for each R, G, then B
        for(int j = 0; j < RW; j++)
        {
          LED_BUFFER[RC+j] += CHSV( RGBHSV[i], 255, RB);
          // Clear LED Array
          //fill_solid( &(LED_BUFFER[RC]), RW, CHSV( RGBHSV[i], 255, RB) );
        }
      }

    // Push LED buffer to LED array for writing
    memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
}

class FunFetti{
  public:
     void callPattern();
};

void FunFetti::callPattern(){
  // Param array: h, v, start, len, dens
  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( funfettiOverlay, NUM_LEDS, 64);

  // Determine which LEDs will light up stochastically
  for(int i = CFIParams[2]; i < (CFIParams[2]+CFIParams[3]); i++)
  {
    int prob = random16(1024); // roll a 100-sided dice
    //funfettiOverlay[i] += CHSV( CFIParams[0] + random8(64), 200, prob);
    
    if(prob < CFIParams[4])
    {
      //glitterPos = i; // report funfetti pixel to serial for potential sonification
      funfettiOverlay[i] += CHSV( CFIParams[0] + random8(64), 200, CFIParams[1]);
    }
    
  }

  // copy funfetti onto buffer
  for(int i = 0; i < NUM_LEDS; i++)
  {
    LED_BUFFER[i] += funfettiOverlay[i];
  }

  // Push LED buffer to LED array for writing
  memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
}

class Bell{
  public:
    void callPattern();
};

void Bell::callPattern(){
  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  // Step 1:
  // First, create overlay of dim colored dots at the ZX sensor positions
  
  uint8_t len = 4; // Length 
  
  // fill overlay cells with dim color blobs at each ZX position
  for( byte i = 0; i < 8; i++)
  { 
    uint8_t index = ZXSensorPos[i];
                                                      // There are 169 LEDs, but 255 steps in the gradient
    int colorindex = map(index, 0, NUM_LEDS, 0, 240); // map total number of LEDs to full gradient value depending on ZX position

    // ADD minimum colors/brightness targets to buffer
    // Have color volume of 10 no matter what so I can see where to hit
      LED_BUFFER[index] += ColorFromPalette(bellPalette, colorindex, 10);
      LED_BUFFER[index+1] += ColorFromPalette(bellPalette, colorindex+1, 10);
    
    // If drumKitSprites at the current ZX sensor have color in them still, add to to LED_BUFFER
    if(bellOverlay[index])
    {
      LED_BUFFER[index] += bellOverlay[index];
      // Set colors of pixels from outside toward center, increasing brightness in 32 steps    
      for(int j = 0; j < len; j++)
      {
        byte mypoint = (len-j);
        
          if((index-mypoint)>=0)  // ensure we're not writing to index off the strip
          {
            // Map to main leds array immediately
            LED_BUFFER[index-mypoint] = bellOverlay[index-mypoint];
          }
          if((index+mypoint)<(NUM_LEDS-1)) // ensure we're not writing to index off the strip
          {
            LED_BUFFER[index+mypoint] = bellOverlay[index+mypoint];
          }

        }
    }

    // Fade all Kit Sprites down a little
    // last number is fade speed
    EVERY_N_MILLISECONDS(9)
    {
      fadeToBlackBy( bellOverlay, NUM_LEDS, 20);
    }
    // Push LED buffer to LED array for writing
    memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
  
  }
}

class Mouth{
  public:
    void callPattern();
};

void Mouth::callPattern(){
  uint8_t randomVar = 40; // ammount of random fuzz in hue
  uint8_t theIndex = MOUParams[3];
  uint8_t theLen = MOUParams[4];
  // Param array: [hue, blend, v, start, len] where blend is ammount of middle leds to mix in, cool when faded completely out
  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  
  // Write mouth effect directly into LED_Buffer
  // Set colors of pixels from outside toward center, increasing brightness in 32 steps    
  for(int i = 0; i < theLen; i++)
  {
    uint8_t myPoint = theLen-i; // Calculate write point
    //float myVol = MOUParams[2] * (myPoint/theLen); // calculate brightness as ratio away from center point
    
    LED_BUFFER[theIndex] = CHSV( MOUParams[0] + random8(randomVar), MOUParams[1], MOUParams[2]);
    
    // write to the start position minus calculated write point
    if((theIndex-myPoint)>=0)
    {
      if(!i) // end points always exact hsv as center point
        LED_BUFFER[theIndex-myPoint] = CHSV( MOUParams[0] + random8(randomVar), 255, MOUParams[2]);
      else // middle stuff takes into account blend
        LED_BUFFER[theIndex-myPoint] = CHSV( MOUParams[0] + random8(randomVar), 255, MOUParams[1]);

    }
    if((theIndex+myPoint)<(NUM_LEDS-1))
    {
      if(!i) // end points always exact hsv as center point
        LED_BUFFER[theIndex+myPoint] = CHSV( MOUParams[0] + random8(randomVar), 255, MOUParams[2]);
      else
        LED_BUFFER[theIndex+myPoint] = CHSV( MOUParams[0] + random8(randomVar), 255, MOUParams[1]);
    }
  }

  // Push LED buffer to LED array for writing
  memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
}

class Glitter{
  public:
    void callPattern();
};

void Glitter::callPattern(){
  if( random8() < glitterForce) {
    uint16_t randLED = random16(NUM_LEDS);
    leds[ randLED ] += CRGB::White;
    glitterPos = randLED; // Report glitter pixel position out serial for sonification
  }
}

class Sinelon{
  public:
    void callPattern();
};

void Sinelon::callPattern(){
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

class Juggle{
  public:
    void callPattern();
};

void Juggle::callPattern(){
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

class FairyFire{
  public:
    void callPattern();
};

void FairyFire::callPattern(){
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random8());

  CRGB darkcolor  = CHSV(FIRParams[0],255,192); // pure hue, three-quarters brightness
  CRGB lightcolor = CHSV(FIRParams[0],128,255); // half 'whitened', full brightness
  gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White); // set gradient color palette
  // User error check to be sure fire len is never more than NUM_LEDS, which will crash the program
  int fireLen = FIRParams[4];
  if(fireLen > NUM_LEDS)
    fireLen = NUM_LEDS;
  
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS]; // cool all leds down regardless of current length for natural cooling effect

  // Step 0: Copy contents of LEDs to LED_Buffer
  memmove8(&LED_BUFFER, &leds, NUM_LEDS*sizeof(CRGB));
  // If active, fade background by 50%
  fadeLightBy( LED_BUFFER, NUM_LEDS, 20);
  
  // Step 1.  Cool down every cell a little, qsub subtracts random ammount from each cell
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((FIRParams[2] * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    // Do this only for desired length of flame
    for( int k= fireLen - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < FIRParams[1] ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED Buffer colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( gPal, colorindex);
      int pixelnumber;
      // If Reverse Direction, flip, else don't
      if( FIRParams[3] )
      {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      LED_BUFFER[pixelnumber] = color;
    }

    // ADD LED buffer to LED array for layering
    for(int i=0; i < NUM_LEDS; i++)
    {
      leds[i] += LED_BUFFER[i];
    }
}

bgi b1;
hsv h1;
RainBow rb;
Ticks t1;
DrumKit DK;
FunFetti FF;
ArGB Ar;
Bell bl;
Mouth mo;
Glitter Gl;
Sinelon sl;
Juggle jg;
FairyFire Fir;
