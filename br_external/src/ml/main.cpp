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

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include "imu.hpp"
#include "logger.hpp"
#include "timer.hpp"

#define TFLITE_MINIMAL_CHECK(x)                              \
  if (!(x)) {                                                \
    fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
    exit(1);                                                 \
  }


int res;
int fd = -1;
volatile bool flag = 1;
const char* zero = "0";

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

void engine_stop(int sig) {
    flag = 0;
}


//   gyro.init();

//   float setpoint = 0.F;
//   timespec current, last;
//   clock_gettime(CLOCK_REALTIME, &last);

//   int n_prev = 20;
//   float* prev_samples = (float*)calloc(n_prev, sizeof(float));
//   size_t filled = 0;

//   char engine_buf[10];
//   float target_dt = 0.1;
//   float dt_sum = 0;
//   std::cout << "befor while" << std::endl;
//   while (flag) {
//     clock_gettime(CLOCK_REALTIME, &current);
//     float dt = (current.tv_nsec - last.tv_nsec) / 1e9 + (last.tv_nsec > current.tv_nsec);
//     dt_sum += dt;
//     if(dt_sum < target_dt) {
//       continue;
//     }
//     dt_sum = std::fmod(dt_sum, target_dt);

//     float sample = (setpoint - gyro.getNewAngle(dt)) / 90.F;
//     //if(sample > 90 || sample < -90) {
//     //  write(fd, zero, 1);
//     //  continue;
//     //}
//     last.tv_nsec = current.tv_nsec;
//     interpreter->typed_input_tensor<float>(0)[0] = sample;
//     memcpy(&interpreter->typed_input_tensor<float>(0)[1], prev_samples, n_prev*sizeof(float));

//     if(filled == n_prev) {
//       interpreter->Invoke();

//       std::cout << "sending to engines" << std::endl;
//       float output = interpreter->typed_output_tensor<float>(0)[0];
//       memset(engine_buf, 0, 10);
//       sprintf(engine_buf, "%d", std::max(-1000, std::min((int32_t)(output * 700), 1000)));
//       write(fd, engine_buf, 10);
//     } else {
//       ++filled;
//     }

//     //DT_history[idx++] = dt;
//   }
//   write(fd, zero, 1);

//   //float sum = 0;
//   //float max = 0;
//   //for(int i = 0; i < idx; ++i) {
//   //  sum += DT_history[i];
//   //  max = std::max(max, DT_history[i]);
//   //}
//   //std::cout << "mean dt " << sum/idx << std::endl;
//   //std::cout << "max dt " << max << std::endl;

//   return 0;
// }

void main_loop(int argc, char **argv)
{
    Logger<PIDLogData, 10000> logger("logs.csv");
    PIDLogData logframe;
    Timer timer(0.02);

    const char* model_name = "new.tflite";
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
    while (imu_angle < 1.6) {}

    int n_prev = 50 - 1;
    float* prev_samples = (float*)calloc(n_prev, sizeof(float));
    size_t filled = 0;

    for (int i = 0; i < n_prev; ++i)
    {
        prev_samples[i] = imu.getNewAngle();
    }

    std::cout << "Starting!" << std::endl;
    while (flag)
    {
        timer.wait();

        float dt = timer.get_dt();
        int c = 0;
        imu_angle = imu.getNewAngle();
        //if (imu_angle > 1.6F || imu_angle < -1.6F)
        //{
        //    break;
        //}
        interpreter->typed_input_tensor<float>(0)[0] = imu_angle;
        memcpy(&interpreter->typed_input_tensor<float>(0)[1], prev_samples, n_prev*sizeof(float));
        interpreter->Invoke();

        float output = interpreter->typed_output_tensor<float>(0)[0];
        // set_engine_speed(output);
        std::cout << output << std::endl;

        for(int i = n_prev-1; i > 0; --i) {
            prev_samples[i] = prev_samples[i-1];
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
