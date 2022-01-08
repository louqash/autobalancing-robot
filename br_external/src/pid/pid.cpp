#include "pid.hpp"
#include <cmath>

#define THRESHOLD 10

float PIDController::get_next_output(float sample, float dt)
{
    sample -= setpoint;
    // sample *= (std::signbit(sample)*-2 + 1) * sample;
    if (sample < 1 && sample > -1)
    {
        sample = std::asin(sample);
    }

    // float last_output = 0;
    // for (int i = 0; i < LAST_OUTPUTS_LENGTH; ++i)
    // {
    //     last_output += last_outputs[i];
    // }
    // += / 2000 PID: 620 3900 15
    // sample -= last_output / LAST_OUTPUTS_LENGTH / 2000.0F;
    float res = (KP * sample + KI * integration * dt + KD / dt * (sample - last_sample));
    // float res = (KP * sample + KI * integration + KD * (sample - last_sample));
    integration += sample;
    last_sample = sample;

    // for (int i = LAST_OUTPUTS_LENGTH-1; i > 0; --i)
    // {
    //     last_outputs[i] = last_outputs[i-1];
    // }
    // last_outputs[0] = res;

    return res;
}
void PIDController::reset()
{
    this->last_sample = 0;
    this->integration = 0;
    for (int i = 0; i < LAST_OUTPUTS_LENGTH; ++i)
    {
        this->last_outputs[i] = 0;
    }
}
void PIDController::setKP(float KP)
{
    this->KP = KP;
}
void PIDController::setKI(float KI)
{
    this->KI = KI;
}
void PIDController::setKD(float KD)
{
    this->KD = KD;
}
void PIDController::setSetpoint(float setpoint)
{
    this->setpoint = setpoint;
}