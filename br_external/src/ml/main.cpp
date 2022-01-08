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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#pragma GCC diagnostic pop

#include "imu.hpp"
#include "logger.hpp"
#include "timer.hpp"

#define TFLITE_MINIMAL_CHECK(x)                                  \
    if (!(x))                                                    \
    {                                                            \
        fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
        exit(1);                                                 \
    }

int res;
int fd = -1;
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

constexpr size_t STRING_BUF_LEN = 5;
char string_buf[STRING_BUF_LEN];
void set_engine_speed(float val)
{
    memset(string_buf, 0, STRING_BUF_LEN);
    sprintf(string_buf, "%d", std::max(-600, std::min((int32_t)val, 600)));
    res = write(fd, string_buf, 5);
}

void engine_stop(int sig)
{
    (void)sig;
    flag = 0;
}

void main_loop(int argc, char **argv)
{
    Logger<PIDLogData, 10000> logger("logs.csv");
    PIDLogData logframe;
    Timer timer(0.02);

    const char *model_name = "model.tflite";
    if (argc > 1)
    {
        model_name = argv[1];
    }
    std::unique_ptr<tflite::FlatBufferModel> model =
        tflite::FlatBufferModel::BuildFromFile(model_name);
    TFLITE_MINIMAL_CHECK(model != nullptr);

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*model, resolver);
    std::unique_ptr<tflite::Interpreter> interpreter;
    builder(&interpreter);
    TFLITE_MINIMAL_CHECK(interpreter != nullptr);
    TFLITE_MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);

    IMU imu;
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

    float imu_angle = imu.getNewAngle();
    while (imu_angle < 1.6)
    {
    }

    int n_prev = 50;
    float *prev_samples = (float *)calloc(n_prev, sizeof(float));

    for (int i = 0; i < n_prev; ++i)
    {
        prev_samples[i] = imu.getNewAngle();
    }

    std::cout << "Starting!" << std::endl;
    while (flag)
    {
        timer.wait();

        float dt = timer.get_dt();
        imu_angle = imu.getNewAngle();
        interpreter->typed_input_tensor<float>(0)[0] = imu_angle;
        memcpy(&interpreter->typed_input_tensor<float>(0)[1], prev_samples, n_prev * sizeof(float));
        interpreter->Invoke();

        float output = interpreter->typed_output_tensor<float>(0)[0];
        set_engine_speed(output);

        for (int i = n_prev - 1; i > 0; --i)
        {
            prev_samples[i] = prev_samples[i - 1];
        }
        prev_samples[0] = imu_angle;

        logframe.PIDValue = output;
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
