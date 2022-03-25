#ifndef __PUMP_H
#define __PUMP_H
#include <Arduino.h>
#include "driver/pcnt.h"
#include "driver/ledc.h"
#include "hal/pcnt_types.h"
#define TACH_EVENTS 100
class WaterPump
{
public:
    WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel);
    void setPWM(float duty);
    float getSpeed(unsigned long window);
    uint32_t getMaxPWMDuty();
    void begin();
    void update();
    void enable();
    void disable();
    bool isEnabled();
private:
    float tachometerRate[10];
    int tachometerIndex;
    pcnt_unit_t pcnt_unit;
    volatile unsigned long totalPulses;
    volatile struct TachEvent
    {
        int edge;
        unsigned long time;
        unsigned long delta;
    } events[100];
    const int pulsesPerRev = 2;
    int _tachPin;
    int _pwmPin;
    int _powerPin;
    int _pwm_channel;
    int _pcnt;
    bool enabled;
    const int _freq = 25000;
    const int pwmResolution = 10;
};

#endif