#include <FastLED.h>
#include <SPI.h>
#include "Pattern.h"
#include "Headers.h"

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif


typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

typedef void (*SimplePatternList[])();//no
SimplePatternList vPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

bool patternEnArray[ARRAY_SIZE( gPatterns)] = {0};
bool patternAnArray[ARRAY_SIZE( vPatterns)] = {0};//no

Pattern p1;
Rainbow r1;
addGlitter rg1;
Confetti c1;
Sinelon s1;
Juggle ja;
Bpm b1;
static uint8_t gHue = 0;
static int mPattern=0;

void setup() {
  delay(3000); // 3 second delay for recovery
  Serial.begin(9600);


  FastLED.show();  // Turn all LEDs off ASAP
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,COLOR_ORDER>(leds, NUM_LEDS);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  querySerial();
  FastLED.delay(1000/FRAMES_PER_SECOND); //1000/120=8miliseconds


  //Serial.println(longitude, 6);
  // do some periodic updates
  EVERY_N_MILLISECONDS( 100 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) {
    p1.setVal(mPattern);
    mPattern= (p1.nextPattern()) % ARRAY_SIZE( gPatterns );
    Serial.print(mPattern);//testing
  } // change patterns periodically
  
  callPatterns();
 
}

void querySerial()
{
  while (Serial.available() > 0) {

    char anal = Serial.read(); 
    switch(anal)
    {
      //************************************************************
      // Serial message is: BGi (R, G, B, LED_start, LED_length)
      case'B':  {// string should start with B
      anal = Serial.read(); //Read second char and analyyze     
      if (anal == 'G'){ 
        if (Serial.read() == 'i'){
          // parse RGB
          BGiParams[0] = Serial.parseInt();     // then an ASCII number for red
          BGiParams[1] = Serial.parseInt();   // then an ASCII number for green
          BGiParams[2] = Serial.parseInt();    // then an ASCII number for blue
          BGiParams[3] = Serial.parseInt();   // then an ASCII number for start
          BGiParams[4] = Serial.parseInt();    // then an ASCII number for length
          //BGi();
          //patternarray[0]=true;
        }
      }

      if(anal == 'O') { 
        if (Serial.read() == 'W'){ 
           if(Serial.parseInt()!=0){
              patternAnArray[0]=true;}
          else{
            patternAnArray[0]=false;
          }
      }
      
      break;      
      }
      }
              
      case'C':  {// string should start with C
      anal = Serial.read(); //Read second char and analyze     
      if (anal == 'O'){ 
        if (Serial.read() == 'F'){ 
          if(Serial.parseInt()!=0){
            patternAnArray[2]=true;}
          else{
            patternAnArray[2]=false;
          }
        }
      }
      break;
      }

      case'S':  {// string should start with C
      anal = Serial.read(); //Read second char and analyze     
      if (anal == 'I'){ 
        if (Serial.read() == 'N'){ 
          if(Serial.parseInt()!=0){
            patternAnArray[3]=true;}
          else{
            patternAnArray[3]=false;
          }
        }
      }
      break;
      }

      case'J':  {// string should start with J
      anal = Serial.read(); //Read second char and analyze     
      if (anal == 'U'){ 
        if (Serial.read() == 'G'){ 
          if(Serial.parseInt()!=0){
            patternAnArray[4]=true;}
          else{
            patternAnArray[4]=false;
          }
        }
      }
      break;
      }

      case'b':  {// string should start with S
      anal = Serial.read(); //Read second char and analyze     
      if (anal == 'P'){ 
        if (Serial.read() == 'M'){ 
          if(Serial.parseInt()!=0){
            patternAnArray[4]=true;}
          else{
            patternAnArray[4]=false;
          }
        }
      }
      break;
      }
      
  }
  
  
  } 
}


void callPatterns(){
  //fill_solid( &(leds[0]), NUM_LEDS, CRGB( 0, 0, 0) );
  gPatterns[mPattern]();
  
  for(int i=0; i < ARRAY_SIZE( vPatterns); i++)
  {
    if(patternAnArray[i]==true) // Check to see if each pattern in array is enabled
    {
      // Call the current pattern if enabled, updating the 'leds' array
      vPatterns[i]();
    }
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  
}

void BGi(){
  fill_solid( &(leds[BGiParams[3]]), BGiParams[4], CRGB( BGiParams[0], BGiParams[1], BGiParams[2]) );
}

void rainbow(){
  r1.callPattern(gHue);
}

void rainbowWithGlitter(){
  rg1.callPattern(gHue);
}

void confetti(){
  c1.drawPattern(gHue);
}

void sinelon(){
  s1.callPattern(gHue);
}

void juggle(){
  ja.callPattern(gHue);
}

void bpm(){
  b1.callPattern(gHue);

}
