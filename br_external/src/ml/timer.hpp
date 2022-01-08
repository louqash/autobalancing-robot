#pragma once

#include <unistd.h>
#include <time.h>

class Timer
{
    timespec current, last;
    float target_dt;
    float dt_sum;
    float dt;

public:
    Timer(float target_dt);
    float get_dt();
    void wait();
};
