//#include <Arduino.h>
#include "Pump.h"
//#include "driver/pcnt.h"
//#include "driver/ledc.h"


typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrArg)(void*);

WaterPump::WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel)
{
    _tachPin = tachpin;
    _pwmPin = pwmpin;
    _powerPin = powerpin;
    _pwm_channel = pwmchannel;
    totalPulses = 0;
    for(int i = 0; i < 10; i++)
    {
        tachometerRate[i] = 0;
    }
    tachometerIndex = 0;
    enabled = false;

    for(int i = 0; i < TACH_EVENTS; i++)
    {
        events[i].time = 0;
        events[i].edge = 0;
        events[i].delta = 0;
    }
}

void WaterPump::begin()
{
    pinMode(_tachPin, INPUT_PULLUP);
    pinMode(_pwmPin, OUTPUT);
    digitalWrite(_pwmPin, LOW);
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);
    
    ledcSetup(_pwm_channel, _freq, pwmResolution);
    ledcAttachPin(_pwmPin, _pwm_channel);
    ledcWrite(_pwm_channel, 0);

}

void WaterPump::update()         //pump speed update task
{
    int previousIndex = totalPulses  % TACH_EVENTS;
    totalPulses++;
    int eventBufferIndex = totalPulses % TACH_EVENTS;
    events[eventBufferIndex].time = micros();
    if(digitalRead(_tachPin)) events[eventBufferIndex].edge = RISING;
    else events[eventBufferIndex].edge = FALLING;
    if(totalPulses == 1) return;
    events[eventBufferIndex].delta = events[eventBufferIndex].time - events[previousIndex].time;
}

float WaterPump::getSpeed(unsigned long window)         //iterate over the array, find all the edges within the window interval from the present, and average them
{//if window == 0, average all edges in the buffer
    unsigned long now = micros();
    float sum = 0;
    int edgeCount = 0;
    for(int i = 0; i < TACH_EVENTS; i++)
    {
        if((now - events[i].time < window) && (events[i].edge != 0))
        {
            sum += ((float)events[i].delta)/1000000.0;
            edgeCount++;
        }
    }
    if(edgeCount == 0) return 0;
    float edgeRate = (float)edgeCount / sum;
    float speed = (60.0 * edgeRate) / ( 2.0 * (float)pulsesPerRev);
    return speed;
}

void WaterPump::setPWM(float duty)
{
    duty = constrain(duty, 0.0, 100.0);
    ledcWrite(_pwm_channel, (uint32_t)((duty / 100.0) * (float)getMaxPWMDuty()));
}

uint32_t WaterPump::getMaxPWMDuty()
{
    return (1 << pwmResolution) - 1;
}

void WaterPump::enable()
{
    enabled = true;
    digitalWrite(_powerPin, HIGH);
}

void WaterPump::disable()
{
    enabled = false;
    digitalWrite(_powerPin, LOW);
}

bool WaterPump::isEnabled()
{
    return enabled;
}