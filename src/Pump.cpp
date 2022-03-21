//#include <Arduino.h>
#include "Pump.h"
//#include "driver/pcnt.h"
//#include "driver/ledc.h"


WaterPump::WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel, int pulsecounter)
{
    _tachPin = tachpin;
    _pwmPin = pwmpin;
    _powerPin = powerpin;
    _pwm_channel = pwmchannel;
    _pcnt = pulsecounter;
    lastRunTime = 0;
    totalPulses = 0;
    for(int i = 0; i < 10; i++)
    {
        tachometerRate[i] = 0;
    }
    tachometerIndex = 0;
    enabled = false;
    pcnt_unit = pcnt_unit_t::PCNT_UNIT_0;
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

    const pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = _tachPin,
        .ctrl_gpio_num = _tachPin,
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?       
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = 32767,
        .counter_l_lim = 0,
        .unit = PCNT_UNIT_0,
        .channel = PCNT_CHANNEL_0 
    };
    
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    /* Configure and enable the input filter */
    pcnt_set_filter_value(pcnt_unit, 100);
    pcnt_filter_enable(pcnt_unit);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(pcnt_unit);
    pcnt_counter_clear(pcnt_unit);

    pcnt_counter_resume(pcnt_unit);
    lastRunTime = micros();
}

float WaterPump::pumpUpdate()         //pump speed update task
{
    unsigned long delta = micros() - lastRunTime;
    lastRunTime = micros();
    int16_t count = 0;
    pcnt_get_counter_value(pcnt_unit, &count);
    pcnt_counter_clear(pcnt_unit);
    totalPulses += count;
    float rate = (float)count/((float)delta/1000000.0);
    tachometerRate[(tachometerIndex++)%10] = rate;
    return rate;
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