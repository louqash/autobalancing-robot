#include "timer.hpp"
#include <cmath>

Timer::Timer(float target_dt)
{
    this->target_dt = target_dt;
    clock_gettime(CLOCK_REALTIME, &last);
}

float Timer::get_dt()
{
    return this->dt;
}

void Timer::wait()
{
    while (dt_sum < target_dt)
    {
        clock_gettime(CLOCK_REALTIME, &current);
        float dt = (current.tv_nsec - last.tv_nsec) / 1e9 + (last.tv_nsec > current.tv_nsec);
        dt_sum += dt;
        last.tv_nsec = current.tv_nsec;
    }

    this->dt = dt_sum;
    this->dt_sum = std::fmod(dt_sum, target_dt);
}
