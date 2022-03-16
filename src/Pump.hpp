#include <Arduino.h>
#include "driver/pcnt.h"

class WaterPump
{
public:
    WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel, int pulsecounter);
    void begin();
private:
    int _tachPin;
    int _pwmPin;
    int _powerPin;
    int _pwm_channel;
    int _pcnt;
};

WaterPump::WaterPump(int tachpin, int pwmpin, int powerpin, int pwmchannel, int pulsecounter)
{
    _tachPin = tachpin;
    _pwmPin = pwmpin;
    _powerPin = powerpin;
    _pwm_channel = pwmchannel;
    _pcnt = pulsecounter;
}

void WaterPump::begin()
{
    pinMode(_tachPin, INPUT_PULLUP);
    pinMode(_pwmPin, OUTPUT);
    digitalWrite(_pwmPin, LOW);
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);
    pcnt_unit_t pcnt_unit;
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = _tachPin,
        .ctrl_gpio_num = _tachPin,
        .unit = pcnt_unit,
        .channel = PCNT_CHANNEL_0, 
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_REVERSE, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = PCNT_EVT_H_LIM,
        .counter_l_lim = PCNT_EVT_L_LIM,
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
}