#pragma once
#include <iostream>

const float def_KP = 620.0F;
const float def_KI = 3900.0F;
const float def_KD = 15.0F;
const float def_setpoint = 0.0365F;
constexpr int LAST_OUTPUTS_LENGTH = 2;
class PIDController
{

public:
    PIDController() : PIDController(def_KP, def_KI, def_KD, def_setpoint){};
    PIDController(float KP, float KI, float KD, float setpoint) : KP(KP), KI(KI), KD(KD), setpoint(setpoint)
    {
        std::cout << "Initlized PID Controller with KP{" << KP << "}, KI{" << KI << "}, KD{" << KD << "}, setpoint{" << setpoint << "}\n";
        reset();
    };

    float get_next_output(float sample, float dt);
    void reset();
    void setKP(float KP);
    void setKI(float KI);
    void setKD(float KD);
    void setSetpoint(float setpoint);

private:
    float KP;
    float KI;
    float KD;
    float setpoint;
    float last_outputs[LAST_OUTPUTS_LENGTH];

    float integration;
    float last_sample;
};
