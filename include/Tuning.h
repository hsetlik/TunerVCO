#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
// Pin Definitions
#define SDA 21
#define SCL 22
#define TUNE_PIN 23

// stuff for the display
#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define OLED_ADDR 0x3C
#define OLED_RESET -1

#define FRAME_INTERVAL 30 // length of time between screen updates in ms

#define RING_BUFFER_SIZE 50 // how many rising edges do we keep track of

class EdgeBuffer
{
private:
    unsigned long data[RING_BUFFER_SIZE];
    int head;
public:
    EdgeBuffer();
    void push(unsigned long num);
    unsigned long& operator[](int idx);
};


//=================================================
class Tuner
{
private:
    EdgeBuffer buf;
    bool prevLevel;
    unsigned long prevUpdateMs;
    Adafruit_SSD1306* display;
public:
    Tuner();
    ~Tuner();
    void init();
    void tick();
    float currentPitchHz();
};