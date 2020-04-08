/*
 * Author: ToraNova, github.com/toranova
 * mailto: chia_jason96@live.com
   ATTiny clock module
   This is a simple clock circuit
   using an ATTiny85
   add board using on File->Preference->Additional Board Manager URL
   https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json
   Install using Tools->Board->Board Manager-> Attiny by David A Mellis

   To flash, an arduino uno works as an ISP
   First, flash ArduinoISP from example onto an UNO
   Then, wire up arduino to Attiny
   RESET [ DOT    ] VCC
   3-A3  [        ] SCK  / 2-A1 (int) <-- pin 13 from uno
   4-A4  [        ] MISO / 1 (PB1)    <-- pin 12 from uno
   GND   [        ] MOSI / 0 (PB0)    <-- pin 11 from uno

   Reset pin has a 10uF cap to GND, and connects to pin 10 from uno

   Flashing configuration
   Board ATTiny25/45/85
   Processor: ATtiny85
   Clock 1MHz
   Port - Arduino Uno
   Programmer - Arduino as ISP

   https://medium.com/jungletronics/attiny85-easy-flashing-through-arduino-b5f896c48189

*/


//#define DEBUG
#define MAXHALFPMS 1000 //maximum half period ( p/2 ) in MILLISECONDS

#define POTEIN A3 //potentiometer at pin 5
#define STEPIN PB1 //step button is 3
#define CLOCK PB4  //clock is the output pin
#define MODEIN 0 //mode select button is 4

//global mode var to know state of tinyclock
unsigned int mode;
static unsigned int state;
unsigned volatile int aread;
unsigned volatile long mt;
unsigned static long lastint;
volatile float tmp;
volatile bool cout;

#define NORMAL 0x00
#define PAUSE  0x01
#define DEBOUNCEMS 200
#define BAUDRATE 1200

#define ASTABLE 0
#define MONOSTABLE 1
#define BISTABLE 2

#ifdef DEBUG
//PB0 is used to serial output
#include <SoftwareSerial.h>
SoftwareSerial Serial(PB5, PB0);
#endif

void setup() {
  // initialize digital ATtiny pin 0 as an output, pin 2,3 5 as output
  pinMode(MODEIN, INPUT);
  pinMode(STEPIN, INPUT);
  pinMode(POTEIN, INPUT);
  pinMode(CLOCK, OUTPUT);

  attachInterrupt( MODEIN, modehandle, FALLING);

  mode = ASTABLE; //default mode is astable
  state = NORMAL; //default state is normal

#ifdef DEBUG
  Serial.begin(BAUDRATE);
#endif
  mt = millis(); //clock
  lastint = millis();
  cout = 0;
  digitalWrite(CLOCK, cout);
}


//interrupt handler for MODE
void modehandle() {
  if ( millis() - lastint > DEBOUNCEMS ) {
    //debounced input
    mode = (mode + 1) % 3; //cycle through 0,1,2
    lastint = millis();
    mt = millis();
    cout = 0;
    digitalWrite(CLOCK, cout);
  }
}

// the loop function runs over and over again forever
void loop() {

  switch (mode) {
    case ASTABLE:
      aread = analogRead(POTEIN); //0-1024
      tmp = (aread / 1024.0) * MAXHALFPMS;
      aread = (int) tmp; //discretize

      if ( digitalRead(STEPIN) == 0 && (millis() - lastint) > DEBOUNCEMS ) {
        if ( state == NORMAL ) state = PAUSE;
        else {
          state = NORMAL;
          mt = millis();
        }
        lastint = millis();
      }

      if (millis() - mt > aread && state == NORMAL) {
        cout = !cout;
        digitalWrite(CLOCK, cout);
        mt = millis();
      }
      break;
    case MONOSTABLE:
      if ( digitalRead(STEPIN) == 0 && (millis() - lastint) > DEBOUNCEMS ) {
        aread = analogRead(POTEIN); //0-1024
        tmp = (aread / 1024.0) * MAXHALFPMS/2;
        aread = (int) tmp; //discretize
        digitalWrite(CLOCK, HIGH);
        delay(aread);
        digitalWrite(CLOCK, LOW);
        lastint = millis();
      }
      break;
    case BISTABLE:
      if ( digitalRead(STEPIN) == 0 && (millis() - lastint) > DEBOUNCEMS ) {
        cout = !cout;
        digitalWrite(CLOCK, cout);
        lastint = millis();
      }
      break;
    default:
      break;
  }
}
