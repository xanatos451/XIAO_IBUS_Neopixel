
// Uses code examples from https://gist.github.com/hsiboy/11545fd0241ab60b567d for FastLED
// Uses code examples from https://medium.com/@werneckpaiva/how-to-read-rc-signal-with-arduino-using-flysky-ibus-73448bc924eb for IBus

#include <Arduino.h>
#include <IBusBM.h>
#include <Adafruit_NeoPixel.h>

//LED Vars
#define LED_PIN 3             // Arduino pin number LED strip is connected to
#define NUM_LEDS 10            // Number of LEDs in strip
//#define COLOR_ORDER RGB       // LED Color order
//#define LED_TYPE SK6812      // LED Strip Type

//#define MIN_BRIGHTNESS 0      // Minimum LED Power (0 = Off)
//#define MAX_BRIGHTNESS 255    // Maximum LED Power (255 = Full Brightness)
//#define MIN_HUE 0             // Minimu LED Hue
//#define MAX_HUE 255           // Maximum LED Hue
//#define MIN_SATURATION 0      // Minimum LED Saturation for HSV
//#define MAX_SATURATION 255    // Maximum LED Saturation for HSV
#define MAX_SPEED 100         // Maximum Animation Speed (percentage)
#define MIN_SPEED 1           // Minimum Animation Speed (percentage)
#define STEP_SPEED 2          // Amount to increase/decrease speed each step (percent)
#define LED_WIDTH 1           // Used to determine how many LEDs to use in animation movement
#define CHASE_WIDTH 3         // Used to determine how wide the chase gap animation is

bool CH_1_ENABLE = false;     // Used to trigger Channel 1 state (true/false) based on last extreme stick position
//int valSpeed = 100;   //Testing placement
//int valBright = 0; 
int valHue = 0;     //Hue
int valSat = 0;     //Saturation
int valWhite = 0;   //White
int currBright= 100;  //Current Brightness
int currAni;        //Current Animation Profile
int currColor;      //Current Color Profile
int currSpeed;      //Current Animation Speed
bool revDir = false;    //Reverse Direction
bool midOut = false;    //Middle-Out

// New vars for non-blocking patterns
unsigned long pInterval = 20 ; // Default delay time between steps in the pattern (milliseconds)
unsigned long lastUpdate = 0 ; // for millis() when last update occured
unsigned long pIntervals [] = { 20, 20, 50, 100 } ; // Delay between steps for each pattern (milliseconds)

Adafruit_NeoPixel neoStrip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_RGBW + NEO_KHZ800);    // Init neoStrip NeoPixel Object

////CRGB leds[NUM_LEDS]; // FastLED Library Init
//CRGBPalette16 currentPalette;
//TBlendType    currentBlending;

//extern CRGBPalette16 myRedWhiteBluePalette;
//extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


IBusBM ibusRc;    // init IBUS library
HardwareSerial& ibusRcSerial = Serial1;   // IBUS communication channel

Serial_ & debugSerial = SerialUSB;    // Used for debugging via serial monitor

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  debugSerial.begin(74880);    // Used for debugging via serial monitor 
  //ibusRc.begin(ibusRcSerial);   // Initialize the IBus connection to the receiver
  ibusRc.begin(ibusRcSerial, IBUSBM_NOTIMER);   // Initialize the IBus connection to the receiver
  
  neoStrip.begin();  // Init LED Strip
  neoStrip.show();   // Set all pixels to 'off'

  //Read Selected Color Profile From Memory to currColor
  //Read Saved Color Profiles [SetColorProfile] 
  //Read Selected Animation Profile From Memory to currAni
   
  pinMode(LED_PIN, OUTPUT); // Set RGB strip pin to an OUTPUT
}

// Read the number of a given channel and convert to the range provided.
// If the channel is off, return the default value
int readChannel(byte channelInput, int minLimit, int maxLimit, int defaultValue){
  ibusRc.loop(); // call internal loop function to update the communication to the receiver
  uint16_t ch = ibusRc.readChannel(channelInput-1);
  if (ch < 1000) return defaultValue;
  return map(ch, 1000, 2000, minLimit, maxLimit);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
   
  //int valPattern = readChannel(1, 1000, 2000, 1000);  
  //int valSpeed = readChannel(2, 1000, 2000, 1500);              // Channel 2 Right Stick (Up/Down)  

  //int valHue = readChannel(5, 0, 255, 0);                    // Channel 5 Pot (VrA)
  //int valSat = readChannel(6, MIN_SATURATION, MAX_SATURATION, MIN_SATURATION);      // Channel 6 Pot (VrB)
  //int adjColor = readChannel(6, 0, 255, 100);      // Channel 6 Pot (VrB)
 
 

  
  
  bool valLock = readChannel(8, false, true, 0);        // Channel 8 Switch (SwB)
  if (valLock)
    {
    currBright = readChannel(6, 0, 255, 0);   // Channel 6 Pot (VrA)
    neoStrip.setBrightness(currBright);
    }
  
  /////////////    Increase or Decrease animation speed by percentage    /////////////
  static int currSpeed = 50;    //Default value
    
  if (valSpeed > 1900 && currSpeed < MAX_SPEED)    //Increase the animation speed
    {currSpeed = currSpeed + STEP_SPEED;
    if(currSpeed > MAX_SPEED) currSpeed = MAX_SPEED;
    delay(50);                                     // debounce delay
    }
  else if (valSpeed < 1100 && currSpeed > MIN_SPEED)     //Decrease the animation speed
    {currSpeed = currSpeed + STEP_SPEED;
    if(currSpeed <= 0) currSpeed = MIN_SPEED;
    delay(50);                                     // debounce delay
    }
  //////////////////////////////

  /////////////    Select pattern    /////////////
  static int currPattern = 0;    //Default value
  
  if(valPattern > 1900)
    {
    currPattern++;                              // increase pattern number
    if(currPattern > 4) currPattern = 0;        // wrap round if too high
    pInterval = pIntervals[currPattern] * (currSpeed/100);        // set speed for this pattern
    wipe();                                   // clear out the buffer
    delay(100);                               // debounce delay
    }
  else if (valPattern < 1100)
    {
    currPattern--;                           // decrease pattern number
    if(currPattern < 0) currPattern = 4;     // wrap round if too low
    pInterval = pIntervals[currPattern] * (currSpeed/100);        // set speed for this pattern
    wipe();                                  // clear out the buffer
    delay(100);                              //debounce delay
    }    
  //////////////////////////////////////////////////////////////////////////////////////////



  ////////////////////////////// Standby / Resume //////////////////////////////
  int valStandby = readChannel(7, 1000, 2000, 2000);    // Channel 7 Switch (SwA)
  int setColor = readChannel(10, 1000, 2000, 1000);      // Channel 10 Switch (SwD) - Allows the color to be set
  
  if (valStandby > 1900 && setColor < 1900)    //Standby enabled
    {
    neoStrip.fill(0,0,NUM_LEDS);    //Turn off all LEDs
    neoStrip.show();
    delay(50);                                     // debounce delay
    }
  else if (valStandby < 1100 && setColor < 1900)     //Resume animation/display
    {
    
    delay(50);                                     // debounce delay
    }
//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////// Set Color Profile ///////////////////////////////
if (setColor > 1900)
  {

  }



///////////////////////////////////////////////

/////////////////Ignore for now

  // Color Palette Selection
  switch (currPattern) {
    case 0:   // Solid  
  
    case 1:   // Color Wipe
        colorWipe(Wheel(valHue), valSpeed, valBright, revDir);    //Wipe color
        colorWipe(neoStrip.Color(0,0,0), valSpeed, valBright, revDir);    //Wipe black
      break;
      
    case 2:
      // Chase
      // rainbow cycle here
      break;
      
    case 3:      // Larson Scanner Animation 
      if (enableMidOut)   // Use middle out animation order
        {    
        // run larson scanner w/middle out
        }

       else   // Use left/right animation order
        {
        // run larson scanner
        }
      break;
    }  


// --- Debugging example below --- 
//  for (byte i = 1; i<=10; i++){
//    int value = readChannel(i, 1000, 2000, 1000);
//    debugSerial.print("Ch");
//    debugSerial.print(i);
//    debugSerial.print(": ");
//    debugSerial.print(value);
//    debugSerial.print(" ");
//  }


if(millis() - lastUpdate > pInterval) updatePattern(currPattern);
delay(10);        
}

void  UpdatePattern(int pat){ // call the pattern currently being created
  switch(pat) {
    case 0:
        rainbow();
        break;
    case 1:
        rainbowCycle();
        break;
    case 2:
        theaterChaseRainbow();
        break;
    case 3:
         colorWipe(strip.Color(255, 0, 0)); // red
         break;     
  } 
}


// Changes the Color Profile Settings
void ChangeColorProfile()
  {
  static uint32_t savedColor = LogWheel(valHue, valSat, valLum)
  int selColorValue = readChannel(9, 1000, 2000, 1000);     // Channel 9 Switch (SwC)
  int adjColor = readChannel(6, 0, 255, 100);      // Channel 6 Pot (VrB)
    // clear the palette and set solid color
  neoStrip.fill(HSLtoPixels(valHue, valSat, valLum), 0, NUM_LEDS);
  neoStrip.show();  
  
  switch (adjColorVal) 
    {
    case 1000:    // Select Hue 
      valHue = adjColor;
      break;
    case 1500:    // Select Saturation
      valSat = adjColor;
      break;
    case 2000:    // Select Luminosity 
      valWhite = adjColor;
      break;
    }    

  delay(10);
  }

// Saves Color Profile
void SaveColor(uint32_t c)
  {
      
  }


// Selects Saved Color Profile
void SelectColor()
  {
    
  }

//// Direction Selection forward/reverse
  // if (valRev == 2000)
  //   {revDir = true;}
  // else 
  //   {revDir = false;}  

// 
//void fadeAll() 
//  { 
//  for(int i = 0; i < NUM_LEDS; i++) 
//    { leds[i].nscale8(250); } 
//  }

/////// NEEDS WORK!!!!
// valDelay(integer): Length of delay in ms of loop which controls speed of animation
// valHue(integer): 0-255 LED hue color value
// valSat(integer): 0-255 LED saturation color value
// valBright(integer): 0-255 LED brightness
// isMidOut(boolean): Enables/disables middle out animation starting point (true / false)
// valPixWidth(integer): Determines how many pixels wide the animation point is

//void larsonScan(uint16_t valDelay, uint8_t valHue, uint8_t valSat, uint8_t valbright, bool isMidOut, uint8_t valPixWidth) 
//  { 
//
////for (i=0; i<70; i++) {
////  set(i, color);
////  set(i+70, color);
////}
////show;
//
//
//  // First slide the led in one direction (forward)
//  for(int i = 0; i < NUM_LEDS; i++) 
//    {
//    //set middle out logic here
//    //set pixel width logic here  
//      
//    leds[i] = CHSV(valHue, valSat, valBright);    // Set the i'th led to red 
//    neoStrip.show(); 
//    
//    leds[i] = CRGB::Black;    // Reset the i'th led to black
//    fadeAll();
//
//    delay(valDelay);    // Wait a little bit before we loop around and do it again
//    }
//
//
//  // Now go in the other direction. (reverse)
//  for(int i = (NUM_LEDS)-1; i >= 0; i--) 
//    {
//    //set middle out logic here
//    //set pixel width logic here    
//          
//    leds[i] = CHSV(valHue, valSat, valBright);    // Set the i'th led to red 
////    FastLED.show();
//    
//    leds[i] = CRGB::Black;    // Reset the i'th led to black
//    fadeAll();
//    
//    delay(valDelay);    // Wait a little bit before we loop around and do it again
//    }
//  }
//////////////////////////////////



// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait, uint8_t bright, bool rev) 
  {
  if(!rev)
    {
    for(uint16_t i=0; i<NUM_LEDS;) 
      {
      neoStrip.setPixelColor(i, c);
      neoStrip.setBrightness(bright);
      neoStrip.show();
      i++;
      delay(wait);
      }      
    }
  else
    {    
    for(uint16_t i=NUM_LEDS; i>0;) 
      {
      neoStrip.setPixelColor(i-1, c);
      neoStrip.setBrightness(bright);
      neoStrip.show();
      i--;
      delay(wait);
      }
    }
  delay(wait);    
  }

void wipe(){ // clear all LEDs
     for(int i=0;i<neoStrip.numPixels();i++){
       neoStrip.setPixelColor(i, neoStrip.Color(0,0,0));
       }
}


void colorWipe(uint32_t c, uint8_t bright, bool rev) { // modified from Adafruit example to make it a state machine
if (!rev) 
  {
  static int i = 0;
  neoStrip.setPixelColor(i, c);
  neoStrip.setBrightness(bright);    
  neoStrip.show();
  i++;
  if(i >= neoStrip.numPixels()){
    i = 0;
    wipe(); // blank out strip
  }
else 
  {
  static int i =    
  }
  lastUpdate = millis(); // time for next change to the display
}  


//// Rainbow Palette
//void rainbow(uint8_t wait) {
//  uint16_t i, j;
//
//  for(j=0; j<256; j++) {
//    for(i=0; i<neoStrip.numPixels(); i++) {
//      neoStrip.setPixelColor(i, Wheel((i+j) & 255));
//    }
//    neoStrip.show();
//    delay(wait);
//  }
//}
//
//// Slightly different, this makes the rainbow equally distributed throughout
//void rainbowCycle(uint8_t wait) {
//  uint16_t i, j;
//
//  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
//    for(i=0; i< neoStrip.numPixels(); i++) {
//      neoStrip.setPixelColor(i, Wheel(((i * 256 / neoStrip.numPixels()) + j) & 255));
//    }
//    neoStrip.show();
//    delay(wait);
//  }
//}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait, uint8_t gap, uint8_t pixWidth) 
  {
  for (int j=0; j<10; j++) //do 10 cycles of chasing
    {  
    for (int q=0; q < gap; q++) 
      {
      for (uint16_t i=0; i < neoStrip.numPixels(); i=i+gap) 
        {
        for (uint8_t w=0; w < pixWidth; w++)
          {          
          neoStrip.setPixelColor(i+q+w, c);    //turn every n'th pixel on (n=gap)
          }
        }
      neoStrip.show();

      delay(wait);

      for (uint16_t i=0; i < neoStrip.numPixels(); i=i+gap)
        {
        for (uint8_t w=0; w < pixWidth; w++)
          {
          neoStrip.setPixelColor(i+q+w, 0);        //turn every n'th set of pixel(s) off (n=gap)  
          }
        }       
      }
    }
  }

//Theatre-style crawling lights with rainbow effect 
//// - Add pixel width
//void theaterChaseRainbow(uint8_t wait) {
//  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
//    for (int q=0; q < 3; q++) {
//      for (uint16_t i=0; i < neoStrip.numPixels(); i=i+3) 
//        {
//        neoStrip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
//        }
//      neoStrip.show();
//
//      delay(wait);
//
//      for (uint16_t i=0; i < neoStrip.numPixels(); i=i+3) {
//        neoStrip.setPixelColor(i+q, 0);        //turn every third pixel off
//      }
//    }
//  }
//}

//  // Reverse direction of the pattern
//  void Reverse()
//  {
//      if (Direction == FORWARD)
//      {
//          Direction = REVERSE;
//          Index = TotalSteps-1;
//      }
//      else
//      {
//          Direction = FORWARD;
//          Index = 0;
//      }
//  }

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return neoStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return neoStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return neoStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// Input values 0 to 255 to get color values that transition R->G->B. 0 and 255
// are the same color. This is based on Adafruit's Wheel() function, which used
// a linear map that resulted in brightness peaks at 0, 85 and 170. This version
// uses a quadratic map to make values approach 255 faster while leaving full
// red or green or blue untouched. For example, Wheel(42) is halfway between
// red and green. The linear function yielded (126, 129, 0), but this one yields
// (219, 221, 0). This function is based on the equation the circle centered at
// (255,0) with radius 255:  (x-255)^2 + (y-0)^2 = r^2
unsigned long LogWheel(byte position) {
  byte R = 0, G = 0, B = 0;
  if (position < 85) {
    R = sqrt32((1530 - 9 * position) * position);
    G = sqrt32(65025 - 9 * position * position);
  } else if (position < 170) {
    position -= 85;
    R = sqrt32(65025 - 9 * position * position);
    B = sqrt32((1530 - 9 * position) * position);
  } else {
    position -= 170;
    G = sqrt32((1530 - 9 * position) * position);
    B = sqrt32(65025 - 9 * position * position);
  }
  return neoStrip.Color(R, G, B);
}

// Adapted from https://www.stm32duino.com/viewtopic.php?t=56#p8160
unsigned int sqrt32(unsigned long n) {
  unsigned int c = 0x8000;
  unsigned int g = 0x8000;
  while(true) {
    if(g*g > n) {
      g ^= c;
    }
    c >>= 1;
    if(c == 0) {
      return g;
    }
    g |= c;
  }
}



// Convert HSL vals to RGB (crude math)
uint32_t HSLtoPixels(uint16_t hue, uint16_t sat, uint16_t lum) {
  uint16_t red, grn, blu;

  // Convert hue to a color hexagon value (transition r y g c b m back to r)
  if(hue < 43) {
   red=255; grn=hue*6; blu=0;      // R to Y
  } else if(hue < 85) {
   hue -= 43;
   red=255-hue*6; grn=255; blu=0;  // Y to G
  } else if(hue < 128) {
   hue -= 85;
   red=0; grn=255; blu=hue*6;      // G to C
  } else if(hue < 170) {
   hue -= 128;
   red=0; grn=255-hue*6; blu=255;  // C to B
  } else if(hue < 213) {
   hue -= 170;
   red=hue*6; grn=0; blu=255;      // B to M
  } else {
   hue -= 213;
   red=255; grn=0; blu=255-hue*6;  // M to R
  }

  // Fade wheel RGB toward either white or black, depending on lum value
  if (lum > 127) {                           // Brighter than 50%?
    red=(255*(lum-128)+red*(255-lum))/127;   // Elevate colors to white
    grn=(255*(lum-128)+grn*(255-lum))/127;
    blu=(255*(lum-128)+blu*(255-lum))/127;
  }
  else {                   // Darker than 50%,
    red = (red*lum)/127;   // Scale colors down to black
    grn = (grn*lum)/127;
    blu = (blu*lum)/127;
  }

  // Fade current RGB toward gray lum value, depending on saturation
  red=(lum*(255-sat) + red*sat)/255;
  grn=(lum*(255-sat) + grn*sat)/255;
  blu=(lum*(255-sat) + blu*sat)/255;

  red=(red*red)>>8;    // Optional: square the RGB for a more pleasing gamma
  grn=(grn*grn)>>8;
  blu=(blu*blu)>>8;

  return neoStrip.Color(red, grn, blu);
}
