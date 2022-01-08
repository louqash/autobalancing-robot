cmake_minimum_required(VERSION 3.16)

project(PID C CXX)

add_executable(pid main.cpp imu.cpp i2cDevice.cpp pid.cpp timer.cpp)
target_link_libraries(pid i2c rt)
target_compile_options(pid PRIVATE -Wall -Wextra -Wpedantic -Werror)

install(TARGETS pid)
