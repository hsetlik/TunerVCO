#include <Arduino.h>
#include "Tuning.h"

Tuner t;

void interruptCallback()
{
  t.triggerRisingEdge();
}

void setup() {
  t.init();
  attachInterrupt(TUNE_PIN, interruptCallback, RISING);
}

void loop() {
  t.tick();
}