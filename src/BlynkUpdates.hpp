#ifndef BLYNKUPDATES_H__
#define BLYNKUPDATES_H__
#include <Arduino.h>
#include <BlynkSimpleEsp32.h>
#include "Pump.h"

extern WaterPump pump;

bool enabled = false;
float dutyCycle = 0;
/*
V0: on/off enable button
V1: terminal
V2: PWM slider
V3: plot - PWM
V4: plot - Speed
V5: plot - Setpoint
*/
BLYNK_WRITE(V0)     //enable     
{
    if(param.asInt())
    {
        //digitalWrite(pump_PowerPin, HIGH);
        //digitalWrite(pump_PwmPin, HIGH);
        //ledcWrite(1, dutyCycle);
        enabled = true;
        pump.enable();
        pump.setPWM(dutyCycle);
    }
    else
    {
        //digitalWrite(pump_PowerPin, LOW);
        //digitalWrite(pump_PwmPin, LOW);
        pump.setPWM(0);
        pump.disable();
        enabled = false;
    }
}

BLYNK_WRITE(V2)     //pwm slider
{
    if(enabled)
    {
        float duty = param.asFloat();
        duty = constrain(duty, 0.0, 100.0);
        dutyCycle = duty;
        pump.setPWM(dutyCycle);
        //ledcWrite(1, duty);
    }
    else pump.setPWM(0);
}




#endif