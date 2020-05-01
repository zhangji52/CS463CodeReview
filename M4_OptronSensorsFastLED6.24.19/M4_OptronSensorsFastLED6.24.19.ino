// Dotstar Includes:
#include <Arduino.h>
#include <AsyncDelay.h>
#include "Headers.h"
#include "Patterns.h"

#define PWM_pin 6     //PWM
#define motor_rate 1000
#define on_percentage 50
#define enable 1

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

//==========================================================
// Sensor Includes:
//==========================================================
    #include "Wire.h"
    #include <ZX_Sensor.h>
    #include <SPI.h>


// uncoment one of these - which format do we want stuff output on serial?
#define OUTPUT_BINARY
//#define OUTPUT_READABLE

unsigned long previousMillis = 0; // will store last time samples were sent
const long interval = 20;  // interval at which to send samples (milliseconds)

const int MPU_ADDR = 0x68; // also define i2c address of MPU

#include "I2Cdev.h"
#include "MPU6050.h"
// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

#define Baud  115200
#define Pin  A3
#define HighThreshold  700  //Approx 2 volts 667
#define LowThreshold 400 //Approx 1 volt 333

  MPU6050 accelgyro; // another called accelgyro
#define INTERRUPT_PIN 11  // use pin 11 on Micro, use pin 2 on Arduino Uno & most boards
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer


// Place to save accel gyro mpu6050 readings
int16_t ax, ay, az;
int16_t gx, gy, gz;

// MPU calibration vars:
int mean_ax,mean_ay,mean_az,mean_gx,mean_gy,mean_gz,state=0;
//int ax_offset,ay_offset,az_offset,gx_offset,gy_offset,gz_offset;
//Change this 3 variables if you want to fine tune the skecth to your needs.
int buffersize=1000;     //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
int acel_deadzone=8;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
int giro_deadzone=1;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

// Sparkfun XZ Sensor stuff:
#define TCAADDR 0x71

// Constants
const int ZX_ADDR = 0x10;  // ZX Sensor I2C address

// Global Variables
ZX_Sensor zx_sensor = ZX_Sensor(ZX_ADDR);
uint8_t x_pos[8];
uint8_t z_pos[8];

// --- TCASESELECT ---
//
// Set port to begin reading from
//
// @param port_num  Port to read from
//
void tcaseselect(uint8_t port_num)
{
  if (port_num < 8) {
    Wire.beginTransmission(TCAADDR);
    Wire.write(1 << port_num);
    Wire.endTransmission();
  }
}


// Analog Sensor Inputs
const int FSRPin = A0;  // Analog input pin that the FSR is attached to, using long FSR, and 10Kohm voltage divider
const int LinPin = A1;  // Analog input pin that the Line Pot is attached to
const int ePicUpPin = A2;  // Analog input pin that the ePic one direction is attached
const int ePicDownPin = A3;  // Analog input pin that the ePic other direction is attached

int fsrVal = 0; // Place to store FSR val
int linVal = 0; // Place to store LinPot val
int ePicUpVal = 0; // Place to store FSR val
int ePicDownVal = 0; // Place to store LinPot val


// ================================================================
// ===           Convert float to binary ROUTINE                ===
// ================================================================
typedef union {
 float floatingPoint;
 byte binary[4];
} binaryFloat;


void setup() {
  Serial.begin(230400); // Initialize serial port
  delay(3000); // 3 second delay for recovery

  Serial.begin(Baud);
  pinMode(A3,INPUT);
  delay(3000);
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  //==========================================================
// Sensors Setup:
//==========================================================
  // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

//  Serial.begin(115200);
  analogReadResolution(12); // 12 bit analog
  delay(5000);

  // *** Init MPU 6050 and serial stuff
  pinMode(INTERRUPT_PIN, INPUT);
#ifdef OUTPUT_READABLE
  Serial.println("Setting up MPU6050 raw");
#endif
  accelgyro.initialize();

    accelgyro.setZAccelOffset(1000); // 1688 factory default for my test chip
    accelgyro.setXGyroOffset(140); // was 220, 120=-.25, 180=.56, 150=.19, 135=-.06, 140=0
    accelgyro.setYGyroOffset(-10); // 76 = 1.192, 100=1.5, 50=0.8, 20=.35, -20=-.15, -10=0
    accelgyro.setZGyroOffset(60); //-75=-1.9, -20=-1., 20=-.5, 40=-.2, 60=.0



  #ifdef OUTPUT_READABLE
  Serial.println("Scanning for i2c MUXed devices...");
  #endif
  for (uint8_t t=0; t<8; t++) {
    delay(100);
      tcaseselect(t);
      #ifdef OUTPUT_READABLE
      Serial.print("TCA Port #"); Serial.println(t);
      #endif
      Wire.beginTransmission(ZX_ADDR);
      byte error = Wire.endTransmission();
      if (error == 0)
      {
         #ifdef OUTPUT_READABLE
         Serial.println("Found ZX Sensor on this port!");
         #endif
         zx_sensor.init();
      }

    }
        #ifdef OUTPUT_READABLE
        Serial.println("\ndone");
        #endif

  delay(1000);
}


void loop()
{

  querySerial();

  callPatterns();
/*
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();
*/
  analogWrite(PWM_pin, LOW);
  delay(motor_rate);///every 100 miliseconds
  analogWrite(PWM_pin, HIGH);
  delay(motor_rate);///every 100 miliseconds

  digitalWrite(A3, LOW);

//==========================================================
// Sensor Stuff
//==========================================================

 // Check to see if it's time to send sensor vals
  unsigned long currentMillis = millis();
   if (currentMillis - previousMillis >= interval) {
    //Serial.print("Time: ");
    //Serial.println(currentMillis);
    // save the last time you sent vals
    previousMillis = currentMillis;

    // clear MPU fifo
    accelgyro.resetFIFO();

    // Read Sparkfun XZ sensor and report vals
    readXZ();

    // Read Analog Sensor Vals
    readAnalog();

    // Read Pick Vals
    readPick();

    // Now measure MPU6050, update values in global registers
    measure_mpu6050();

    // Send new data out to Serial (either binary or human redable specified up top
    sendVals();

   }

}
