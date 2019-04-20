/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

*/



#include <EEPROM.h>
#define SWITCH_LEVEL        1800  // micros
#define SWITCH_DELAY        500   // millis
#define NUMBER_OF_OUT_PINS  16
#define NUMBER_OF_OUT_STEPS 10
#define START_ADDRESS       30    // EEPROM start address for the config     
#define INPUT_PIN           2     // pin to reciver

byte outPins[ NUMBER_OF_OUT_PINS ] =  { 3, 4, 5, 6, 7, 8, 9, 10, A5, A4, A3, A2, A1, A0, 11, 12 };

volatile int pulsLength;
unsigned long switchDelay = 0;
volatile boolean newSignal = false;
volatile boolean serialMode = false;

unsigned long previousMillis = 0;
int ledState = LOW;

byte currentStep = 0;
char inBuffer[ NUMBER_OF_OUT_STEPS * 2];
int switchSteps[ NUMBER_OF_OUT_STEPS ];


void setup() {
  Serial.begin( 9600 );
  Serial.setTimeout( 5000 );


  // init input pin: enable internal pullup resistor
  pinMode( INPUT_PIN, INPUT_PULLUP );

  // init output pins
  for ( byte i = 0; i < NUMBER_OF_OUT_PINS; i++ ) {
    pinMode( outPins[ i ], OUTPUT );
    digitalWrite( outPins[ i ], LOW );
  }

  // led
  pinMode( LED_BUILTIN, OUTPUT );

  // init step array
  for ( byte i = 0; i < NUMBER_OF_OUT_STEPS; i++ ) {
    switchSteps[ i ] = 0;
  }

  // read the config from eeprom
  for ( byte i = START_ADDRESS; i < ( START_ADDRESS + NUMBER_OF_OUT_STEPS * 2 ); i = i + 2 ) {
    switchSteps[ ( i - START_ADDRESS ) / 2 ] = EEPROM.read( i );
    switchSteps[ ( i - START_ADDRESS ) / 2 ] |= EEPROM.read( i + 1 ) << 8;
  }

  for ( int i = 0; i < 5 && !serialMode ; i++) {
    if (Serial.available() && Serial.read() == 'C') {
      Serial.print("C");
      serialMode = true;
      Serial.flush();
      Serial.end();
      delay(250);
      Serial.begin(9600);
    }
    delay(200);
  }


  if ( serialMode ) {
    digitalWrite( LED_BUILTIN , HIGH );
    Serial.println( "switchSteps: {" );
    for ( byte i = 0; i < NUMBER_OF_OUT_STEPS; i++ ) {
      Serial.print( i );
      Serial.print( ": " );
      Serial.print( switchSteps[ i ], BIN );
      Serial.println( "," );
    }
    Serial.println("}");
  } else {
    digitalWrite( LED_BUILTIN , LOW );
    currentStep = 0;
    // load first step
    for ( byte i = 0; i < NUMBER_OF_OUT_PINS; i++ ) {
      digitalWrite( outPins[ i ], bitRead( switchSteps[ currentStep ], i ) );
    }
    currentStep = 1;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if ( serialMode) {
    int bytes = Serial.readBytesUntil( 0xFF, inBuffer, NUMBER_OF_OUT_STEPS * 2 );
    if ( bytes != 0 ) {
      for ( byte i = START_ADDRESS; i < ( START_ADDRESS + NUMBER_OF_OUT_STEPS * 2 ); i = i + 2 ) {
        EEPROM.write( i, inBuffer[ i - START_ADDRESS ] );
        EEPROM.write( i + 1, inBuffer[ i - START_ADDRESS + 1 ] );
        delay( 1 );
      }
      digitalWrite( LED_BUILTIN, LOW );
      Serial.print( "get bytes: ");
      Serial.println( bytes );
    }
  }
  else {
    if ( !newSignal && ((currentMillis - switchDelay) > SWITCH_DELAY )) {
      pulsLength = pulseIn( INPUT_PIN, HIGH, 25000 );
      if ( pulsLength > SWITCH_LEVEL) {
        newSignal = true;
      }
    }

    if ( newSignal ) {
      newSignal = false;
      switchDelay = millis();

      Serial.print( currentStep );
      Serial.print( ": " );

      for ( byte i = 0; i < NUMBER_OF_OUT_PINS; i++ ) {
        digitalWrite( outPins[ i ], bitRead( switchSteps[ currentStep ], i ) );
      }
      Serial.println( switchSteps[ currentStep ], BIN );

      currentStep++;
      if ( currentStep >= NUMBER_OF_OUT_STEPS )
        currentStep = 0;
    }
  }

}
