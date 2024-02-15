#pragma once
#include <Arduino.h>
// Pin Definitions
#define SDA 21
#define SCL 22
#define TUNE_PIN 23

#include <Adafruit_SSD1306.h>
#include <Wire.h>

// stuff for the display
#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define OLED_ADDR 0x3C //I2C address
#define OLED_RESET -1

#define FRAME_INTERVAL 30 // length of time between screen updates in ms
#define RING_BUFFER_SIZE 250 // how many rising edges we keep track of

#define TOLERANCE_CENTS 5 // how close we have to be to the fundamental to count as "in tune" 

#define NUM_NOTES 128
#define SEMITONE_RATIO 1.05946309436f
//=================================================
enum NoteName
{
 C,
 Cs,
 D,
 Ds,
 E,
 F,
 Fs,
 G,
 Gs,
 A,
 As,
 B
};

struct Note
{
    NoteName name;
    int midiNum;
    float pitch;
};
//=================================================
// just a basic one-sided ring buffer to help us keep a running average of the distance between rising edges
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
    //keep track of the pitches for each midi note, calculate this in init
    Note allNotes[NUM_NOTES];
    //calculate the nearest note to the given pitch, determine how in-tune we are from there
    Note* nearestNote(float hz);
    //determine how out of tune we are
    int tuningErrorCents(Note* target, float hz);
    //this goes through our EdgeBuffer and calculates the VCO's current pitch
    float currentPitchHz();
    //this updates the OLED display with the current tuning state
    void displayTuning(float hz);
    String stringForNoteName(NoteName n);
public:
    Tuner();
    ~Tuner();
    void init();
    void tick();
    //this just pushes the current time into the EdgeBuffer, it should be connected to an interrupt on the tuning pin in main.cpp
    void triggerRisingEdge();
};