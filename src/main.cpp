#include <Arduino.h>
#include "Tuning.h"

Tuner t;

void interruptCallback()
{
  t.triggerRisingEdge();
}

void setup() {
  // put your setup code here, to run once:
  t.init();
  attachInterrupt(TUNE_PIN, interruptCallback, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  t.tick();
}