
#include <Arduino.h>

#include <LiquidCrystal.h> //wlaczenie biblioteki

LiquidCrystal lcd(10, 9, 8, 7, 6, 5); // deklaracja pinów wyswietlacza polaczonych z Arduino

#define SDA 3 // data pin from caliper
#define SCK 2 // clock pin from caliper

#define MAX_SCK 64
#define MAX_OVF 4
#define MAX_DIG 8

#define EDGE_UP 0
#define EDGE_DOWN 1

unsigned char gv_mode = 0;

int gv_arClock[MAX_SCK] = {0}; // this is mostly used to record clock duration, to gain insight, useless otherwise.
int gv_ixClock = 0;
int gv_arValue[MAX_DIG] = {0};

int gv_ctReset = 0;

void setup()
{
  // put your setup code here, to run once:
  pinMode(SCK, INPUT); // make SCK (Digital 2) as input pin
  pinMode(SDA, INPUT); // make SDA (Digital 3) as input pin

  digitalWrite(SCK, HIGH); // pull up SCK pin so we do not need a pullup resistor
  digitalWrite(SDA, HIGH); // pull up SDA pin so we do not need a pullup resistor

  TCCR1A = 0x00;
  TCCR1B = 0x02; // half us, overflow at 32ms. 03= 4us
  TCCR1C = 0x00;

  Serial.begin(115200);
}

void loop()
{
  // put your main code here, to run repeatedly:

  while (1)
  {
    if (TIFR1 & 0b00000001)
    {
      // see if it is timing out,
      if (gv_ctReset < MAX_OVF)
      {
        gv_ctReset++;
        TIFR1 = TIFR1 | 0x01;
      }
      else
      {
        // see if there are any data captured
        // from experiment, the caliper sends out
        // 24 bits data periodically
        if (gv_ixClock == 24)
        {
          // if so, print it out
          long v = 0;
          v += gv_arValue[2] & 0x1F;
          v = v << 8;
          v += gv_arValue[1];
          v = v << 8;
          v += gv_arValue[0];

          // it seems that when the 5th bit is set in 3rd byte,
          // the value is negative
          if (gv_arValue[2] & 0b00100000)
          {
            v = -v;
          }
          // value must be divided by 200 to get measurement in MM
          double d = (double)v / 200.0;
          Serial.println(d);
        }
        else
        {
          Serial.println("a");
        }

        // timed out, re-initialize all variables and try to resync
        for (int i = 0; i < 3; i++)
        {
          gv_arValue[i] = 0;
        }
        gv_ixClock = 0;
        // wait for a LOW on SCK pin,
        // since signal is inverted, we are waiting for SCK high
        gv_mode = EDGE_UP;
        // reset TIMER1 overflow to start over
        TIFR1 = TIFR1 | 0x01;
      }
    }

    // When SCK is LOW, it means clock pin is high from the caliper.
    if (digitalRead(SCK) == LOW)
    {
      // if current mode is detecting a LOW
      // this means an LOW to HIGH edge detected
      if (gv_mode == EDGE_UP)
      {
        // reset timer 1
        TCNT1 = 0;
        TIFR1 = TIFR1 | 0x01;
        gv_ctReset = 0;

        // wait for edge from UP to DOWN
        gv_mode = EDGE_DOWN;
      }
    }
    else
    {
      if (gv_mode == EDGE_DOWN)
      {
        // this means a HIGH to LOW edge detected
        // which in turn means data should be read from SDA pin
        int value = digitalRead(SDA);
        // since data is inverted, we need to invert it back
        if (value == HIGH)
        {
          value = 0;
        }
        else
        {
          value = 1;
        }
        gv_arValue[gv_ixClock / 8] |= value << (gv_ixClock % 8);

        if (gv_ixClock < MAX_SCK)
        {
          gv_arClock[gv_ixClock] = TCNT1;
          gv_ixClock++;

          // reset timer and time-out
          TCNT1 = 0;
          TIFR1 = TIFR1 | 0x01;
          gv_ctReset = 0;
        }

        // wait for next LOW to HIGH edge
        gv_mode = EDGE_UP;
      }
    }
  }
}