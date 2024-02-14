#include <Arduino.h>
#include "Tuning.h"

Tuner t;

void setup() {
  // put your setup code here, to run once:
  t.init();

}

void loop() {
  // put your main code here, to run repeatedly:
  t.tick();
}

