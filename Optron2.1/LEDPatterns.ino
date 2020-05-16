#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void querySerial()
{
  // listen for serial:
  // set color arrays
  //String serialstring= Serial.readString();
  //Serial.println("This: ");
  //Serial.println(serialstring[1]);
  while (Serial.available() > 0) {

    char anal = Serial.read(); 
    switch(anal)
    {
      //************************************************************
      // Serial message is: BGi (R, G, B, LED_start, LED_length)
      case'B':  // string should start with B
      anal = Serial.read(); //Read second char and analyyze     
      if (anal == 'G')  // then next should be G
      { 
      if (Serial.read() == 'i')  // then next should be i
      { 
        // Enable Pattern in refresh:
        patternEnArray[0] = true;  // BGi
        patternEnArray[1] = false; // HSv
        patternEnArray[2] = false; // Rainbow
        
     // parse RGB

        BGiParams[0] = Serial.parseInt();     // then an ASCII number for red    
        BGiParams[1] = Serial.parseInt();   // then an ASCII number for green
        BGiParams[2] = Serial.parseInt();    // then an ASCII number for blue
        if(BGiParams[0]==0 && BGiParams[1]==0 && BGiParams[2]==0){
          patternEnArray[0] = false;  // BGi turned off due to red/green/blue 0
        }
        else{
        BGiParams[3] = Serial.parseInt();   // then an ASCII number for start
        BGiParams[4] = Serial.parseInt();    // then an ASCII number for length

        clipLen(&BGiParams[3],&BGiParams[4]);
        }
      }}

      if(anal == 'O') 
      { 
      if (Serial.read() == 'W')  // then next should be i
      { 

        // Enable Pattern in refresh:
        patternEnArray[0] = false; // BGi
        patternEnArray[1] = false; // HSv
        patternEnArray[2] = true;  // Rainbow
        
        BOWParams[0] = Serial.parseInt();  // hue delta (length of hue transitions)
        BOWParams[1] = Serial.parseInt();   // Start LED
        BOWParams[2] = Serial.parseInt();   // Len
        BOWParams[3] = Serial.parseInt();   // Fade ammount, 0 strong to 255 weak

        if(BOWParams[0]==0 && BOWParams[1]==0 && BOWParams[2]==0){
          patternEnArray[2] = false;      
        }else{
          clipLen(&BOWParams[1],&BOWParams[2]);
        }
      }}

      //************************************************************
      // Bell Pad
      // Serial message is: BEL on/off h, v, start, len, dens
      // Notice: Missing saturation as it is assumed confetti will have fixed value
    
      if(anal == 'E')  
      { 
      if (Serial.read() == 'L')  
      { 
        patternEnArray[8] = Serial.parseInt(); // then an ASCII number for 1/0 on off

        // fill overlay cells with dim color blobs at each ZX position
        for( byte i = 0; i < 8; i++)
        { 
         uint8_t index = ZXSensorPos[i];
         int colorindex = map(index, 0, NUM_LEDS, 0, 240); // map total number of LEDs to full gradient value
         
          // Have color volume of 10 no matter what so I can see where to hit
          bellOverlay[index] += ColorFromPalette(bellPalette, colorindex, 10);
          bellOverlay[index+1] += ColorFromPalette(bellPalette, colorindex+1, 10);
        }
        
      } break; }
      
// //************************************************************
      // Bell Pad sprite trigger
      // Serial message is: BIL on/off h, v, start, len, dens
      // Notice: Missing saturation as it is assumed confetti will have fixed value
      if(anal == 'I')  
      { 
      if (Serial.read() == 'L')  
      { 
     
     // parse kit sprites vars
        uint8_t bellSpriteNum = Serial.parseInt(); // then an ASCII number for sprite number
        uint8_t bellSpriteAmp = Serial.parseInt();
        uint8_t len = 4; // length of lit up sprite

        uint8_t index = ZXSensorPos[bellSpriteNum]; // calculate index at drumSprite

        int colorindex = map(index, 0, NUM_LEDS, 0, 240); // map total number of LEDs to full gradient value
        // Give sprite color boost
        //drumKitOverlay[index] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
        //drumKitOverlay[index+1] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
      // Set colors of pixels from outside toward center, increasing brightness in 32 steps    
      bellOverlay[index] = ColorFromPalette(bellPalette, colorindex, bellSpriteAmp);
      
      for(int j = 0; j < len; j++)
      {
        byte mypoint = (len-j);
        
          if((index-mypoint)>=0)  // ensure we're not writing to index off the strip
          {
            colorindex = map(index-mypoint, 0, NUM_LEDS, 0, 255); // map total number of LEDs to full gradient value
            // Give sprite color boost
            bellOverlay[index-mypoint] = ColorFromPalette(bellPalette, colorindex, bellSpriteAmp);
          } 
          if((index+mypoint)<(NUM_LEDS-1)) // ensure we're not writing to index off the strip
          {
            colorindex = map(index+mypoint, 0, NUM_LEDS, 0, 255); // map total number of LEDs to full gradient value
            // Give sprite color boost
            bellOverlay[index+mypoint] = ColorFromPalette(bellPalette, colorindex, bellSpriteAmp);
          }
          
        }
  
      }}

      break;

      //************************************************************
      // Serial message is: HSV (h, s, v, start, len)
      case'H':     
      if (Serial.read() == 'S')  
      { 
      if (Serial.read() == 'v')  
      { 
        // Enable Pattern in refresh:
        patternEnArray[0] = false; // BGi
        patternEnArray[1] = true;  // HSv
        patternEnArray[2] = false; // Rainbow
        
        // parse HSV
        HSVParams[0] = Serial.parseInt();     // then an ASCII number for Hue
        HSVParams[1] = Serial.parseInt();   // then an ASCII number for Saturation
        HSVParams[2] = Serial.parseInt();    // then an ASCII number for Brightness
        HSVParams[3] = Serial.parseInt();   // then an ASCII number for start
        HSVParams[4] = Serial.parseInt();    // then an ASCII number for len

        if(HSVParams[0]==0 && HSVParams[1]==0 && HSVParams[2]==0){
          patternEnArray[1]=false;
          Serial.println("closing thread");
        }
        else{
          Serial.println("yes thread");
          clipLen(&HSVParams[3],&HSVParams[4]);
        }
      }}
      break;

      //************************************************************
      // Serial message is: TCK on/off (density, direction, hue, saturation, volume)
      case'T':  // string should start with B
      anal = Serial.read(); //Read second char and analyyze     
      if (anal == 'C')  // then next should be G
      { 
      if (Serial.read() == 'K')  // then next should be i
      {
        // Enable Pattern in refresh: 
        patternEnArray[3] = Serial.parseInt();  // On/Off TCK

      for(int i = 0; i < 5; i++) // There are 5 parameters passing in here
      {
        TCKParams[i] = Serial.parseInt();  
      }

      // Clear Tick array
      fill_solid( &(tickOverlay[0]), NUM_LEDS, CRGB( 0, 0, 0) );
      uint8_t numTicks = (int)(NUM_LEDS/TCKParams[0]); // calculate the modulo value to draw sprites based on density value
      
      // calculate number of tick steps to take in the following animation
      // based on tick sprite index
      TCKParams[5] = numTicks;
      
      for(int i = 0; i < NUM_LEDS; i++) // Fill tick array with sprites
      {
        if((i%numTicks) < 1) // Draw sprite spaced evenly based on density value
        {
          tickOverlay[i] = CHSV( TCKParams[2], TCKParams[3], TCKParams[4]);
          //tickOverlay[i] = CHSV( 0, 127, 127);

        }
      }
        
      }}
      break;

      //************************************************************
      // Serial message is: DKT (on/off)
      // Add glitter with 0 - 100 percent density
      case'D':     
      if (Serial.read() == 'K') 
      { 
      anal = Serial.read(); //Read second char and analyyze      
      if (anal == 'T') 
      { 
        
        patternEnArray[4] = Serial.parseInt(); // then an ASCII number for 1/0 on off

        // fill overlay cells with dim color blobs at each ZX position
        for( byte i = 0; i < 8; i++)
        { 
         uint8_t index = ZXSensorPos[i];
         int colorindex = map(index, 0, NUM_LEDS, 0, 240); // map total number of LEDs to full gradient value
         
          // Have color volume of 10 no matter what so I can see where to hit
          drumKitOverlay[index] += ColorFromPalette(drumPalette, colorindex, 10);
          drumKitOverlay[index+1] += ColorFromPalette(drumPalette, colorindex+1, 10);
        }
        
        break;
      }


      if (anal == 'N') 
      { 
     
     // parse kit sprites vars
        uint8_t drumSpriteNum = Serial.parseInt(); // then an ASCII number for sprite number
        uint8_t drumSpriteAmp = Serial.parseInt();
        uint8_t len = 4; // length of lit up sprite

        uint8_t index = ZXSensorPos[drumSpriteNum]; // calculate index at drumSprite

        int colorindex = map(index, 0, NUM_LEDS, 0, 255); // map total number of LEDs to full gradient value
        // Give sprite color boost
        //drumKitOverlay[index] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
        //drumKitOverlay[index+1] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
      // Set colors of pixels from outside toward center, increasing brightness in 32 steps    
      drumKitOverlay[index] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
      
      for(int j = 0; j < len; j++)
      {
        byte mypoint = (len-j);
        
          if((index-mypoint)>=0)  // ensure we're not writing to index off the strip
          {
            colorindex = map(index-mypoint, 0, NUM_LEDS, 0, 255); // map total number of LEDs to full gradient value
            // Give sprite color boost
            drumKitOverlay[index-mypoint] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
          } 
          if((index+mypoint)<(NUM_LEDS-1)) // ensure we're not writing to index off the strip
          {
            colorindex = map(index+mypoint, 0, NUM_LEDS, 0, 255); // map total number of LEDs to full gradient value
            // Give sprite color boost
            drumKitOverlay[index+mypoint] = ColorFromPalette(drumPalette, colorindex, drumSpriteAmp);
          }
          
        }
  
      }}
      break;
      
      //************************************************************
    // ARGB commands: ex, ARGB on/off [ CenterPoint wideness brightness, ... , ...]
    case 'A':  // string should start with A
      
    if (Serial.read() == 'R')  // then next should be R
    {
    if (Serial.read() == 'G')  // then next should be G
    { 
    if (Serial.read() == 'B')  // then next should be B
    {
      // Enable Pattern in refresh: 
      patternEnArray[5] = Serial.parseInt();  // On/Off FairyFire
        
      for(int i = 0; i < 9; i++) // There are 9 parameters passing in here
      {
        ARGBParams[i] = Serial.parseInt();  
      }

      // Be sure we do not write over led length boundaries
      clipLen(&ARGBParams[0],&ARGBParams[1]);
      clipLen(&ARGBParams[3],&ARGBParams[4]);
      clipLen(&ARGBParams[6],&ARGBParams[7]);
    }
    }}
    break;

    //************************************************************
      // Serial message is: HSV (h, s, v, start, len)
      case'F':     
      anal = Serial.read(); //Read second char and analyyze     
      if (anal == 'R')  // then next should be G
      { 
      if (Serial.read() == 'E')  // then next should be i
      {  
        // Enable Pattern in refresh: 
        patternEnArray[6] = Serial.parseInt();  // On/Off FairyFire
        
     // parse Fairy Fire Params
        FIRParams[0] = Serial.parseInt();     // then an ASCII number for Hue
        FIRParams[1] = Serial.parseInt();   // then an ASCII number for sparking
        FIRParams[2] = Serial.parseInt();    // then an ASCII number for cooling
        FIRParams[3] = Serial.parseInt();   // then an ASCII number for start
        FIRParams[4] = Serial.parseInt();    // then an ASCII number for len

        clipLen(&HSVParams[3],&HSVParams[4]);
        
      }break;}
      
      //************************************************************
      // Flicker LEDs
      // Serial message is: FLR (h, s, v, start, len)
      if(anal == 'L') 
      { 
      if (Serial.read() == 'R')  // then next should be i
      {
        
      }}
      break;

      //************************************************************
      // FunFetti
      // Serial message is: CFI on/off h, v, start, len, dens
      // Notice: Missing saturation as it is assumed confetti will have fixed value
      case'C':     
      if (Serial.read() == 'F')  
      { 
      if (Serial.read() == 'I')  
      { 
        // parse glitter vars
        patternEnArray[7] = Serial.parseInt(); // then an ASCII number for 1/0 on off
        for(int i = 0; i < 5; i++) // There are 5 parameters passing in here
        {
         CFIParams[i] = Serial.parseInt();  
        }
        // constrain lengths to not write off max strip length
        clipLen(&CFIParams[2],&CFIParams[3]);
        
      }}
      break;

      

      //************************************************************
      // Mouth
      // Serial message is: MOU on/off h, s, v, index, len
      // Notice: Missing saturation as it is assumed confetti will have fixed value
      case'M':     
      if (Serial.read() == 'O')  
      { 
      if (Serial.read() == 'U')  
      { 
        // parse glitter vars
        patternEnArray[9] = Serial.parseInt(); // then an ASCII number for 1/0 on off

        for(int i = 0; i < 5; i++) // There are 5 parameters passing in here
        {
         MOUParams[i] = Serial.parseInt();  
        }
        
      }}
      break;
/*
      //************************************************************
      // Serial message is: WP (h, s, v, ms_interval, dir, animation counter)
      case'W':     
      if (Serial.read() == 'P')  
      { 
        // Enable Pattern in refresh: 
        patternEnArray[7] = 1;  // Turn On Automatically
        
        for(int i = 0; i < 5; i++) // There are 5 parameters passing in here
        {
         WPParams[i] = Serial.parseInt();  
        }
        WPParams[5] = NUM_LEDS; //reset animation counter

        // Clear wiper Array
        fill_solid( &(wiperOverlay[0]), NUM_LEDS, CRGB( 0, 0, 0) );

        if(WPParams[4]) // Set first LED position and color based on direction
        {
          wiperOverlay[NUM_LEDS-1] = CHSV( WPParams[0], WPParams[1], WPParams[2]);
        }
        else
        {
          wiperOverlay[0] = CHSV( WPParams[0], WPParams[1], WPParams[2]);
        }
        
      }
      break;
*/     
      //************************************************************
      // Serial message is: GLR (on/off, percent)
      // Add glitter with 0 - 100 percent density
      case'G':     
      if (Serial.read() == 'L') 
      { 
      if (Serial.read() == 'R') 
      { 
     
     // parse glitter vars
        patternEnArray[10] = Serial.parseInt(); // then an ASCII number for 1/0 on off
        glitterForce = Serial.parseInt();     // then an ASCII number for glitter density
        
        //glitterFlag = onOff;
      }}
      break;
      
      // Do nothing if no match
      default:
      break;
    }
  }
}

void callPatterns()
{
  // Clear LED Array
  fill_solid( &(leds[0]), NUM_LEDS, CRGB( 0, 0, 0) );
  
  for(int i=0; i < ARRAY_SIZE( gPatterns); i++)
  {
    if(patternEnArray[i]) // Check to see if each pattern in array is enabled
    {
      // Call the current pattern if enabled, updating the 'leds' array
      gPatterns[i]();
    }
  }

  #if POSITION_CTRL
    mask_by_position();
  #endif
  #if IR_CTRL
    mask_by_IR();
  #endif

  // send the 'leds' array out to the actual LED strip
  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do periodic updates here if needed
  //EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically


}

void mask_by_IR()
{
  // find masking section size
  int chunk_sz = NUM_LEDS / max(IR_CNT, 1);
  
  // make mask
  uint8_t tdx;
  for (tdx = 0; tdx < IR_CNT; tdx++) 
  {
    int st_idx = (tdx * chunk_sz) - (chunk_sz / 2);
    int en_idx = (tdx * chunk_sz) + (chunk_sz / 2);
    // un-mask
    if (z_pos[tdx] < IR_ACT_Z_DIST && z_pos[tdx] > IR_ACT_Z_ERR_DIST)
    {
      LEDFillMask(st_idx, en_idx, true);
    }
    // mask
    else
    {
      LEDFillMask(st_idx, en_idx, false);
    }
  }

  // apply mask
  LEDApplyMask();
}

// set length mask based on position sensor
void mask_by_position()
{
  int end_idx = (NUM_LEDS*linVal)/(1<<ANLG_RES);
  LEDFillMask(0, end_idx, true);
  LEDFillMask(end_idx+1, NUM_LEDS, false);
  LEDApplyMask();
}

void LEDFillMask(int start_idx, int end_idx, bool onoff)
{
  start_idx = max(start_idx, 0);
  end_idx = min(end_idx, NUM_LEDS);
  while (start_idx < end_idx)
    ledMask[start_idx++] = onoff;
}

void LEDApplyMask()
{
  int idx;
  for (idx = 0; idx < NUM_LEDS; idx++ )
  {
    if (!ledMask[idx])
    {
      // set black
      leds[idx] = mask_color;
    }
  }  
}

void FastLEDShowMask()
{
  // copy leds
  memcpy(ledbuffer, leds, NUM_LEDS * sizeof(CRGB));

  // fill leds with mask
  int idx;
  for (idx = 0; idx < NUM_LEDS; idx++ )
  {
    if (!ledMask[idx])
    {
      // set black
      leds[idx] = mask_color;
    }
  }  
  
  // show leds
  FastLED.show();  
  
  // restore leds
  memcpy(leds, ledbuffer, NUM_LEDS * sizeof(CRGB));
}

// ================================================================
// Function: changeBrightness
// Description: Change Brightness
// ================================================================
void changeBrightness()
{
  myBrightness=fsrVal;
  #ifdef OUTPUT_READABLE      
    Serial.println("------ MY BRIGHTNESS ------");
  #endif
  if(myBrightness<0) myBrightness=0;
  if(myBrightness>780) myBrightness=780;
  myBrightness=map(myBrightness,0,780,10,255);
  #ifdef OUTPUT_READABLE
    Serial.println(myBrightness);
    Serial.println("---------------------------");
  #endif
}  //  changeBrightness  /

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

// This function takes pointers to array values to check if
// the start LED + LED length is less than total number of LEDs. If greater,
// set length to the correct ammount to just the end of the strip
// https://www.geeksforgeeks.org/pointers-in-c-and-c-set-1-introduction-arithmetic-and-array/
void clipLen(uint8_t *ledStart, uint8_t *ledLen)
{
   if(*ledStart > NUM_LEDS)
    *ledStart = *ledStart%NUM_LEDS; // Automatically warp back to begining if start is beyond number of LEDs
  
  if((*ledStart+*ledLen) > NUM_LEDS)
    *ledLen = NUM_LEDS - *ledStart;
}



void BGi(){
     //Set LEDs and save BG color values
     //fill_solid( &(leds[BGiParams[3]]), BGiParams[4], CRGB( BGiParams[0], BGiParams[1], BGiParams[2]) );
     b1.callPattern();
}

void HSv(){
     //Set LEDs and save BG color values
     //fill_solid( &(leds[HSVParams[3]]), HSVParams[4], CHSV( HSVParams[0], HSVParams[1], HSVParams[2]) );
     h1.callPattern();
}

void rainbow() {
    // FastLED's built-in rainbow generator
    //fill_rainbow( &(leds[BOWParams[1]]), BOWParams[2], gHue, BOWParams[0]);
    //fadeLightBy( leds, NUM_LEDS, BOWParams[3]);
    //EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    rb.callPattern();
}


void ticks(){
    t1.callPattern();
}

void drumKit(){
    DK.callPattern(); 
}


void ARGB(){
    Ar.callPattern();
}


void funfetti() 
{
    FF.callPattern();
}


// Bell Cloud colors
void bell()
{
    bl.callPattern();
}


// Mouth effect in foreground, overwrite LEDs at this effect
// random variation in hue for fuzz effect because... why not?
void mouth()
{
    mo.callPattern();
  
}

// glitterForce var is percent change or density of glitter effect
void addGlitter() 
{
    Gl.callPattern();
  
}


void sinelon()
{
    sl.callPattern();
}


void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void setRGB()
{
  for(int i = 0; i < NUM_LEDS/2; i++) {   
    // fade everything out
    //leds.fadeToBlackBy(40);
    fadeToBlackBy( leds, NUM_LEDS, 40);
    // let's set an led value
    leds[i] = CHSV(gHue++,255,255);

    // now, let's first 20 leds to the top 20 leds, 
    leds[NUM_LEDS/2,NUM_LEDS-1] = leds[NUM_LEDS/2 - 1 ,0];
    FastLED.delay(33);
  }
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120
// June 12, 2019 Chet adds Serial control of parameters. Looks awesome.

void fairyFire(){
  Fir.callPattern();
    
}

// Old stuff:
/*
void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}



 */

 /*
void wiper()
{
  uint8_t numTicks = WPParams[5]; // pull num ticks 
  
  for(int i=0; i < NUM_LEDS; i++)
  {
    LED_BUFFER[i] += wiperOverlay[i];
  }
  
  // Fade out and shift over in direction till you hit original position of next led over
  EVERY_N_MILLISECONDS(1)
    {
      if(numTicks>=0)
      {
        if(WPParams[4]) // Set first LED position and color based on direction
        {
          //wiperOverlay[NUM_LEDS-1] = CHSV( WPParams[0], WPParams[1], WPParams[2]);
          for(int i=0; i < NUM_LEDS-1; i++)
          {
            wiperOverlay[i] = wiperOverlay[i+1];
          }
        }
        else
        {
          //wiperOverlay[0] = CHSV( WPParams[0], WPParams[1], WPParams[2]);
          for(int i=NUM_LEDS-1; i > 0; i--)
          {
            wiperOverlay[i] = wiperOverlay[i-1];
          }
        }
        
      WPParams[5] = numTicks-1; // decrease tick count by 1
      
      }

      else
      {
        // Turn yourself off, animation over 
        patternEnArray[7] = 0;  // Turn off Automatically
      }
      
      //fadeLightBy( wiperOverlay, NUM_LEDS, 10);
    }

  memmove8(&leds, &LED_BUFFER, NUM_LEDS*sizeof(CRGB));
}

*/
