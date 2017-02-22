/*
 _            _  ___         _            _    
(_)_   __    / |( _ )    ___| | ___   ___| | __
| \ \ / /____| |/ _ \   / __| |/ _ \ / __| |/ /
| |\ V /_____| | (_) | | (__| | (_) | (__|   < 
|_| \_/      |_|\___/   \___|_|\___/ \___|_|\_\
                                               
Using the adaptor board by Awesomenesser:
https://oshpark.com/shared_projects/K4IOvS0o

*/

// Using digitalWriteFast from https://github.com/watterott/Arduino-Libs
#include "digitalWriteFast.h"

#include "digits.h"

// Easy lookup for each number we wish to display
// We also define "." as digit # 10
const uint32_t digit[] = {
  ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, DP,
  MINUS, SPACE
};

// Easy lookup for the nine digits of the clock
const uint32_t clockDigit[] = {
    0UL, DIG1, DIG2, DIG3, DIG4, DIG5, DIG6, DIG7, DIG8, DIG9
};

// Interface to the MAX 6921
#define LOAD_PIN 9
#define CLOCK_PIN 8
#define DATA_PIN 7

// We'll store the bits we need to send in data
uint32_t data = 0;

// Simulate current time
byte hh=9, mm=59, ss=32;
uint32_t last = 0;

void xfer(uint32_t d) {
    uint32_t mask = 1UL << 16;
    bool z;
    digitalWriteFast(LOAD_PIN, LOW);
    for (int i=16; i>=0; i--) {
        z = d & mask;
        digitalWriteFast(CLOCK_PIN, LOW);
        digitalWriteFast(DATA_PIN, z);
        digitalWriteFast(CLOCK_PIN, HIGH);
        mask = mask >> 1;
    }
    digitalWriteFast(LOAD_PIN, HIGH);
}

void selectDigit(byte d) {
    data = clockDigit[d];
}

void writeValue(byte d) {
    data |= digit[d];
    xfer(data);
}

void write(byte pos, byte val) {
    selectDigit(pos);
    writeValue(val);
}

void finish(byte pos) {
    selectDigit(pos);
    xfer(data);
}

void setup() {
    // Set SPI SS pin as output 
    pinModeFast(10, OUTPUT);

    // Initialise the MAX 6921 LOAD pin and set it LOW
    pinModeFast(LOAD_PIN, OUTPUT);
    digitalWriteFast(LOAD_PIN, LOW);

    pinModeFast(CLOCK_PIN, OUTPUT);
    pinModeFast(DATA_PIN, OUTPUT);

    Serial.begin(115200);

}

void loop() {
    uint32_t now = millis()/1000;
    if ( now != last ) {
        ss += 1;
        if (ss == 60) {
            ss = 0;
            mm += 1;
            if ( mm == 60 ) {
                mm = 0;
                hh = (hh+1) % 24;
            }
        }
        last = now;
    }

    uint32_t start = micros();
    if ( hh / 10 > 0 ) {
        write(8, hh / 10);
    }
    write(7, hh % 10);

    write(5, mm / 10);
    write(4, mm % 10);

    write(2, ss / 10);
    write(1, ss % 10);

    finish(1);
    uint32_t end = micros();
    //Serial.println(end-start);
}
