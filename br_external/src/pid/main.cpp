#include <iostream>
#include <fstream>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctime>
#include <cmath>
#include <algorithm>

#include "imu.hpp"
#include "pid.hpp"
#include "logger.hpp"
#include "timer.hpp"

int res;
int fd;
volatile bool flag = 1;
const char *zero = "0";

class PIDLogData : DataTemplate
{
public:
    float PIDValue;
    float IMUAngle;
    float CurrentTD;

    virtual void format(char *buffer, int size)
    {
        snprintf(buffer, size, "%.8f,%.8f,%.8f", PIDValue, IMUAngle, CurrentTD);
    }
};

void engine_stop(int sig)
{
    flag = 0;
}

constexpr size_t STRING_BUF_LEN = 5;
char string_buf[STRING_BUF_LEN];
void set_engine_speed(float val)
{
    memset(string_buf, 0, STRING_BUF_LEN);
    sprintf(string_buf, "%d", std::max(-600, std::min((int32_t)val, 600)));
    res = write(fd, string_buf, 5);
}

void main_loop(int argc, char **argv)
{
    Logger<PIDLogData, 10000> logger("logs.csv");
    PIDLogData logframe;
    Timer timer(0.02);

    IMU imu;
    PIDController pid;
    if (argc > 3)
    {
        pid.setKP(atof(argv[1]));
        pid.setKI(atof(argv[2]));
        pid.setKD(atof(argv[3]));
    }

    if (!imu.isConnected())
    {
        std::cerr << "IMU not responding!" << std::endl;
        return;
    }
    std::cout << "Running init..." << std::endl;
    if (imu.init())
    {
        std::cerr << "IMU init failed" << std::endl;
        return;
    }

    std::cout << "Calibrating IMU..." << std::endl;
    // imu.clearOffsets();
    // imu.calibrateGyro(15);
    // imu.calibrateAccel(15);
    imu.setAccelOffset(-2289, 1146, 1843);
    imu.setGyroOffset(69, -15, -7);

    imu.enableDMP();

    std::cout << "Starting!" << std::endl;
    while (flag)
    {
        timer.wait();

        float dt = timer.get_dt();
        float imu_angle = imu.getNewAngle();
        int c = 0;
        while (imu_angle > 1.6F || imu_angle < -1.6F)
        {
            if (c++ > 10)
            {
                return;
            }
            imu_angle = imu.getNewAngle();
            pid.reset();
            set_engine_speed(0);
        }
        float pid_val = pid.get_next_output(imu_angle, dt);
        set_engine_speed(pid_val);

        logframe.PIDValue = pid_val;
        logframe.IMUAngle = imu_angle;
        logframe.CurrentTD = dt;
        logger.log(logframe);
    }
}

int main(int argc, char **argv)
{
    if ((fd = open("/sys/devices/platform/motor_controller/engine_power", O_WRONLY)) < 0)
    {
        perror("open");
        return 1;
    }
    signal(SIGINT, engine_stop);
    signal(SIGKILL, engine_stop);

    main_loop(argc, argv);

    res = write(fd, zero, 1);

    close(fd);

    return 0;
}
