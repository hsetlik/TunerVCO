#include "Tuning.h"

//======================
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

String Tuner::stringForNoteName(NoteName n)
{
    const String noteNames[] = 
    {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B"
    };
    return noteNames[(int)n];
}

void Tuner::triggerRisingEdge()
{
    buf.push(micros());
}

void Tuner::init()
{
    Serial.begin(9600);
    pinMode(TUNE_PIN, INPUT);
    //initialize the display
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    if(!display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    //calculate the note pitches
    for(int i = 0; i < NUM_NOTES; i++)
    {
        auto& note = allNotes[i];
        float fNum = (float)i - 69.0f;
        note.pitch = 440.0f * std::pow(SEMITONE_RATIO, fNum);
        note.name = (NoteName)(i % 12);
        note.midiNum = i;
    }

}

void Tuner::tick()
{
    //step 1: check for new rising edges
    unsigned long now = millis();
    //step 2: update the screen if needed
    if(now - prevUpdateMs > FRAME_INTERVAL)
    {
        prevFrameLength = now - prevUpdateMs;
        prevUpdateMs = now;
        float pitch = currentPitchHz();
        displayTuning(pitch);
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

Note* Tuner::nearestNote(float hz)
{
    float prevDistance = 500000.0f;
    for(int i = 1; i < NUM_NOTES; i++)
    {
        float currentDistance = std::fabs(allNotes[i].pitch - hz);
        if(currentDistance >= prevDistance)
            return &allNotes[i - 1];
        prevDistance = currentDistance;    
    }
    return &allNotes[0];
}

int Tuner::tuningErrorCents(Note* target, float hz)
{
    if (hz > target->pitch) // if we're sharp
    {
        int higherNote = std::min({target->midiNum + 1, NUM_NOTES - 1});
        Note* upper = &allNotes[higherNote];
        float semitoneHz = upper->pitch - target->pitch;
        float fError = semitoneHz / (hz - target->pitch);
        return (int)fError * 100.0f;
    }
    else if(hz < target->pitch) // if we're flat
    {
        int lowerNote = std::max({target->midiNum - 1, 0});
        Note* lower = &allNotes[lowerNote];
        float semitoneHz = target->pitch - lower->pitch;
        float fError = semitoneHz / (target->pitch - hz);
        return (int)fError * -100.0f; //flipping the sign here so we can draw the tuning bar the right direction
    }
    return 0;
}


void Tuner::displayTuning(float hz)
{
    //step 1: grip the info for the display
    Note* nearest = nearestNote(hz);
    int tuningError = tuningErrorCents(nearest, hz);
    const bool inTune = std::abs(tuningError) <= TOLERANCE_CENTS;
    //step 2: update the display
    display->clearDisplay();
    auto str = String(hz);
    display->setTextSize(3);
    display->setCursor(0, 0);
    if(inTune)
    {
        //if we're in tune we start on a white background and draw inverse text
        display->fillScreen(SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display->println(str);
        display->setTextSize(2);
        display->println(String(prevFrameLength) + "ms");
    }
    else
    {
        display->setTextColor(SSD1306_WHITE);
        display->println(str);
        //now draw the graphic bar to indicate how out of tune we are
        float fBarLength = (float)std::abs(tuningError) / 100.0f;
        const int16_t barHeight = 8;
        int16_t x; 
        int16_t y = display->height() - barHeight;
        int16_t h = barHeight;
        int16_t center = display->width() / 2;
        int16_t w = (int16_t)(fBarLength * (float)center);
        if(tuningError > 0)
            x = center;
        else
            x = center - w;
        display->fillRect(x, y, w, h, SSD1306_WHITE);
        display->setTextSize(2);
        display->println(String(prevFrameLength) + "ms");
    }
    display->display();
}