#include <Arduino.h>
#include "samples.h"

#define LED_BUILTIN 13

int i = 0;
bool play = true;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, bonnie!");
}

void loop()
{
  if (play) {
    dacWrite(25, sample_hi[i++]);
    delayMicroseconds(125);
    if (i == sample_hi_size)  {
      i = 0;
      delay(1000);
      play = false;
    }
  } else {
      if (touchRead(13) < 15) {
        play = true;
      }
  }
}
