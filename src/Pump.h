#ifndef __PUMP_H
#define __PUMP_H
#include <Arduino.h>
#include "driver/pcnt.h"
#include "driver/ledc.h"
#include "hal/pcnt_types.h"

class WaterPump
{
public:
    WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel, int pulsecounter);
    void setPWM(float duty);
    uint32_t getMaxPWMDuty();
    void begin();
    float pumpUpdate();
    void enable();
    void disable();
private:
    float tachometerRate[10];
    int tachometerIndex;
    pcnt_unit_t pcnt_unit;
    unsigned long totalPulses;
    
    unsigned long lastRunTime;
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