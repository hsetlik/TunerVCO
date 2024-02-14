#include "Tuning.h"
EdgeBuffer::EdgeBuffer() : head(0)
{

}
void EdgeBuffer::push(unsigned long num)
{
    data[head] = num;
    head = (head + 1) % RING_BUFFER_SIZE;
}

unsigned long& EdgeBuffer::operator[](int idx)
{
    unsigned int i = (head + (unsigned int)idx) % RING_BUFFER_SIZE;
    return data[i];
}

//===============================================================================

Tuner::Tuner() : prevLevel(false), display(nullptr)
{
    Wire.begin(SDA, SCL);
}

Tuner::~Tuner()
{
    delete display;
}

void Tuner::init()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    if(!display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
}
void Tuner::tick()
{
    //step 1: check for new resing edges
    bool currentLevel = digitalRead(TUNE_PIN);
    unsigned long now = millis();
    if(currentLevel && !prevLevel) // check for the rising edge
    {
        buf.push(micros());
    }
    prevLevel = currentLevel;
    //step 2: update the screen if needed
    if(now - prevUpdateMs > FRAME_INTERVAL)
    {
        prevUpdateMs = now;
        float pitch = currentPitchHz();
        //TODO: update the screen here
    }
}

float Tuner::currentPitchHz()
{
    float sum = 0.0f;
    for(int i = 1; i < RING_BUFFER_SIZE; i++)
    {
        float diff = (float)buf[i] - buf[i - 1];
        sum += diff / 1000000.0f; //convert from microseconds to seconds
    }
    return (float)RING_BUFFER_SIZE / sum;
}